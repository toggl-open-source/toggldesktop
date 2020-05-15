
// Copyright 2014 Toggl Desktop developers

// No exceptions should be thrown from this class.
// If pointers to models are returned from this
// class, the ownership does not change and you
// must not delete the pointers you got.

// All public methods should start with an uppercase name.
// All public methods should catch their exceptions.

#include "context.h"

#include <iostream>  // NOLINT

#include "model/autotracker.h"
#include "model/client.h"
#include "const.h"
#include "database/database.h"
#include "error.h"
#include "util/formatter.h"
#include "https_client.h"
#include "model/obm_action.h"
#include "model/project.h"
#include "util/random.h"
#include "model/settings.h"
#include "model/task.h"
#include "model/time_entry.h"
#include "timeline_uploader.h"
#include "urls.h"
#include "window_change_recorder.h"
#include "model/workspace.h"

#include <Poco/Crypto/OpenSSLInitializer.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Environment.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/FormattingChannel.h>
#include <Poco/Net/FilePartSource.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPStreamFactory.h>
#include <Poco/Net/HTTPSStreamFactory.h>
#include <Poco/Net/NetSSL.h>
#include <Poco/Net/StringPartSource.h>
#include <Poco/Path.h>
#include <Poco/PatternFormatter.h>
#include <Poco/SimpleFileChannel.h>
#include <Poco/Stopwatch.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/UTF8String.h>
#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/URIStreamOpener.h>
#include <Poco/Util/TimerTask.h>
#include <Poco/Util/TimerTaskAdapter.h>

#include <mutex> // NOLINT
#include <thread>

namespace toggl {

std::string Context::log_path_ = "";

Context::Context(const std::string &app_name, const std::string &app_version)
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
, environment_(APP_ENVIRONMENT)
, idle_(&ui_)
, last_sync_started_(0)
, sync_interval_seconds_(0)
, update_check_disabled_(UPDATE_CHECK_DISABLED)
, trigger_sync_(false)
, trigger_push_(false)
, quit_(false)
, ui_updater_(this, &Context::uiUpdaterActivity)
, reminder_(this, &Context::reminderActivity)
, syncer_(this, &Context::syncerActivityWrapper)
, update_path_("")
, overlay_visible_(false)
, last_message_id_("") {
    if (!Poco::URIStreamOpener::defaultOpener().supportsScheme("http")) {
        Poco::Net::HTTPStreamFactory::registerFactory();
    }
    if (!Poco::URIStreamOpener::defaultOpener().supportsScheme("https")) {
        Poco::Net::HTTPSStreamFactory::registerFactory();
    }

#ifndef TOGGL_PRODUCTION_BUILD
    urls::SetUseStagingAsBackend(true);
#endif

    Poco::ErrorHandler::set(&error_handler_);
    Poco::Net::initializeSSL();

    HTTPClient::Config.AppName = app_name;
    HTTPClient::Config.AppVersion = app_version;

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

    TogglClient::GetInstance().SetSyncStateMonitor(UI());
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
        logger.debug(exc.displayText());
    } catch(const std::exception& ex) {
        logger.debug(ex.what());
    } catch(const std::string & ex) {
        logger.debug(ex);
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
        logger.debug("StartEvents");

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (user_) {
                return displayError("Cannot start UI, user already logged in!");
            }
        }

        if (HTTPClient::Config.CACertPath().empty()) {
            return displayError("Missing CA cert bundle path!");
        }

        // Check that UI is wired up
        error err = UI()->VerifyCallbacks();
        if (err != noError) {
            logger.error(err);
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
        user->Since.Set(0);
        logger.debug("fullSyncOnAppStart");

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
            analytics_.TrackOSDetails(db_->AnalyticsClientID());
            fetchMessage(0);
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string & ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::save(const bool push_changes) {
    logger.debug("save");
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
            logger.debug("onPushChanges executing");

            // Always sync asyncronously with syncerActivity
            trigger_push_ = true;
            if (!syncer_.isRunning()) {
                syncer_.start();
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
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
    render.display_timeline = true;

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
    if (display_timeline) {
        ss << " display_timeline=" << display_timeline;
    }
    if (open_timeline) {
        ss << " open_timeline=" << open_timeline;
    }
    return ss.str();
}

void UIElements::ApplyChanges(
    const std::string &editor_guid,
    const std::vector<ModelChange> &changes) {

    time_entry_editor_guid = editor_guid;

    // Check what needs to be updated in UI
    for (std::vector<ModelChange>::const_iterator it =
        changes.begin();
            it != changes.end();
            ++it) {
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

        if (ch.ModelType() == kModelTimelineEvent && ch.ChangeType() == kChangeTypeInsert) {
            display_timeline = true;
        }
    }
}

void Context::OpenTimeEntryList() {
    logger.debug("OpenTimeEntryList");

    UIElements render;
    render.open_time_entry_list = true;
    render.display_time_entries = true;
    updateUI(render);
}

void Context::updateUI(const UIElements &what) {
    logger.debug("updateUI " + what.String());

    view::TimeEntry editor_time_entry_view;

    std::vector<view::Autocomplete> time_entry_autocompletes;
    std::vector<view::Autocomplete> minitimer_autocompletes;
    std::vector<view::Autocomplete> project_autocompletes;

    // For timeline UI view data
    std::vector<TimelineEvent> timeline;

    bool use_proxy(false);
    bool record_timeline(false);
    Poco::Int64 unsynced_item_count(0);
    Proxy proxy;

    view::TimeEntry running_entry_view;

    std::vector<view::TimeEntry> time_entry_views;
    std::vector<view::TimeEntry> timeline_views;

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
                        ++it) {
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
                    ++it) {
                Workspace *ws = *it;
                view::Generic view;
                view.GUID = ws->GUID();
                view.ID = ws->ID();
                view.WID = ws->ID();
                view.Name = ws->Name();
                view.WorkspaceName = ws->Name();
                view.Premium = ws->Premium();
                workspace_views.push_back(view);
            }
        }

        if (what.display_client_select && user_) {
            std::vector<Client *> models;
            user_->related.ClientList(&models);
            for (std::vector<Client *>::const_iterator it = models.begin();
                    it != models.end();
                    ++it) {
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
                        view.Premium = ws->Premium();
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
                    Formatter::FormatDurationForDateHeader(Formatter::AbsDuration(running_entry->Duration()));
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
            std::map<std::string, Poco::UInt64> group_header_id;
            std::map<std::string, std::vector<Poco::UInt64> > group_items;

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
                    std::string group_name = te->GroupHash();

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
                            group_view.DurationInSeconds = group_durations[view.GroupName];
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

            HTTPClient::Config.UseProxy = use_proxy;
            HTTPClient::Config.ProxySettings = proxy;
            HTTPClient::Config.AutodetectProxy = settings_.autodetect_proxy;
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
                        ++it) {
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

        if (what.display_timeline && user_) {
            // Get Timeline data
            Poco::LocalDateTime date(UI()->TimelineDateAt());
            timeline = user_->CompressedTimelineForUI(&date);

            // Get a sorted list of time entries
            std::vector<TimeEntry *> time_entries =
                user_->related.VisibleTimeEntries();
            std::sort(time_entries.begin(), time_entries.end(),
                      CompareByStart);

            // Collect the time entries into a list
            for (unsigned int i = 0; i < time_entries.size(); i++) {
                TimeEntry *te = time_entries[i];
                if (te->Duration() < 0) {
                    // Don't account running entries
                    continue;
                }

                Poco::LocalDateTime te_date(Poco::Timestamp::fromEpochTime(te->Start()));
                if (te_date.year() == UI()->TimelineDateAt().year()
                        && te_date.month() == UI()->TimelineDateAt().month()
                        && te_date.day() == UI()->TimelineDateAt().day()) {

                    view::TimeEntry view;
                    view.Fill(te);
                    view.GenerateRoundedTimes();
                    view.Duration = toggl::Formatter::FormatDuration(
                        view.DurationInSeconds,
                        Formatter::DurationFormat);
                    view.DateDuration = Formatter::FormatDurationForDateHeader(view.DurationInSeconds);
                    user_->related.ProjectLabelAndColorCode(
                        te,
                        &view);
                    timeline_views.push_back(view);
                }
            }
        }
    }

    // Render data
    if (what.display_time_entry_editor
            && !editor_time_entry_view.GUID.empty()) {
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

    if (what.display_timeline) {
        UI()->DisplayTimeline(what.open_timeline, timeline, timeline_views);
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

    // if `now` is only slighly smaller than `value` it's probably the same task and not postponed
    // hence perform comparison using epsilon = `kTimeComparisonEpsilonMicroSeconds`
    if (now + kTimeComparisonEpsilonMicroSeconds > value) {
        return false;
    }
    Poco::Timestamp::TimeDiff diff = value - now;
    if (diff > 2*throttleMicros) {
        logger.warning(
            "Cannot postpone task, its foo far in the future");
        return false;
    }
    return true;
}

error Context::displayError(const error &err) {
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
        error err = pullWorkspaces();
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
    int n = static_cast<int>(Random::next(kSyncIntervalRangeSeconds)) + kSyncIntervalRangeSeconds;
    logger.trace("Next autosync in ", n, " seconds");
    return n;
}

void Context::scheduleSync() {
    Poco::Int64 elapsed_seconds = Poco::Int64(time(nullptr)) - last_sync_started_;
    logger.debug("scheduleSync elapsed_seconds=", elapsed_seconds);

    if (elapsed_seconds < sync_interval_seconds_) {
        logger.trace("Last sync attempt less than ", sync_interval_seconds_, " seconds ago, chill");
        return;
    }

    Sync();
}

void Context::FullSync() {
    user_->Since.Set(0);
    Sync();
}

void Context::Sync() {
    logger.debug("Sync");

    if (!user_) {
        return;
    }

    overlay_visible_ = false;

    Poco::Int64 elapsed_seconds = Poco::Int64(time(nullptr)) - last_sync_started_;

    // 2 seconds backoff to avoid too many sync requests
    if (elapsed_seconds < kRequestThrottleSeconds) {
        return;
    }

    last_sync_started_ = time(nullptr);

    // Always sync asyncronously with syncerActivity
    trigger_sync_ = true;
    if (!syncer_.isRunning()) {
        syncer_.start();
    }
}

void Context::onTimeEntryAutocompletes(Poco::Util::TimerTask&) {  // NOLINT
    std::vector<view::Autocomplete> time_entry_autocompletes;
    if (user_) {
        user_->related.TimeEntryAutocompleteItems(&time_entry_autocompletes);
    }
    UI()->DisplayTimeEntryAutocomplete(&time_entry_autocompletes);
}

void Context::onMiniTimerAutocompletes(Poco::Util::TimerTask&) {  // NOLINT
    std::vector<view::Autocomplete> minitimer_autocompletes;
    if (user_) {
        user_->related.MinitimerAutocompleteItems(&minitimer_autocompletes);
    }
    UI()->DisplayMinitimerAutocomplete(&minitimer_autocompletes);
}

void Context::onProjectAutocompletes(Poco::Util::TimerTask&) {  // NOLINT
    std::vector<view::Autocomplete> project_autocompletes;
    if (user_) {
        user_->related.ProjectAutocompleteItems(&project_autocompletes);
    }
    UI()->DisplayProjectAutocomplete(&project_autocompletes);
}

void Context::setOnline(const std::string &reason) {
    logger.debug("setOnline, reason:", reason);

    if (quit_) {
        return;
    }

    UI()->DisplayOnlineState(kOnlineStateOnline);

    scheduleSync();
}

void Context::switchWebSocketOff() {
    logger.debug("switchWebSocketOff");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchWebSocketOff);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchWebSocketOff(Poco::Util::TimerTask&) {  // NOLINT
    logger.debug("onSwitchWebSocketOff");

    Poco::Mutex::ScopedLock lock(ws_client_m_);
    ws_client_.Shutdown();
}

error Context::LoadUpdateFromJSONString(const std::string &json) {
    logger.debug("LoadUpdateFromJSONString json=", json);

    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger.warning("User is logged out, cannot update");
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

    return displayError(save(false));
}

void Context::switchWebSocketOn() {
    logger.debug("switchWebSocketOn");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchWebSocketOn);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchWebSocketOn(Poco::Util::TimerTask&) {  // NOLINT
    logger.debug("onSwitchWebSocketOn");

    std::string apitoken("");

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (user_) {
            apitoken = user_->APIToken();
        }
    }

    if (apitoken.empty()) {
        logger.error("No API token, cannot switch Websocket on");
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(ws_client_m_);
        ws_client_.Start(this, apitoken, on_websocket_message);
    }
}

// Start/stop timeline recording on local machine
void Context::switchTimelineOff() {
    logger.debug("switchTimelineOff");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchTimelineOff);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchTimelineOff(Poco::Util::TimerTask&) {  // NOLINT
    logger.debug("onSwitchTimelineOff");

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            delete timeline_uploader_;
            timeline_uploader_ = nullptr;
        }
    }
}

void Context::switchTimelineOn() {
    logger.debug("switchTimelineOn");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchTimelineOn);

