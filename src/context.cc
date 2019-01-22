
// Copyright 2014 Toggl Desktop developers

// No exceptions should be thrown from this class.
// If pointers to models are returned from this
// class, the ownership does not change and you
// must not delete the pointers you got.

// All public methods should start with an uppercase name.
// All public methods should catch their exceptions.

#include "../src/context.h"

#include <iostream>  // NOLINT

#include "./autotracker.h"
#include "./client.h"
#include "./const.h"
#include "./database.h"
#include "./error.h"
#include "./formatter.h"
#include "./https_client.h"
#include "./obm_action.h"
#include "./project.h"
#include "./settings.h"
#include "./task.h"
#include "./time_entry.h"
#include "./timeline_uploader.h"
#include "./urls.h"
#include "./window_change_recorder.h"
#include "./workspace.h"

#include "Poco/Crypto/OpenSSLInitializer.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Environment.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/FormattingChannel.h"
#include "Poco/Logger.h"
#include "Poco/Net/FilePartSource.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/Net/HTTPSStreamFactory.h"
#include "Poco/Net/NetSSL.h"
#include "Poco/Net/StringPartSource.h"
#include "Poco/Path.h"
#include "Poco/PatternFormatter.h"
#include "Poco/Random.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/Stopwatch.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/UTF8String.h"
#include "Poco/URIStreamOpener.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Util/TimerTaskAdapter.h"
#include <mutex> // NOLINT

