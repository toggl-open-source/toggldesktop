
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
#include "Poco/Net/NetSSL.h"
#include "Poco/Path.h"
#include "Poco/PatternFormatter.h"
#include "Poco/Random.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/Stopwatch.h"
#include "Poco/URI.h"
#include "Poco/UTF8String.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Util/TimerTaskAdapter.h"

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
, next_reminder_at_(0)
, next_analytics_at_(0)
, next_wake_at_(0)
, environment_("production")
, idle_(&ui_)
, last_sync_started_(0)
, sync_interval_seconds_(0)
, update_check_disabled_(false)
, quit_(false)
, ui_updater_(this, &Context::uiUpdaterActivity)
, update_path_("") {
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
    {
        Poco::Mutex::ScopedLock lock(ui_updater_m_);
        if (ui_updater_.isRunning()) {
            ui_updater_.stop();
            ui_updater_.wait();
        }
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
        timer_.cancel(true);
    }

    // Stops all running threads and waits for their completion.
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

        updateUI(UIElements::Reset());
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
        render.ApplyChanges(UI()->TimeEntryEditorGUID(), changes);
        updateUI(render);

        if (push_changes) {
            next_push_changes_at_ =
                postpone(kRequestThrottleSeconds * kOneSecondInMicros);
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
    render.display_time_entries = true;
    render.display_time_entry_autocomplete = true;
    render.display_mini_timer_autocomplete = true;
    render.display_project_autocomplete = true;
    render.display_client_select = true;
    render.display_tags = true;
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
    ss << "display_time_entries=" << display_time_entries
       << " display_time_entry_autocomplete=" << display_time_entry_autocomplete
       << " display_mini_timer_autocomplete=" << display_mini_timer_autocomplete
       << " display_project_autocomplete=" << display_project_autocomplete
       << " display_client_select=" << display_client_select
       << " display_tags=" << display_tags
       << " display_workspace_select=" << display_workspace_select
       << " display_timer_state=" << display_timer_state
       << " display_time_entry_editor=" << display_time_entry_editor
       << " open_settings=" << open_settings
       << " open_time_entry_list=" << open_time_entry_list
       << " open_time_entry_editor=" << open_time_entry_editor
       << " display_autotracker_rules=" << display_autotracker_rules
       << " display_settings=" << display_settings
       << " time_entry_editor_guid=" << time_entry_editor_guid
       << " time_entry_editor_field=" << time_entry_editor_field
       << " display_unsynced_items=" << display_unsynced_items
       << " display_timeline=" << display_timeline
       << " open_timeline=" << open_timeline;
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

        if (ch.ModelType() == kModelTag) {
            display_tags = true;
        }

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

    TimeEntry *editor_time_entry = nullptr;
    TimeEntry *running_entry = nullptr;

    std::vector<view::Autocomplete> time_entry_autocompletes;
    std::vector<view::Autocomplete> minitimer_autocompletes;
    std::vector<view::Autocomplete> project_autocompletes;

    std::vector<Workspace *> workspaces;
    std::vector<TimeEntry *> time_entries;
    std::vector<Client *> clients;

    std::vector<std::string> tags;

    std::vector<TimelineEvent> timeline;

    Poco::Int64 total_duration_for_date(0);

    bool use_proxy(false);
    bool record_timeline(false);

    Poco::Int64 unsynced_item_count(0);

    Proxy proxy;

    // Collect data
    {
        Poco::Mutex::ScopedLock lock(user_m_);

        if (what.display_project_autocomplete && user_) {
            user_->related.ProjectAutocompleteItems(&project_autocompletes);
        }
        if (what.display_time_entry_editor && user_) {
            editor_time_entry =
                user_->related.TimeEntryByGUID(what.time_entry_editor_guid);
            if (editor_time_entry) {
                total_duration_for_date =
                    user_->related.TotalDurationForDate(editor_time_entry);
            }
        }
        if (what.display_time_entry_autocomplete && user_) {
            user_->related.TimeEntryAutocompleteItems(
                &time_entry_autocompletes);
        }
        if (what.display_mini_timer_autocomplete && user_) {
            user_->related.MinitimerAutocompleteItems(&minitimer_autocompletes);
        }
        if (what.display_workspace_select && user_) {
            user_->related.WorkspaceList(&workspaces);
        }
        if (what.display_client_select && user_) {
            user_->related.ClientList(&clients);
        }
        if (what.display_tags) {
            user_->related.TagList(&tags);
        }
        if (what.display_timer_state && user_) {
            running_entry = user_->RunningTimeEntry();
            if (running_entry) {
                total_duration_for_date =
                    user_->related.TotalDurationForDate(running_entry);
            }
        }
        if (what.display_time_entries && user_) {
            time_entries = user_->related.VisibleTimeEntries();
            std::sort(time_entries.begin(), time_entries.end(), CompareByStart);
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
            HTTPSClient::Config.IgnoreCert = false;
            HTTPSClient::Config.ProxySettings = proxy;
            HTTPSClient::Config.AutodetectProxy = settings_.autodetect_proxy;
        }
        if (what.display_unsynced_items && user_) {
            unsynced_item_count = user_->related.NumberOfUnsyncedTimeEntries();
        }
        if (what.display_timeline) {
            if (user_) {
                Poco::LocalDateTime date(UI()->TimelineDateAt());
                timeline = user_->CompressedTimeline(&date);
            }
        }
    }

    // Render data
    if (what.display_time_entry_editor && editor_time_entry) {
        if (what.open_time_entry_editor) {
            UI()->DisplayApp();
        }
        // FIXME: should not touch related data here any more,
        // data should be already collected in previous, locked step
        UI()->DisplayTimeEntryEditor(
            what.open_time_entry_editor,
            user_->related,
            editor_time_entry,
            what.time_entry_editor_field,
            total_duration_for_date,
            user_);
    }
    if (what.display_time_entries) {
        // FIXME: should not touch related data here any more,
        // data should be already collected in previous, locked step
        UI()->DisplayTimeEntryList(
            what.open_time_entry_list,
            user_->related,
            time_entries);
        last_time_entry_list_render_at_ = Poco::LocalDateTime();
    }
    if (what.display_timeline) {
        UI()->DisplayTimeline(what.open_timeline, timeline);
    }
    if (what.display_time_entry_autocomplete) {
        UI()->DisplayTimeEntryAutocomplete(&time_entry_autocompletes);
    }
    if (what.display_mini_timer_autocomplete) {
        UI()->DisplayMinitimerAutocomplete(&minitimer_autocompletes);
    }
    if (what.display_workspace_select) {
        UI()->DisplayWorkspaceSelect(&workspaces);
    }
    if (what.display_client_select) {
        // FIXME: should not touch related data here any more,
        // data should be already collected in previous, locked step
        UI()->DisplayClientSelect(
            user_->related,
            &clients);
    }
    if (what.display_tags) {
        UI()->DisplayTags(&tags);
    }
    if (what.display_timer_state) {
        // FIXME: should not touch related data here any more,
        // data should be already collected in previous, locked step
        UI()->DisplayTimerState(
            user_->related,
            running_entry,
            total_duration_for_date);
    }
    if (what.display_autotracker_rules) {
        if (UI()->CanDisplayAutotrackerRules() && user_) {
            // FIXME: should not touch related data here any more,
            // data should be already collected in previous, locked step
            UI()->DisplayAutotrackerRules(user_->related, autotracker_titles_);
        }
    }
    if (what.display_settings) {
        UI()->DisplaySettings(what.open_settings,
                              record_timeline,
                              settings_,
                              use_proxy,
                              proxy);
    }
    // Apply autocomplete as last element,
    // as its depending on selects on Windows
    if (what.display_project_autocomplete) {
        UI()->DisplayProjectAutocomplete(&project_autocompletes);
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
    if ((err.find(kForbiddenError) != std::string::npos)
            || (err.find(kUnauthorizedError) != std::string::npos)) {
        if (user_) {
            setUser(nullptr);
        }
    }
    if (err.find(kUnsupportedAppError) != std::string::npos) {
        urls::SetImATeapot(true);
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

    last_sync_started_ = time(0);

    TogglClient client(UI());
    error err = pullAllUserData(&client);
    if (err != noError) {
        displayError(err);
        return;
    }

    setOnline("Data pulled");

    bool had_something_to_push(true);
    err = pushChanges(&client, &had_something_to_push);
    if (err != noError) {
        displayError(err);
        return;
    }

    if (had_something_to_push) {
        setOnline("Data pushed");
    }

    displayError(save(false));
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
    logger().debug("onPushChanges executing");

    TogglClient client(UI());
    bool had_something_to_push(true);
    error err = pushChanges(&client, &had_something_to_push);
    if (err != noError) {
        displayError(err);
    } else if (had_something_to_push) {
        setOnline("Changes pushed");
    }

    err = save(false);
    if (err != noError) {
        displayError(err);
        return;
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

    error err = user_->LoadUserUpdateFromJSONString(json);
    if (err != noError) {
        return displayError(err);
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
            std::string body("");
            TogglClient client;
            err = client.Get(urls::API(),
                             update_url,
                             std::string(""),
                             std::string(""),
                             &body);
            if (err != noError) {
                return err;
            }

            if ("null" == body) {
                logger().debug("The app is up to date");
                if (UI()->CanDisplayUpdate()) {
                    UI()->DisplayUpdate("");
                }
                return noError;
            }

            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(body, root)) {
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

            // Download file
            std::string body("");
            TogglClient client;
            err = client.Get(uri.getScheme() + "://" + uri.getHost(),
                             uri.getPathEtc(),
                             std::string(""),
                             std::string(""),
                             &body);
            if (err != noError) {
                return err;
            }

            if ("null" == body || !body.size()) {
                return error("Failed to download update");
            }

            std::stringstream ss;
            ss << "Writing update to file " << file;
            logger().debug(ss.str());

            Poco::FileOutputStream fos(file, std::ios::binary);
            fos << body;
            fos.close();

            logger().debug("Update written");

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

    std::string response_body("");
    TogglClient client(UI());
    error err = client.Post(urls::TimelineUpload(),
                            "/api/v8/timeline_settings",
                            json,
                            apitoken,
                            "api_token",
                            &response_body);
    if (err != noError) {
        displayError(err);
        logger().error(response_body);
        logger().error(err);
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

    form.addPart("files",
                 new Poco::Net::FilePartSource(log_path_));

    std::string response_body("");
    TogglClient client(UI());
    error err = client.Post(urls::API(),
                            "/api/v8/feedback/web",
                            "",
                            api_token_value,
                            api_token_name,
                            &response_body,
                            &form);
    logger().debug("Feedback result: " + err);
    if (err != noError) {
        displayError(err);
        return;
    }
}

error Context::SetSettingsRemindTimes(
    const std::string remind_starts,
    const std::string remind_ends) {
    error err = db()->SetSettingsRemindTimes(remind_starts, remind_ends);

    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    remindToTrackTime();

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsRemindDays(
    const bool remind_mon,
    const bool remind_tue,
    const bool remind_wed,
    const bool remind_thu,
    const bool remind_fri,
    const bool remind_sat,
    const bool remind_sun) {
    error err = db()->SetSettingsRemindDays(
        remind_mon,
        remind_tue,
        remind_wed,
        remind_thu,
        remind_fri,
        remind_sat,
        remind_sun);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    remindToTrackTime();

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsAutodetectProxy(const bool autodetect_proxy) {
    error err = db()->SetSettingsAutodetectProxy(autodetect_proxy);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsUseIdleDetection(const bool use_idle_detection) {
    error err = db()->SetSettingsUseIdleDetection(use_idle_detection);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsAutotrack(const bool value) {
    error err = db()->SetSettingsAutotrack(value);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsOpenEditorOnShortcut(const bool value) {
    error err = db()->SetSettingsOpenEditorOnShortcut(value);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsMenubarTimer(const bool menubar_timer) {
    error err = db()->SetSettingsMenubarTimer(menubar_timer);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsMenubarProject(const bool menubar_project) {
    error err = db()->SetSettingsMenubarProject(menubar_project);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsDockIcon(const bool dock_icon) {
    error err = db()->SetSettingsDockIcon(dock_icon);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsOnTop(const bool on_top) {
    error err = db()->SetSettingsOnTop(on_top);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    trackSettingsUsage();

    return noError;
}


error Context::SetSettingsReminder(const bool reminder) {
    error err = db()->SetSettingsReminder(reminder);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    remindToTrackTime();

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsIdleMinutes(const Poco::UInt64 idle_minutes) {
    error err = db()->SetSettingsIdleMinutes(idle_minutes);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsFocusOnShortcut(const bool focus_on_shortcut) {
    error err = db()->SetSettingsFocusOnShortcut(focus_on_shortcut);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsManualMode(const bool manual_mode) {
    error err = db()->SetSettingsManualMode(manual_mode);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    trackSettingsUsage();

    return noError;
}

error Context::SetSettingsReminderMinutes(const Poco::UInt64 reminder_minutes) {
    error err = db()->SetSettingsReminderMinutes(reminder_minutes);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    remindToTrackTime();

    trackSettingsUsage();

    return noError;
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

void Context::SetCompactMode(
    const bool value) {
    displayError(db()->SetCompactMode(value));
}

bool Context::GetCompactMode() {
    bool value(false);
    displayError(db()->GetCompactMode(&value));
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

        err = save();
        if (err != noError) {
            return displayError(err);
        }

        trackSettingsUsage();
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

void Context::trackSettingsUsage() {
    if (tracked_settings_.IsSame(settings_)) {
        // settings have not changed, will not track
        return;
    }

    tracked_settings_ = settings_;

    next_analytics_at_ =
        postpone(kRequestThrottleSeconds * kOneSecondInMicros);

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onTrackSettingsUsage);

    Poco::Mutex::ScopedLock lock(timer_m_);

    timer_.schedule(ptask, next_analytics_at_);
}

void Context::onTrackSettingsUsage(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_analytics_at_,
                    (kRequestThrottleSeconds * kOneSecondInMicros))) {
        return;
    }

    std::string update_channel("");
    std::string desktop_id("");
    if (db_) {
        error err = UpdateChannel(&update_channel);
        if (err != noError) {
            logger().error(err);
            return;
        }
        err = db_->EnsureDesktopID();
        if (err != noError) {
            logger().error(err);
            return;
        }
        desktop_id = db_->DesktopID();
    }

    std::string apitoken("");
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return;
        }
        apitoken = user_->APIToken();
    }

    analytics_.TrackSettingsUsage(
        apitoken,
        tracked_settings_,
        update_channel,
        desktop_id);
}

error Context::Signup(
    const std::string email,
    const std::string password) {

    TogglClient client(UI());
    std::string json("");
    error err = signup(&client, email, password, &json);
    if (kBadRequestError == err) {
        return displayError(kCheckYourSignupError);
    }
    if (err != noError) {
        return displayError(err);
    }

    return SetLoggedInUserFromJSON(json);
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

    remindToTrackTime();

    displayError(offerBetaChannel());
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

    return displayError(save());
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
    const std::string tags) {

    // Do not even allow to add new time entries,
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        displayError(kUnsupportedAppError);
        return nullptr;
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot start tracking, user logged out");
            return nullptr;
        }

        // Check if there's a default TID set
        Poco::UInt64 tid(task_id);
        if (!tid) {
            tid = user_->DefaultTID();
        }

        // Check if there's a default PID set
        Poco::UInt64 pid(project_id);
        if (!pid && project_guid.empty()) {
            pid = user_->DefaultPID();
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

    // just show the app
    UI()->DisplayApp();

    if ("production" == environment_) {
        analytics_.TrackAutocompleteUsage(db_->AnalyticsClientID(),
                                          task_id || project_id);
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
    if (UI()->TimeEntryEditorGUID() == te->GUID()) {
        render.open_time_entry_editor = false;
        render.display_time_entry_editor = false;
        render.time_entry_editor_guid = "";
        render.time_entry_editor_field = "";

        render.open_time_entry_list = true;
        render.display_time_entries = true;
    }

    updateUI(render);
}

TimeEntry *Context::ContinueLatest() {
    // Do not even allow to continue entries,
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        displayError(kUnsupportedAppError);
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

    if (settings_.focus_on_shortcut) {
        UI()->DisplayApp();
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

    if (settings_.focus_on_shortcut) {
        UI()->DisplayApp();
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
    }

    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return noError;
    }
    if (te->DeletedAt()) {
        return displayError(kCannotDeleteDeletedTimeEntry);
    }
    if (te->IsTracking()) {
        error err = Stop();
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

        Project *p = nullptr;
        if (project_id) {
            p = user_->related.ProjectByID(project_id);
        }
        if (!project_guid.empty()) {
            p = user_->related.ProjectByGUID(project_guid);
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

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot change date, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);
    }

    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return noError;
    }

    Poco::LocalDateTime loco(
        Poco::Timestamp::fromEpochTime(unix_timestamp));

    Poco::LocalDateTime date_part(
        Poco::Timestamp::fromEpochTime(unix_timestamp));

    Poco::LocalDateTime time_part(
        Poco::Timestamp::fromEpochTime(te->Start()));

    Poco::LocalDateTime dt(
        date_part.year(), date_part.month(), date_part.day(),
        time_part.hour(), time_part.minute(), time_part.second());

    std::string s = Poco::DateTimeFormatter::format(
        dt,
        Poco::DateTimeFormat::ISO8601_FORMAT);

    te->SetStartUserInput(s);

    return displayError(save());
}

error Context::SetTimeEntryStart(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot change start time, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);
    }

    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return noError;
    }

    Poco::LocalDateTime local(Poco::Timestamp::fromEpochTime(te->Start()));

    int hours(0), minutes(0);
    if (!toggl::Formatter::ParseTimeInput(value, &hours, &minutes)) {
        return error("invalid time format");
    }

    Poco::LocalDateTime dt(
        local.year(), local.month(), local.day(),
        hours, minutes, local.second());

    std::string s = Poco::DateTimeFormatter::format(
        dt, Poco::DateTimeFormat::ISO8601_FORMAT);

    te->SetStartUserInput(s);

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
    }

    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return noError;
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
    }

    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return noError;
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
    }

    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return noError;
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
    }

    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return noError;
    }
    te->SetDescription(value);

    if (te->Dirty()) {
        te->ClearValidationError();
        te->SetUIModified();
    }

    return displayError(save());
}

error Context::Stop() {
    std::vector<TimeEntry *> stopped;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot stop tracking, user logged out");
            return noError;
        }
        user_->Stop(&stopped);
    }

    if (stopped.empty()) {
        logger().warning("No time entry was found to stop");
        return noError;
    }

    if (settings_.focus_on_shortcut) {
        UI()->DisplayApp();
    }

    return displayError(save());
}

error Context::DiscardTimeAt(
    const std::string guid,
    const Poco::Int64 at,
    const bool split_into_new_entry) {

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
    trackSettingsUsage();
    return noError;
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
        *name = Formatter::JoinTaskName(t, p, nullptr);
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
    const bool is_private) {

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
            if ((p->Name() == trimmed_project_name)
                    && (workspace_id == p->WID())
                    && (client_id == p->CID())) {
                displayError(kProjectNameAlreadyExists);
                return nullptr;
            }
        }
        result = user_->CreateProject(
            workspace_id,
            client_id,
            client_guid,
            trimmed_project_name,
            is_private);
    }

    err = save();
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    return result;
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
            if (c->Name() == trimmed_client_name) {
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
    logger().debug("SetSleep");
    idle_.SetSleep();
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

    std::string response_body("");
    TogglClient client(UI());
    error err = client.Post(urls::API(),
                            "/api/v8/desktop_login_tokens",
                            "{}",
                            apitoken,
                            "api_token",
                            &response_body);
    if (err != noError) {
        return displayError(err);
    }
    if (response_body.empty()) {
        return displayError("Unexpected empty response from API");
    }

    std::string login_token("");
    err = User::LoginToken(response_body, &login_token);
    if (err != noError) {
        return displayError(err);
    }

    if (login_token.empty()) {
        return displayError("Could not extract login token from JSON");
    }

    std::stringstream ss;
    ss  << urls::API() << "/api/v8/desktop_login"
        << "?login_token=" << login_token
        << "&goto=reports";
    UI()->DisplayURL(ss.str());

    return noError;
}

error Context::offerBetaChannel() {
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
        scheduleSync();

        Poco::LocalDateTime now;
        if (now.year() != last_time_entry_list_render_at_.year()
                || now.month() != last_time_entry_list_render_at_.month()
                || now.day() != last_time_entry_list_render_at_.day()) {
            UIElements render;
            render.display_time_entries = true;
            updateUI(render);
        }

        idle_.SetWake(user_);
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

void Context::remindToTrackTime() {
    if (!settings_.reminder) {
        logger().debug("Reminder is not enabled by user");
        return;
    }

    next_reminder_at_ =
        postpone((settings_.reminder_minutes * 60) * kOneSecondInMicros);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onRemind);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_reminder_at_);

    std::stringstream ss;
    ss << "Next reminder to track time at "
       << Formatter::Format8601(next_reminder_at_);
    logger().debug(ss.str());
}

void Context::onRemind(Poco::Util::TimerTask& task) {  // NOLINT
    // if some code scheduled a reminder for a later time,
    // meanwhile, then let the later reminder be executed
    // not this one.
    if (isPostponed(next_reminder_at_,
                    (settings_.reminder_minutes * 60) * kOneSecondInMicros)) {
        logger().debug("onRemind postponed");
        return;
    }

    displayReminder();

    remindToTrackTime();
}

void Context::displayReminder() {
    if (!settings_.reminder) {
        logger().debug("Reminder is not enabled by user");
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("User logged out, cannot remind");
            return;
        }

        if (user_ && user_->RunningTimeEntry()) {
            logger().debug("User is already tracking time, no need to remind");
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
                logger().debug("its too early for reminders");
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
                logger().debug("its too late for reminders");
                return;
            }
        }
    }

    UI()->DisplayReminder();
}

error Context::StartAutotrackerEvent(const TimelineEvent event) {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        return noError;
    }

    if (!settings_.autotrack) {
        return noError;
    }

    // Update the autotracker titles
    if (event.Title().size()) {
        autotracker_titles_.insert(event.Title());
        UIElements render;
        render.display_autotracker_rules = true;
        updateUI(render);
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
        error err = save();
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
            return displayError(save());
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
        return displayError(save());
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

    if (kPromotionJoinBetaChannel != promotion_type) {
        return error("bad promotion type");
    }

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
            user_->LoadUserAndRelatedDataFromJSONString(user_data_json, !since);
        }

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

        std::string json("");

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
            if (time_entries.empty() && projects.empty() && clients.empty()) {
                *had_something_to_push = false;
                return noError;
            }

            error err = user_->UpdateJSON(
                &clients,
                &projects,
                &time_entries,
                &json);
            if (err != noError) {
                return err;
            }
        }

        logger().debug(json);

        std::string response_body("");
        error err = toggl_client->Post(urls::API(),
                                       "/api/v8/batch_updates",
                                       json,
                                       api_token,
                                       "api_token",
                                       &response_body);
        if (err != noError) {
            return err;
        }

        std::vector<BatchUpdateResult> results;
        err = BatchUpdateResult::ParseResponseArray(response_body, &results);
        if (err != noError) {
            return err;
        }

        std::vector<error> errors;

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            BatchUpdateResult::ProcessResponseArray(&results, &models, &errors);
        }

        if (!errors.empty()) {
            return Formatter::CollectErrors(&errors);
        }

        stopwatch.stop();
        std::stringstream ss;
        ss << "Changes data JSON pushed and responses parsed in "
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

        return toggl_client->Get(urls::API(),
                                 ss.str(),
                                 email,
                                 password,
                                 user_data_json);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
}

error Context::signup(
    TogglClient *toggl_client,
    const std::string email,
    const std::string password,
    std::string *user_data_json) {

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

        Json::Value root;
        root["user"] = user;

        return toggl_client->Post(urls::API(),
                                  "/api/v8/signups",
                                  Json::StyledWriter().write(root),
                                  "",
                                  "",
                                  user_data_json);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
}

void Context::OpenTimelineDataView() {
    logger().debug("OpenTimelineDataView");

    UI()->SetTimelineDateAt(Poco::LocalDateTime());

    UIElements render;
    render.open_timeline = true;
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

}  // namespace toggl