    if (quit_) {
        return;
    }

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchTimelineOn(Poco::Util::TimerTask&) {  // NOLINT
    logger.debug("onSwitchTimelineOn");

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
    logger.debug("fetchUpdates");

    next_fetch_updates_at_ =
        postpone(kRequestThrottleSeconds * kOneSecondInMicros);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onFetchUpdates);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_fetch_updates_at_);

    logger.debug("Next update fetch at ", Formatter::Format8601(next_fetch_updates_at_));
}

void Context::onFetchUpdates(Poco::Util::TimerTask&) {  // NOLINT
    if (isPostponed(next_fetch_updates_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger.debug("onFetchUpdates postponed");
        return;
    }

    executeUpdateCheck();
}

void Context::startPeriodicSync() {
    logger.trace("startPeriodicSync");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>
    (*this, &Context::onPeriodicSync);

    sync_interval_seconds_ = nextSyncIntervalSeconds();

    Poco::Timestamp next_periodic_sync_at_ =
        Poco::Timestamp() + (sync_interval_seconds_ * kOneSecondInMicros);
    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_periodic_sync_at_);

    logger.debug("Next periodic sync at ", Formatter::Format8601(next_periodic_sync_at_));
}

void Context::onPeriodicSync(Poco::Util::TimerTask&) {  // NOLINT
    logger.debug("onPeriodicSync");

    scheduleSync();

    startPeriodicSync();
}

void Context::startPeriodicUpdateCheck() {
    logger.debug("startPeriodicUpdateCheck");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>
    (*this, &Context::onPeriodicUpdateCheck);

    Poco::Int64 micros = kCheckUpdateIntervalSeconds *
                         Poco::Int64(kOneSecondInMicros);
    Poco::Timestamp next_periodic_check_at = Poco::Timestamp() + micros;
    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_periodic_check_at);

    logger.debug("Next periodic update check at ", Formatter::Format8601(next_periodic_check_at));
}

void Context::onPeriodicUpdateCheck(Poco::Util::TimerTask&) {  // NOLINT
    logger.debug("onPeriodicUpdateCheck");

    executeUpdateCheck();

    startPeriodicUpdateCheck();
}

void Context::startPeriodicInAppMessageCheck() {
    logger.debug("startPeriodicInAppMessageCheck");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>
    (*this, &Context::onPeriodicInAppMessageCheck);

    Poco::Int64 micros = kCheckInAppMessageIntervalSeconds *
                         Poco::Int64(kOneSecondInMicros);
    Poco::Timestamp next_periodic_check_at = Poco::Timestamp() + micros;
    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_periodic_check_at);

    logger.debug("Next periodic in-app message check at ", Formatter::Format8601(next_periodic_check_at));
}

void Context::onPeriodicInAppMessageCheck(Poco::Util::TimerTask&) {  // NOLINT
    logger.debug("onPeriodicInAppMessageChec");

    fetchMessage(1);
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
    logger.debug("executeUpdateCheck");

    displayError(downloadUpdate());
}