namespace toggl {

std::string Context::log_path_ = "";

Context::Context(const std::string app_name, const std::string app_version)
    : db_(nullptr)
, user_(nullptr)
, timeline_uploader_(nullptr)
, window_change_recorder_(nullptr)
, next_sync_at_(0)
, next_push_changes_at_(0)
, next_fetch_updates_at_(0)
, next_update_timeline_settings_at_(0)
, next_wake_at_(0)
, time_entry_editor_guid_("")
, environment_("production")
, idle_(&ui_)
, last_sync_started_(0)
, sync_interval_seconds_(0)
, update_check_disabled_(false)
, trigger_sync_(false)
, trigger_push_(false)
, quit_(false)
, ui_updater_(this, &Context::uiUpdaterActivity)
, reminder_(this, &Context::reminderActivity)
, syncer_(this, &Context::syncerActivity)
, update_path_("")
, overlay_visible_(false) {
    if (!Poco::URIStreamOpener::defaultOpener().supportsScheme("http")) {
        Poco::Net::HTTPStreamFactory::registerFactory();
    }
    if (!Poco::URIStreamOpener::defaultOpener().supportsScheme("https")) {
        Poco::Net::HTTPSStreamFactory::registerFactory();
    }

    urls::SetUseStagingAsBackend(
        app_version.find("7.0.0") != std::string::npos);

    Poco::ErrorHandler::set(&error_handler_);
    Poco::Net::initializeSSL();

    HTTPSClient::Config.AppName = app_name;
    HTTPSClient::Config.AppVersion = app_version;

    Poco::Crypto::OpenSSLInitializer::initialize();

    startPeriodicUpdateCheck();

    startPeriodicSync();

    if (!ui_updater_.isRunning()) {
        ui_updater_.start();
    }

    if (!reminder_.isRunning()) {
        reminder_.start();
    }

    resetLastTrackingReminderTime();

    pomodoro_break_entry_ = nullptr;
}

Context::~Context() {
    SetQuit();

    stopActivities();

    {
        Poco::Mutex::ScopedLock lock(window_change_recorder_m_);
        if (window_change_recorder_) {
            delete window_change_recorder_;
            window_change_recorder_ = nullptr;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            delete timeline_uploader_;
            timeline_uploader_ = nullptr;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(db_m_);
        if (db_) {
            delete db_;
            db_ = nullptr;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (user_) {
            delete user_;
            user_ = nullptr;
        }
    }

    Poco::Net::uninitializeSSL();
}

void Context::stopActivities() {
    try {
        {
            Poco::Mutex::ScopedLock lock(reminder_m_);
            if (reminder_.isRunning()) {
                reminder_.stop();
                reminder_.wait(2000);
            }
        }

        {
            Poco::Mutex::ScopedLock lock(ui_updater_m_);
            if (ui_updater_.isRunning()) {
                ui_updater_.stop();
                ui_updater_.wait(2000);
            }
        }


        {
            Poco::Mutex::ScopedLock lock(syncer_m_);
            if (syncer_.isRunning()) {
                syncer_.stop();
                syncer_.wait(2000);
            }
        }
    } catch(const Poco::Exception& exc) {
        logger().debug(exc.displayText());
    } catch(const std::exception& ex) {
        logger().debug(ex.what());
    } catch(const std::string& ex) {
        logger().debug(ex);
    }

    {
        Poco::Mutex::ScopedLock lock(window_change_recorder_m_);
        if (window_change_recorder_) {
            window_change_recorder_->Shutdown();
        }
    }

    {
        Poco::Mutex::ScopedLock lock(ws_client_m_);
        ws_client_.Shutdown();
    }

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            timeline_uploader_->Shutdown();
        }
    }

    TogglClient::TogglStatus.DisableStatusCheck();
}

void Context::Shutdown() {
    stopActivities();

    // cancel tasks but allow them finish
    {
        Poco::Mutex::ScopedLock lock(timer_m_);
        timer_.cancel(false);
    }

    // Stops all running threads and waits
    // for their completion (maximum 10 seconds).
    Poco::ThreadPool::defaultPool().stopAll();
}

error Context::StartEvents() {
    try {
        logger().debug("StartEvents");

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (user_) {
                return displayError("Cannot start UI, user already logged in!");
            }
        }

        if (HTTPSClient::Config.CACertPath.empty()) {
            return displayError("Missing CA cert bundle path!");
        }

        // Check that UI is wired up
        error err = UI()->VerifyCallbacks();
        if (err != noError) {
            logger().error(err);
            std::cerr << err << std::endl;
            std::cout << err << std::endl;
            return displayError("UI is not properly wired up!");
        }

        UIElements render;
        render.display_settings = true;
        updateUI(render);

        // See if user was logged in into app previously
        User *user = new User();
        err = db()->LoadCurrentUser(user);
        if (err != noError) {
            delete user;
            setUser(nullptr);
            return displayError(err);
        }
        if (!user->ID()) {
            delete user;
            setUser(nullptr);
            return noError;
        }
        setUser(user);

        // Set since param to 0 to force full sync on app start
        user->SetSince(0);
        logger().debug("fullSyncOnAppStart");

        updateUI(UIElements::Reset());

        if ("production" == environment_) {
            std::string update_channel("");
            UpdateChannel(&update_channel);

            analytics_.TrackChannel(db_->AnalyticsClientID(), update_channel);

            // Track user os version
            std::stringstream os_info;
            os_info << Poco::Environment::osDisplayName()
                    << "_" << Poco::Environment::osVersion()
                    << "_" << Poco::Environment::osArchitecture();

            analytics_.TrackOs(db_->AnalyticsClientID(), os_info.str());
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::save(const bool push_changes) {
    logger().debug("save");
    try {
        std::vector<ModelChange> changes;

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            error err = db()->SaveUser(user_, true, &changes);
            if (err != noError) {
                return err;
            }
        }

        UIElements render;
        render.display_unsynced_items = true;
        render.display_timer_state = true;
        render.ApplyChanges(time_entry_editor_guid_, changes);
        updateUI(render);

        if (push_changes) {
            next_push_changes_at_ =
                postpone(kRequestThrottleSeconds * kOneSecondInMicros / 100);
            Poco::Util::TimerTask::Ptr ptask =
                new Poco::Util::TimerTaskAdapter<Context>(
                    *this, &Context::onPushChanges);

            Poco::Mutex::ScopedLock lock(timer_m_);
            timer_.schedule(ptask, next_push_changes_at_);

            std::stringstream ss;
            ss << "Next push at "
               << Formatter::Format8601(next_push_changes_at_);
            logger().debug(ss.str());
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

UIElements UIElements::Reset() {
    UIElements render;
    render.first_load = true;
    render.display_time_entries = true;
    render.display_time_entry_autocomplete = true;
    render.display_mini_timer_autocomplete = true;
    render.display_project_autocomplete = true;
    render.display_client_select = true;
    render.display_workspace_select = true;
    render.display_timer_state = true;
    render.display_time_entry_editor = true;
    render.display_autotracker_rules = true;
    render.display_settings = true;
    render.display_unsynced_items = true;

    render.open_time_entry_list = true;

    return render;
}

std::string UIElements::String() const {
    std::stringstream ss;
    if (display_time_entries) {
        ss << "display_time_entries ";
    }
    if (display_time_entry_autocomplete) {
        ss << "display_time_entry_autocomplete ";
    }
    if (display_mini_timer_autocomplete) {
        ss << "display_mini_timer_autocomplete ";
    }
    if (display_project_autocomplete) {
        ss << "display_project_autocomplete ";
    }
    if (display_client_select) {
        ss << "display_client_select ";
    }
    if (display_client_select) {
        ss << "display_client_select ";
    }
    if (display_workspace_select) {
        ss << "display_workspace_select ";
    }
    if (display_timer_state) {
        ss << "display_timer_state ";
    }
    if (display_time_entry_editor) {
        ss << "display_time_entry_editor ";
    }
    if (open_settings) {
        ss << "open_settings ";
    }
    if (open_time_entry_list) {
        ss << "open_time_entry_list ";
    }
    if (open_time_entry_editor) {
        ss << "open_time_entry_editor ";
    }
    if (display_autotracker_rules) {
        ss << "display_autotracker_rules ";
    }
    if (display_settings) {
        ss << "display_settings ";
    }
    if (!time_entry_editor_guid.empty()) {
        ss << "time_entry_editor_guid=" << time_entry_editor_guid << " ";
    }
    if (!time_entry_editor_field.empty()) {
        ss << "time_entry_editor_field=" << time_entry_editor_field << " ";
    }
    if (display_unsynced_items) {
        ss << "display_unsynced_items ";
    }
    return ss.str();
}

void UIElements::ApplyChanges(
    const std::string editor_guid,
    const std::vector<ModelChange> &changes) {

    time_entry_editor_guid = editor_guid;

    // Check what needs to be updated in UI
    for (std::vector<ModelChange>::const_iterator it =
        changes.begin();
            it != changes.end();
            it++) {
        ModelChange ch = *it;

        if (ch.ModelType() == kModelWorkspace
                || ch.ModelType() == kModelClient
                || ch.ModelType() == kModelProject
                || ch.ModelType() == kModelTask
                || ch.ModelType() == kModelTimeEntry) {
            display_time_entry_autocomplete = true;
            display_time_entries = true;
            display_mini_timer_autocomplete = true;
        }

        if (ch.ModelType() == kModelWorkspace
                || ch.ModelType() == kModelClient
                || ch.ModelType() == kModelProject
                || ch.ModelType() == kModelTask) {
            display_project_autocomplete = true;
        }

        if (ch.ModelType() == kModelClient
                || ch.ModelType() == kModelWorkspace) {
            display_client_select = true;
        }

        // Check if time entry editor needs to be updated
        if (ch.ModelType() == kModelTimeEntry) {
            display_timer_state = true;
            // If time entry was edited, check further
            if (time_entry_editor_guid == ch.GUID()) {
                // If time entry was deleted, close editor
                // and open list view
                if (ch.ChangeType() == kChangeTypeDelete) {
                    open_time_entry_list = true;
                    display_time_entries = true;
                } else {
                    display_time_entry_editor = true;
                }
            }
        }

        if (ch.ModelType() == kModelAutotrackerRule) {
            display_autotracker_rules = true;
        }

        if (ch.ModelType() == kModelSettings) {
            display_settings = true;
        }
    }
}

void Context::OpenTimeEntryList() {
    logger().debug("OpenTimeEntryList");

    UIElements render;
    render.open_time_entry_list = true;
    render.display_time_entries = true;
    updateUI(render);
}

void Context::updateUI(const UIElements &what) {
    logger().debug("updateUI " + what.String());

    view::TimeEntry editor_time_entry_view;

    std::vector<view::Autocomplete> time_entry_autocompletes;
    std::vector<view::Autocomplete> minitimer_autocompletes;
    std::vector<view::Autocomplete> project_autocompletes;

    bool use_proxy(false);
    bool record_timeline(false);
    Poco::Int64 unsynced_item_count(0);
    Proxy proxy;

    view::TimeEntry running_entry_view;

    std::vector<view::TimeEntry> time_entry_views;

    std::vector<view::Generic> client_views;
    std::vector<view::Generic> workspace_views;
    std::vector<view::Generic> tag_views;

    std::vector<view::AutotrackerRule> autotracker_rule_views;
    std::vector<std::string> autotracker_title_views;

    // Collect data
    {
        Poco::Mutex::ScopedLock lock(user_m_);

        if (what.display_time_entry_editor && user_) {
            TimeEntry *editor_time_entry =
                user_->related.TimeEntryByGUID(what.time_entry_editor_guid);
            if (editor_time_entry) {
                if (what.open_time_entry_editor) {
                    time_entry_editor_guid_ = editor_time_entry->GUID();
                }

                editor_time_entry_view.Fill(editor_time_entry);
                if (editor_time_entry->IsTracking()) {
                    editor_time_entry_view.Duration =
                        toggl::Formatter::FormatDuration(
                            editor_time_entry->DurationInSeconds(),
                            Format::Classic);
                } else {
                    editor_time_entry_view.Duration =
                        toggl::Formatter::FormatDuration(
                            editor_time_entry->DurationInSeconds(),
                            Formatter::DurationFormat);
                }
                editor_time_entry_view.DateDuration =
                    Formatter::FormatDurationForDateHeader(
                        user_->related.TotalDurationForDate(
                            editor_time_entry));
                user_->related.ProjectLabelAndColorCode(
                    editor_time_entry,
                    &editor_time_entry_view);

                // Various fields in TE editor related to workspace
                // and user permissions
                Workspace *ws = nullptr;
                if (editor_time_entry->WID()) {
                    ws = user_->related.WorkspaceByID(editor_time_entry->WID());
                }
                if (ws) {
                    editor_time_entry_view.CanAddProjects =
                        ws->Admin() || !ws->OnlyAdminsMayCreateProjects();
                } else {
                    editor_time_entry_view.CanAddProjects =
                        user_->CanAddProjects();
                }
                editor_time_entry_view.CanSeeBillable =
                    user_->CanSeeBillable(ws);
                editor_time_entry_view.DefaultWID = user_->DefaultWID();

                editor_time_entry_view.Locked = isTimeEntryLocked(
                    editor_time_entry);

                // Display tags also when time entry is being edited,
                // because tags are filtered by TE WID
                std::vector<std::string> tags;
                user_->related.TagList(&tags, editor_time_entry->WID());
                for (std::vector<std::string>::const_iterator
                        it = tags.begin();
                        it != tags.end();
                        it++) {
                    view::Generic view;
                    view.Name = *it;
                    tag_views.push_back(view);
                }
            }
        }

        if (what.display_workspace_select && user_) {
            std::vector<Workspace *> workspaces;
            user_->related.WorkspaceList(&workspaces);
            for (std::vector<Workspace *>::const_iterator
                    it = workspaces.begin();
                    it != workspaces.end();
                    it++) {
                Workspace *ws = *it;
                view::Generic view;
                view.GUID = ws->GUID();
                view.ID = ws->ID();
                view.WID = ws->ID();
                view.Name = ws->Name();
                view.WorkspaceName = ws->Name();
                workspace_views.push_back(view);
            }
        }

        if (what.display_client_select && user_) {
            std::vector<Client *> models;
            user_->related.ClientList(&models);
            for (std::vector<Client *>::const_iterator it = models.begin();
                    it != models.end();
                    it++) {
                Client *c = *it;
                view::Generic view;
                view.GUID = c->GUID();
                view.ID = c->ID();
                view.WID = c->WID();
                view.Name = c->Name();
                if (c->WID()) {
                    Workspace *ws = user_->related.WorkspaceByID(c->WID());
                    if (ws) {
                        view.WorkspaceName = ws->Name();
                    }
                }
                client_views.push_back(view);
            }
        }

        if (what.display_timer_state && user_) {
            TimeEntry *running_entry = user_->RunningTimeEntry();
            if (running_entry) {
                running_entry_view.Fill(running_entry);
                running_entry_view.Duration =
                    toggl::Formatter::FormatDuration(
                        running_entry->DurationInSeconds(),
                        Format::Classic);
                running_entry_view.DateDuration =
                    Formatter::FormatDurationForDateHeader(
                        user_->related.TotalDurationForDate(
                            running_entry));
                user_->related.ProjectLabelAndColorCode(
                    running_entry,
                    &running_entry_view);
            }
        }

        if (what.display_time_entries && user_) {
            if (what.open_time_entry_list) {
                time_entry_editor_guid_ = "";
            }

            // Get a sorted list of time entries
            std::vector<TimeEntry *> time_entries =
                user_->related.VisibleTimeEntries();
            std::sort(time_entries.begin(), time_entries.end(),
                      CompareByStart);

            // Collect the time entries into a list
            std::map<std::string, Poco::Int64> date_durations;

            // Group data maps
            std::map<std::string, Poco::Int64> group_durations;
            std::map<std::string, Poco::Int64> group_header_id;
            std::map<std::string, std::vector<Poco::Int64> > group_items;

            for (unsigned int i = 0; i < time_entries.size(); i++) {
                TimeEntry *te = time_entries[i];

                std::string date_header =
                    toggl::Formatter::FormatDateHeader(te->Start());

                // Calculate total duration for each date:
                // will be displayed in date header
                Poco::Int64 duration = date_durations[date_header];
                duration += Formatter::AbsDuration(te->Duration());
                date_durations[date_header] = duration;

                // Dont render running entry in list,
                // although its calculated into totals per date.
                if (te->Duration() < 0) {
                    // Don't display running entries
                    continue;
                }

                // Calculate total duration of group
                if (user_->CollapseEntries()) {
                    std::stringstream ss;
                    ss << date_header << te->Description()
                       << te->PID() << te->TID()
                       << te->Billable() << te->Tags();
                    std::string group_name = ss.str();

                    group_header_id[group_name] = i;
                    duration = group_durations[group_name];
                    duration += Formatter::AbsDuration(te->Duration());
                    group_durations[group_name] = duration;
                    group_items[group_name].push_back(i);
                }
            }

            // Assign the date durations we calculated previously
            for (unsigned int i = 0; i < time_entries.size(); i++) {
                TimeEntry *te = time_entries[i];

                // Dont render running entry in list,
                // although its calculated into totals per date.
                if (te->Duration() < 0) {
                    // Don't display running entries
                    continue;
                }

                view::TimeEntry view;
                view.Fill(te);

                // Assign group info
                if (user_->CollapseEntries()) {
                    if (group_items[view.GroupName].size() > 1) {
                        if (group_header_id[view.GroupName] == i) {
                            // If Group open add all entries in group
                            if (entry_groups[view.GroupName]) {
                                for (unsigned int j = 0; j < group_items[view.GroupName].size(); j++) {
                                    TimeEntry *group_entry =
                                        time_entries[group_items[view.GroupName][j]];

                                    view::TimeEntry group_entry_view;
                                    group_entry_view.Fill(group_entry);

                                    group_entry_view.GroupOpen = entry_groups[view.GroupName];

                                    user_->related.ProjectLabelAndColorCode(
                                        group_entry,
                                        &group_entry_view);

                                    group_entry_view.Locked = isTimeEntryLocked(group_entry);

                                    group_entry_view.Duration = toggl::Formatter::FormatDuration(
                                        group_entry_view.DurationInSeconds,
                                        Formatter::DurationFormat);
                                    group_entry_view.DateDuration =
                                        Formatter::FormatDurationForDateHeader(
                                            date_durations[group_entry_view.DateHeader]);
                                    group_entry_view.GroupItemCount = entry_groups[group_entry_view.GroupName];
                                    time_entry_views.push_back(group_entry_view);
                                }
                            }

                            // Add Group header
                            view::TimeEntry group_view;
                            group_view.Fill(te);
                            user_->related.ProjectLabelAndColorCode(
                                te,
                                &group_view);
                            group_view.Group = true;
                            group_view.GroupOpen = entry_groups[group_view.GroupName];
                            group_view.Duration =
                                Formatter::FormatDuration(
                                    group_durations[view.GroupName],
                                    Formatter::DurationFormat);
                            group_view.DateDuration =
                                Formatter::FormatDurationForDateHeader(
                                    date_durations[view.DateHeader]);
                            group_view.GroupItemCount = group_items[group_view.GroupName].size();
                            time_entry_views.push_back(group_view);
                        }
                        continue;
                    }
                    view.GroupItemCount = 1;
                }
                user_->related.ProjectLabelAndColorCode(
                    te,
                    &view);

                view.Locked = isTimeEntryLocked(te);
                view.GroupOpen = false;

                view.Duration = toggl::Formatter::FormatDuration(
                    view.DurationInSeconds,
                    Formatter::DurationFormat);
                view.DateDuration =
                    Formatter::FormatDurationForDateHeader(
                        date_durations[view.DateHeader]);
                time_entry_views.push_back(view);
            }
        }

        if (what.display_settings) {
            error err = db()->LoadSettings(&settings_);
            if (err != noError) {
                setUser(nullptr);
                displayError(err);
                return;
            }
            err = db()->LoadProxySettings(&use_proxy, &proxy);
            if (err != noError) {
                setUser(nullptr);
                displayError(err);
                return;
            }
            if (user_) {
                record_timeline = user_->RecordTimeline();
            }
            idle_.SetSettings(settings_);

            HTTPSClient::Config.UseProxy = use_proxy;
            HTTPSClient::Config.ProxySettings = proxy;
            HTTPSClient::Config.AutodetectProxy = settings_.autodetect_proxy;
        }

        if (what.display_unsynced_items && user_) {
            unsynced_item_count = user_->related.NumberOfUnsyncedTimeEntries();
        }

        if (what.display_autotracker_rules && user_) {
            if (UI()->CanDisplayAutotrackerRules()) {
                // Collect rules
                for (std::vector<toggl::AutotrackerRule *>::const_iterator
                        it = user_->related.AutotrackerRules.begin();
                        it != user_->related.AutotrackerRules.end();
                        it++) {
                    AutotrackerRule *model = *it;
                    Project *p = user_->related.ProjectByID(model->PID());
                    Task *t = user_->related.TaskByID(model->TID());

                    view::AutotrackerRule rule;
                    rule.ProjectName = Formatter::JoinTaskName(t, p);
                    rule.ID = model->LocalID();
                    rule.Term = model->Term();
                    autotracker_rule_views.push_back(rule);
                }

                // Collect titles
                for (std::set<std::string>::const_iterator
                        it = autotracker_titles_.begin();
                        it != autotracker_titles_.end();
                        ++it) {
                    autotracker_title_views.push_back(*it);
                }
                std::sort(autotracker_title_views.begin(),
                          autotracker_title_views.end(),
                          CompareAutotrackerTitles);
            }
        }
    }

    // Render data
    if (what.display_time_entry_editor
            && !editor_time_entry_view.GUID.empty()) {
        if (what.open_time_entry_editor) {
            UI()->DisplayApp();
        }
        UI()->DisplayTags(tag_views);
        UI()->DisplayTimeEntryEditor(
            what.open_time_entry_editor,
            editor_time_entry_view,
            what.time_entry_editor_field);
    }

    if (what.display_time_entries) {
        UI()->DisplayTimeEntryList(
            what.open_time_entry_list,
            time_entry_views,
            !user_->HasLoadedMore());
        last_time_entry_list_render_at_ = Poco::LocalDateTime();
    }

    if (what.display_time_entry_autocomplete) {
        if (what.first_load) {
            if (user_) {
                user_->related.TimeEntryAutocompleteItems(&time_entry_autocompletes);
            }
            UI()->DisplayTimeEntryAutocomplete(&time_entry_autocompletes);
        } else {
            Poco::Util::TimerTask::Ptr teTask =
                new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onTimeEntryAutocompletes);
            timer_.schedule(teTask, Poco::Timestamp());
        }
    }

    if (what.display_mini_timer_autocomplete) {
        if (what.first_load) {
            if (user_) {
                user_->related.MinitimerAutocompleteItems(&minitimer_autocompletes);
            }
            UI()->DisplayMinitimerAutocomplete(&minitimer_autocompletes);
        } else {
            Poco::Util::TimerTask::Ptr mtTask =
                new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onMiniTimerAutocompletes);
            timer_.schedule(mtTask, Poco::Timestamp());
        }
    }

    if (what.display_workspace_select) {
        UI()->DisplayWorkspaceSelect(workspace_views);
    }

    if (what.display_client_select) {
        UI()->DisplayClientSelect(client_views);
    }

    if (what.display_timer_state) {
        if (!running_entry_view.GUID.empty() && user_) {
            UI()->DisplayTimerState(running_entry_view);
        } else {
            UI()->DisplayEmptyTimerState();
        }
    }

    if (what.display_autotracker_rules) {
        if (UI()->CanDisplayAutotrackerRules()) {
            UI()->DisplayAutotrackerRules(
                autotracker_rule_views,
                autotracker_title_views);
        }
    }

    if (what.display_settings) {
        UI()->DisplaySettings(what.open_settings,
                              record_timeline,
                              settings_,
                              use_proxy,
                              proxy);
        // Tracking Settings
        if ("production" == environment_) {
            analytics_.TrackSettings(db_->AnalyticsClientID(),
                                     record_timeline,
                                     settings_,
                                     use_proxy,
                                     proxy);
        }
    }

    // Apply autocomplete as last element,
    // as its depending on selects on Windows
    if (what.display_project_autocomplete) {
        if (what.first_load) {
            if (user_) {
                user_->related.ProjectAutocompleteItems(&project_autocompletes);
            }
            UI()->DisplayProjectAutocomplete(&project_autocompletes);
        } else {
            Poco::Util::TimerTask::Ptr prTask =
                new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onProjectAutocompletes);
            timer_.schedule(prTask, Poco::Timestamp());
        }
    }

    if (what.display_unsynced_items) {
        UI()->DisplayUnsyncedItems(unsynced_item_count);
    }
}

Poco::Timestamp Context::postpone(
    const Poco::Timestamp::TimeDiff throttleMicros) const {
    return Poco::Timestamp() + throttleMicros;
}

bool Context::isPostponed(
    const Poco::Timestamp value,
    const Poco::Timestamp::TimeDiff throttleMicros) const {
    Poco::Timestamp now;
    if (now > value) {
        return false;
    }
    Poco::Timestamp::TimeDiff diff = value - now;
    if (diff > 2*throttleMicros) {
        logger().warning(
            "Cannot postpone task, its foo far in the future");
        return false;
    }
    return true;
}

error Context::displayError(const error err) {
    if ((err.find(kUnauthorizedError) != std::string::npos)) {
        if (user_) {
            setUser(nullptr);
        }
    }
    if (err.find(kUnsupportedAppError) != std::string::npos) {
        urls::SetImATeapot(true);
    } else {
        urls::SetImATeapot(false);
    }

    if (user_ && (err.find(kRequestIsNotPossible) != std::string::npos
                  || (err.find(kForbiddenError) != std::string::npos))) {
        TogglClient toggl_client(UI());

        error err = pullWorkspaces(&toggl_client);
        if (err != noError) {
            // Check for missing WS error and
            if (err.find(kMissingWS) != std::string::npos) {
                overlay_visible_ = true;
                UI()->DisplayWSError();
                return noError;
            }
        }
    }

    return UI()->DisplayError(err);
}

int Context::nextSyncIntervalSeconds() const {
    Poco::Random random;
    random.seed();
    int n = random.next(kSyncIntervalRangeSeconds) + kSyncIntervalRangeSeconds;
    std::stringstream ss;
    ss << "Next autosync in " << n << " seconds";
    logger().trace(ss.str());
    return n;
}

void Context::scheduleSync() {
    Poco::Int64 elapsed_seconds = Poco::Int64(time(0)) - last_sync_started_;

    {
        std::stringstream ss;
        ss << "scheduleSync elapsed_seconds=" << elapsed_seconds;
        logger().debug(ss.str());
    }

    if (elapsed_seconds < sync_interval_seconds_) {
        std::stringstream ss;
        ss << "Last sync attempt less than " << sync_interval_seconds_
           << " seconds ago, chill";
        logger().trace(ss.str());
        return;
    }

    Sync();
}

void Context::FullSync() {
    user_->SetSince(0);
    Sync();
}

void Context::Sync() {
    logger().debug("Sync");

    Poco::Timestamp::TimeDiff delay = 0;
    if (next_sync_at_ > 0) {
        delay = kRequestThrottleSeconds * kOneSecondInMicros;
    }

    next_sync_at_ = postpone(delay);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onSync);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_sync_at_);

    std::stringstream ss;
    ss << "Next sync at "
       << Formatter::Format8601(next_sync_at_);
    logger().debug(ss.str());
}

void Context::onSync(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_sync_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger().debug("onSync postponed");
        return;
    }
    logger().debug("onFullSync executing");

    if (!user_) {
        return;
    }

    overlay_visible_ = false;
    last_sync_started_ = time(0);

    // Always sync asyncronously with syncerActivity
    trigger_sync_ = true;
    if (!syncer_.isRunning()) {
        syncer_.start();
    }
}

void Context::onTimeEntryAutocompletes(Poco::Util::TimerTask& task) {  // NOLINT
    std::vector<view::Autocomplete> time_entry_autocompletes;
    if (user_) {
        user_->related.TimeEntryAutocompleteItems(&time_entry_autocompletes);
    }
    UI()->DisplayTimeEntryAutocomplete(&time_entry_autocompletes);
}

void Context::onMiniTimerAutocompletes(Poco::Util::TimerTask& task) {  // NOLINT
    std::vector<view::Autocomplete> minitimer_autocompletes;
    if (user_) {
        user_->related.MinitimerAutocompleteItems(&minitimer_autocompletes);
    }
    UI()->DisplayMinitimerAutocomplete(&minitimer_autocompletes);
}