error Context::downloadUpdate() {
    try {
        if (update_check_disabled_) {
            return noError;
        }

        // To test updater in development, comment this block out:
        if ("production" != environment_) {
            logger.debug("Not in production, will not download updates");
            return noError;
        }

        // Load current update channel
        std::string update_channel("");
        error err = db()->LoadUpdateChannel(&update_channel);
        if (err != noError) {
            return err;
        }

        if (HTTPClient::Config.AppVersion.empty()) {
            return error("This version cannot check for updates. This has been probably already fixed. Please check https://toggl.com/toggl-desktop/ for a newer version.");
        }

        // Ask Toggl server if we have updates
        std::string url("");
        std::string version_number("");
        {
            HTTPRequest req;
            req.host = "https://toggl.github.io";
            req.relative_url = "/toggldesktop/assets/updates-link.txt";

            TogglClient client = TogglClient::GetInstance();
            HTTPResponse resp = client.silentGet(req);
            if (resp.err != noError) {
                return resp.err;
            }

            Poco::URI uri(resp.body);
            req.host = uri.getScheme() + "://" + uri.getHost();
            req.relative_url = uri.getPathEtc();
            resp = client.silentGet(req);
            if (resp.err != noError) {
                return resp.err;
            }

            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(resp.body, root)) {
                return error("Error parsing update check response body");
            }
            auto latestVersion = root[shortOSName()][update_channel];
            url = latestVersion[installerPlatform()].asString();
            auto versionNumberJsonToken = latestVersion["version"];
            if (versionNumberJsonToken.empty()) {
                return error("No versions found for OS " + shortOSName() + ", platform " + installerPlatform() + ", channel " + update_channel);
            }
            version_number = versionNumberJsonToken.asString();

            if (lessThanVersion(HTTPClient::Config.AppVersion, version_number)) {
                logger.debug("Found update ", version_number, " (", url, ")");
            } else {
                logger.debug("The app is up to date");
                if (UI()->CanDisplayUpdate()) {
                    UI()->DisplayUpdate("");
                }
                return noError;
            }
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
            logger.debug("Update is not compatible with this client,"
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
                logger.debug("File already exists: " + file);
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
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Context::fetchMessage(const bool periodic) {
    try {

        // Check if in-app messaging is supported and show
        if (!UI()->CanDisplayMessage()) {
            logger.debug("In-app messages not supported on this platform");
            return noError;
        }

        // To test in-app message fetch in development, comment this block out:
        if ("production" != environment_) {
            logger.debug("Not in production, will not fetch in-app messages");
            return noError;
        }

        // Load last showed message id
        Poco::Int64 old_id(0);
        error err = db()->GetMessageSeen(&old_id);
        if (err != noError) {
            return err;
        }

        if (HTTPClient::Config.AppVersion.empty()) {
            return error("AppVersion missing!");
        }

        // Fetch latest message
        std::string title("");
        std::string text("");
        std::string button("");
        std::string url("");
        std::string appversion("");
        {
            HTTPRequest req;
            if ("production" != environment_) {
                // testing location
                req.host = "https://indrekv.github.io";
                req.relative_url = "/message.json";
            } else {
                req.host = "https://raw.githubusercontent.com";
                req.relative_url = "/toggl-open-source/toggldesktop/master/releases/message.json";
            }

            TogglClient client = TogglClient::GetInstance();
            HTTPResponse resp = client.silentGet(req);
            if (resp.err != noError) {
                return resp.err;
            }

            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(resp.body, root)) {
                return error("Error parsing in-app message response body");
            }

            // check all required fields
            if (!root.isMember("id") ||
                    !root.isMember("from") ||
                    (root.isMember("appversion") && !root.isMember("type")) ||
                    !root.isMember("title") ||
                    !root.isMember("text") ||
                    !root.isMember("button") ||
                    !root.isMember("url-mac") ||
                    !root.isMember("url-win") ||
                    !root.isMember("url-linux")) {
                logger.debug("Required fields are missing in in-app message JSON");
                return noError;
            }

            auto messageID = root["id"].asInt64();
            auto type = root["type"].asInt64();
            appversion = root["appversion"].asString();

            // check if message id is bigger than the saved one
            if (old_id >= messageID) {
                return noError;
            }

            // check appversion and version compare type
            if (!appversion.empty()) {
                if (type == 0) {
                    // exactly same version as in message
                    if (appversion.compare(HTTPClient::Config.AppVersion) != 0) {
                        return noError;
                    }

                } else if (type == 1) {
                    // we need older version to show message
                    if (!lessThanVersion(HTTPClient::Config.AppVersion, appversion)) {
                        return noError;
                    }

                } else if (type == 2) {
                    // we need newer version to show message
                    if (lessThanVersion(HTTPClient::Config.AppVersion, appversion)) {
                        return noError;
                    }
                }
            }

            // check if message is active (current time is between from and to)
            Poco::LocalDateTime now;
            Poco::LocalDateTime from(
                Poco::Timestamp::fromEpochTime(root["from"].asInt64()));

            // message is not active yet
            if (from.utcTime() > now.utcTime()) {
                return noError;
            }

            if (root.isMember("to")) {
                Poco::LocalDateTime to(
                    Poco::Timestamp::fromEpochTime(root["to"].asInt64()));

                // message is already out of date
                if (now.utcTime() > to.utcTime()) {
                    return noError;
                }
            }

            // update message id in database
            err = db()->SetSettingsMessageSeen(messageID);
            if (err != noError) {
                return err;
            }

            title = root["title"].asString();
            text = root["text"].asString();
            button = root["button"].asString();
            url = root["url-" + shortOSName()].asString();

            last_message_id_ = root["id"].asString();
        }

        analytics_.TrackInAppMessage(db_->AnalyticsClientID(),
                                     last_message_id_,
                                     periodic);

        startPeriodicInAppMessageCheck();

        UI()->DisplayMessage(
            title,
            text,
            button,
            url);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}




const std::string Context::linuxPlatformName() {
    if (kDebianPackage) {
        return "linux_deb64";
    }
    return std::string("linux_tar.gz");
}

const std::string Context::windowsPlatformName() {
#if defined(_WIN64)
    return "windows64";
#elif defined(_WIN32)
    BOOL f64 = FALSE;
    if (IsWow64Process(GetCurrentProcess(), &f64) && f64)
        return "windows64";
    return "windows";
#else
    return "windows";
#endif
}

const std::string Context::installerPlatform() {
    std::stringstream ss;
    if (POCO_OS_LINUX == POCO_OS) {
        ss <<  linuxPlatformName();
    } else if (POCO_OS_WINDOWS_NT == POCO_OS) {
        ss << windowsPlatformName();
    } else {
        ss << "macos";
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

void Context::parseVersion(int result[4], const std::string& input) {
    std::istringstream parser(input);
    parser >> result[0];
    for (auto idx = 1; idx < 4; idx++) {
        parser.get(); //Skip period
        parser >> result[idx];
    }
}

bool Context::lessThanVersion(const std::string& version1, const std::string& version2) {
    int parsed1[4] {}, parsed2[4] {};
    parseVersion(parsed1, version1);
    parseVersion(parsed2, version2);
    return std::lexicographical_compare(parsed1, &parsed1[4], parsed2, &parsed2[4]);
}

void Context::TimelineUpdateServerSettings() {
    logger.debug("TimelineUpdateServerSettings");

    next_update_timeline_settings_at_ =
        postpone(kRequestThrottleSeconds * kOneSecondInMicros);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this,
                &Context::onTimelineUpdateServerSettings);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_update_timeline_settings_at_);

    logger.debug("Next timeline settings update at ", Formatter::Format8601(next_update_timeline_settings_at_));
}

const std::string kRecordTimelineEnabledJSON = "{\"record_timeline\": true}";
const std::string kRecordTimelineDisabledJSON = "{\"record_timeline\": false}";

void Context::onTimelineUpdateServerSettings(Poco::Util::TimerTask&) {  // NOLINT
    if (isPostponed(next_update_timeline_settings_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger.debug("onTimelineUpdateServerSettings postponed");
        return;
    }

    logger.debug("onTimelineUpdateServerSettings executing");

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
    HTTPRequest req;
    req.host = urls::TimelineUpload();
    req.relative_url = "/api/v8/timeline_settings";
    req.payload = json;
    req.basic_auth_username = apitoken;
    req.basic_auth_password = "api_token";

    HTTPResponse resp = TogglClient::GetInstance().Post(req);
    if (resp.err != noError) {
        displayError(resp.err);
        logger.error(resp.body);
        logger.error(resp.err);
    }
}

error Context::SendFeedback(const Feedback &fb) {
    if (!user_) {
        logger.warning("Cannot send feedback, user logged out");
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

void Context::onSendFeedback(Poco::Util::TimerTask&) {  // NOLINT
    logger.debug("onSendFeedback");

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
    form.set("toggl_version", HTTPClient::Config.AppVersion);
    form.set("details", Formatter::EscapeJSONString(feedback_.Details()));
    form.set("subject", Formatter::EscapeJSONString(feedback_.Subject()));
    form.set("date", Formatter::Format8601(time(nullptr)));
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
    HTTPRequest req;
    req.host = urls::API();
    req.relative_url ="/api/v8/feedback/web";
    req.basic_auth_username = api_token_value;
    req.basic_auth_password = api_token_name;
    req.form = &form;

    HTTPResponse resp = TogglClient::GetInstance().Post(req);
    logger.debug("Feedback result: " + resp.err);
    if (resp.err != noError) {
        displayError(resp.err);
        return;
    }
}

error Context::SetSettingsRemindTimes(
    const std::string &remind_starts,
    const std::string &remind_ends) {
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

error Context::applySettingsSaveResultToUI(const error &err) {
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

error Context::SetSettingsShowTouchBar(const bool show_touch_bar) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsShowTouchBar(show_touch_bar));
}

error Context::SetSettingsActiveTab(const uint8_t active_tab) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsActiveTab(active_tab));
}

error Context::SetSettingsColorTheme(const uint8_t color_theme) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsColorTheme(color_theme));
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
    const error &err = applySettingsSaveResultToUI(
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

bool Context::GetShowTouchBar() {
    bool value(false);
    displayError(db()->GetShowTouchBar(&value));
    return value;
}

uint8_t Context::GetActiveTab() {
    uint8_t value(0);
    displayError(db()->GetActiveTab(&value));
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

void Context::SetMessageSeen(
    const int64_t value) {
    displayError(db()->SetSettingsMessageSeen(value));
}

int64_t Context::GetMessageSeen() {
    Poco::Int64 value(0);
    displayError(db()->GetMessageSeen(&value));
    return value;
}

void Context::SetKeyStart(
    const std::string &value) {
    displayError(db()->SetKeyStart(value));
}

std::string Context::GetKeyStart() {
    std::string value("");
    displayError(db()->GetKeyStart(&value));
    return value;
}

void Context::SetKeyShow(
    const std::string &value) {
    displayError(db()->SetKeyShow(value));
}

std::string Context::GetKeyShow() {
    std::string value("");
    displayError(db()->GetKeyShow(&value));
    return value;
}

void Context::SetKeyModifierShow(
    const std::string &value) {
    displayError(db()->SetKeyModifierShow(value));
}

std::string Context::GetKeyModifierShow() {
    std::string value("");
    displayError(db()->GetKeyModifierShow(&value));
    return value;
}

void Context::SetKeyModifierStart(
    const std::string &value) {
    displayError(db()->SetKeyModifierStart(value));
}

std::string Context::GetKeyModifierStart() {
    std::string value("");
    displayError(db()->GetKeyModifierStart(&value));
    return value;
}

error Context::SetProxySettings(
    const bool use_proxy,
    const Proxy &proxy) {

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
    logger.debug("OpenSettings");

    UIElements render;
    render.display_settings = true;
    render.open_settings = true;
    updateUI(render);
}

error Context::SetDBPath(
    const std::string &path) {
    try {
        logger.debug("SetDBPath ", path);

        Poco::Mutex::ScopedLock lock(db_m_);
        if (db_) {
            logger.debug("delete db_ from SetDBPath()");
            delete db_;
            db_ = nullptr;
        }
        db_ = new Database(path);
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string & ex) {
        return displayError(ex);
    }
    return noError;
}

void Context::SetEnvironment(const std::string &value) {
    if (!("production" == value ||
            "development" == value ||
            "test" == value)) {
        logger.error("Invalid environment '", value, "'!");
        return;
    }
    logger.debug("SetEnvironment " + value);
    environment_ = value;

    TogglClient::GetInstance().SetIgnoreCert(("development" == environment_));
    urls::SetRequestsAllowed("test" != environment_);
}

Database *Context::db() const {
    poco_check_ptr(db_);
    return db_;
}

error Context::GoogleLogin(const std::string &access_token) {
    return Login(access_token, kGoogleAccessToken);
}

error Context::AsyncGoogleLogin(const std::string &access_token) {
    return AsyncLogin(access_token, kGoogleAccessToken);
}

error Context::AppleLogin(const std::string &access_token) {
    return Login(access_token, kAppleAccessToken);
}

error Context::AsyncAppleLogin(const std::string &access_token) {
    return AsyncLogin(access_token, kAppleAccessToken);
}

error Context::attemptOfflineLogin(const std::string &email,
                                   const std::string &password) {
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
        logger.debug("User data not found in local database for " + email);
        return error(kEmailNotFoundCannotLogInOffline);
    }

    if (user->OfflineData().empty()) {
        delete user;
        logger.debug("Offline data not found in local database for "
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

    return save(false);
}

error Context::AsyncLogin(const std::string &email,
                          const std::string &password) {
    std::thread backgroundThread([&](std::string email, std::string password) {
        return this->Login(email, password);
    }, email, password);
    backgroundThread.detach();
    return noError;
}

error Context::Login(
    const std::string &email,
    const std::string &password,
    const bool isSignup) {
    try {
        std::string json("");
        error err = me(email, password, &json, 0);
        if (err != noError) {

            // Workaround to Fulfill Apple Review team
            // If the apple email doesn't exist on Toggl system
            // Instead of presenting an error, we have to show a different window to select Country and TOS then creating a new account
            // Same behavior with Mobile app
            //
            // Apple considers that Toggl will create account regardless of "Sign In / Sign Up with Apple"
            // At the moment, Toggl BE doesn't automatically create new Toggl Account with Apple Email by "Sign In with Apple"
            //
            // Discussion: https://toggl.slack.com/archives/CSE5U3ZUN/p1586418153111700
            //
            #if defined(__APPLE__)
            if ((password.compare(kAppleAccessToken) == 0 || password.compare(kGoogleAccessToken) == 0) // Applied for Google and Apple Sign In
                && IsAuthenticationError(err)) {
                UI()->DisplayOnContinueSignIn();
                return err;
            }
            #endif

            if (!IsNetworkingError(err)) {
                return displayError(err);
            }

            // Indicate we're offline
            displayError(err);

            logger.debug("Got networking error ", err, " will attempt offline login");

            return displayError(attemptOfflineLogin(email, password));
        }

        err = SetLoggedInUserFromJSON(json, isSignup);
        if (err != noError) {
            return displayError(err);
        }

        err = pullWorkspacePreferences();
        if (err != noError) {
            return displayError(err);
        }

        err = pullUserPreferences();
        if (err != noError) {
            return displayError(err);
        }

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger.error("cannot enable offline login, no user");
                return noError;
            }

            err = user_->EnableOfflineLogin(password);
            if (err != noError) {
                return displayError(err);
            }
        }

        if ("production" == environment_) {
            if (password.compare(kGoogleAccessToken) == 0) {
                analytics_.TrackLoginWithGoogle(db_->AnalyticsClientID());
            } else if (password.compare(kAppleAccessToken) == 0) {
                analytics_.TrackLoginWithApple(db_->AnalyticsClientID());
            } else {
                analytics_.TrackLoginWithUsernamePassword(db_->AnalyticsClientID());
            }
        }

        overlay_visible_ = false;
        return displayError(save(false));
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string & ex) {
        return displayError(ex);
    }
}

error Context::AsyncSignup(const std::string &email,
                           const std::string &password,
                           const uint64_t country_id) {
    std::thread backgroundThread([&](std::string email, std::string password, uint64_t country_id) {
        return this->Signup(email, password, country_id);
    }, email, password, country_id);
    backgroundThread.detach();
    return noError;
}

error Context::Signup(
    const std::string &email,
    const std::string &password,
    const uint64_t country_id) {

    std::string json("");
    error err = signup(email, password, &json, country_id);
    if (err != noError) {
        return displayError(err);
    }

    return Login(email, password, true);
}

error Context::GoogleSignup(
    const std::string &access_token,
    const uint64_t country_id) {

    std::string json("");
    error err = signupGoogle(access_token, &json, country_id);
    if (err != noError) {
        return displayError(err);
    }
    return Login(access_token, kGoogleAccessToken, true);
}

error Context::AsyncGoogleSignup(const std::string &access_token,
                                 const uint64_t country_id) {
    std::thread backgroundThread([&](std::string access_token, uint64_t country_id) {
        return this->GoogleSignup(access_token, country_id);
    }, access_token, country_id);
    backgroundThread.detach();
    return noError;
}

error Context::AppleSignup(
    const std::string &access_token,
    const uint64_t country_id,
    const std::string full_name) {
    std::string json("");
    error err = signupApple(access_token, &json, full_name, country_id);
    if (err != noError) {
        return displayError(err);
    }
    return Login(access_token, kAppleAccessToken);
}

error Context::AsyncApleSignup(
    const std::string &access_token,
    const uint64_t country_id,
    const std::string full_name) {
    std::thread backgroundThread([&](std::string access_token, uint64_t country_id, std::string full_name) {
        return this->AppleSignup(access_token, country_id, full_name);
    }, access_token, country_id, full_name);
    backgroundThread.detach();
    return noError;
}

void Context::setUser(User *value, const bool logged_in) {
    logger.debug("setUser user_logged_in=", logged_in);

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
    const std::string &json,
    const bool isSignup) {

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
    if (isSignup && user_) {
        user_->HasLoadedMore.Set(true);
    }

    updateUI(UIElements::Reset());

    err = save(false);
    if (err != noError) {
        return displayError(err);
    }

    // Fetch OBM experiments..
    err = pullObmExperiments();
    if (err != noError) {
        logger.error("Error pulling OBM experiments: " + err);
    }

    // ..and run the OBM experiments
    err = runObmExperiments();
    if (err != noError) {
        logger.error("Error running OBM experiments: " + err);
    }

    return noError;
}

error Context::Logout() {
    try {
        if (!user_) {
            logger.warning("User is logged out, cannot logout again");
            return noError;
        }

        error err = db()->ClearCurrentAPIToken();
        if (err != noError) {
            return displayError(err);
        }

        logger.debug("setUser from Logout");
        overlay_visible_ = false;
        setUser(nullptr);

        UI()->resetFirstLaunch();
        UI()->DisplayApp();

        Shutdown();
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string & ex) {
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
                logger.warning("User is logged out, cannot clear cache");
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
    } catch(const std::string & ex) {
        return displayError(ex);
    }
}

TimeEntry *Context::Start(
    const std::string &description,
    const std::string &duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string project_guid,
    const std::string tags,
    const bool prevent_on_app,
    const time_t started,
    const time_t ended,
    const bool stop_current_running) {

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
            logger.warning("Cannot start tracking, user logged out");
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
                          tags,
                          started,
                          ended,
                          stop_current_running);
    }

    error err = save(true);
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    if ("production" == environment_) {
        analytics_.TrackAutocompleteUsage(db_->AnalyticsClientID(),
                                          task_id || project_id);
        analytics_.TrackStartTimeEntry(db_->AnalyticsClientID(), shortOSName(), GetActiveTab());
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
    const std::string &GUID,
    const bool edit_running_entry,
    const std::string &focused_field_name) {
    if (!edit_running_entry && GUID.empty()) {
        logger.error("Cannot edit time entry without a GUID");
        return;
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot edit time entry, user logged out");
            return;
        }

        if (edit_running_entry) {
            te = user_->RunningTimeEntry();
        } else {
            te = user_->related.TimeEntryByGUID(GUID);
        }
    }

    if (!te) {
        logger.warning("Time entry not found for edit " + GUID);
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

    if ("production" == environment_) {
        analytics_.TrackEditTimeEntry(db_->AnalyticsClientID(), shortOSName(), GetActiveTab());
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
            logger.warning("Cannot continue tracking, user logged out");
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



    error err = save(true);
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
    const std::string &GUID) {

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
        displayError(std::string(__FUNCTION__) + ": Missing GUID");
        return nullptr;
    }

    TimeEntry *result = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot continue time entry, user logged out");
            return nullptr;
        }

        result = user_->Continue(
            GUID,
            settings_.manual_mode);
    }

    error err = save(true);
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

error Context::DeleteTimeEntryByGUID(const std::string &GUID) {
    // Do not even allow to delete time entries,
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        return displayError(kUnsupportedAppError);
    }

    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot delete time entry, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger.warning("Time entry not found: " + GUID);
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
    te->ValidationError.Set(noError);
    te->Delete();

    if ("production" == environment_) {
        analytics_.TrackDeleteTimeEntry(db_->AnalyticsClientID(), shortOSName(), GetActiveTab());
    }

    return displayError(save(true));
}

error Context::SetTimeEntryDuration(
    const std::string &GUID,
    const std::string &duration) {
    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger.warning("Cannot set duration, user logged out");
        return noError;
    }
    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger.warning("Time entry not found: " + GUID);
        return noError;
    }

    if (isTimeEntryLocked(te)) {
        return logAndDisplayUserTriedEditingLockedEntry();
    }

    // validate the value
    int seconds = Formatter::ParseDurationString(duration);
    if (seconds >= kMaxDurationSeconds) {
        return displayError(error(kOverMaxDurationError));
    }

    te->SetDurationUserInput(duration);
    return displayError(save(true));
}

error Context::SetTimeEntryProject(
    const std::string &GUID,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string &project_guid) {
    try {
        if (GUID.empty()) {
            return displayError(std::string(__FUNCTION__) + ": Missing GUID");
        }

        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot set project, user logged out");
            return noError;
        }

        TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
        if (!te) {
            logger.warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }

        error err = updateTimeEntryProject(te, task_id, project_id, project_guid);
        if (err != noError) {
            return err;
        }
        if (te->Dirty()) {
            te->ValidationError.Set(noError);
            te->SetUIModified();
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string & ex) {
        return displayError(ex);
    }
    return displayError(save(true));
}

error Context::updateTimeEntryProject(
    TimeEntry *te,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string &project_guid) {
    Project *p = nullptr;
    if (project_id) {
        p = user_->related.ProjectByID(project_id);
    }
    if (p == nullptr && !project_guid.empty()) {
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
        if (p->ID() != te->PID()
                || (!project_guid.empty() && p->GUID().compare(te->ProjectGUID()) != 0)) {
            te->Billable.Set(p->Billable());
        }
        te->WID.Set(p->WID());
    }
    te->TID.Set(task_id);
    te->PID.Set(project_id);
    te->ProjectGUID.Set(project_guid);
    return noError;
}

error Context::SetTimeEntryDate(
    const std::string &GUID,
    const Poco::Int64 unix_timestamp) {

    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    TimeEntry *te;
    Poco::LocalDateTime dt;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot change date, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger.warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return displayError(error("Cannot change locked time entry."));
        }

        Poco::LocalDateTime date_part(
            Poco::Timestamp::fromEpochTime(unix_timestamp));

        Poco::LocalDateTime time_part(
            Poco::Timestamp::fromEpochTime(te->Start()));

        // Validate date input
        if (date_part.year() < kMinimumAllowedYear || date_part.year() > kMaximumAllowedYear) {
            return displayError(error(kInvalidDateError));
        }

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

    return displayError(save(true));
}

error Context::SetTimeEntryStart(const std::string GUID,
                                 const Poco::Int64 startAt) {
    return SetTimeEntryStartWithOption(GUID, startAt, GetKeepEndTimeFixed());
}

error Context::SetTimeEntryStartWithOption(const std::string GUID,
        const Poco::Int64 startAt,
        const bool keepEndTimeFixed) {
    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot change start time, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger.warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }

        Poco::LocalDateTime start(Poco::Timestamp::fromEpochTime(startAt));
        std::string s = Poco::DateTimeFormatter::format(start, Poco::DateTimeFormat::ISO8601_FORMAT);
        te->SetStartUserInput(s, keepEndTimeFixed);
        return displayError(save(true));
    }
}

error Context::SetTimeEntryStart(
    const std::string &GUID,
    const std::string &value) {
    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }
    Poco::LocalDateTime now;

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot change start time, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger.warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    Poco::LocalDateTime local(Poco::Timestamp::fromEpochTime(te->Start()));

    // Validate time input
    if (local.year() < kMinimumAllowedYear || local.year() > kMaximumAllowedYear) {
        return displayError(error(kInvalidStartTimeError));
    }

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

    return displayError(save(true));
}

error Context::SetTimeEntryStop(const std::string GUID,
                                const Poco::Int64 endAt) {
    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot change stop time, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger.warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }

        Poco::LocalDateTime stop(Poco::Timestamp::fromEpochTime(endAt));
        std::string s = Poco::DateTimeFormatter::format(stop, Poco::DateTimeFormat::ISO8601_FORMAT);
        te->SetStopUserInput(s);
        return displayError(save(true));
    }
}