void Context::onProjectAutocompletes(Poco::Util::TimerTask& task) {  // NOLINT
    std::vector<view::Autocomplete> project_autocompletes;
    if (user_) {
        user_->related.ProjectAutocompleteItems(&project_autocompletes);
    }
    UI()->DisplayProjectAutocomplete(&project_autocompletes);
}

void Context::setOnline(const std::string reason) {
    std::stringstream ss;
    ss << "setOnline, reason:" << reason;
    logger().debug(ss.str());

    if (quit_) {
        return;
    }

    UI()->DisplayOnlineState(kOnlineStateOnline);

    scheduleSync();
}

void Context::onPushChanges(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_push_changes_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger().debug("onPushChanges postponed");
        return;
    }

    if (!user_) {
        logger().debug("onPushChanges cancelled, user not logged in");
        return;
    }
    logger().debug("onPushChanges executing");

    // Always sync asyncronously with syncerActivity
    trigger_push_ = true;
    if (!syncer_.isRunning()) {
        syncer_.start();
    }
}

void Context::switchWebSocketOff() {
    logger().debug("switchWebSocketOff");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchWebSocketOff);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchWebSocketOff(Poco::Util::TimerTask& task) {  // NOLINT
    logger().debug("onSwitchWebSocketOff");

    Poco::Mutex::ScopedLock lock(ws_client_m_);
    ws_client_.Shutdown();
}

error Context::LoadUpdateFromJSONString(const std::string json) {
    std::stringstream ss;
    ss << "LoadUpdateFromJSONString json=" << json;
    logger().debug(ss.str());

    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger().warning("User is logged out, cannot update");
        return noError;
    }

    TimeEntry *running_entry = user_->RunningTimeEntry();

    error err = user_->LoadUserUpdateFromJSONString(json);
    if (err != noError) {
        return displayError(err);
    }

    TimeEntry *new_running_entry = user_->RunningTimeEntry();

    // Reset reminder time when entry stopped by websocket
    if (running_entry && !new_running_entry) {
        resetLastTrackingReminderTime();
    }

    return displayError(save());
}

void Context::switchWebSocketOn() {
    logger().debug("switchWebSocketOn");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchWebSocketOn);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchWebSocketOn(Poco::Util::TimerTask& task) {  // NOLINT
    logger().debug("onSwitchWebSocketOn");

    std::string apitoken("");

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (user_) {
            apitoken = user_->APIToken();
        }
    }

    if (apitoken.empty()) {
        logger().error("No API token, cannot switch Websocket on");
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(ws_client_m_);
        ws_client_.Start(this, apitoken, on_websocket_message);
    }
}

// Start/stop timeline recording on local machine
void Context::switchTimelineOff() {
    logger().debug("switchTimelineOff");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchTimelineOff);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchTimelineOff(Poco::Util::TimerTask& task) {  // NOLINT
    logger().debug("onSwitchTimelineOff");

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            delete timeline_uploader_;
            timeline_uploader_ = nullptr;
        }
    }
}

void Context::switchTimelineOn() {
    logger().debug("switchTimelineOn");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchTimelineOn);

    if (quit_) {
        return;
    }

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchTimelineOn(Poco::Util::TimerTask& task) {  // NOLINT
    logger().debug("onSwitchTimelineOn");

    if (quit_) {
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_ || !user_->RecordTimeline()) {
            return;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            delete timeline_uploader_;
            timeline_uploader_ = nullptr;
        }
        timeline_uploader_ = new TimelineUploader(this);
    }
}

void Context::fetchUpdates() {
    logger().debug("fetchUpdates");

    next_fetch_updates_at_ =
        postpone(kRequestThrottleSeconds * kOneSecondInMicros);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onFetchUpdates);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_fetch_updates_at_);

    std::stringstream ss;
    ss << "Next update fetch at "
       << Formatter::Format8601(next_fetch_updates_at_);
    logger().debug(ss.str());
}

void Context::onFetchUpdates(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_fetch_updates_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger().debug("onFetchUpdates postponed");
        return;
    }

    executeUpdateCheck();
}

void Context::startPeriodicSync() {
    logger().trace("startPeriodicSync");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>
    (*this, &Context::onPeriodicSync);

    sync_interval_seconds_ = nextSyncIntervalSeconds();

    Poco::Timestamp next_periodic_sync_at_ =
        Poco::Timestamp() + (sync_interval_seconds_ * kOneSecondInMicros);
    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_periodic_sync_at_);

    std::stringstream ss;
    ss << "Next periodic sync at "
       << Formatter::Format8601(next_periodic_sync_at_);
    logger().debug(ss.str());
}

void Context::onPeriodicSync(Poco::Util::TimerTask& task) {  // NOLINT
    logger().debug("onPeriodicSync");

    scheduleSync();

    startPeriodicSync();
}

void Context::startPeriodicUpdateCheck() {
    logger().debug("startPeriodicUpdateCheck");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>
    (*this, &Context::onPeriodicUpdateCheck);

    Poco::UInt64 micros = kCheckUpdateIntervalSeconds *
                          Poco::UInt64(kOneSecondInMicros);
    Poco::Timestamp next_periodic_check_at = Poco::Timestamp() + micros;
    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_periodic_check_at);

    std::stringstream ss;
    ss << "Next periodic update check at "
       << Formatter::Format8601(next_periodic_check_at);
    logger().debug(ss.str());
}

void Context::onPeriodicUpdateCheck(Poco::Util::TimerTask& task) {  // NOLINT
    logger().debug("onPeriodicUpdateCheck");

    executeUpdateCheck();

    startPeriodicUpdateCheck();
}

error Context::UpdateChannel(
    std::string *update_channel) {
    poco_check_ptr(update_channel);

    error err = db()->LoadUpdateChannel(update_channel);

    if (err.find(kDatabaseDiskMalformed) != std::string::npos) {
        err = noError;
    }
    return displayError(err);
}

std::string Context::UserFullName() {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        return "";
    }
    return user_->Fullname();
}

std::string Context::UserEmail() {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        return "";
    }
    return user_->Email();
}

void Context::executeUpdateCheck() {
    logger().debug("executeUpdateCheck");

    displayError(downloadUpdate());
}

error Context::downloadUpdate() {
    try {
        if (update_check_disabled_) {
            return noError;
        }

        // To test updater in development, comment this block out:
        if ("production" != environment_) {
            logger().debug("Not in production, will not download updates");
            return noError;
        }

        // Load current update channel
        std::string update_channel("");
        error err = db()->LoadUpdateChannel(&update_channel);
        if (err != noError) {
            return err;
        }

        // Get update check URL
        std::string update_url("");
        err = updateURL(&update_url);
        if (err != noError) {
            return err;
        }

        // Ask Toggl server if we have updates
        std::string url("");
        std::string version_number("");
        {
            HTTPSRequest req;
            req.host = urls::API();
            req.relative_url = update_url;

            TogglClient client;
            HTTPSResponse resp = client.Get(req);
            if (resp.err != noError) {
                return resp.err;
            }

            if ("null" == resp.body) {
                logger().debug("The app is up to date");
                if (UI()->CanDisplayUpdate()) {
                    UI()->DisplayUpdate("");
                }
                return noError;
            }

            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(resp.body, root)) {
                return error("Error parsing update check response body");
            }

            url = root["url"].asString();
            version_number = root["version"].asString();

            std::stringstream ss;
            ss << "Found update " << version_number
               << " (" << url << ")";
            logger().debug(ss.str());
        }

        // linux has non-silent updates, just pass on the URL
        // linux users will download the update themselves
        if (UI()->CanDisplayUpdate()) {
            UI()->DisplayUpdate(url);
            return noError;
        }

        // we need a path to download to, when going this way
        if (update_path_.empty()) {
            return error("update path is empty, cannot download update");
        }

        // Ignore update if not compatible with this client version
        // only windows .exe installers are supported atm
        if (url.find(".exe") == std::string::npos) {
            logger().debug("Update is not compatible with this client,"
                           " will ignore");
            return noError;
        }

        // Download update if it's not downloaded yet.
        {
            Poco::URI uri(url);

            std::vector<std::string> path_segments;
            uri.getPathSegments(path_segments);

            Poco::Path save_location(update_path_);
            save_location.append(path_segments.back());
            std::string file = save_location.toString();

            Poco::File f(file);
            if (f.exists()) {
                logger().debug("File already exists: " + file);
                return noError;
            }

            Poco::File(update_path_).createDirectory();

            if (UI()->CanDisplayUpdateDownloadState()) {
                UI()->DisplayUpdateDownloadState(
                    version_number,
                    kDownloadStatusStarted);
            }

            std::auto_ptr<std::istream> stream(
                Poco::URIStreamOpener::defaultOpener().open(uri));
            Poco::FileOutputStream fos(file, std::ios::binary);
            Poco::StreamCopier::copyStream(*stream.get(), fos);
            fos.flush();
            fos.close();

            if (UI()->CanDisplayUpdateDownloadState()) {
                UI()->DisplayUpdateDownloadState(
                    version_number,
                    kDownloadStatusDone);
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Context::updateURL(std::string *result) {
    std::string update_channel("");
    error err = db()->LoadUpdateChannel(&update_channel);
    if (err != noError) {
        return err;
    }

    if (HTTPSClient::Config.AppVersion.empty()) {
        return error("Cannot check for updates without app version");
    }

    // Not implemented in v9 as of 12.05.2017
    std::stringstream relative_url;
    relative_url << "/api/v8/updates?app=td"
                 << "&channel=" << update_channel
                 << "&platform=" << installerPlatform()
                 << "&version=" << HTTPSClient::Config.AppVersion
                 << "&osname=" << Poco::Environment::osName()
                 << "&osversion=" << Poco::Environment::osVersion()
                 << "&osarch=" << Poco::Environment::osArchitecture();
    *result = relative_url.str();

    return noError;
}

const std::string Context::linuxPlatformName() {
    if (kDebianPackage) {
        return "deb64";
    }
    return std::string("linux");
}

const std::string Context::installerPlatform() {
    std::stringstream ss;
    if (POCO_OS_LINUX == POCO_OS) {
        ss <<  linuxPlatformName();
    } else if (POCO_OS_WINDOWS_NT == POCO_OS) {
        ss << "windows";
    } else {
        ss << "darwin";
    }
    if (kEnterpriseInstall) {
        ss << "_enterprise";
    }
    return ss.str();
}

const std::string Context::shortOSName() {
    if (POCO_OS_LINUX == POCO_OS) {
        return "linux";
    } else if (POCO_OS_WINDOWS_NT == POCO_OS) {
        return "win";
    } else if (POCO_OS_MAC_OS_X == POCO_OS) {
        return "mac";
    }
    return "unknown";
}

void Context::TimelineUpdateServerSettings() {
    logger().debug("TimelineUpdateServerSettings");

    next_update_timeline_settings_at_ =
        postpone(kRequestThrottleSeconds * kOneSecondInMicros);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this,
                &Context::onTimelineUpdateServerSettings);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_update_timeline_settings_at_);

    std::stringstream ss;
    ss << "Next timeline settings update at "
       << Formatter::Format8601(next_update_timeline_settings_at_);
    logger().debug(ss.str());
}

const std::string kRecordTimelineEnabledJSON = "{\"record_timeline\": true}";
const std::string kRecordTimelineDisabledJSON = "{\"record_timeline\": false}";

void Context::onTimelineUpdateServerSettings(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_update_timeline_settings_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger().debug("onTimelineUpdateServerSettings postponed");
        return;
    }

    logger().debug("onTimelineUpdateServerSettings executing");

    std::string apitoken("");
    std::string json(kRecordTimelineDisabledJSON);

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return;
        }
        if (user_->RecordTimeline()) {
            json = kRecordTimelineEnabledJSON;
        }
        apitoken = user_->APIToken();
    }

    // Not implemented in v9 as of 12.05.2017
    HTTPSRequest req;
    req.host = urls::TimelineUpload();
    req.relative_url = "/api/v8/timeline_settings";
    req.payload = json;
    req.basic_auth_username = apitoken;
    req.basic_auth_password = "api_token";

    TogglClient client(UI());
    HTTPSResponse resp = client.Post(req);
    if (resp.err != noError) {
        displayError(resp.err);
        logger().error(resp.body);
        logger().error(resp.err);
    }
}

error Context::SendFeedback(Feedback fb) {
    if (!user_) {
        logger().warning("Cannot send feedback, user logged out");
        return noError;
    }

    error err = fb.Validate();
    if (err != noError) {
        return displayError(err);
    }

    feedback_ = fb;

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSendFeedback);

    {
        Poco::Mutex::ScopedLock lock(timer_m_);
        timer_.schedule(ptask, Poco::Timestamp());
    }

    return noError;
}

void Context::onSendFeedback(Poco::Util::TimerTask& task) {  // NOLINT
    logger().debug("onSendFeedback");

    std::string api_token_value("");
    std::string api_token_name("");

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (user_) {
            api_token_value = user_->APIToken();
            api_token_name = "api_token";
        }
    }

    std::string update_channel("");
    UpdateChannel(&update_channel);

    Poco::Net::HTMLForm form;
    Json::Value settings_json;

    form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);

    form.set("desktop", "true");
    form.set("toggl_version", HTTPSClient::Config.AppVersion);
    form.set("details", Formatter::EscapeJSONString(feedback_.Details()));
    form.set("subject", Formatter::EscapeJSONString(feedback_.Subject()));
    form.set("date", Formatter::Format8601(time(0)));
    form.set("update_channel", Formatter::EscapeJSONString(update_channel));

    if (!feedback_.AttachmentPath().empty()) {
        form.addPart("files",
                     new Poco::Net::FilePartSource(feedback_.AttachmentPath()));
    }

    // Add all log files to feedback
    int count = 0;
    bool exists = true;

    while (exists) {
        std::stringstream ss;
        ss << log_path_ << "." << std::to_string(count);
        Poco::File file(ss.str());
        exists = file.exists();
        if (exists) {
            form.addPart("files",
                         new Poco::Net::FilePartSource(ss.str()));
        }
        count++;
    }

    form.addPart("files",
                 new Poco::Net::FilePartSource(log_path_));

    settings_json = settings_.SaveToJSON();
    if (user_) {
        settings_json["record_timeline"] = user_->RecordTimeline();
    }

    form.addPart("files",
                 new Poco::Net::StringPartSource(
                     Json::StyledWriter().write(settings_json),
                     "application/json",
                     "settings.json"));

    // Not implemented in v9 as of 12.05.2017
    HTTPSRequest req;
    req.host = urls::API();
    req.relative_url ="/api/v8/feedback/web";
    req.basic_auth_username = api_token_value;
    req.basic_auth_password = api_token_name;
    req.form = &form;

    TogglClient client(UI());
    HTTPSResponse resp = client.Post(req);
    logger().debug("Feedback result: " + resp.err);
    if (resp.err != noError) {
        displayError(resp.err);
        return;
    }
}

error Context::SetSettingsRemindTimes(
    const std::string remind_starts,
    const std::string remind_ends) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsRemindTimes(remind_starts, remind_ends));
}

error Context::SetSettingsRemindDays(
    const bool remind_mon,
    const bool remind_tue,
    const bool remind_wed,
    const bool remind_thu,
    const bool remind_fri,
    const bool remind_sat,
    const bool remind_sun) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsRemindDays(
            remind_mon,
            remind_tue,
            remind_wed,
            remind_thu,
            remind_fri,
            remind_sat,
            remind_sun));
}

error Context::SetSettingsAutodetectProxy(const bool autodetect_proxy) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsAutodetectProxy(autodetect_proxy));
}

error Context::SetSettingsUseIdleDetection(const bool use_idle_detection) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsUseIdleDetection(use_idle_detection));
}

error Context::applySettingsSaveResultToUI(const error err) {
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    return noError;
}

error Context::SetSettingsAutotrack(const bool value) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsAutotrack(value));
}

error Context::SetSettingsOpenEditorOnShortcut(const bool value) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsOpenEditorOnShortcut(value));
}

error Context::SetSettingsMenubarTimer(const bool menubar_timer) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsMenubarTimer(menubar_timer));
}

error Context::SetSettingsMenubarProject(const bool menubar_project) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsMenubarProject(menubar_project));
}

error Context::SetSettingsDockIcon(const bool dock_icon) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsDockIcon(dock_icon));
}

error Context::SetSettingsOnTop(const bool on_top) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsOnTop(on_top));
}

error Context::SetSettingsReminder(const bool reminder) {
    error err = applySettingsSaveResultToUI(
        db()->SetSettingsReminder(reminder));
    if (err == noError) {
        resetLastTrackingReminderTime();
    }
    return err;
}

error Context::SetSettingsPomodoro(const bool pomodoro) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsPomodoro(pomodoro));
}

error Context::SetSettingsPomodoroBreak(const bool pomodoro_break) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsPomodoroBreak(pomodoro_break));
}

error Context::SetSettingsStopEntryOnShutdownSleep(const bool stop_entry) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsStopEntryOnShutdownSleep(stop_entry));
}

error Context::SetSettingsIdleMinutes(const Poco::UInt64 idle_minutes) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsIdleMinutes(idle_minutes));
}

error Context::SetSettingsFocusOnShortcut(const bool focus_on_shortcut) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsFocusOnShortcut(focus_on_shortcut));
}

error Context::SetSettingsManualMode(const bool manual_mode) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsManualMode(manual_mode));
}

error Context::SetSettingsReminderMinutes(const Poco::UInt64 reminder_minutes) {
    const error err = applySettingsSaveResultToUI(
        db()->SetSettingsReminderMinutes(reminder_minutes));
    if (err == noError) {
        resetLastTrackingReminderTime();
    }
    return err;
}

error Context::SetSettingsPomodoroMinutes(const Poco::UInt64 pomodoro_minutes) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsPomodoroMinutes(pomodoro_minutes));
}

error Context::SetSettingsPomodoroBreakMinutes(
    const Poco::UInt64 pomodoro_break_minutes) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsPomodoroBreakMinutes(pomodoro_break_minutes));
}

error Context::LoadWindowSettings(
    int64_t *window_x,
    int64_t *window_y,
    int64_t *window_height,
    int64_t *window_width) {

    Poco::Int64 x(0), y(0), h(0), w(0);

    error err = db()->LoadWindowSettings(&x, &y, &h, &w);
    if (noError == err) {
        *window_x = x;
        *window_y = y;
        *window_height = h;
        *window_width = w;
    }
    return displayError(err);
}

error Context::SaveWindowSettings(
    const int64_t window_x,
    const int64_t window_y,
    const int64_t window_height,
    const int64_t window_width) {

    error err = db()->SaveWindowSettings(
        window_x,
        window_y,
        window_height,
        window_width);
    return displayError(err);
}

Poco::Int64 Context::GetMiniTimerX() {
    Poco::Int64 value(0);
    displayError(db()->GetMiniTimerX(&value));
    return value;
}

void Context::SetMiniTimerX(const int64_t x) {
    displayError(db()->SetMiniTimerX(x));
}

Poco::Int64 Context::GetMiniTimerY() {
    Poco::Int64 value(0);
    displayError(db()->GetMiniTimerY(&value));
    return value;
}

void Context::SetMiniTimerY(const int64_t y) {
    displayError(db()->SetMiniTimerY(y));
}

Poco::Int64 Context::GetMiniTimerW() {
    Poco::Int64 value(0);
    displayError(db()->GetMiniTimerW(&value));
    return value;
}

void Context::SetMiniTimerW(const int64_t y) {
    displayError(db()->SetMiniTimerW(y));
}

void Context::SetCompactMode(
    const bool value) {
    displayError(db()->SetCompactMode(value));
}

bool Context::GetCompactMode() {
    bool value(false);
    displayError(db()->GetCompactMode(&value));
    return value;
}

void Context::SetMiniTimerVisible(
    const bool value) {
    displayError(db()->SetMiniTimerVisible(value));
}

bool Context::GetMiniTimerVisible() {
    bool value(false);
    displayError(db()->GetMiniTimerVisible(&value));
    return value;
}

void Context::SetKeepEndTimeFixed
(const bool value) {
    displayError(db()->SetKeepEndTimeFixed(value));
}

bool Context::GetKeepEndTimeFixed() {
    bool value(false);
    displayError(db()->GetKeepEndTimeFixed(&value));
    return value;
}

void Context::SetWindowMaximized(
    const bool value) {
    displayError(db()->SetWindowMaximized(value));
}

bool Context::GetWindowMaximized() {
    bool value(false);
    displayError(db()->GetWindowMaximized(&value));
    return value;
}

void Context::SetWindowMinimized(
    const bool_t value) {
    displayError(db()->SetWindowMinimized(value));
}

bool Context::GetWindowMinimized() {
    bool value(false);
    displayError(db()->GetWindowMinimized(&value));
    return value;
}

void Context::SetWindowEditSizeHeight(
    const int64_t value) {
    displayError(db()->SetWindowEditSizeHeight(value));
}

int64_t Context::GetWindowEditSizeHeight() {
    Poco::Int64 value(0);
    displayError(db()->GetWindowEditSizeHeight(&value));
    return value;
}

void Context::SetWindowEditSizeWidth(
    const int64_t value) {
    displayError(db()->SetWindowEditSizeWidth(value));
}

int64_t Context::GetWindowEditSizeWidth() {
    Poco::Int64 value(0);
    displayError(db()->GetWindowEditSizeWidth(&value));
    return value;
}

void Context::SetKeyStart(
    const std::string value) {
    displayError(db()->SetKeyStart(value));
}

std::string Context::GetKeyStart() {
    std::string value("");
    displayError(db()->GetKeyStart(&value));
    return value;
}

void Context::SetKeyShow(
    const std::string value) {
    displayError(db()->SetKeyShow(value));
}

std::string Context::GetKeyShow() {
    std::string value("");
    displayError(db()->GetKeyShow(&value));
    return value;
}

void Context::SetKeyModifierShow(
    const std::string value) {
    displayError(db()->SetKeyModifierShow(value));
}

std::string Context::GetKeyModifierShow() {
    std::string value("");
    displayError(db()->GetKeyModifierShow(&value));
    return value;
}

void Context::SetKeyModifierStart(
    const std::string value) {
    displayError(db()->SetKeyModifierStart(value));
}

std::string Context::GetKeyModifierStart() {
    std::string value("");
    displayError(db()->GetKeyModifierStart(&value));
    return value;
}

error Context::SetProxySettings(
    const bool use_proxy,
    const Proxy proxy) {

    bool was_using_proxy(false);
    Proxy previous_proxy_settings;
    error err = db()->LoadProxySettings(&was_using_proxy,
                                        &previous_proxy_settings);
    if (err != noError) {
        return displayError(err);
    }

    err = db()->SaveProxySettings(use_proxy, proxy);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    if (use_proxy != was_using_proxy
            || proxy.Host() != previous_proxy_settings.Host()
            || proxy.Port() != previous_proxy_settings.Port()
            || proxy.Username() != previous_proxy_settings.Username()
            || proxy.Password() != previous_proxy_settings.Password()) {
        Sync();
        switchWebSocketOn();
    }

    return noError;
}

void Context::OpenSettings() {
    logger().debug("OpenSettings");

    UIElements render;
    render.display_settings = true;
    render.open_settings = true;
    updateUI(render);
}

error Context::SetDBPath(
    const std::string path) {
    try {
        std::stringstream ss;
        ss << "SetDBPath " << path;
        logger().debug(ss.str());

        Poco::Mutex::ScopedLock lock(db_m_);
        if (db_) {
            logger().debug("delete db_ from SetDBPath()");
            delete db_;
            db_ = nullptr;
        }
        db_ = new Database(path);
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

void Context::SetEnvironment(const std::string value) {
    if (!("production" == value ||
            "development" == value ||
            "test" == value)) {
        std::stringstream ss;
        ss << "Invalid environment '" << value << "'!";
        logger().error(ss.str());
        return;
    }
    logger().debug("SetEnvironment " + value);
    environment_ = value;

    HTTPSClient::Config.IgnoreCert = ("development" == environment_);
    urls::SetRequestsAllowed("test" != environment_);
}

Database *Context::db() const {
    poco_check_ptr(db_);
    return db_;
}

error Context::GoogleLogin(const std::string access_token) {
    return Login(access_token, "google_access_token");
}

error Context::attemptOfflineLogin(const std::string email,
                                   const std::string password) {
    if (email.empty()) {
        return error("cannot login offline without an e-mail");
    }

    if (password.empty()) {
        return error("cannot login offline without a password");
    }

    User *user = new User();

    error err = db()->LoadUserByEmail(email, user);
    if (err != noError) {
        delete user;
        return err;
    }

    if (!user->ID()) {
        delete user;
        logger().debug("User data not found in local database for " + email);
        return error(kEmailNotFoundCannotLogInOffline);
    }

    if (user->OfflineData().empty()) {
        delete user;
        logger().debug("Offline data not found in local database for "
                       + email);
        return error(kEmailNotFoundCannotLogInOffline);
    }

    err = user->SetAPITokenFromOfflineData(password);
    if ("I/O error" == err || err.find("bad decrypt") != std::string::npos) {
        delete user;
        return error(kInvalidPassword);
    }
    if (err != noError) {
        delete user;
        return err;
    }

    err = db()->SetCurrentAPIToken(user->APIToken(), user->ID());
    if (err != noError) {
        delete user;
        return err;
    }

    setUser(user, true);

    updateUI(UIElements::Reset());

    return save();
}

error Context::Login(
    const std::string email,
    const std::string password) {
    try {
        TogglClient client(UI());
        std::string json("");
        error err = me(&client, email, password, &json, 0);
        if (err != noError) {
            if (!IsNetworkingError(err)) {
                return displayError(err);
            }
            // Indicate we're offline
            displayError(err);

            std::stringstream ss;
            ss << "Got networking error " << err
               << " will attempt offline login";
            logger().debug(ss.str());

            return displayError(attemptOfflineLogin(email, password));
        }

        err = SetLoggedInUserFromJSON(json);
        if (err != noError) {
            return displayError(err);
        }

        err = pullWorkspacePreferences(&client);
        if (err != noError) {
            return displayError(err);
        }

        err = pullUserPreferences(&client);
        if (err != noError) {
            return displayError(err);
        }

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().error("cannot enable offline login, no user");
                return noError;
            }

            err = user_->EnableOfflineLogin(password);
            if (err != noError) {
                return displayError(err);
            }
        }
        overlay_visible_ = false;
        return displayError(save());
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::Signup(
    const std::string email,
    const std::string password,
    const uint64_t country_id) {

    TogglClient client(UI());
    std::string json("");
    error err = signup(&client, email, password, &json, country_id);
    if (kBadRequestError == err) {
        return displayError(kCheckYourSignupError);
    }
    if (err != noError) {
        return displayError(err);
    }

    return Login(email, password);
}

void Context::setUser(User *value, const bool logged_in) {
    {
        std::stringstream ss;
        ss << "setUser user_logged_in=" << logged_in;
        logger().debug(ss.str());
    }

    Poco::UInt64 user_id(0);

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (user_) {
            delete user_;
        }
        user_ = value;
        if (user_) {
            user_id = user_->ID();
        }
    }

    if (quit_) {
        return;
    }

    if (!user_id) {
        UI()->DisplayLogin(true, 0);

        {
            Poco::Mutex::ScopedLock l(window_change_recorder_m_);
            if (window_change_recorder_) {
                delete window_change_recorder_;
                window_change_recorder_ = nullptr;
            }
        }

        switchTimelineOff();

        switchWebSocketOff();

        // Reset autotracker view
        // Autotracker rules has a project autocomplete, too
        autotracker_titles_.clear();
        UIElements render;
        render.display_autotracker_rules = true;
        render.display_project_autocomplete = true;
        updateUI(render);

        return;
    }

    UI()->DisplayLogin(false, user_id);

    OpenTimeEntryList();

    {
        Poco::Mutex::ScopedLock l(window_change_recorder_m_);
        if (window_change_recorder_) {
            delete window_change_recorder_;
            window_change_recorder_ = nullptr;
        }
        window_change_recorder_ = new WindowChangeRecorder(this);
    }

    switchTimelineOn();

    switchWebSocketOn();

    if (!logged_in) {
        Sync();
    }

    fetchUpdates();

    if (!ui_updater_.isRunning()) {
        ui_updater_.start();
    }

    if (!reminder_.isRunning()) {
        reminder_.start();
    }

    // Offer beta channel, if not offered yet
    bool did_offer_beta_channel(false);
    error err = offerBetaChannel(&did_offer_beta_channel);
    if (err != noError) {
        displayError(err);
    }
}

error Context::SetLoggedInUserFromJSON(
    const std::string json) {

    if (json.empty()) {
        return displayError("empty JSON");
    }

    Poco::UInt64 userID(0);
    error err = User::UserID(json, &userID);
    if (err != noError) {
        return displayError(err);
    }

    if (!userID) {
        return displayError("missing user ID in JSON");
    }

    User *user = new User();

    err = db()->LoadUserByID(userID, user);
    if (err != noError) {
        delete user;
        return displayError(err);
    }

    err = user->LoadUserAndRelatedDataFromJSONString(json, true);
    if (err != noError) {
        delete user;
        return displayError(err);
    }

    err = db()->SetCurrentAPIToken(user->APIToken(), user->ID());
    if (err != noError) {
        delete user;
        return displayError(err);
    }

    setUser(user, true);

    updateUI(UIElements::Reset());

    err = save();
    if (err != noError) {
        return displayError(err);
    }

    // Fetch OBM experiments..
    err = pullObmExperiments();
    if (err != noError) {
        logger().error("Error pulling OBM experiments: " + err);
    }

    // ..and run the OBM experiments
    err = runObmExperiments();
    if (err != noError) {
        logger().error("Error running OBM experiments: " + err);
    }

    return noError;
}

error Context::Logout() {
    try {
        if (!user_) {
            logger().warning("User is logged out, cannot logout again");
            return noError;
        }

        error err = db()->ClearCurrentAPIToken();
        if (err != noError) {
            return displayError(err);
        }

        logger().debug("setUser from Logout");
        overlay_visible_ = false;
        setUser(nullptr);

        UI()->DisplayApp();

        Shutdown();
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::ClearCache() {
    try {
        error err = noError;

        err = db()->ResetWindow();

        if (err != noError) {
            return displayError(err);
        }

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("User is logged out, cannot clear cache");
                return noError;
            }
            err = db()->DeleteUser(user_, true);
        }

        if (err != noError) {
            return displayError(err);
        }

        return Logout();
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

TimeEntry *Context::Start(
    const std::string description,
    const std::string duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string project_guid,
    const std::string tags,
    const bool prevent_on_app) {

    // Do not even allow to add new time entries,
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        displayError(kUnsupportedAppError);
        return nullptr;
    }

    // Discard Start if WS missing error is present
    if (overlay_visible_) {
        return nullptr;
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot start tracking, user logged out");
            return nullptr;
        }

        Poco::UInt64 tid(task_id);

        // Check if there's a default PID set
        Poco::UInt64 pid(project_id);
        if (!pid && project_guid.empty()) {
            pid = user_->DefaultPID();
            // Check if there's a default TID set
            tid = user_->DefaultTID();
        }

        te = user_->Start(description,
                          duration,
                          tid,
                          pid,
                          project_guid,
                          tags);
    }

    error err = save();
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    if ("production" == environment_) {
        analytics_.TrackAutocompleteUsage(db_->AnalyticsClientID(),
                                          task_id || project_id);
    }

    OpenTimeEntryList();

    if (!prevent_on_app && settings_.focus_on_shortcut) {
        // Show app
        UI()->DisplayApp();
    }

    if (te && settings_.open_editor_on_shortcut) {
        if (!prevent_on_app && !settings_.focus_on_shortcut) {
            // Show app
            UI()->DisplayApp();
        }
        // Open time entry in editor
        OpenTimeEntryEditor(te->GUID(), true, "");
    }

    return te;
}

void Context::OpenTimeEntryEditor(
    const std::string GUID,
    const bool edit_running_entry,
    const std::string focused_field_name) {
    if (!edit_running_entry && GUID.empty()) {
        logger().error("Cannot edit time entry without a GUID");
        return;
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot edit time entry, user logged out");
            return;
        }

        if (edit_running_entry) {
            te = user_->RunningTimeEntry();
        } else {
            te = user_->related.TimeEntryByGUID(GUID);
        }
    }

    if (!te) {
        logger().warning("Time entry not found for edit " + GUID);
        return;
    }

    UIElements render;
    render.open_time_entry_editor = true;
    render.display_time_entry_editor = true;
    render.time_entry_editor_guid = te->GUID();
    render.time_entry_editor_field = focused_field_name;

    // If user is already editing the time entry, toggle the editor
    // instead of doing nothing
    if (time_entry_editor_guid_ == te->GUID()) {
        render.open_time_entry_editor = false;
        render.display_time_entry_editor = false;
        render.time_entry_editor_guid = "";
        render.time_entry_editor_field = "";

        render.open_time_entry_list = true;
        render.display_time_entries = true;
    }

    updateUI(render);
}