error Context::SetTimeEntryStop(
    const std::string &GUID,
    const std::string &value) {
    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot change stop time, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger.warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    Poco::LocalDateTime stop(
        Poco::Timestamp::fromEpochTime(te->StopTime()));

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

    return displayError(save(true));
}

error Context::SetTimeEntryTags(
    const std::string &GUID,
    const std::string &value) {
    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot set tags, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger.warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }

        te->SetTags(value);
    }

    if (te->Dirty()) {
        te->ValidationError.Set(noError);
        te->SetUIModified();
    }

    return displayError(save(true));
}

error Context::SetTimeEntryBillable(
    const std::string &GUID,
    const bool value) {
    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot set billable, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger.warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }

        te->Billable.Set(value);
    }

    if (te->Dirty()) {
        te->ValidationError.Set(noError);
        te->SetUIModified();
    }

    return displayError(save(true));
}

error Context::SetTimeEntryDescription(
    const std::string &GUID,
    const std::string &value) {
    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot set description, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger.warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }

        error err = updateTimeEntryDescription(te, value);
        if (err != noError) {
            return err;
        }

        if (te->Dirty()) {
            te->ValidationError.Set(noError);
            te->SetUIModified();
        }
    }
    return displayError(save(true));
}

error Context::updateTimeEntryDescription(
    TimeEntry *te,
    const std::string &value) {
    // Validate description length
    if (value.length() > kMaximumDescriptionLength) {
        return displayError(error(kMaximumDescriptionLengthError));
    }

    te->Description.Set(value);
    return noError;
}

error Context::Stop(const bool prevent_on_app) {
    std::vector<TimeEntry *> stopped;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot stop tracking, user logged out");
            return noError;
        }
        user_->Stop(&stopped);

        resetLastTrackingReminderTime();
    }

    if (stopped.empty()) {
        logger.warning("No time entry was found to stop");
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
    const std::string &guid,
    const Poco::Int64 at,
    const bool split_into_new_entry) {

    // Reset reminder count when doing idle actions
    resetLastTrackingReminderTime();

    // Tracking action
    if ("production" == environment_) {
        std::string method;
        if (split_into_new_entry) {
            method = "idle-as-new-entry";
        } else {
            method = "discard-and-stop";
        }

        analytics_.TrackIdleDetectionClick(db_->AnalyticsClientID(),
                                           method);
    }

    TimeEntry *split = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot stop time entry, user logged out");
            return noError;
        }

        split = user_->DiscardTimeAt(guid, at, split_into_new_entry);
    }

    error err = save(true);
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
    const std::string &guid,
    const Poco::Int64 at) {

    // Reset reminder count when doing idle actions
    last_tracking_reminder_time_ = time(nullptr);

    // Tracking action
    if ("production" == environment_) {
        analytics_.TrackIdleDetectionClick(db_->AnalyticsClientID(),
                                           "discard-and-continue");
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("Cannot stop time entry, user logged out");
            return nullptr;
        }
        user_->DiscardTimeAt(guid, at, false);
    }

    error err = save(true);
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    return Continue(guid);
}

TimeEntry *Context::RunningTimeEntry() {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger.warning("Cannot fetch time entry, user logged out");
        return nullptr;
    }
    return user_->RunningTimeEntry();
}

error Context::ToggleTimelineRecording(const bool record_timeline) {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger.warning("Cannot toggle timeline, user logged out");
        return noError;
    }
    try {
        user_->RecordTimeline.Set(record_timeline);

        error err = save(false);
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
    } catch(const std::string & ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::SetUpdateChannel(const std::string &channel) {
    error err = db()->SaveUpdateChannel(channel);
    if (err != noError) {
        return displayError(err);
    }
    fetchUpdates();
    return noError;
}

void Context::SearchHelpArticles(
    const std::string &keywords) {
    UI()->DisplayHelpArticles(help_database_.GetArticles(keywords));
}

error Context::SetDefaultProject(
    const Poco::UInt64 pid,
    const Poco::UInt64 tid) {
    try {
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger.warning("Cannot set default PID, user logged out");
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
                user_->DefaultPID.Set(p->ID());
            } else {
                user_->DefaultPID.Set(0);
            }

            if (t) {
                user_->DefaultTID.Set(t->ID());
            } else {
                user_->DefaultTID.Set(0);
            }
        }
        return displayError(save(false));
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string & ex) {
        return displayError(ex);
    }
}

error Context::DefaultProjectName(std::string *name) {
    try {
        poco_check_ptr(name);
        Project *p = nullptr;
        Task *t = nullptr;
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger.warning("Cannot get default PID, user logged out");
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
    } catch(const std::string & ex) {
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
                logger.warning("Cannot get default PID, user logged out");
                return noError;
            }
            *result = user_->DefaultPID();
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string & ex) {
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
                logger.warning("Cannot get default PID, user logged out");
                return noError;
            }
            *result = user_->DefaultTID();
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string & ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::AddAutotrackerRule(
    const std::string &term,
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
            logger.warning("cannot add autotracker rule, user logged out");
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
        rule->Term.Set(lowercase);
        if (t) {
            rule->TID.Set(t->ID());
        }
        if (p) {
            rule->PID.Set(p->ID());
        }
        rule->UID.Set(user_->ID());
        user_->related.AutotrackerRules.push_back(rule);
    }

    error err = save(false);
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
            logger.warning("cannot delete rule, user is logged out");
            return noError;
        }

        error err = user_->related.DeleteAutotrackerRule(id);
        if (err != noError) {
            return displayError(err);
        }
    }

    return displayError(save(false));
}

Project *Context::CreateProject(
    const Poco::UInt64 workspace_id,
    const Poco::UInt64 client_id,
    const std::string &client_guid,
    const std::string &project_name,
    const bool is_private,
    const std::string &project_color) {

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
            logger.warning("Cannot add project, user logged out");
            return nullptr;
        }
        for (std::vector<Project *>::iterator it =
            user_->related.Projects.begin();
                it != user_->related.Projects.end(); ++it) {
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

    err = save(false);
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    return result;
}

error Context::AddObmAction(
    const Poco::UInt64 experiment_id,
    const std::string &key,
    const std::string &value) {
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
            logger.warning("Cannot create a OBM action, user logged out");
            return noError;
        }
        ObmAction *action = new ObmAction();
        action->ExperimentID.Set(experiment_id);
        action->UID.Set(user_->ID());
        action->Key.Set(trimmed_key);
        action->Value.Set(trimmed_value);
        user_->related.ObmActions.push_back(action);
    }
    return displayError(save(false));
}

Client *Context::CreateClient(
    const Poco::UInt64 workspace_id,
    const std::string &client_name) {

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
            logger.warning("Cannot create a client, user logged out");
            return nullptr;
        }
        for (std::vector<Client *>::iterator it =
            user_->related.Clients.begin();
                it != user_->related.Clients.end(); ++it) {
            Client *c = *it;
            if (c->WID() == workspace_id && c->Name() == trimmed_client_name) {
                displayError(kClientNameAlreadyExists);
                return nullptr;
            }
        }
        result = user_->CreateClient(workspace_id, trimmed_client_name);
    }

    err = save(false);
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
        logger.debug("SetSleep");
        idle_.SetSleep();
        if (window_change_recorder_) {
            window_change_recorder_->SetIsSleeping(true);
        }
    }
}

error Context::AsyncOpenReportsInBrowser() {
    std::thread backgroundThread([&]() {
        return this->OpenReportsInBrowser();
    });
    backgroundThread.detach();
    return noError;
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
    HTTPRequest req;
    req.host = urls::API();
    req.relative_url = "/api/v8/desktop_login_tokens";
    req.payload = "{}";
    req.basic_auth_username = apitoken;
    req.basic_auth_password = "api_token";

    HTTPResponse resp = TogglClient::GetInstance().Post(req);
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
    ss  << urls::Main() << "/api/v8/desktop_login"
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
    } catch(const std::string & ex) {
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
                logger.warning("User logged out, cannot OBM experiment");
                return noError;
            }
            for (std::vector<ObmExperiment *>::const_iterator it =
                user_->related.ObmExperiments.begin();
                    it != user_->related.ObmExperiments.end();
                    ++it) {
                ObmExperiment *model = *it;
                if (!model->DeletedAt()) {
                    experiments[model->Nr()] = *model;
                    model->HasSeen.Set(true);
                }
            }
        }
        // Save the (seen/unseen) state
        error err = save(false);
        if (err != noError) {
            return err;
        }
        // Now pass the experiments on to UI
        for (std::map<Poco::UInt64, ObmExperiment>::const_iterator
                it = experiments.begin();
                it != experiments.end();
                ++it) {
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
    } catch(const std::string & ex) {
        return displayError(ex);
    }
    return noError;
}

void Context::SetWake() {
    logger.debug("SetWake");

    Poco::Timestamp::TimeDiff delay = 0;
    if (next_wake_at_ > 0) {
        delay = kRequestThrottleSeconds * kOneSecondInMicros;
    }

    next_wake_at_ = postpone(delay);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onWake);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_wake_at_);

    logger.debug("Next wake at ", Formatter::Format8601(next_wake_at_));
}

void Context::onWake(Poco::Util::TimerTask&) {  // NOLINT
    if (isPostponed(next_wake_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger.debug("onWake postponed");
        return;
    }
    logger.debug("onWake executing");

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
        if (window_change_recorder_) {
            window_change_recorder_->SetIsSleeping(false);
        }

        Sync();
    }
    catch (const Poco::Exception& exc) {
        logger.error(exc.displayText());
    }
    catch (const std::exception& ex) {
        logger.error(ex.what());
    }
    catch (const std::string & ex) {
        logger.error(ex);
    }
}

void Context::SetLocked() {
    logger.debug("SetLocked");
    if (window_change_recorder_) {
        window_change_recorder_->SetIsLocked(true);
    }
}

void Context::SetUnlocked() {
    logger.debug("SetUnlocked");
    if (window_change_recorder_) {
        window_change_recorder_->SetIsLocked(false);
    }
}