TimeEntry *Context::ContinueLatest(const bool prevent_on_app) {
    // Do not even allow to continue entries,
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        displayError(kUnsupportedAppError);
        return nullptr;
    }

    // Discard Start if WS missing error is present
    if (overlay_visible_) {
        return nullptr;
    }

    TimeEntry *result = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot continue tracking, user logged out");
            return nullptr;
        }

        TimeEntry *latest = user_->related.LatestTimeEntry();

        if (!latest) {
            return nullptr;
        }

        result = user_->Continue(
            latest->GUID(),
            settings_.manual_mode);
    }



    error err = save();
    if (noError != err) {
        displayError(err);
        return nullptr;
    }

    if (settings_.manual_mode && result) {
        UIElements render;
        render.open_time_entry_editor = true;
        render.display_time_entry_editor = true;
        render.time_entry_editor_guid = result->GUID();
        updateUI(render);
    }

    if (!prevent_on_app && settings_.focus_on_shortcut) {
        // Show app
        UI()->DisplayApp();
    }

    if (result && settings_.open_editor_on_shortcut) {
        if (!prevent_on_app && !settings_.focus_on_shortcut) {
            // Show app
            UI()->DisplayApp();
        }
        // Open time entry in editor
        OpenTimeEntryEditor(result->GUID(), true, "");
    }

    return result;
}

TimeEntry *Context::Continue(
    const std::string GUID) {

    // Do not even allow to continue entries,
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        displayError(kUnsupportedAppError);
        return nullptr;
    }

    // Discard Start if WS missing error is present
    if (overlay_visible_) {
        return nullptr;
    }

    if (GUID.empty()) {
        displayError("Missing GUID");
        return nullptr;
    }

    TimeEntry *result = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot continue time entry, user logged out");
            return nullptr;
        }

        result = user_->Continue(
            GUID,
            settings_.manual_mode);
    }

    error err = save();
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    if (settings_.manual_mode && result) {
        UIElements render;
        render.open_time_entry_editor = true;
        render.display_time_entry_editor = true;
        render.time_entry_editor_guid = result->GUID();
        updateUI(render);
    } else {
        OpenTimeEntryList();
    }

    return result;
}

error Context::DeleteTimeEntryByGUID(const std::string GUID) {
    // Do not even allow to delete time entries,
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        return displayError(kUnsupportedAppError);
    }

    if (GUID.empty()) {
        return displayError("Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot delete time entry, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (te->DeletedAt()) {
            return displayError(kCannotDeleteDeletedTimeEntry);
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    if (te->IsTracking()) {
        error err = Stop(false);
        if (err != noError) {
            return displayError(err);
        }
    }
    te->ClearValidationError();
    te->Delete();
    return displayError(save());
}

error Context::SetTimeEntryDuration(
    const std::string GUID,
    const std::string duration) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }

    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger().warning("Cannot set duration, user logged out");
        return noError;
    }
    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return noError;
    }

    if (isTimeEntryLocked(te)) {
        return logAndDisplayUserTriedEditingLockedEntry();
    }

    te->SetDurationUserInput(duration);
    return displayError(save());
}

error Context::SetTimeEntryProject(
    const std::string GUID,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string project_guid) {
    try {
        if (GUID.empty()) {
            return displayError("Missing GUID");
        }

        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot set project, user logged out");
            return noError;
        }

        TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }

        Project *p = nullptr;
        if (project_id) {
            p = user_->related.ProjectByID(project_id);
        }
        if (!project_guid.empty()) {
            p = user_->related.ProjectByGUID(project_guid);
        }

        if (p && !canChangeProjectTo(te, p)) {
            return displayError(error(
                "Cannot change project: would end up with locked time entry"));
        }

        if (p) {
            // If user re-assigns project, don't mess with the billable
            // flag any more. (User selected billable project, unchecked
            // billable, // then selected the same project again).
            if (p->ID() != te->PID()) {
                te->SetBillable(p->Billable());
            }
            te->SetWID(p->WID());
        }
        te->SetTID(task_id);
        te->SetPID(project_id);
        te->SetProjectGUID(project_guid);

        if (te->Dirty()) {
            te->ClearValidationError();
            te->SetUIModified();
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return displayError(save());
}

error Context::SetTimeEntryDate(
    const std::string GUID,
    const Poco::Int64 unix_timestamp) {

    if (GUID.empty()) {
        return displayError("Missing GUID");
    }

    TimeEntry *te;
    Poco::LocalDateTime dt;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot change date, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return displayError(error("Cannot change locked time entry."));
        }

        Poco::LocalDateTime date_part(
            Poco::Timestamp::fromEpochTime(unix_timestamp));

        Poco::LocalDateTime time_part(
            Poco::Timestamp::fromEpochTime(te->Start()));

        dt = Poco::LocalDateTime(
            date_part.year(), date_part.month(), date_part.day(),
            time_part.hour(), time_part.minute(), time_part.second());

        if (!canChangeStartTimeTo(te, dt.timestamp().epochTime())) {
            return displayError(
                error(
                    "Failed to change time entry date: workspace is locked."));
        }
    }

    std::string s = Poco::DateTimeFormatter::format(
        dt,
        Poco::DateTimeFormat::ISO8601_FORMAT);

    te->SetStartUserInput(s, false);

    return displayError(save());
}


error Context::SetTimeEntryStart(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }
    Poco::LocalDateTime now;

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot change start time, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    Poco::LocalDateTime local(Poco::Timestamp::fromEpochTime(te->Start()));

    int hours(0), minutes(0);
    if (!toggl::Formatter::ParseTimeInput(value, &hours, &minutes)) {
        return error("invalid time format");
    }

    Poco::LocalDateTime dt(
        local.year(), local.month(), local.day(),
        hours, minutes, local.second());

    // check if time is in future and subtrack 1 day if needed
    if (dt.utcTime() > now.utcTime()) {
        Poco::LocalDateTime new_date =
            dt - Poco::Timespan(1 * Poco::Timespan::DAYS);
        dt = new_date;
    }

    std::string s = Poco::DateTimeFormatter::format(
        dt, Poco::DateTimeFormat::ISO8601_FORMAT);

    te->SetStartUserInput(s, GetKeepEndTimeFixed());

    return displayError(save());
}

error Context::SetTimeEntryStop(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot change stop time, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    Poco::LocalDateTime stop(
        Poco::Timestamp::fromEpochTime(te->Stop()));

    int hours(0), minutes(0);
    if (!toggl::Formatter::ParseTimeInput(value, &hours, &minutes)) {
        return error("invalid time format");
    }

// By default, keep end date, only change hour && minute
    Poco::LocalDateTime new_stop(
        stop.tzd(),
        stop.year(), stop.month(), stop.day(),
        hours, minutes, stop.second(), 0, 0);

// If end time is on same date as start,
// but is not less than start by hour & minute, then
// assume that the end must be on same date as start.
    Poco::LocalDateTime start(
        Poco::Timestamp::fromEpochTime(te->Start()));
    if (new_stop.day() != start.day()) {
        if (new_stop.hour() >= start.hour()) {
            new_stop = Poco::LocalDateTime(
                start.tzd(),
                start.year(), start.month(), start.day(),
                hours, minutes, stop.second(), 0, 0);
        }
    }

    std::string s = Poco::DateTimeFormatter::format(
        new_stop, Poco::DateTimeFormat::ISO8601_FORMAT);

    te->SetStopUserInput(s);

    return displayError(save());
}

error Context::SetTimeEntryTags(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot set tags, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    te->SetTags(value);

    if (te->Dirty()) {
        te->ClearValidationError();
        te->SetUIModified();
    }

    return displayError(save());
}

error Context::SetTimeEntryBillable(
    const std::string GUID,
    const bool value) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot set billable, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    te->SetBillable(value);

    if (te->Dirty()) {
        te->ClearValidationError();
        te->SetUIModified();
    }

    return displayError(save());
}

error Context::SetTimeEntryDescription(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot set description, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    te->SetDescription(value);

    if (te->Dirty()) {
        te->ClearValidationError();
        te->SetUIModified();
    }

    return displayError(save());
}

error Context::Stop(const bool prevent_on_app) {
    std::vector<TimeEntry *> stopped;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot stop tracking, user logged out");
            return noError;
        }
        user_->Stop(&stopped);

        resetLastTrackingReminderTime();
    }

    if (stopped.empty()) {
        logger().warning("No time entry was found to stop");
        return noError;
    }

    if (!prevent_on_app && settings_.focus_on_shortcut) {
        UI()->DisplayApp();
    }

    error err = save(true);
    if (err != noError) {
        return displayError(err);
    }

    OpenTimeEntryList();

    return noError;
}

error Context::DiscardTimeAt(
    const std::string guid,
    const Poco::Int64 at,
    const bool split_into_new_entry) {

    // Reset reminder count when doing idle actions
    resetLastTrackingReminderTime();

    // Tracking action
    if ("production" == environment_) {
        std::stringstream ss;
        if (split_into_new_entry) {
            ss << "idle-as-new-entry";
        } else {
            ss << "discard-and-stop";
        }

        analytics_.TrackIdleDetectionClick(db_->AnalyticsClientID(),
                                           ss.str());
    }

    TimeEntry *split = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot stop time entry, user logged out");
            return noError;
        }

        split = user_->DiscardTimeAt(guid, at, split_into_new_entry);
    }

    error err = save();
    if (err != noError) {
        return displayError(err);
    }

    if (split_into_new_entry && split) {
        UIElements render;
        render.open_time_entry_editor = true;
        render.display_time_entry_editor = true;
        render.time_entry_editor_guid = split->GUID();
        render.time_entry_editor_field = "";
        updateUI(render);
    }

    return noError;
}

TimeEntry *Context::DiscardTimeAndContinue(
    const std::string guid,
    const Poco::Int64 at) {

    // Reset reminder count when doing idle actions
    last_tracking_reminder_time_ = time(0);

    // Tracking action
    if ("production" == environment_) {
        analytics_.TrackIdleDetectionClick(db_->AnalyticsClientID(),
                                           "discard-and-continue");
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot stop time entry, user logged out");
            return nullptr;
        }
        user_->DiscardTimeAt(guid, at, false);
    }

    error err = save();
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    return Continue(guid);
}

TimeEntry *Context::RunningTimeEntry() {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger().warning("Cannot fetch time entry, user logged out");
        return nullptr;
    }
    return user_->RunningTimeEntry();
}

error Context::ToggleTimelineRecording(const bool record_timeline) {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger().warning("Cannot toggle timeline, user logged out");
        return noError;
    }
    try {
        user_->SetRecordTimeline(record_timeline);

        error err = save();
        if (err != noError) {
            return displayError(err);
        }

        UIElements render;
        render.display_settings = true;
        updateUI(render);

        TimelineUpdateServerSettings();

        if (user_->RecordTimeline()) {
            switchTimelineOn();
        } else {
            switchTimelineOff();
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::SetUpdateChannel(const std::string channel) {
    error err = db()->SaveUpdateChannel(channel);
    if (err != noError) {
        return displayError(err);
    }
    fetchUpdates();
    return noError;
}

void Context::SearchHelpArticles(
    const std::string keywords) {
    UI()->DisplayHelpArticles(help_database_.GetArticles(keywords));
}

error Context::SetDefaultProject(
    const Poco::UInt64 pid,
    const Poco::UInt64 tid) {
    try {
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("Cannot set default PID, user logged out");
                return noError;
            }

            Task *t = nullptr;
            if (tid) {
                t = user_->related.TaskByID(tid);
            }
            if (tid && !t) {
                return displayError("task not found");
            }

            Project *p = nullptr;
            if (pid) {
                p = user_->related.ProjectByID(pid);
            }
            if (pid && !p) {
                return displayError("project not found");
            }
            if (!p && t && t->PID()) {
                p = user_->related.ProjectByID(t->PID());
            }

            if (p && t && p->ID() != t->PID()) {
                return displayError("task does not belong to project");
            }

            if (p) {
                user_->SetDefaultPID(p->ID());
            } else {
                user_->SetDefaultPID(0);
            }

            if (t) {
                user_->SetDefaultTID(t->ID());
            } else {
                user_->SetDefaultTID(0);
            }
        }
        return displayError(save());
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::DefaultProjectName(std::string *name) {
    try {
        poco_check_ptr(name);
        Project *p = nullptr;
        Task *t = nullptr;
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("Cannot get default PID, user logged out");
                return noError;
            }
            if (user_->DefaultPID()) {
                p = user_->related.ProjectByID(user_->DefaultPID());
            }
            if (user_->DefaultTID()) {
                t = user_->related.TaskByID(user_->DefaultTID());
            }
        }
        *name = Formatter::JoinTaskName(t, p);
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::DefaultPID(Poco::UInt64 *result) {
    try {
        poco_check_ptr(result);
        *result = 0;
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("Cannot get default PID, user logged out");
                return noError;
            }
            *result = user_->DefaultPID();
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::DefaultTID(Poco::UInt64 *result) {
    try {
        poco_check_ptr(result);
        *result = 0;
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("Cannot get default PID, user logged out");
                return noError;
            }
            *result = user_->DefaultTID();
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::AddAutotrackerRule(
    const std::string term,
    const Poco::UInt64 pid,
    const Poco::UInt64 tid,
    Poco::Int64 *rule_id) {

    poco_check_ptr(rule_id);
    *rule_id = 0;

    if (term.empty()) {
        return displayError("missing term");
    }
    if (!pid && !tid) {
        return displayError("missing project and task");
    }

    std::string lowercase = Poco::UTF8::toLower(term);

    AutotrackerRule *rule = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("cannot add autotracker rule, user logged out");
            return noError;
        }
        if (user_->related.HasMatchingAutotrackerRule(lowercase)) {
            // avoid duplicates
            return displayError(kErrorRuleAlreadyExists);
        }

        Task *t = nullptr;
        if (tid) {
            t = user_->related.TaskByID(tid);
        }
        if (tid && !t) {
            return displayError("task not found");
        }

        Project *p = nullptr;
        if (pid) {
            p = user_->related.ProjectByID(pid);
        }
        if (pid && !p) {
            return displayError("project not found");
        }
        if (t && t->PID() && !p) {
            p = user_->related.ProjectByID(t->PID());
        }

        if (p && t && p->ID() != t->PID()) {
            return displayError("task does not belong to project");
        }

        rule = new AutotrackerRule();
        rule->SetTerm(lowercase);
        if (t) {
            rule->SetTID(t->ID());
        }
        if (p) {
            rule->SetPID(p->ID());
        }
        rule->SetUID(user_->ID());
        user_->related.AutotrackerRules.push_back(rule);
    }

    error err = save();
    if (noError != err) {
        return displayError(err);
    }

    if (rule) {
        *rule_id = rule->LocalID();
    }

    return noError;
}

error Context::DeleteAutotrackerRule(
    const Poco::Int64 id) {

    if (!id) {
        return displayError("cannot delete rule without an ID");
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("cannot delete rule, user is logged out");
            return noError;
        }

        error err = user_->related.DeleteAutotrackerRule(id);
        if (err != noError) {
            return displayError(err);
        }
    }

    return displayError(save());
}

Project *Context::CreateProject(
    const Poco::UInt64 workspace_id,
    const Poco::UInt64 client_id,
    const std::string client_guid,
    const std::string project_name,
    const bool is_private,
    const std::string project_color) {

    if (!workspace_id) {
        displayError(kPleaseSelectAWorkspace);
        return nullptr;
    }

    std::string trimmed_project_name("");
    error err = db_->Trim(project_name, &trimmed_project_name);
    if (err != noError) {
        displayError(err);
        return nullptr;
    }
    if (trimmed_project_name.empty()) {
        displayError(kProjectNameMustNotBeEmpty);
        return nullptr;
    }

    Project *result = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot add project, user logged out");
            return nullptr;
        }
        for (std::vector<Project *>::iterator it =
            user_->related.Projects.begin();
                it != user_->related.Projects.end(); it++) {
            Project *p = *it;

            bool clientIsSame = false;
            if (!client_guid.empty()) {
                if (!p->ClientGUID().empty()) {
                    clientIsSame = client_guid == p->ClientGUID();
                }
            } else {
                clientIsSame = client_id == p->CID();
            }

            if (clientIsSame && p->Name() == trimmed_project_name) {
                displayError(kProjectNameAlreadyExists);
                return nullptr;
            }
        }
        // Check if projects are billable by default
        Workspace *ws = nullptr;
        bool billable = false;

        ws = user_->related.WorkspaceByID(workspace_id);
        if (ws) {
            billable = ws->ProjectsBillableByDefault();
        }

        std::string client_name("");
        Poco::UInt64 cid(0);
        Client *c = nullptr;

        // Search by client ID
        if (client_id != 0) {
            c = user_->related.ClientByID(client_id);
        } else {
            // Search by Client GUID (when Client is not synced to server yet)
            c = user_->related.ClientByGUID(client_guid);
        }

        if (c) {
            client_name = c->Name();
            cid = c->ID();
        }

        result = user_->CreateProject(
            workspace_id,
            cid,
            client_guid,
            client_name,
            trimmed_project_name,
            is_private,
            project_color,
            billable);
    }

    err = save();
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    return result;
}

error Context::AddObmAction(
    const Poco::UInt64 experiment_id,
    const std::string key,
    const std::string value) {
    // Check input
    if (!experiment_id) {
        return error("missing experiment_id");
    }
    std::string trimmed_key("");
    error err = db_->Trim(key, &trimmed_key);
    if (err != noError) {
        return displayError(err);
    }
    if (trimmed_key.empty()) {
        return error("missing key");
    }
    std::string trimmed_value("");
    err = db_->Trim(value, &trimmed_value);
    if (err != noError) {
        return displayError(err);
    }
    if (trimmed_value.empty()) {
        return error("missing value");
    }
    // Add OBM action and save
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot create a OBM action, user logged out");
            return noError;
        }
        ObmAction *action = new ObmAction();
        action->SetExperimentID(experiment_id);
        action->SetUID(user_->ID());
        action->SetKey(trimmed_key);
        action->SetValue(trimmed_value);
        user_->related.ObmActions.push_back(action);
    }
    return displayError(save());
}

Client *Context::CreateClient(
    const Poco::UInt64 workspace_id,
    const std::string client_name) {

    if (!workspace_id) {
        displayError(kPleaseSelectAWorkspace);
        return nullptr;
    }

    std::string trimmed_client_name("");
    error err = db_->Trim(client_name, &trimmed_client_name);
    if (err != noError) {
        displayError(err);
        return nullptr;
    }
    if (trimmed_client_name.empty()) {
        displayError(kClientNameMustNotBeEmpty);
        return nullptr;
    }

    Client *result = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot create a client, user logged out");
            return nullptr;
        }
        for (std::vector<Client *>::iterator it =
            user_->related.Clients.begin();
                it != user_->related.Clients.end(); it++) {
            Client *c = *it;
            if (c->WID() == workspace_id && c->Name() == trimmed_client_name) {
                displayError(kClientNameAlreadyExists);
                return nullptr;
            }
        }
        result = user_->CreateClient(workspace_id, trimmed_client_name);
    }

    err = save();
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    return result;
}

void Context::SetSleep() {

    // Stop running entry if need
    const bool isHandled = handleStopRunningEntry();

    // Set Sleep as usual
    if (!isHandled) {
        logger().debug("SetSleep");
        idle_.SetSleep();
    }
}

error Context::OpenReportsInBrowser() {
    // Do not even allow to open reports
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        return displayError(kUnsupportedAppError);
    }

    std::string apitoken("");

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return displayError("You must log in to view reports");
        }
        apitoken = user_->APIToken();
    }

    // Not implemented in v9 as of 12.05.2017
    HTTPSRequest req;
    req.host = urls::API();
    req.relative_url = "/api/v8/desktop_login_tokens";
    req.payload = "{}";
    req.basic_auth_username = apitoken;
    req.basic_auth_password = "api_token";

    TogglClient client(UI());
    HTTPSResponse resp = client.Post(req);
    if (resp.err != noError) {
        return displayError(resp.err);
    }
    if (resp.body.empty()) {
        return displayError("Unexpected empty response from API");
    }

    std::string login_token("");
    error err = User::LoginToken(resp.body, &login_token);
    if (err != noError) {
        return displayError(err);
    }

    if (login_token.empty()) {
        return displayError("Could not extract login token from JSON");
    }

    // Not implemented in v9 as of 12.05.2017
    std::stringstream ss;
    ss  << urls::API() << "/api/v8/desktop_login"
        << "?login_token=" << login_token
        << "&goto=reports";
    UI()->DisplayURL(ss.str());

    return noError;
}

error Context::offerBetaChannel(bool *did_offer) {
    try {
        poco_check_ptr(did_offer);

        *did_offer = false;

        if (update_check_disabled_) {
            // if update check is disabled, then
            // the channel selection won't be ever
            // used anyway
            return noError;
        }

        if (settings_.has_seen_beta_offering) {
            return noError;
        }

        if (!UI()->CanDisplayPromotion()) {
            return noError;
        }

        std::string update_channel("");
        error err = db()->LoadUpdateChannel(&update_channel);
        if (err != noError) {
            return err;
        }

        if ("beta" == update_channel) {
            return noError;
        }

        UI()->DisplayPromotion(kPromotionJoinBetaChannel);

        err = db()->SetSettingsHasSeenBetaOffering(true);
        if (err != noError) {
            return err;
        }

        UIElements render;
        render.display_settings = true;
        updateUI(render);

        *did_offer = true;
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::runObmExperiments() {
    try {
        // Collect OBM experiments
        std::map<Poco::UInt64, ObmExperiment> experiments;
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("User logged out, cannot OBM experiment");
                return noError;
            }
            for (std::vector<ObmExperiment *>::const_iterator it =
                user_->related.ObmExperiments.begin();
                    it != user_->related.ObmExperiments.end();
                    it++) {
                ObmExperiment *model = *it;
                if (!model->DeletedAt()) {
                    experiments[model->Nr()] = *model;
                    model->SetHasSeen(true);
                }
            }
        }
        // Save the (seen/unseen) state
        error err = save();
        if (err != noError) {
            return err;
        }
        // Now pass the experiments on to UI
        for (std::map<Poco::UInt64, ObmExperiment>::const_iterator
                it = experiments.begin();
                it != experiments.end();
                it++) {
            ObmExperiment experiment = it->second;
            UI()->DisplayObmExperiment(
                experiment.Nr(),
                experiment.Included(),
                experiment.HasSeen());
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

void Context::SetWake() {
    logger().debug("SetWake");

    Poco::Timestamp::TimeDiff delay = 0;
    if (next_wake_at_ > 0) {
        delay = kRequestThrottleSeconds * kOneSecondInMicros;
    }

    next_wake_at_ = postpone(delay);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onWake);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_wake_at_);

    std::stringstream ss;
    ss << "Next wake at "
       << Formatter::Format8601(next_wake_at_);
    logger().debug(ss.str());
}

void Context::onWake(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_wake_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger().debug("onWake postponed");
        return;
    }
    logger().debug("onWake executing");

    try {
        Poco::LocalDateTime now;
        if (now.year() != last_time_entry_list_render_at_.year()
                || now.month() != last_time_entry_list_render_at_.month()
                || now.day() != last_time_entry_list_render_at_.day()) {
            UIElements render;
            render.display_time_entries = true;
            updateUI(render);
        }

        idle_.SetWake(user_);

        Sync();
    }
    catch (const Poco::Exception& exc) {
        logger().error(exc.displayText());
    }
    catch (const std::exception& ex) {
        logger().error(ex.what());
    }
    catch (const std::string& ex) {
        logger().error(ex);
    }
}

void Context::SetOnline() {
    logger().debug("SetOnline");

    // Schedule a sync, a but a bit later
    // For example, on Windows we're not yet online although
    // we're told we are. So wait a bit
    next_sync_at_ = postpone(2 * kRequestThrottleSeconds * kOneSecondInMicros);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onSync);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_sync_at_);

    std::stringstream ss;
    ss << "Next sync at "
       << Formatter::Format8601(next_sync_at_);
    logger().debug(ss.str());
}

void Context::osShutdown() {
    handleStopRunningEntry();
}

const bool Context::handleStopRunningEntry() {

    // Skip if this feature is not enable
    if (!settings_.stop_entry_on_shutdown_sleep) {
        return false;
    }

    // Stop running entry
    return Stop(false) == noError;
}

void Context::displayReminder() {
    if (!settings_.reminder) {
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return;
        }

        if (user_ && user_->RunningTimeEntry()) {
            return;
        }

        if (time(0) - last_tracking_reminder_time_
                < settings_.reminder_minutes * 60) {
            return;
        }
    }

    // Check if allowed to display reminder on this weekday
    Poco::LocalDateTime now;
    int wday = now.dayOfWeek();
    if (
        (Poco::DateTime::MONDAY == wday && !settings_.remind_mon) ||
        (Poco::DateTime::TUESDAY == wday && !settings_.remind_tue) ||
        (Poco::DateTime::WEDNESDAY == wday && !settings_.remind_wed) ||
        (Poco::DateTime::THURSDAY == wday && !settings_.remind_thu) ||
        (Poco::DateTime::FRIDAY == wday && !settings_.remind_fri) ||
        (Poco::DateTime::SATURDAY == wday && !settings_.remind_sat) ||
        (Poco::DateTime::SUNDAY == wday && !settings_.remind_sun)) {
        logger().debug("reminder is not enabled on this weekday");
        return;
    }

    // Check if allowed to display reminder at this time
    if (!settings_.remind_starts.empty()) {
        int h(0), m(0);
        if (toggl::Formatter::ParseTimeInput(settings_.remind_starts, &h, &m)) {
            Poco::LocalDateTime start(
                now.year(), now.month(), now.day(), h, m, now.second());
            if (now < start) {
                std::stringstream ss;
                ss << "Reminder - its too early for reminders"
                   << " [" << now.hour() << ":" << now.minute() << "]"
                   << " (allowed from " << h << ":" << m << ")";
                logger().debug(ss.str());
                return;
            }
        }
    }
    if (!settings_.remind_ends.empty()) {
        int h(0), m(0);
        if (toggl::Formatter::ParseTimeInput(settings_.remind_ends, &h, &m)) {
            Poco::LocalDateTime end(
                now.year(), now.month(), now.day(), h, m, now.second());
            if (now > end) {
                std::stringstream ss;
                ss << "Reminder - its too late for reminders"
                   << " [" << now.hour() << ":" << now.minute() << "]"
                   << " (allowed until " << h << ":" << m << ")";
                logger().debug(ss.str());
                return;
            }
        }
    }

    resetLastTrackingReminderTime();

    UI()->DisplayReminder();
}

void Context::resetLastTrackingReminderTime() {
    last_tracking_reminder_time_ = time(0);
}