void Context::SetOnline() {
    logger.debug("SetOnline");
    Sync();
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

        if (time(nullptr) - last_tracking_reminder_time_
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
        logger.debug("reminder is not enabled on this weekday");
        return;
    }

    // Check if allowed to display reminder at this time
    if (!settings_.remind_starts->empty()) {
        int h(0), m(0);
        if (toggl::Formatter::ParseTimeInput(settings_.remind_starts, &h, &m)) {
            Poco::LocalDateTime start(
                now.year(), now.month(), now.day(), h, m, now.second());
            if (now < start) {
                logger.debug("Reminder - its too early for reminders", " [", now.hour(), ":", now.minute(), "]", " (allowed from ", h, ":", m, ")");
                return;
            }
        }
    }
    if (!settings_.remind_ends->empty()) {
        int h(0), m(0);
        if (toggl::Formatter::ParseTimeInput(settings_.remind_ends, &h, &m)) {
            Poco::LocalDateTime end(now.year(), now.month(), now.day(), h, m, now.second());
            if (now > end) {
                logger.debug("Reminder - its too late for reminders", " [", now.hour(), ":", now.minute(), "]", " (allowed until ", h, ":", m, ")");
                return;
            }
        }
    }

    resetLastTrackingReminderTime();

    UI()->DisplayReminder();
}

void Context::resetLastTrackingReminderTime() {
    last_tracking_reminder_time_ = time(nullptr);
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
            if (time(nullptr) - current_te->LastStartAt()
                    < settings_.pomodoro_minutes * 60) {
                return;
            }
        } else {
            if (time(nullptr) - current_te->Start()
                    < settings_.pomodoro_minutes * 60) {
                return;
            }
        }
        const Poco::Int64 pomodoroDuration = settings_.pomodoro_minutes * 60;
        wid = current_te->WID();
        Stop(true);
        current_te->DurationInSeconds.Set(pomodoroDuration);
        current_te->StopTime.Set(current_te->StartTime() + pomodoroDuration);
    }
    UI()->DisplayPomodoro(settings_.pomodoro_minutes);

    if (settings_.pomodoro_break) {
        //  Start a new task with the tag "pomodoro-break"
        pomodoro_break_entry_ = user_->Start("Pomodoro Break",  // description
                                             "",  // duration
                                             0,  // task_id
                                             0,  // project_id
                                             "",  // project_guid
                                             "pomodoro-break",
                                             0,
                                             0,
                                             true);  // tags

        // Set workspace id to same as the previous entry
        pomodoro_break_entry_->WID.Set(wid);
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

        if (time(nullptr) - current_te->Start()
                < settings_.pomodoro_break_minutes * 60) {
            return;
        }
        const Poco::Int64 pomodoroDuration = settings_.pomodoro_break_minutes * 60;
        Stop(true);
        current_te->DurationInSeconds.Set(pomodoroDuration);
        current_te->StopTime.Set(current_te->StartTime() + pomodoroDuration);
    }
    pomodoro_break_entry_ = nullptr;

    UI()->DisplayPomodoroBreak(settings_.pomodoro_break_minutes);
}