void Context::displayPomodoro() {
    if (!settings_.pomodoro) {
        return;
    }

    Poco::UInt64 wid(0);

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return;
        }

        TimeEntry *current_te = user_->RunningTimeEntry();
        if (!current_te) {
            return;
        }
        if (pomodoro_break_entry_ != nullptr
                && !current_te->GUID().empty()
                && current_te->GUID().compare(
                    pomodoro_break_entry_->GUID()) == 0) {
            return;
        }

        if (current_te->DurOnly() && current_te->LastStartAt() != 0) {
            if (time(0) - current_te->LastStartAt()
                    < settings_.pomodoro_minutes * 60) {
                return;
            }
        } else {
            if (time(0) - current_te->Start()
                    < settings_.pomodoro_minutes * 60) {
                return;
            }
        }
        const Poco::Int64 pomodoroDuration = settings_.pomodoro_minutes * 60;
        wid = current_te->WID();
        Stop(true);
        current_te->SetDurationInSeconds(pomodoroDuration);
        current_te->SetStop(current_te->Start() + pomodoroDuration);
    }
    UI()->DisplayPomodoro(settings_.pomodoro_minutes);

    if (settings_.pomodoro_break) {
        //  Start a new task with the tag "pomodoro-break"
        pomodoro_break_entry_ = user_->Start("Pomodoro Break",  // description
                                             "",  // duration
                                             0,  // task_id
                                             0,  // project_id
                                             "",  // project_guid
                                             "pomodoro-break");  // tags

        // Set workspace id to same as the previous entry
        pomodoro_break_entry_->SetWID(wid);
    }
}

void Context::displayPomodoroBreak() {
    if (!settings_.pomodoro_break) {
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return;
        }

        TimeEntry *current_te = user_->RunningTimeEntry();

        if (!current_te) {
            return;
        }

        if (pomodoro_break_entry_ == nullptr
                || current_te->GUID().compare(
                    pomodoro_break_entry_->GUID()) != 0) {
            return;
        }

        if (time(0) - current_te->Start()
                < settings_.pomodoro_break_minutes * 60) {
            return;
        }
        const Poco::Int64 pomodoroDuration = settings_.pomodoro_break_minutes * 60;
        Stop(true);
        current_te->SetDurationInSeconds(pomodoroDuration);
        current_te->SetStop(current_te->Start() + pomodoroDuration);
    }
    pomodoro_break_entry_ = nullptr;

    UI()->DisplayPomodoroBreak(settings_.pomodoro_break_minutes);
}

error Context::StartAutotrackerEvent(const TimelineEvent event) {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        return noError;
    }

    if (!settings_.autotrack) {
        return noError;
    }

    // Notify user to track using autotracker rules:
    if (user_ && user_->RunningTimeEntry()) {
        return noError;
    }
    AutotrackerRule *rule = user_->related.FindAutotrackerRule(event);
    if (!rule) {
        return noError;
    }

    Project *p = nullptr;
    if (rule->PID()) {
        p = user_->related.ProjectByID(rule->PID());
    }
    if (rule->PID() && !p) {
        return error("autotracker project not found");
    }

    Task *t = nullptr;
    if (rule->TID()) {
        t = user_->related.TaskByID(rule->TID());
    }
    if (rule->TID() && !t) {
        return error("autotracker task not found");
    }

    if (!p && !t) {
        return error("no project or task specified in autotracker rule");
    }

    UI()->DisplayAutotrackerNotification(p, t);

    return noError;
}

error Context::CreateCompressedTimelineBatchForUpload(TimelineBatch *batch) {
    try {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("cannot create timeline batch, user logged out");
            return noError;
        }

        poco_check_ptr(batch);

        if (quit_) {
            return noError;
        }

        user_->CompressTimeline();
        error err = save(false);
        if (err != noError) {
            return displayError(err);
        }

        batch->SetEvents(user_->CompressedTimeline());
        batch->SetUserID(user_->ID());
        batch->SetAPIToken(user_->APIToken());
        batch->SetDesktopID(db_->DesktopID());
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::StartTimelineEvent(TimelineEvent *event) {
    try {
        poco_check_ptr(event);

        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return noError;
        }

        if (user_ && user_->RecordTimeline()) {
            event->SetUID(static_cast<unsigned int>(user_->ID()));
            user_->related.TimelineEvents.push_back(event);
            return displayError(save(false));
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::MarkTimelineBatchAsUploaded(
    const std::vector<TimelineEvent> &events) {
    try {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("cannot mark timeline events as uploaded, "
                             "user is already logged out");
            return noError;
        }
        user_->MarkTimelineBatchAsUploaded(events);
        return displayError(save(false));
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
}

error Context::SetPromotionResponse(
    const int64_t promotion_type,
    const int64_t promotion_response) {

    if (kPromotionJoinBetaChannel == promotion_type && promotion_response) {
        return SetUpdateChannel("beta");
    }

    return noError;
}

void Context::uiUpdaterActivity() {
    std::string running_time("");
    while (!ui_updater_.isStopped()) {
        // Sleep in increments for faster shutdown.
        for (unsigned int i = 0; i < 4*10; i++) {
            if (ui_updater_.isStopped()) {
                return;
            }
            Poco::Thread::sleep(250);
        }

        TimeEntry *te = nullptr;
        Poco::Int64 duration(0);
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                continue;
            }
            te = user_->RunningTimeEntry();
            if (!te) {
                continue;
            }
            duration = user_->related.TotalDurationForDate(te);
        }

        std::string date_duration =
            Formatter::FormatDurationForDateHeader(duration);

        if (running_time != date_duration) {
            UIElements render;
            render.display_time_entries = true;
            updateUI(render);
        }

        running_time = date_duration;
    }
}

void Context::checkReminders() {
    displayReminder();
    displayPomodoro();
    displayPomodoroBreak();
}

void Context::reminderActivity() {
    while (true) {
        // Sleep in increments for faster shutdown.
        for (int i = 0; i < 4; i++) {
            if (reminder_.isStopped()) {
                return;
            }
            Poco::Thread::sleep(250);
        }

        checkReminders();
    }
}

void Context::syncerActivity() {
    while (true) {
        // Sleep in increments for faster shutdown.
        for (int i = 0; i < 4; i++) {
            if (syncer_.isStopped()) {
                return;
            }
            Poco::Thread::sleep(250);
        }

        {
            Poco::Mutex::ScopedLock lock(syncer_m_);

            if (trigger_sync_) {
                TogglClient client(UI());

                error err = pullAllUserData(&client);
                if (err != noError) {
                    displayError(err);
                    return;
                }

                setOnline("Data pulled");

                err = pushChanges(&client, &trigger_sync_);
                if (err != noError) {
                    displayError(err);
                } else {
                    setOnline("Data pushed");
                }
                trigger_sync_ = false;

                // Push cached OBM action
                err = pushObmAction();
                if (err != noError) {
                    std::cout << "SYNC: sync-pushObm ERROR\n";
                    logger().error("Error pushing OBM action: " + err);
                }

                displayError(save(false));
            }

        }

        {
            Poco::Mutex::ScopedLock lock(syncer_m_);

            if (trigger_push_) {
                TogglClient client(UI());

                error err = pushChanges(&client, &trigger_sync_);
                if (err != noError) {
                    displayError(err);
                } else {
                    setOnline("Data pushed");
                }
                trigger_push_ = false;

                // Push cached OBM action
                err = pushObmAction();
                if (err != noError) {
                    std::cout << "SYNC: pushObm ERROR\n";
                    logger().error("Error pushing OBM action: " + err);
                }

                displayError(save(false));
            }
        }
    }
}

void Context::LoadMore() {
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_ || user_->HasLoadedMore()) {
            return;
        }
    }
    {
        Poco::Util::TimerTask::Ptr task =
            new Poco::Util::TimerTaskAdapter<Context>(*this,
                    &Context::onLoadMore);
        Poco::Mutex::ScopedLock lock(timer_m_);
        timer_.schedule(task, postpone(0));
    }
}

void Context::onLoadMore(Poco::Util::TimerTask& task) {
    bool needs_render = !user_->HasLoadedMore();
    std::string api_token;
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_ || user_->HasLoadedMore()) {
            return;
        }
        api_token = user_->APIToken();
    }

    if (api_token.empty()) {
        return logger().warning(
            "cannot load more time entries without API token");
    }

    try {
        std::stringstream ss;
        ss << "/api/v9/me/time_entries?since="
           << (Poco::Timestamp() - Poco::Timespan(60, 0, 0, 0, 0)).epochTime();

        std::stringstream l;
        l << "loading more: " << ss.str();
        logger().debug(l.str());

        TogglClient client(UI());
        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = client.Get(req);
        if (resp.err != noError) {
            logger().warning(resp.err);
            return;
        }

        std::string json = resp.body;

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_)
                return;
            error err = user_->LoadTimeEntriesFromJSONString(json);

            if (err != noError) {
                logger().error(err);
                return;
            }

            user_->ConfirmLoadedMore();

            // Removes load more button if nothing is to be loaded
            if (needs_render) {
                UIElements render;
                render.display_time_entries = true;
                updateUI(render);
            }
        }

        displayError(save(false));
    }
    catch (const Poco::Exception& exc) {
        logger().warning(exc.displayText());
    }
    catch (const std::exception& ex) {
        logger().warning(ex.what());
    }
    catch (const std::string& ex) {
        logger().warning(ex);
    }
}



void Context::SetLogPath(const std::string path) {
    Poco::AutoPtr<Poco::SimpleFileChannel> simpleFileChannel(
        new Poco::SimpleFileChannel);
    simpleFileChannel->setProperty(
        Poco::SimpleFileChannel::PROP_PATH, path);
    simpleFileChannel->setProperty(
        Poco::SimpleFileChannel::PROP_ROTATION, "1 M");
    simpleFileChannel->setProperty(
        Poco::SimpleFileChannel::PROP_FLUSH, "false");

    Poco::AutoPtr<Poco::FormattingChannel> formattingChannel(
        new Poco::FormattingChannel(
            new Poco::PatternFormatter(
                "%Y-%m-%d %H:%M:%S.%i [%P %I]:%s:%q:%t")));
    formattingChannel->setChannel(simpleFileChannel);

    Poco::Logger::get("").setChannel(formattingChannel);

    log_path_ = path;
}

Poco::Logger &Context::logger() const {
    return Poco::Logger::get("context");
}

error Context::pullAllUserData(
    TogglClient *toggl_client) {

    std::string api_token("");
    Poco::UInt64 since(0);
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("cannot pull user data when logged out");
            return noError;
        }
        api_token = user_->APIToken();
        if (user_->HasValidSinceDate()) {
            since = user_->Since();
        }
    }

    if (api_token.empty()) {
        return error("cannot pull user data without API token");
    }

    try {
        Poco::Stopwatch stopwatch;
        stopwatch.start();

        std::string user_data_json("");
        error err = me(
            toggl_client,
            api_token,
            "api_token",
            &user_data_json,
            since);
        if (err != noError) {
            return err;
        }

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                return error("cannot load user data when logged out");
            }
            TimeEntry *running_entry = user_->RunningTimeEntry();

            error err = user_->LoadUserAndRelatedDataFromJSONString(user_data_json, !since);

            if (err != noError) {
                return err;
            }
            overlay_visible_ = false;
            TimeEntry *new_running_entry = user_->RunningTimeEntry();

            // Reset reminder time when entry stopped by sync
            if (running_entry && !new_running_entry) {
                resetLastTrackingReminderTime();
            }
        }

        err = pullWorkspaces(toggl_client);
        if (err != noError) {
            return err;
        }

        pullWorkspacePreferences(toggl_client);

        pullUserPreferences(toggl_client);

        stopwatch.stop();
        std::stringstream ss;
        ss << "User with related data JSON fetched and parsed in "
           << stopwatch.elapsed() / 1000 << " ms";
        logger().debug(ss.str());
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Context::pushChanges(
    TogglClient *toggl_client,
    bool *had_something_to_push) {
    try {
        Poco::Stopwatch stopwatch;
        stopwatch.start();

        poco_check_ptr(had_something_to_push);

        *had_something_to_push = true;

        std::map<std::string, BaseModel *> models;

        std::vector<TimeEntry *> time_entries;
        std::vector<Project *> projects;
        std::vector<Client *> clients;

        std::string api_token("");

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("cannot push changes when logged out");
                return noError;
            }

            api_token = user_->APIToken();
            if (api_token.empty()) {
                return error("cannot push changes without API token");
            }

            collectPushableModels(
                user_->related.TimeEntries,
                &time_entries,
                &models);
            collectPushableModels(
                user_->related.Projects,
                &projects,
                &models);
            collectPushableModels(
                user_->related.Clients,
                &clients,
                &models);
            if (time_entries.empty()
                    && projects.empty()
                    && clients.empty()) {
                *had_something_to_push = false;
                return noError;
            }
        }

        std::stringstream ss;
        ss << "Sync success (";

        // Clients first as projects may depend on clients
        if (clients.size() > 0) {
            Poco::Stopwatch client_stopwatch;
            client_stopwatch.start();
            error err = pushClients(
                clients,
                api_token,
                *toggl_client);
            if (err != noError &&
                    err.find(kClientNameAlreadyExists) == std::string::npos) {
                return err;
            }
            client_stopwatch.stop();
            ss << clients.size() << " clients in "
               << client_stopwatch.elapsed() / 1000 << " ms";
        }

        // Projects second as time entries may depend on projects
        if (projects.size() > 0) {
            Poco::Stopwatch project_stopwatch;
            project_stopwatch.start();
            error err = pushProjects(
                projects,
                clients,
                api_token,
                *toggl_client);
            if (err != noError &&
                    err.find(kProjectNameAlready) == std::string::npos) {
                return err;
            }

            // Update project id on time entries if needed
            err = updateEntryProjects(
                projects,
                time_entries);
            if (err != noError) {
                return err;
            }
            project_stopwatch.stop();
            ss << " | " << projects.size() << " projects in "
               << project_stopwatch.elapsed() / 1000 << " ms";
        }

        // Time entries last to be sure clients and projects are synced
        if (time_entries.size() > 0) {
            Poco::Stopwatch entry_stopwatch;
            entry_stopwatch.start();
            error err = pushEntries(
                models,
                time_entries,
                api_token,
                *toggl_client);
            if (err != noError) {
                return err;
            }

            entry_stopwatch.stop();
            ss << " | " << time_entries.size() << " time entries in "
               << entry_stopwatch.elapsed() / 1000 << " ms";
        }

        stopwatch.stop();
        ss << ") Total = " << stopwatch.elapsed() / 1000 << " ms";
        logger().debug(ss.str());
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Context::pushClients(
    std::vector<Client *> clients,
    std::string api_token,
    TogglClient toggl_client) {
    std::string client_json("");
    error err = noError;
    for (std::vector<Client *>::const_iterator it =
        clients.begin();
            it != clients.end(); it++) {
        Json::Value clientJson = (*it)->SaveToJSON();

        Json::StyledWriter writer;
        client_json = writer.write(clientJson);

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = (*it)->ModelURL();
        req.payload = client_json;
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = toggl_client.Post(req);

        if (resp.err != noError) {
            // if we're able to solve the error
            if ((*it)->ResolveError(resp.body)) {
                displayError(save());
            }
            continue;
        }

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(resp.body, root)) {
            err = error("error parsing client POST response");
            continue;
        }

        (*it)->LoadFromJSON(root);
    }

    return err;
}