error Context::StartAutotrackerEvent(const TimelineEvent &event) {
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
            logger.warning("cannot create timeline batch, user logged out");
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

        batch->SetEvents(user_->CompressedTimelineForUpload());
        batch->SetUserID(user_->ID());
        batch->SetAPIToken(user_->APIToken());
        batch->SetDesktopID(db_->DesktopID());
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string & ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::StartTimelineEvent(TimelineEvent *event) {
    // Prevent a leak in case of an early exit
    std::unique_ptr<TimelineEvent> handler { event };

    try {
        poco_check_ptr(event);

        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return noError;
        }

        if (user_ && user_->RecordTimeline()) {
            event->UID.Set(static_cast<unsigned int>(user_->ID()));
            user_->related.TimelineEvents.push_back(handler.release());
            return displayError(save(false));
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string & ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::MarkTimelineBatchAsUploaded(
    const std::vector<TimelineEvent> &events) {
    try {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("cannot mark timeline events as uploaded, "
                           "user is already logged out");
            return noError;
        }
        user_->MarkTimelineBatchAsUploaded(events);
        return displayError(save(false));
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string & ex) {
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

void Context::syncerActivityWrapper() {
    enum {
        STARTUP,
        LEGACY,
        BATCHED
    } state
#if defined(TOGGL_SYNC_FORCE_BATCHED)
    { BATCHED };
#elif defined(TOGGL_SYNC_FORCE_LEGACY)
    { BATCHED };
#else
    { BATCHED };
#endif

    while (true) {
        // Sleep in increments for faster shutdown.
        for (int i = 0; i < 4; i++) {
            if (syncer_.isStopped()) {
                return;
            }
            Poco::Thread::sleep(250);
        }

        switch (state) {
        case STARTUP: {
            logger.log("Syncer bootup, will attempt to determine which protocol to use");

            Poco::Mutex::ScopedLock lock(user_m_);
            std::string api_token = user_->APIToken();

            HTTPRequest req;
            req.host = urls::API();
            req.relative_url = "/api/v9/me/flags";
            req.basic_auth_username = api_token;
            req.basic_auth_password = "api_token";

            HTTPResponse resp = TogglClient::GetInstance().Get(req);

            // don't do anything if the error is not noError, that's usually network error and this will try to connect every second
            if (resp.err == noError) {
                // if the server doesn't respond OK to this request, fall back to the legacy sync protocol
                if (resp.status_code != 200) {
                    logger.log("Syncer - Server didn't respond 200 to /me/flags, fallback to LEGACY");
                    state = LEGACY;
                }
                else { // is OK - 200
                    // otherwise, parse the response and see if the desktop_sync_client flag is there
                    Json::Value root;
                    Json::Reader reader;
                    if (!reader.parse(resp.body, root)) {
                        logger.log("Syncer - /me/flags response couldn't be parsed as JSON, fallback to LEGACY");
                        state = LEGACY;
                    }
                    else {
                        // there was a typo in the initial set of flags, use both variants to be sure
                        if (root[kSyncStrategyLegacy1].isBool() && root[kSyncStrategyLegacy1].asBool())
                            state = BATCHED;
                        else if (root[kSyncStrategyLegacy2].isBool() && root[kSyncStrategyLegacy2].asBool())
                            state = BATCHED;
                        else
                            state = LEGACY;
                        logger.log("Syncer - Syncing protocol was selected: ", (state == BATCHED ? "BATCHED" : "LEGACY"));
                    }
                }
            }
            break;
        }
        case LEGACY:
            legacySyncerActivity();
            break;
        case BATCHED:
            batchedSyncerActivity();
            break;
        }
    }
}

void Context::batchedSyncerActivity() {
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

                error err = pullAllUserData();
                if (err != noError) {
                    displayError(err);
                }

                setOnline("Data pulled");

                err = pushBatchedChanges(&trigger_sync_);
                trigger_push_ = false;
                if (err != noError) {
                    user_->HasLoadedMore.Set(true);
                    displayError(err);
                    return;
                } else {
                    setOnline("Data pushed");
                }
                trigger_sync_ = false;

                // Push cached OBM action
                err = pushObmAction();
                if (err != noError) {
                    std::cout << "SYNC: sync-pushObm ERROR\n";
                    logger.error("Error pushing OBM action: " + err);
                }

                displayError(save(false));
            }

        }

        {
            Poco::Mutex::ScopedLock lock(syncer_m_);

            if (trigger_push_) {
                error err = pushBatchedChanges(&trigger_sync_);
                if (err != noError) {
                    user_->HasLoadedMore.Set(true);
                    displayError(err);
                } else {
                    setOnline("Data pushed");
                }
                trigger_push_ = false;

                // Push cached OBM action
                err = pushObmAction();
                if (err != noError) {
                    std::cout << "SYNC: pushObm ERROR\n";
                    logger.error("Error pushing OBM action: " + err);
                }

                displayError(save(false));
            }
        }
    }
}

void Context::legacySyncerActivity() {
    {
        Poco::Mutex::ScopedLock lock(syncer_m_);

        if (trigger_sync_) {

            error err = pullAllUserData();
            if (err != noError) {
                displayError(err);
            }

            setOnline("Data pulled");

            err = pushChanges(&trigger_sync_);
            trigger_push_ = false;
            if (err != noError) {
                user_->HasLoadedMore.Set(true);
                displayError(err);
                return;
            } else {
                setOnline("Data pushed");
            }
            trigger_sync_ = false;

            // Push cached OBM action
            err = pushObmAction();
            if (err != noError) {
                std::cout << "SYNC: sync-pushObm ERROR\n";
                logger.error("Error pushing OBM action: " + err);
            }

            displayError(save(false));
        }

    }

    {
        Poco::Mutex::ScopedLock lock(syncer_m_);

        if (trigger_push_) {
            error err = pushChanges(&trigger_sync_);
            if (err != noError) {
                user_->HasLoadedMore.Set(true);
                displayError(err);
            } else {
                setOnline("Data pushed");
            }
            trigger_push_ = false;

            // Push cached OBM action
            err = pushObmAction();
            if (err != noError) {
                std::cout << "SYNC: pushObm ERROR\n";
                logger.error("Error pushing OBM action: " + err);
            }

            displayError(save(false));
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

void Context::onLoadMore(Poco::Util::TimerTask&) {
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
        return logger.warning(
            "cannot load more time entries without API token");
    }

    try {
        std::stringstream ss;
        ss << "/api/v9/me/time_entries?since="
           << (Poco::Timestamp() - Poco::Timespan(60, 0, 0, 0, 0)).epochTime();

        logger.debug("loading more: ", ss.str());

        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPResponse resp = TogglClient::GetInstance().Get(req);
        if (resp.err != noError) {
            logger.warning(resp.err);
            return;
        }

        std::string json = resp.body;

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_)
                return;
            error err = user_->LoadTimeEntriesFromJSONString(json);

            if (err != noError) {
                logger.error(err);
                return;
            }

            user_->HasLoadedMore.Set(true);

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
        logger.warning(exc.displayText());
    }
    catch (const std::exception& ex) {
        logger.warning(ex.what());
    }
    catch (const std::string & ex) {
        logger.warning(ex);
    }
}



void Context::SetLogPath(const std::string &path) {
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

error Context::pullAllUserData() {
    std::string api_token("");
    Poco::Int64 since(0);
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("cannot pull user data when logged out");
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

        err = pullWorkspaces();
        if (err != noError) {
            return err;
        }

        pullWorkspacePreferences();

        pullUserPreferences();

        stopwatch.stop();
        logger.debug("User with related data JSON fetched and parsed in ", stopwatch.elapsed() / 1000, " ms");
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Context::pushBatchedChanges(
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
                logger.warning("cannot push changes when logged out");
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

        Json::Value request;

        auto collectJson = [](auto &jsonItem, auto &list) -> void {
            for (auto i : list) {
                Json::Value item, meta;

                i->EnsureGUID();

                item["payload"] = i->SaveToJSON(9);

                // this member removal is not ideal
                // that will get fixed in a next PR implementing changed property pushing
                if (i->NeedsPOST()) {
                    item["type"] = "create";
                    meta["client_assigned_id"] = i->GUID();
                    item.removeMember("id");
                }
                else if (i->NeedsPUT()) {
                    item["type"] = "update";
                    item.removeMember("id");
                    item.removeMember("workspace_id");
                    item.removeMember("wid");
                    meta["id"] = static_cast<Json::UInt64>(i->ID());
                    meta["workspace_id"] = static_cast<Json::UInt64>(i->WID());
                }
                else if (i->NeedsDELETE()) {
                    item["type"] = "delete";
                    item.removeMember("id");
                    item.removeMember("workspace_id");
                    item.removeMember("wid");
                    meta["id"] = static_cast<Json::UInt64>(i->ID());
                    meta["workspace_id"] = static_cast<Json::UInt64>(i->WID());
                }

                item["meta"] = meta;

                jsonItem.append(item);
            }
        };

        if (!clients.empty())
            collectJson(request["clients"], clients);
        if (!projects.empty())
            collectJson(request["projects"], projects);
        if (!time_entries.empty())
            collectJson(request["time_entries"], time_entries);

        Json::FastWriter w;
        std::string payload = w.write(request);

        std::string requestID = Poco::UUIDGenerator().createOne().toString();

        HTTPRequest req;
        req.payload = payload;
        req.host = urls::SyncAPI();
        req.relative_url = "/push/" + requestID;
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";
        req.compress = true;

        auto response = TogglClient::GetInstance().Post(req);

        auto handleResponse = [this](Json::Value &json, auto &list) -> error {
            auto findByGuid = [](auto &list, auto guid) -> typename std::remove_reference<decltype(list)>::type::value_type {
                for (auto i : list) {
                    if (i->GUID() == guid) {
                        return i;
                    }
                }
                return nullptr;
            };

            for (auto i : json) {
                if (!i["payload"].empty() && i["payload"]["success"].asBool()) {
                    auto &root = i["payload"]["result"];
                    auto model = findByGuid(list, i["meta"]["client_assigned_id"].asString());
                    if (model) {
                        auto id = root["id"].asUInt64();
                        if (!id) {
                            continue;
                        }

                        if (!model->ID()) {
                            user_->SetModelID(id, model);
                        }

                        if (model->ID() != id) {
                            return error("Backend has changed the ID of the entry");
                        }

                        model->LoadFromJSON(i["payload"]["result"]);
                        model->ClearDirty();
                        model->Unsynced.Set(false);
                    }
                }
            }
            return noError;
        };

        Json::Reader reader;
        Json::Value responseJson;
        reader.parse(response.body, responseJson);

        handleResponse(responseJson["clients"], clients);
        updateProjectClients(clients, projects);
        handleResponse(responseJson["projects"], projects);
        updateEntryProjects(projects, time_entries);
        handleResponse(responseJson["time_entries"], time_entries);

        stopwatch.stop();
        ss << ") Total = " << stopwatch.elapsed() / 1000 << " ms";
        logger.debug(ss.str());
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Context::pushChanges(
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
                logger.warning("cannot push changes when logged out");
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
                api_token);
            if (err != noError &&
                    err.find(kClientNameAlreadyExists) == std::string::npos) {
                return err;
            }
            // update client ids on projects if needed
            err = updateProjectClients(clients, projects);
            if (err != noError) {
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
                api_token);
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
                api_token);
            if (err != noError) {
                // Hide load more button when offline
                user_->HasLoadedMore.Set(true);
                // Reload list to show unsynced icons in items
                UIElements render;
                render.display_time_entries = true;
                updateUI(render);
                return err;
            }

            entry_stopwatch.stop();
            ss << " | " << time_entries.size() << " time entries in "
               << entry_stopwatch.elapsed() / 1000 << " ms";
        }

        stopwatch.stop();
        ss << ") Total = " << stopwatch.elapsed() / 1000 << " ms";
        logger.debug(ss.str());
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Context::pushClients(
    const std::vector<Client *> &clients,
    const std::string &api_token) {
    std::string client_json("");
    error err = noError;
    for (std::vector<Client *>::const_iterator it =
        clients.begin();
            it != clients.end(); ++it) {
        Json::Value clientJson = (*it)->SaveToJSON();

        Json::StyledWriter writer;
        client_json = writer.write(clientJson);

        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = (*it)->ModelURL();
        req.payload = client_json;
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPResponse resp = TogglClient::GetInstance().Post(req);

        if (resp.err != noError) {
            // if we're able to solve the error
            if ((*it)->ResolveError(resp.body)) {
                displayError(save(false));
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
    const std::vector<Project *> &projects,
    const std::vector<Client *> &clients,
    const std::string &api_token) {
    error err = noError;
    std::string project_json("");
    for (std::vector<Project *>::const_iterator it =
        projects.begin();
            it != projects.end(); ++it) {

        Json::Value projectJson = (*it)->SaveToJSON();

        Json::StyledWriter writer;
        project_json = writer.write(projectJson);

        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = (*it)->ModelURL();
        req.payload = project_json;
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPResponse resp = TogglClient::GetInstance().Post(req);

        if (resp.err != noError) {
            // if we're able to solve the error
            if ((*it)->ResolveError(resp.body)) {
                displayError(save(false));
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

error Context::updateEntryProjects(const std::vector<Project *> &projects,
                                   const std::vector<TimeEntry *> &time_entries) {
    for (std::vector<TimeEntry *>::const_iterator it =
        time_entries.begin();
            it != time_entries.end(); ++it) {
        if (!(*it)->PID() && !(*it)->ProjectGUID().empty()) {
            // Find project id
            for (std::vector<Project *>::const_iterator itc =
                projects.begin();
                    itc != projects.end(); ++itc) {
                if ((*itc)->GUID().compare((*it)->ProjectGUID()) == 0) {
                    (*it)->PID.Set((*itc)->ID());
                    break;
                }
            }
        }
    }

    return noError;
}

error Context::updateProjectClients(const std::vector<Client *> &clients,
                                     const std::vector<Project *> &projects) {
    for (auto it = projects.cbegin(); it != projects.cend(); ++it) {
        if (!(*it)->CID() && !(*it)->ClientGUID().empty()) {
            // Find client id
            for (auto itc = clients.begin(); itc != clients.end(); ++itc) {
                if ((*itc)->GUID().compare((*it)->ClientGUID()) == 0) {
                    (*it)->CID.Set((*itc)->ID());
                    break;
                }
            }
        }
    }

    return noError;
}

error Context::pushEntries(
    const std::map<std::string, BaseModel *>&,
    const std::vector<TimeEntry *> &time_entries,
    const std::string &api_token) {

    std::string entry_json("");
    std::string error_message("");
    bool error_found = false;
    bool offline = false;

    for (std::vector<TimeEntry *>::const_iterator it =
        time_entries.begin();
            it != time_entries.end(); ++it) {
        // Avoid trying to POST when we're offline
        if (offline) {
            // Mark the time entry as unsynced now
            (*it)->Unsynced.Set(true);
            continue;
        }

        Json::Value entryJson = (*it)->SaveToJSON();

        Json::StyledWriter writer;
        entry_json = writer.write(entryJson);

        // std::cout << entry_json;

        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = (*it)->ModelURL();
        req.payload = entry_json;
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPResponse resp;

        if ((*it)->NeedsDELETE()) {
            req.payload = "";
            resp = TogglClient::GetInstance().Delete(req);
        } else if ((*it)->ID()) {
            resp = TogglClient::GetInstance().Put(req);
        } else {
            resp = TogglClient::GetInstance().Post(req);
        }

        if (resp.err != noError) {
            // if we're able to solve the error
            if ((*it)->ResolveError(resp.body)) {
                displayError(save(false));
            }

            // Not found on server. Probably deleted already.
            if ((*it)->isNotFound(resp.body)) {
                (*it)->IsMarkedAsDeletedOnServer.Set(true);
                continue;
            }
            error_found = true;
            error_message = resp.err;
            if (resp.status_code == 429) {
                error_message = error(kRateLimit);
            }

            // Mark the time entry as unsynced now
            (*it)->Unsynced.Set(true);

            offline = IsNetworkingError(resp.err);

            if (offline) {
                trigger_sync_ = false;
            }

            if (kBadRequestError == resp.err) {
                error_message = resp.body;
            }

            continue;
        }

        if ((*it)->NeedsDELETE()) {
            // Successfully deleted entry
            (*it)->IsMarkedAsDeletedOnServer.Set(true);
            continue;
        }

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(resp.body, root)) {
            return error("error parsing time entry POST response");
        }

        auto id = root["id"].asUInt64();
        if (!id) {
            logger.error("Backend is sending invalid data: ignoring update without an ID");
            continue;
        }

        if (!(*it)->ID()) {
            if (!(user_->SetModelID<TimeEntry>(id, (*it)))) {
                continue;
            }
        }

        if ((*it)->ID() != id) {
            return error("Backend has changed the ID of the entry");
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
        if (HTTPClient::Config.OBMExperimentNrs.empty()) {
            logger.debug("No OBM experiment enabled by UI");
            return noError;
        }

        logger.trace("Fetching OBM experiments from backend");

        std::string apitoken("");
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger.warning("Cannot fetch OBM experiments without user");
                return noError;
            }
            apitoken = user_->APIToken();
        }

        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/me/experiments";
        req.basic_auth_username = apitoken;
        req.basic_auth_password = "api_token";

        HTTPResponse resp = TogglClient::GetInstance().Get(req);
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
                logger.warning("Cannot apply OBM experiments without user");
                return noError;
            }
            user_->LoadObmExperiments(json);
        }

        return noError;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
}

error Context::pushObmAction() {
    try {
        ObmAction *for_upload = nullptr;
        HTTPRequest req;
        req.host = urls::API();
        req.basic_auth_password = "api_token";

        // Get next OBM action for upload
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger.warning("cannot push changes when logged out");
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
                    ++it) {
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

        logger.debug(req.payload);

        HTTPResponse resp = TogglClient::GetInstance().silentPost(req);
        if (resp.err != noError) {
            // backend responds 204 on success
            if (resp.status_code != 204) {
                return resp.err;
            }
        }

        // mark as deleted to prevent duplicate uploading
        // (and make sure all other actions are uploaded)
        for_upload->IsMarkedAsDeletedOnServer.Set(true);
        for_upload->Delete();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}


error Context::me(
    const std::string &email,
    const std::string &password,
    std::string *user_data_json,
    const Poco::Int64 since) {

    if (email.empty()) {
        return "Empty email or API token";
    }

    if (password.empty()) {
        return "Empty password";
    }

    try {
        poco_check_ptr(user_data_json);

        std::stringstream ss;
        ss << "/api/"
           << kAPIV8
           << "/me"
           << "?app_name=" << TogglClient::Config.AppName
           << "&with_related_data=true";
        if (since) {
            ss << "&since=" << since;
        }

        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = email;
        req.basic_auth_password = password;

        HTTPResponse resp = TogglClient::GetInstance().Get(req);
        if (resp.err != noError) {
            return resp.err;
        }

        *user_data_json = resp.body;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
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
    logger.warning("User tried editing locked time entry");
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

error Context::pullWorkspaces() {
    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error("cannot pull user data without API token");
    }

    std::string json("");

    try {
        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/me/workspaces";
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPResponse resp = TogglClient::GetInstance().Get(req);
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
    catch (const std::string & ex) {
        return ex;
    }
    return noError;
}

error Context::pullWorkspacePreferences() {
    std::vector<Workspace*> workspaces;
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        logger.debug("user mutex lock success - c:pullWorkspacePreferences");

        user_->related.WorkspaceList(&workspaces);
    }
    for (std::vector<Workspace*>::const_iterator
            it = workspaces.begin();
            it != workspaces.end();
            ++it) {
        Workspace* ws = *it;

        if (!ws->Business())
            continue;

        std::string json("");

        error err = pullWorkspacePreferences(ws, &json);
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

        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPResponse resp = TogglClient::GetInstance().Get(req);
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
    catch (const std::string & ex) {
        return ex;
    }
    return noError;
}

error Context::pullUserPreferences() {
    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error("cannot pull user data without API token");
    }

    try {
        std::string json("");

        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/me/preferences/desktop";
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPResponse resp = TogglClient::GetInstance().Get(req);
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
    catch (const std::string & ex) {
        return ex;
    }
    return noError;
}

error Context::signupGoogle(
    const std::string &access_token,
    std::string *user_data_json,
    const uint64_t country_id) {
    return signUpWithProvider(access_token, user_data_json, country_id, "", kGoogleProvider);
}

error Context::signupApple(
    const std::string &access_token,
    std::string *user_data_json,
    const std::string &full_name,
    const uint64_t country_id) {
    return signUpWithProvider(access_token, user_data_json, country_id, full_name, kAppleProvider);
}

error Context::signUpWithProvider(
    const std::string &access_token,
    std::string *user_data_json,
    const uint64_t country_id,
    const std::string &full_name,
    const std::string provider) {
    try {
        poco_check_ptr(user_data_json);

        Json::Value user;
        user["token"] = access_token;
        user["created_with"] = Formatter::EscapeJSONString(
            HTTPClient::Config.UserAgent());
        user["tos_accepted"] = true;
        user["country_id"] = Json::UInt64(country_id);
        user["provider"] = provider;
        if (!full_name.empty()) {
            user["full_name"] = full_name;
        }

        Json::Value ws;
        ws["initial_pricing_plan"] = 0;
        user["workspace"] = ws;

        std::stringstream ss;
        ss << "/api/v9/signup?app_name=" << TogglClient::Config.AppName;

        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.payload = Json::StyledWriter().write(user);

        HTTPResponse resp = TogglClient::GetInstance().Post(req);
        if (resp.err != noError) {
            if (kBadRequestError == resp.err) {
                return resp.body;
            }
            return resp.err;
        }

        *user_data_json = resp.body;

        if ("production" == environment_) {
            if (provider == kAppleProvider) {
                analytics_.TrackSignupWithApple(db_->AnalyticsClientID());
            } else if (provider == kGoogleProvider) {
                analytics_.TrackSignupWithGoogle(db_->AnalyticsClientID());
            }
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
    const std::string &email,
    const std::string &password,
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

        Json::Value user;
        user["email"] = email;
        user["password"] = password;
        user["created_with"] = Formatter::EscapeJSONString(
            HTTPClient::Config.UserAgent());
        user["tos_accepted"] = true;
        user["country_id"] = Json::UInt64(country_id);

        Json::Value ws;
        ws["initial_pricing_plan"] = 0;
        user["workspace"] = ws;

        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/signup";
        req.payload = Json::StyledWriter().write(user);

        HTTPResponse resp = TogglClient::GetInstance().Post(req);
        if (resp.err != noError) {
            if (kBadRequestError == resp.err) {
                return resp.body;
            }
            return resp.err;
        }

        *user_data_json = resp.body;

        if ("production" == environment_) {
            analytics_.TrackSignupWithUsernamePassword(db_->AnalyticsClientID());
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

void Context::OpenTimelineDataView() {
    logger.debug("OpenTimelineDataView");

    UI()->SetTimelineDateAt(Poco::LocalDateTime());

    UIElements render;
    render.open_timeline = true;
    render.display_timeline = true;
    updateUI(render);
}

void Context::ViewTimelineCurrentDay() {
    UI()->SetTimelineDateAt(UI()->TimelineDateAt());
    UIElements render;
    render.display_timeline = true;
    updateUI(render);
}

void Context::ViewTimelinePrevDay() {
    UI()->SetTimelineDateAt(
        UI()->TimelineDateAt() - Poco::Timespan(1 * Poco::Timespan::DAYS));

    UIElements render;
    render.display_timeline = true;
    updateUI(render);
}

void Context::ViewTimelineNextDay() {
    UI()->SetTimelineDateAt(
        UI()->TimelineDateAt() + Poco::Timespan(1 * Poco::Timespan::DAYS));

    UIElements render;
    render.display_timeline = true;
    updateUI(render);
}

void Context::ViewTimelineSetDate(const Poco::Int64 unix_timestamp) {
    Poco::LocalDateTime date(Poco::Timestamp::fromEpochTime(unix_timestamp));
    UI()->SetTimelineDateAt(date);

    UIElements render;
    render.display_timeline = true;
    updateUI(render);
}

error Context::ToSAccept() {
    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error("cannot pull user data without API token");
    }

    try {
        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/me/accept_tos";
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPResponse resp = TogglClient::GetInstance().Post(req);
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
    } catch(const std::string & ex) {
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

error Context::AsyncPullCountries() {
    std::thread backgroundThread([&]() {
        return this->PullCountries();
    });
    backgroundThread.detach();
    return noError;
}

error Context::PullCountries() {
    try {
        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/countries";
        HTTPResponse resp = TogglClient::GetInstance().Get(req);
        if (resp.err != noError) {
            return resp.err;
        }
        Json::Value root;
        Json::Reader reader;

        if (!reader.parse(resp.body, root)) {
            return error("Error parsing countries response body");
        }

        std::vector<TogglCountryView> countries;
        TogglCountryView *first = nullptr;
        for (unsigned int i = root.size(); i > 0; i--) {
            TogglCountryView *item = country_view_item_init(root[i - 1]);
            countries.push_back(*item);
            item->Next = first;
            first = item;
        }

        // update country selectbox
        UI()->DisplayCountries(&countries);
        country_list_delete_item(first);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

template<typename T>
void Context::collectPushableModels(
    const std::vector<T *> &list,
    std::vector<T *> *result,
    std::map<std::string, BaseModel *> *models) const {

    poco_check_ptr(result);

    for (typename std::vector<T *>::const_iterator it =
        list.begin();
            it != list.end();
            ++it) {
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

void Context::TrackWindowSize(const Poco::UInt64 width,
                              const Poco::UInt64 height) {
    if ("production" == environment_) {
        analytics_.TrackWindowSize(db_->AnalyticsClientID(),
                                   shortOSName(),
                                   toggl::Rectangle(width, height));
    }
}

void Context::TrackEditSize(const Poco::Int64 width,
                            const Poco::Int64 height) {
    if ("production" == environment_) {
        analytics_.TrackEditSize(db_->AnalyticsClientID(),
                                 shortOSName(),
                                 toggl::Rectangle(width, height));
    }
}

void Context::TrackInAppMessage(const Poco::Int64 type) {
    if ("production" == environment_) {
        analytics_.TrackInAppMessage(db_->AnalyticsClientID(),
                                     last_message_id_,
                                     type);
    }
}

void Context::TrackCollapseDay() {
    if ("production" == environment_) {
        analytics_.Track(db_->AnalyticsClientID(),
                         "time_entry_list",
                         "collapse_day");
    }
}

void Context::TrackExpandDay() {
    if ("production" == environment_) {
        analytics_.Track(db_->AnalyticsClientID(),
                         "time_entry_list",
                         "expand_day");
    }
}

void Context::TrackCollapseAllDays() {
    if ("production" == environment_) {
        analytics_.Track(db_->AnalyticsClientID(),
                         "time_entry_list",
                         "collapse_all_days");
    }
}

void Context::TrackExpandAllDays() {
    if ("production" == environment_) {
        analytics_.Track(db_->AnalyticsClientID(),
                         "time_entry_list",
                         "expand_all_days");
    }
}

error Context::UpdateTimeEntry(
    const std::string &GUID,
    const std::string &description,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string &project_guid,
    const std::string &tags,
    const bool billable) {

    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger.warning("Cannot set project, user logged out");
        return noError;
    }

    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger.warning("Time entry not found: " + GUID);
        return noError;
    }

    if (isTimeEntryLocked(te)) {
        return logAndDisplayUserTriedEditingLockedEntry();
    }

    // Update
    error err = updateTimeEntryDescription(te, description);
    if (err != noError) {
        return err;
    }
    err = updateTimeEntryProject(te, task_id, project_id, project_guid);
    if (err != noError) {
        return err;
    }

    // Tag + billable
    te->SetTags(tags);
    te->Billable.Set(billable);

    if (te->Dirty()) {
        te->ValidationError.Set(noError);
        te->SetUIModified();
    }

    return displayError(save(true));
}
}  // namespace toggl