error Context::pushProjects(
    std::vector<Project *> projects,
    std::vector<Client *> clients,
    std::string api_token,
    TogglClient toggl_client) {
    error err = noError;
    std::string project_json("");
    for (std::vector<Project *>::const_iterator it =
        projects.begin();
            it != projects.end(); it++) {
        if (!(*it)->CID() && !(*it)->ClientGUID().empty()) {
            // Find client id
            for (std::vector<Client *>::const_iterator itc =
                clients.begin();
                    itc != clients.end(); itc++) {
                if ((*itc)->GUID().compare((*it)->ClientGUID()) == 0) {
                    (*it)->SetCID((*itc)->ID());
                    break;
                }
            }
        }

        Json::Value projectJson = (*it)->SaveToJSON();

        Json::StyledWriter writer;
        project_json = writer.write(projectJson);

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = (*it)->ModelURL();
        req.payload = project_json;
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = toggl_client.Post(req);

        if (resp.err != noError) {
            // if we're able to solve the error
            if ((*it)->ResolveError(resp.body)) {
                displayError(save());
            }
            continue;
        }

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(resp.body, root)) {
            err = error("error parsing project POST response");
            continue;
        }

        (*it)->LoadFromJSON(root);
    }

    return err;
}

error Context::updateEntryProjects(
    std::vector<Project *> projects,
    std::vector<TimeEntry *> time_entries) {
    for (std::vector<TimeEntry *>::const_iterator it =
        time_entries.begin();
            it != time_entries.end(); it++) {
        if (!(*it)->PID() && !(*it)->ProjectGUID().empty()) {
            // Find project id
            for (std::vector<Project *>::const_iterator itc =
                projects.begin();
                    itc != projects.end(); itc++) {
                if ((*itc)->GUID().compare((*it)->ProjectGUID()) == 0) {
                    (*it)->SetPID((*itc)->ID());
                    break;
                }
            }
        }
    }

    return noError;
}

error Context::pushEntries(
    std::map<std::string, BaseModel *> models,
    std::vector<TimeEntry *> time_entries,
    std::string api_token,
    TogglClient toggl_client) {

    std::string entry_json("");
    std::string error_message("");
    bool error_found = false;
    bool offline = false;

    for (std::vector<TimeEntry *>::const_iterator it =
        time_entries.begin();
            it != time_entries.end(); it++) {
        // Avoid trying to POST when we're offline
        if (offline) {
            // Mark the time entry as unsynced now
            (*it)->SetUnsynced();
            continue;
        }

        Json::Value entryJson = (*it)->SaveToJSON();

        Json::StyledWriter writer;
        entry_json = writer.write(entryJson);

        // std::cout << entry_json;

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = (*it)->ModelURL();
        req.payload = entry_json;
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp;

        if ((*it)->NeedsDELETE()) {
            req.payload = "";
            resp = toggl_client.Delete(req);
        } else if ((*it)->ID()) {
            resp = toggl_client.Put(req);
        } else {
            resp = toggl_client.Post(req);
        }

        if (resp.err != noError) {
            // if we're able to solve the error
            if ((*it)->ResolveError(resp.body)) {
                displayError(save());
            }

            // Not found on server. Probably deleted already.
            if ((*it)->isNotFound(resp.body)) {
                (*it)->MarkAsDeletedOnServer();
                continue;
            }
            error_found = true;
            error_message = resp.body;
            if (error_message == noError) {
                error_message = resp.err;
            }
            // Mark the time entry as unsynced now
            (*it)->SetUnsynced();

            offline = IsNetworkingError(resp.err);

            if (offline) {
                trigger_sync_ = false;
            }

            continue;
        }

        if ((*it)->NeedsDELETE()) {
            // Successfully deleted entry
            (*it)->MarkAsDeletedOnServer();
            continue;
        }

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(resp.body, root)) {
            return error("error parsing time entry POST response");
        }

        (*it)->LoadFromJSON(root);
    }

    if (error_found) {
        return error_message;
    }
    return noError;
}

error Context::pullObmExperiments() {
    try {
        if (HTTPSClient::Config.OBMExperimentNrs.empty()) {
            logger().debug("No OBM experiment enabled by UI");
            return noError;
        }

        logger().trace("Fetching OBM experiments from backend");

        std::string apitoken("");
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("Cannot fetch OBM experiments without user");
                return noError;
            }
            apitoken = user_->APIToken();
        }

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/me/experiments";
        req.basic_auth_username = apitoken;
        req.basic_auth_password = "api_token";

        TogglClient client(UI());
        HTTPSResponse resp = client.Get(req);
        if (resp.err != noError) {
            return resp.err;
        }

        Json::Value json;
        Json::Reader reader;
        if (!reader.parse(resp.body, json)) {
            return error("Error in OBM experiments response body");
        }

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("Cannot apply OBM experiments without user");
                return noError;
            }
            user_->LoadObmExperiments(json);
        }

        return noError;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Context::pushObmAction() {
    try {
        ObmAction *for_upload = nullptr;
        HTTPSRequest req;
        req.host = urls::API();
        req.basic_auth_password = "api_token";

        // Get next OBM action for upload
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("cannot push changes when logged out");
                return noError;
            }

            if (user_->related.ObmActions.empty()) {
                return noError;
            }

            req.basic_auth_username = user_->APIToken();
            if (req.basic_auth_username.empty()) {
                return error("cannot push OBM actions without API token");
            }

            // find action that has not been uploaded yet
            for (std::vector<ObmAction *>::iterator it =
                user_->related.ObmActions.begin();
                    it != user_->related.ObmActions.end();
                    it++) {
                ObmAction *model = *it;
                if (!model->IsMarkedAsDeletedOnServer()) {
                    for_upload = model;
                    break;
                }
            }

            if (!for_upload) {
                return noError;
            }

            Json::Value root = for_upload->SaveToJSON();
            req.relative_url = for_upload->ModelURL();
            req.payload = Json::StyledWriter().write(root);
        }

        logger().debug(req.payload);

        TogglClient toggl_client;
        HTTPSResponse resp = toggl_client.Post(req);
        if (resp.err != noError) {
            // backend responds 204 on success
            if (resp.status_code != 204) {
                return resp.err;
            }
        }

        // mark as deleted to prevent duplicate uploading
        // (and make sure all other actions are uploaded)
        for_upload->MarkAsDeletedOnServer();
        for_upload->Delete();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Context::me(
    TogglClient *toggl_client,
    const std::string email,
    const std::string password,
    std::string *user_data_json,
    const Poco::UInt64 since) {

    if (email.empty()) {
        return "Empty email or API token";
    }

    if (password.empty()) {
        return "Empty password";
    }

    try {
        poco_check_ptr(user_data_json);
        poco_check_ptr(toggl_client);

        std::stringstream ss;
        ss << "/api/v8/me"
           << "?app_name=" << TogglClient::Config.AppName
           << "&with_related_data=true";
        if (since) {
            ss << "&since=" << since;
        }

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = email;
        req.basic_auth_password = password;

        HTTPSResponse resp = toggl_client->Get(req);
        if (resp.err != noError) {
            return resp.err;
        }

        *user_data_json = resp.body;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

bool Context::isTimeEntryLocked(TimeEntry* te) {
    return isTimeLockedInWorkspace(te->Start(),
                                   user_->related.WorkspaceByID(te->WID()));
}

bool Context::canChangeStartTimeTo(TimeEntry* te, time_t t) {
    return !isTimeLockedInWorkspace(t, user_->related.WorkspaceByID(te->WID()));
}

bool Context::canChangeProjectTo(TimeEntry* te, Project* p) {
    return !isTimeLockedInWorkspace(te->Start(),
                                    user_->related.WorkspaceByID(p->WID()));
}

error Context::logAndDisplayUserTriedEditingLockedEntry() {
    logger().warning("User tried editing locked time entry");
    return displayError(error("Cannot change locked time entry"));
}

bool Context::isTimeLockedInWorkspace(time_t t, Workspace* ws) {
    if (!ws)
        return false;
    if (!ws->Business())
        return false;
    if (ws->Admin())
        return false;
    auto lockedTime = ws->LockedTime();
    if (lockedTime == 0)
        return false;
    return t < lockedTime;
}

error Context::pullWorkspaces(TogglClient* toggl_client) {
    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error("cannot pull user data without API token");
    }

    std::string json("");

    try {
        std::stringstream ss;
        ss << "/api/v9/me/workspaces";

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = toggl_client->Get(req);
        if (resp.err != noError) {
            if (resp.err.find(kForbiddenError) != std::string::npos) {
                // User has no workspaces
                return error(kMissingWS); // NOLINT
            }
            return resp.err;
        }

        json = resp.body;

        user_->LoadWorkspacesFromJSONString(json);

    }
    catch (const Poco::Exception& exc) {
        return exc.displayText();
    }
    catch (const std::exception& ex) {
        return ex.what();
    }
    catch (const std::string& ex) {
        return ex;
    }
    return noError;
}

error Context::pullWorkspacePreferences(TogglClient* toggl_client) {
    std::vector<Workspace*> workspaces;
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        logger().debug("user mutex lock success - c:pullWorkspacePreferences");

        user_->related.WorkspaceList(&workspaces);
    }
    for (std::vector<Workspace*>::const_iterator
            it = workspaces.begin();
            it != workspaces.end();
            it++) {
        Workspace* ws = *it;

        if (!ws->Business())
            continue;

        std::string json("");

        error err = pullWorkspacePreferences(toggl_client, ws, &json);
        if (err != noError) {
            return err;
        }

        if (json.empty())
            continue;

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) {
            return error("Failed to load workspace preferences");
        }

        ws->LoadSettingsFromJson(root);
    }

    return noError;
}

error Context::pullWorkspacePreferences(
    TogglClient* toggl_client,
    Workspace* workspace,
    std::string* json) {

    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error("cannot pull user data without API token");
    }

    try {
        std::stringstream ss;
        ss << "/api/v9/workspaces/"
           << workspace->ID()
           << "/preferences";

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = toggl_client->Get(req);
        if (resp.err != noError) {
            return resp.err;
        }

        *json = resp.body;
    }
    catch (const Poco::Exception& exc) {
        return exc.displayText();
    }
    catch (const std::exception& ex) {
        return ex.what();
    }
    catch (const std::string& ex) {
        return ex;
    }
    return noError;
}

error Context::pullUserPreferences(
    TogglClient* toggl_client) {
    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error("cannot pull user data without API token");
    }

    try {
        std::string json("");
        std::stringstream ss;
        ss << "/api/v9/me/preferences/desktop";

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = toggl_client->Get(req);
        if (resp.err != noError) {
            return resp.err;
        }

        json = resp.body;

        if (json.empty())
            return noError;

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) {
            return error("Failed to load user preferences");
        }

        if (user_->LoadUserPreferencesFromJSON(root)) {
            // Reload list if user preferences
            // have changed (collapse time entries)
            UIElements render;
            render.display_time_entries = true;
            updateUI(render);
        }

        // Show tos accept overlay
        if (root.isMember("ToSAcceptNeeded") && root["ToSAcceptNeeded"].asBool()) {
            overlay_visible_ = true;
            UI()->DisplayTosAccept();
        }
    }
    catch (const Poco::Exception& exc) {
        return exc.displayText();
    }
    catch (const std::exception& ex) {
        return ex.what();
    }
    catch (const std::string& ex) {
        return ex;
    }
    return noError;
}

error Context::signup(
    TogglClient *toggl_client,
    const std::string email,
    const std::string password,
    std::string *user_data_json,
    const uint64_t country_id) {

    if (email.empty()) {
        return "Empty email";
    }

    if (password.empty()) {
        return "Empty password";
    }

    try {
        poco_check_ptr(user_data_json);
        poco_check_ptr(toggl_client);

        Json::Value user;
        user["email"] = email;
        user["password"] = password;
        user["created_with"] = Formatter::EscapeJSONString(
            HTTPSClient::Config.UserAgent());
        user["tos_accepted"] = true;
        user["country_id"] = Json::UInt64(country_id);

        Json::Value ws;
        ws["initial_pricing_plan"] = 0;
        user["workspace"] = ws;

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/signup";
        req.payload = Json::StyledWriter().write(user);

        HTTPSResponse resp = toggl_client->Post(req);
        if (resp.err != noError) {
            return resp.err;
        }

        *user_data_json = resp.body;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Context::ToSAccept() {
    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error("cannot pull user data without API token");
    }

    TogglClient toggl_client(UI());
    try {
        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/me/accept_tos";
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = toggl_client.Post(req);
        if (resp.err != noError) {
            return displayError(resp.err);
        }
        overlay_visible_ = false;
        OpenTimeEntryList();
    } catch(const Poco::Exception& exc) {
        displayError(kCannotConnectError);
        return exc.displayText();
    } catch(const std::exception& ex) {
        displayError(kCannotConnectError);
        return ex.what();
    } catch(const std::string& ex) {
        displayError(kCannotConnectError);
        return ex;
    }
    return noError;
}

error Context::ToggleEntriesGroup(std::string name) {
    entry_groups[name] = !entry_groups[name];
    OpenTimeEntryList();
    return noError;
}

error Context::PullCountries() {
    try {
        TogglClient toggl_client(UI());

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/countries";
        HTTPSResponse resp = toggl_client.Get(req);
        if (resp.err != noError) {
            return resp.err;
        }
        Json::Value root;
        Json::Reader reader;

        if (!reader.parse(resp.body, root)) {
            return error("Error parsing countries response body");
        }

        std::vector<TogglCountryView> countries;

        for (int i = root.size() - 1; i >= 0; i--) {
            TogglCountryView *item = country_view_item_init(root[i]);
            countries.push_back(*item);
        }

        // update country selectbox
        UI()->DisplayCountries(&countries);

        //country_item_clear(first);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

template<typename T>
void Context::collectPushableModels(
    const std::vector<T *> list,
    std::vector<T *> *result,
    std::map<std::string, BaseModel *> *models) const {

    poco_check_ptr(result);

    for (typename std::vector<T *>::const_iterator it =
        list.begin();
            it != list.end();
            it++) {
        T *model = *it;
        if (!model->NeedsPush()) {
            continue;
        }
        user_->EnsureWID(model);
        model->EnsureGUID();
        result->push_back(model);
        if (models && !model->GUID().empty()) {
            (*models)[model->GUID()] = model;
        }
    }
}

void on_websocket_message(
    void *context,
    std::string json) {

    poco_check_ptr(context);

    if (json.empty()) {
        return;
    }

    Context *ctx = reinterpret_cast<Context *>(context);
    ctx->LoadUpdateFromJSONString(json);
}

void Context::TrackWindowSize(const Poco::Int64 width,
                              const Poco::Int64 height) {
    if ("production" == environment_) {
        analytics_.TrackWindowSize(db_->AnalyticsClientID(),
                                   shortOSName(),
                                   toggl::Rectangle(width, height));
    }
}

}  // namespace toggl
