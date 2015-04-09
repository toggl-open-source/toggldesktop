
// Copyright 2014 Toggl Desktop developers

// No exceptions should be thrown from this class.
// If pointers to models are returned from this
// class, the ownership does not change and you
// must not delete the pointers you got.

#include "../src/context.h"

#include <iostream>  // NOLINT

#include "./const.h"
#include "./database.h"
#include "./error.h"
#include "./formatter.h"
#include "./https_client.h"
#include "./project.h"
#include "./settings.h"
#include "./time_entry.h"
#include "./timeline_uploader.h"
#include "./toggl_api_private.h"
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
#include "Poco/Util/TimerTask.h"
#include "Poco/Util/TimerTaskAdapter.h"

namespace toggl {

std::string Context::log_path_ = "";

Context::Context(const std::string app_name, const std::string app_version)
    : db_(0)
, user_(0)
, timeline_uploader_(0)
, window_change_recorder_(0)
, feedback_("", "", "")
, next_sync_at_(0)
, next_push_changes_at_(0)
, next_fetch_updates_at_(0)
, next_update_timeline_settings_at_(0)
, next_reminder_at_(0)
, time_entry_editor_guid_("")
, environment_("production")
, idle_(&ui_)
, last_sync_started_(0)
, sync_interval_seconds_(0)
, update_check_disabled_(false)
, quit_(false)
, ui_updater_(this, &Context::uiUpdaterActivity)
, update_path_("")
, im_a_teapot_(false) {
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
            window_change_recorder_ = 0;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            delete timeline_uploader_;
            timeline_uploader_ = 0;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(db_m_);
        if (db_) {
            delete db_;
            db_ = 0;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (user_) {
            delete user_;
            user_ = 0;
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

_Bool Context::StartEvents() {
    logger().debug("StartEvents");

    if (user_) {
        return displayError("Cannot start UI, user already logged in!");
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

    if (!DisplaySettings(false)) {
        return false;
    }

    // See if user was logged in into app previously
    User *user = new User();
    err = db()->LoadCurrentUser(user);
    if (err != noError) {
        delete user;
        setUser(0);
        return displayError(err);
    }
    if (!user->ID()) {
        delete user;
        setUser(0);
        return true;
    }
    setUser(user);

    displayUI();

    return true;
}

void Context::displayUI() {
    displayTimerState();
    displayWorkspaceSelect();
    displayClientSelect();
    displayTags();
    displayTimeEntryAutocomplete();
    displayMinitimerAutocomplete();
    displayProjectAutocomplete();
}

error Context::save(const bool push_changes) {
    logger().debug("save");

    std::vector<ModelChange> changes;
    error err = db()->SaveUser(user_, true, &changes);
    if (err != noError) {
        return err;
    }

    updateUI(&changes);

    if (push_changes) {
        pushChanges();
    }

    // Display number of unsynced time entries
    Poco::Int64 count(0);
    if (user_) {
        for (std::vector<TimeEntry *>::const_iterator it =
            user_->related.TimeEntries.begin();
                it != user_->related.TimeEntries.end(); it++) {
            TimeEntry *te = *it;
            if (te->NeedsPush()) {
                count++;
            }
        }
    }
    UI()->DisplayUnsyncedItems(count);

    return noError;
}

void Context::updateUI(std::vector<ModelChange> *changes) {
    // Assume nothing needs to be updated
    bool display_time_entries(false);
    bool display_time_entry_autocomplete(false);
    bool display_mini_timer_autocomplete(false);
    bool display_project_autocomplete(false);
    bool display_client_select(false);
    bool display_tags(false);
    bool display_workspace_select(false);
    bool display_timer_state(false);
    bool display_time_entry_editor(false);
    bool open_time_entry_list(false);

    // Check what needs to be updated in UI
    for (std::vector<ModelChange>::const_iterator it =
        changes->begin();
            it != changes->end();
            it++) {
        ModelChange ch = *it;

        if (ch.ModelType() == "tag") {
            display_tags = true;
        }

        if (ch.ModelType() != "tag" && ch.ModelType() != "user") {
            display_time_entry_autocomplete = true;
            display_time_entries = true;
            display_mini_timer_autocomplete = true;
        }

        if (ch.ModelType() != "tag" && ch.ModelType() != "user"
                && ch.ModelType() != "time_entry") {
            display_project_autocomplete = true;
        }

        if (ch.ModelType() == "client" || ch.ModelType() == "workspace") {
            display_client_select = true;
        }

        // Check if time entry editor needs to be updated
        if (ch.ModelType() == "time_entry") {
            display_timer_state = true;
            // If time entry was edited, check further
            if (time_entry_editor_guid_ == ch.GUID()) {
                // If time entry was deleted, close editor and open list view
                if (ch.ChangeType() == "delete") {
                    open_time_entry_list = true;
                    display_time_entries = true;
                } else {
                    display_time_entry_editor = true;
                }
            }
        }
    }

    // Apply updates to UI
    if (display_time_entry_editor) {
        TimeEntry *te = 0;
        if (user_) {
            te = user_->related.TimeEntryByGUID(time_entry_editor_guid_);
        }
        if (te) {
            displayTimeEntryEditor(false, te, "");
        }
    }
    if (display_time_entries) {
        DisplayTimeEntryList(open_time_entry_list);
    }
    if (display_time_entry_autocomplete) {
        displayTimeEntryAutocomplete();
    }
    if (display_mini_timer_autocomplete) {
        displayMinitimerAutocomplete();
    }
    if (display_project_autocomplete) {
        displayProjectAutocomplete();
    }
    if (display_workspace_select) {
        displayWorkspaceSelect();
    }
    if (display_client_select) {
        displayClientSelect();
    }
    if (display_tags) {
        displayTags();
    }
    if (display_timer_state) {
        displayTimerState();
    }
}

void Context::displayTimeEntryAutocomplete() {
    if (user_) {
        std::vector<AutocompleteItem> list =
            user_->related.TimeEntryAutocompleteItems();
        UI()->DisplayTimeEntryAutocomplete(&list);
    }
}

void Context::displayMinitimerAutocomplete() {
    if (user_) {
        std::vector<AutocompleteItem> list =
            user_->related.MinitimerAutocompleteItems();
        UI()->DisplayMinitimerAutocomplete(&list);
    }
}

void Context::displayProjectAutocomplete() {
    if (user_) {
        std::vector<AutocompleteItem> list =
            user_->related.ProjectAutocompleteItems();
        UI()->DisplayProjectAutocomplete(&list);
    }
}

void Context::displayClientSelect() {
    if (user_) {
        std::vector<Client *> list =
            user_->related.ClientList();
        UI()->DisplayClientSelect(&list);
    }
}

void Context::displayWorkspaceSelect() {
    std::vector<Workspace *> list =
        user_->related.WorkspaceList();
    UI()->DisplayWorkspaceSelect(&list);
}

void Context::displayTags() {
    if (user_) {
        std::vector<std::string> list = user_->related.TagList();
        UI()->DisplayTags(&list);
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

_Bool Context::displayError(const error err) {
    if ((err.find(kForbiddenError) != std::string::npos)
            || (err.find(kUnauthorizedError) != std::string::npos)) {
        if (user_) {
            setUser(0);
        }
    }
    if (err.find(kUnsupportedAppError) != std::string::npos) {
        im_a_teapot_ = true;
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
    if (im_a_teapot_) {
        displayError(kUnsupportedAppError);
        return;
    }

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

    if ("test" == environment_) {
        logger().debug(kCannotSyncInTestEnv);
        return;
    }

    if (im_a_teapot_) {
        displayError(kUnsupportedAppError);
        return;
    }

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

    if (!user_) {
        logger().warning("User is not logged in, cannot sync yet");
        return;
    }

    if (im_a_teapot_) {
        displayError(kUnsupportedAppError);
        return;
    }

    TogglClient client(UI());
    error err = user_->PullAllUserData(&client);
    if (err != noError) {
        displayError(err);
        return;
    }

    setOnline("Data pulled");

    bool had_something_to_push(true);
    err = user_->PushChanges(&client, &had_something_to_push);
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

void Context::pushChanges() {
    logger().debug("pushChanges");

    if ("test" == environment_) {
        logger().debug(kCannotSyncInTestEnv);
        return;
    }

    if (im_a_teapot_) {
        displayError(kUnsupportedAppError);
        return;
    }

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

void Context::onPushChanges(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_push_changes_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger().debug("onPushChanges postponed");
        return;
    }
    logger().debug("onPushChanges executing");

    if (im_a_teapot_) {
        displayError(kUnsupportedAppError);
        return;
    }

    TogglClient client(UI());
    bool had_something_to_push(true);
    error err = user_->PushChanges(&client, &had_something_to_push);
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

_Bool Context::LoadUpdateFromJSONString(const std::string json) {
    std::stringstream ss;
    ss << "LoadUpdateFromJSONString json=" << json;
    logger().debug(ss.str());

    if (!user_) {
        return false;
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

    if (user_->APIToken().empty()) {
        logger().error("No API token, cannot switch Websocket on");
        return;
    }

    Poco::Mutex::ScopedLock lock(ws_client_m_);
    ws_client_.Start(this, user_->APIToken(), on_websocket_message);
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
        Poco::Mutex::ScopedLock lock(window_change_recorder_m_);
        if (window_change_recorder_) {
            delete window_change_recorder_;
            window_change_recorder_ = 0;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            delete timeline_uploader_;
            timeline_uploader_ = 0;
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

    if (!user_) {
        return;
    }

    if (!user_->RecordTimeline()) {
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            delete timeline_uploader_;
            timeline_uploader_ = 0;
        }
        timeline_uploader_ = new TimelineUploader(this);
    }

    {
        Poco::Mutex::ScopedLock lock(window_change_recorder_m_);
        if (window_change_recorder_) {
            delete window_change_recorder_;
            window_change_recorder_ = 0;
        }
        window_change_recorder_ = new WindowChangeRecorder(this);
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

    if (ws_client_.Up()) {
        logger().debug("Skipping periodic sync, because Websocket is up");
    } else {
        scheduleSync();
    }

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

_Bool Context::UpdateChannel(
    std::string *update_channel) {
    poco_check_ptr(update_channel);

    error err = db()->LoadUpdateChannel(update_channel);
    if (err != noError) {
        return displayError(err);
    }
    return true;
}

std::string Context::UserFullName() const {
    if (!user_) {
        return "";
    }
    return user_->Fullname();
}

std::string Context::UserEmail() const {
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
        {
            std::string body("");
            TogglClient client;
            err = client.Get(kAPIURL,
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

            std::stringstream ss;
            ss << "Found update " << root["version"].asString()
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
        // only windows .exe installers ar supported atm
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
    if (im_a_teapot_) {
        displayError(kUnsupportedAppError);
        return;
    }

    if (isPostponed(next_update_timeline_settings_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger().debug("onTimelineUpdateServerSettings postponed");
        return;
    }

    logger().debug("onTimelineUpdateServerSettings executing");

    std::string json(kRecordTimelineDisabledJSON);
    if (user_->RecordTimeline()) {
        json = kRecordTimelineEnabledJSON;
    }

    std::string response_body("");
    TogglClient client(UI());
    error err = client.Post(kTimelineUploadURL,
                            "/api/v8/timeline_settings",
                            json,
                            user_->APIToken(),
                            "api_token",
                            &response_body);
    if (err != noError) {
        displayError(err);
        logger().error(err);
    }
}

_Bool Context::SendFeedback(Feedback fb) {
    if (!user_) {
        logger().warning("Cannot send feedback, user logged out");
        return true;
    }

    error err = fb.Validate();
    if (err != noError) {
        return displayError(err);
    }

    feedback_ = fb;

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSendFeedback);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());

    return true;
}

void Context::onSendFeedback(Poco::Util::TimerTask& task) {  // NOLINT
    if (im_a_teapot_) {
        displayError(kUnsupportedAppError);
        return;
    }

    logger().debug("onSendFeedback");

    Poco::Net::HTMLForm form;
    form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);

    form.set("desktop", "true");
    form.set("toggl_version", HTTPSClient::Config.AppVersion);
    form.set("details", Formatter::EscapeJSONString(feedback_.Details()));
    form.set("subject", Formatter::EscapeJSONString(feedback_.Subject()));
    form.set("date", Formatter::Format8601(time(0)));

    if (!feedback_.AttachmentPath().empty()) {
        form.addPart("files",
                     new Poco::Net::FilePartSource(feedback_.AttachmentPath()));
    }

    form.addPart("files",
                 new Poco::Net::FilePartSource(log_path_));

    std::string response_body("");
    TogglClient client(UI());
    error err = client.Post(kAPIURL,
                            "/api/v8/feedback/web",
                            "",
                            user_->APIToken(),
                            "api_token",
                            &response_body,
                            &form);
    if (err != noError) {
        displayError(err);
        return;
    }
}

_Bool Context::LoadSettings(Settings *settings) {
    poco_check_ptr(settings);
    return displayError(db()->LoadSettings(settings));
}

_Bool Context::SetSettingsAutodetectProxy(const _Bool autodetect_proxy) {
    error err = db()->SetSettingsAutodetectProxy(autodetect_proxy);
    if (err != noError) {
        return displayError(err);
    }
    return DisplaySettings(false);
}

_Bool Context::SetSettingsUseIdleDetection(const bool use_idle_detection) {
    error err = db()->SetSettingsUseIdleDetection(use_idle_detection);
    if (err != noError) {
        return displayError(err);
    }
    return DisplaySettings(false);
}

_Bool Context::SetSettingsMenubarTimer(const _Bool menubar_timer) {
    error err = db()->SetSettingsMenubarTimer(menubar_timer);
    if (err != noError) {
        return displayError(err);
    }
    return DisplaySettings(false);
}

_Bool Context::SetSettingsMenubarProject(const _Bool
        menubar_project) {
    error err = db()->SetSettingsMenubarProject(menubar_project);
    if (err != noError) {
        return displayError(err);
    }
    return DisplaySettings(false);
}

_Bool Context::SetSettingsDockIcon(const _Bool dock_icon) {
    error err = db()->SetSettingsDockIcon(dock_icon);
    if (err != noError) {
        return displayError(err);
    }
    return DisplaySettings(false);
}

_Bool Context::SetSettingsOnTop(const _Bool on_top) {
    error err = db()->SetSettingsOnTop(on_top);
    if (err != noError) {
        return displayError(err);
    }
    return DisplaySettings(false);
}

_Bool Context::SetSettingsReminder(const _Bool reminder) {
    error err = db()->SetSettingsReminder(reminder);
    if (err != noError) {
        return displayError(err);
    }

    remindToTrackTime();

    return DisplaySettings(false);
}

_Bool Context::SetSettingsIdleMinutes(const Poco::UInt64 idle_minutes) {
    error err = db()->SetSettingsIdleMinutes(idle_minutes);
    if (err != noError) {
        return displayError(err);
    }
    return DisplaySettings(false);
}

_Bool Context::SetSettingsFocusOnShortcut(const _Bool focus_on_shortcut) {
    error err = db()->SetSettingsFocusOnShortcut(focus_on_shortcut);
    if (err != noError) {
        return displayError(err);
    }
    return DisplaySettings(false);
}

_Bool Context::SetSettingsManualMode(const _Bool manual_mode) {
    error err = db()->SetSettingsManualMode(manual_mode);
    if (err != noError) {
        return displayError(err);
    }
    return DisplaySettings(false);
}

_Bool Context::SetSettingsReminderMinutes(const Poco::UInt64 reminder_minutes) {
    error err = db()->SetSettingsReminderMinutes(reminder_minutes);
    if (err != noError) {
        return displayError(err);
    }

    remindToTrackTime();

    return DisplaySettings(false);
}

_Bool Context::LoadWindowSettings(
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

_Bool Context::SaveWindowSettings(
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

_Bool Context::SetProxySettings(
    const _Bool use_proxy,
    const Proxy proxy) {

    _Bool was_using_proxy(false);
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

    if (!DisplaySettings(false)) {
        return false;
    }

    if (!user_) {
        return true;
    }

    if (use_proxy != was_using_proxy
            || proxy.Host() != previous_proxy_settings.Host()
            || proxy.Port() != previous_proxy_settings.Port()
            || proxy.Username() != previous_proxy_settings.Username()
            || proxy.Password() != previous_proxy_settings.Password()) {
        Sync();
        switchWebSocketOn();
    }

    return true;
}

void Context::displayTimerState() {
    if (!user_) {
        UI()->DisplayTimerState(0);
        return;
    }

    TimeEntry *te = user_->RunningTimeEntry();
    TogglTimeEntryView *view = timeEntryViewItem(te);
    UI()->DisplayTimerState(view);
    time_entry_view_item_clear(view);
}

TogglTimeEntryView *Context::timeEntryViewItem(TimeEntry *te) {
    if (!te) {
        return 0;
    }

    std::string workspace_name("");
    std::string project_and_task_label("");
    std::string task_label("");
    std::string project_label("");
    std::string client_label("");
    std::string color("");
    user_->related.ProjectLabelAndColorCode(te,
                                            &workspace_name,
                                            &project_and_task_label,
                                            &task_label,
                                            &project_label,
                                            &client_label,
                                            &color);

    Poco::Int64 duration = totalDurationForDate(te);
    std::string date_duration =
        Formatter::FormatDurationForDateHeader(duration);

    return time_entry_view_item_init(te,
                                     workspace_name,
                                     project_and_task_label,
                                     task_label,
                                     project_label,
                                     client_label,
                                     color,
                                     date_duration,
                                     true);
}

_Bool Context::DisplaySettings(const _Bool open) {
    Settings settings;

    error err = db()->LoadSettings(&settings);
    if (err != noError) {
        setUser(0);
        return displayError(err);
    }

    bool use_proxy(false);
    Proxy proxy;
    err = db()->LoadProxySettings(&use_proxy, &proxy);
    if (err != noError) {
        setUser(0);
        return displayError(err);
    }

    bool record_timeline(false);
    if (user_) {
        record_timeline = user_->RecordTimeline();
    }

    idle_.SetSettings(settings);

    HTTPSClient::Config.UseProxy = use_proxy;
    HTTPSClient::Config.IgnoreCert = false;
    HTTPSClient::Config.ProxySettings = proxy;
    HTTPSClient::Config.AutodetectProxy = settings.autodetect_proxy;

    UI()->DisplaySettings(open,
                          record_timeline,
                          settings,
                          use_proxy,
                          proxy);

    return true;
}

_Bool Context::SetDBPath(
    const std::string path) {
    try {
        std::stringstream ss;
        ss << "SetDBPath " << path;
        logger().debug(ss.str());

        Poco::Mutex::ScopedLock lock(db_m_);
        if (db_) {
            logger().debug("delete db_ from SetDBPath()");
            delete db_;
            db_ = 0;
        }
        db_ = new Database(path);
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return true;
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
}

Database *Context::db() const {
    poco_check_ptr(db_);
    return db_;
}

_Bool Context::GoogleLogin(const std::string access_token) {
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

    displayUI();

    return save();
}

_Bool Context::Login(
    const std::string email,
    const std::string password) {

    if (im_a_teapot_) {
        return displayError(kUnsupportedAppError);
    }

    TogglClient client(UI());
    std::string user_data_json("");
    error err = User::Me(&client, email, password, &user_data_json, 0);
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

    if (!SetLoggedInUserFromJSON(user_data_json)) {
        return false;
    }

    if (!user_) {
        logger().error("cannot enable offline login, no user");
        return true;
    }

    err = user_->EnableOfflineLogin(password);
    if (err != noError) {
        return displayError(err);
    }

    return displayError(save());
}

_Bool Context::Signup(
    const std::string email,
    const std::string password) {

    if (im_a_teapot_) {
        return displayError(kUnsupportedAppError);
    }

    TogglClient client(UI());
    std::string user_data_json("");
    error err = User::Signup(&client, email, password, &user_data_json);
    if (kBadRequestError == err) {
        return displayError(kCheckYourSignupError);
    }
    if (err != noError) {
        return displayError(err);
    }

    return SetLoggedInUserFromJSON(user_data_json);
}

void Context::setUser(User *value, const bool user_logged_in) {
    {
        std::stringstream ss;
        ss << "setUser user_logged_in=" << user_logged_in;
        logger().debug(ss.str());
    }

    Poco::Mutex::ScopedLock lock(user_m_);
    if (user_) {
        delete user_;
    }
    user_ = value;

    if (quit_) {
        return;
    }

    if (!user_) {
        UI()->DisplayLogin(true, 0);

        switchTimelineOff();
        switchWebSocketOff();

        return;
    }

    UI()->DisplayLogin(false, user_->ID());

    DisplayTimeEntryList(true);

    switchTimelineOn();
    switchWebSocketOn();

    if (!user_logged_in) {
        Sync();
    }

    fetchUpdates();

    if (!ui_updater_.isRunning()) {
        ui_updater_.start();
    }

    remindToTrackTime();
}

_Bool Context::SetLoggedInUserFromJSON(
    const std::string user_data_json) {

    if (user_data_json.empty()) {
        return false;
    }

    Poco::UInt64 userID(0);
    error err = User::UserID(user_data_json, &userID);
    if (err != noError) {
        return displayError(err);
    }

    if (!userID) {
        return false;
    }

    User *user = new User();

    err = db()->LoadUserByID(userID, user);
    if (err != noError) {
        delete user;
        return displayError(err);
    }

    err = user->LoadUserAndRelatedDataFromJSONString(user_data_json, true);
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

    displayUI();

    return displayError(save());
}

_Bool Context::Logout() {
    try {
        if (!user_) {
            logger().warning("User is logged out, cannot logout again");
            return true;
        }

        error err = db()->ClearCurrentAPIToken();
        if (err != noError) {
            return displayError(err);
        }

        logger().debug("setUser from Logout");

        setUser(0);

        UI()->DisplayApp();

        Shutdown();
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return true;
}

_Bool Context::ClearCache() {
    try {
        if (!user_) {
            logger().warning("User is logged out, cannot clear cache");
            return true;
        }
        error err = db()->DeleteUser(user_, true);
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
    return true;
}

bool Context::canSeeBillable(
    Workspace *ws) const {
    if (!user_->HasPremiumWorkspaces()) {
        return false;
    }
    if (ws && !ws->Premium()) {
        return false;
    }
    return true;
}

TimeEntry *Context::Start(
    const std::string description,
    const std::string duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id) {

    if (im_a_teapot_) {
        displayError(kUnsupportedAppError);
        return 0;
    }

    if (!user_) {
        logger().warning("Cannot start tracking, user logged out");
        return 0;
    }

    TimeEntry *te = user_->Start(description, duration, task_id, project_id);

    UI()->DisplayApp();

    error err = save();
    if (err != noError) {
        displayError(err);
        return 0;
    }

    if ("production" == environment_) {
        analytics_.TrackAutocompleteUsage(db_->AnalyticsClientID(),
                                          task_id || project_id);
    }

    return te;
}

Poco::Int64 Context::totalDurationForDate(TimeEntry *match) const {
    Poco::Int64 duration(0);
    std::string date_header = match->DateHeaderString();
    std::vector<TimeEntry *> list = timeEntries(true);
    for (unsigned int i = 0; i < list.size(); i++) {
        TimeEntry *te = list.at(i);
        if (te->DateHeaderString() == date_header) {
            duration += TimeEntry::AbsDuration(te->DurationInSeconds());
        }
    }
    return duration;
}

void Context::DisplayTimeEntryList(const _Bool open) {
    if (!user_) {
        logger().warning("Cannot view time entries, user logged out");
        return;
    }

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    std::vector<TimeEntry *> list = timeEntries(true);

    std::map<std::string, Poco::Int64> date_durations;
    for (unsigned int i = 0; i < list.size(); i++) {
        TimeEntry *te = list.at(i);

        std::string date_header = te->DateHeaderString();
        Poco::Int64 duration = date_durations[date_header];
        duration += TimeEntry::AbsDuration(te->DurationInSeconds());
        date_durations[date_header] = duration;
    }

    TogglTimeEntryView *first = 0;
    for (unsigned int i = 0; i < list.size(); i++) {
        TimeEntry *te = list.at(i);

        if (te->DurationInSeconds() < 0) {
            // Don't display running entries
            continue;
        }

        std::string workspace_name("");
        std::string project_and_task_label("");
        std::string task_label("");
        std::string project_label("");
        std::string client_label("");
        std::string color("");
        user_->related.ProjectLabelAndColorCode(te,
                                                &workspace_name,
                                                &project_and_task_label,
                                                &task_label,
                                                &project_label,
                                                &client_label,
                                                &color);

        Poco::Int64 duration = date_durations[te->DateHeaderString()];
        std::string date_duration =
            Formatter::FormatDurationForDateHeader(duration);

        TogglTimeEntryView *item =
            time_entry_view_item_init(te,
                                      workspace_name,
                                      project_and_task_label,
                                      task_label,
                                      project_label,
                                      client_label,
                                      color,
                                      date_duration,
                                      false);
        item->Next = first;
        if (first && compare_string(item->DateHeader, first->DateHeader) != 0) {
            first->IsHeader = true;
        }
        first = item;
    }

    if (first) {
        first->IsHeader = true;
    }

    if (open) {
        time_entry_editor_guid_ = "";
    }

    UI()->DisplayTimeEntryList(open, first);
    time_entry_view_item_clear(first);

    last_time_entry_list_render_at_ = Poco::LocalDateTime();

    stopwatch.stop();
    std::stringstream ss;
    ss << "Time entry list rendered in "
       << stopwatch.elapsed() / 1000 << " ms";
    logger().debug(ss.str());
}

void Context::Edit(const std::string GUID,
                   const _Bool edit_running_entry,
                   const std::string focused_field_name) {
    if (!edit_running_entry && GUID.empty()) {
        logger().error("Cannot edit time entry without a GUID");
        return;
    }

    if (!user_) {
        logger().warning("Cannot edit time entry, user logged out");
        return;
    }

    TimeEntry *te = 0;
    if (edit_running_entry) {
        te = user_->RunningTimeEntry();
    } else {
        te = user_->related.TimeEntryByGUID(GUID);
    }

    if (!te) {
        logger().warning("Time entry not found for edit " + GUID);
        return;
    }

    displayTimeEntryEditor(true, te, focused_field_name);
}

void Context::displayTimeEntryEditor(const _Bool open,
                                     TimeEntry *te,
                                     const std::string focused_field_name) {
    poco_check_ptr(te);

    if (open) {
        UI()->DisplayApp();
    }

    // If user is already editing the time entry, toggle the editor
    // instead of doing nothing
    if (open && (time_entry_editor_guid_ == te->GUID())) {
        DisplayTimeEntryList(true);
        return;
    }

    time_entry_editor_guid_ = te->GUID();
    TogglTimeEntryView *view = timeEntryViewItem(te);

    Workspace *ws = 0;
    if (te->WID()) {
        ws = user_->related.WorkspaceByID(te->WID());
    }
    view->CanSeeBillable = canSeeBillable(ws);
    view->DefaultWID = user_->DefaultWID();
    if (ws) {
        view->CanAddProjects = ws->Admin() ||
                               !ws->OnlyAdminsMayCreateProjects();
    } else {
        view->CanAddProjects = user_->CanAddProjects();
    }

    UI()->DisplayTimeEntryEditor(open, view, focused_field_name);

    time_entry_view_item_clear(view);
}

_Bool Context::ContinueLatest() {
    if (im_a_teapot_) {
        return displayError(kUnsupportedAppError);
    }

    if (!user_) {
        logger().warning("Cannot continue tracking, user logged out");
        return true;
    }

    TimeEntry *latest = 0;

    // Find the time entry that was stopped most recently
    for (std::vector<TimeEntry *>::const_iterator it =
        user_->related.TimeEntries.begin();
            it != user_->related.TimeEntries.end(); it++) {
        TimeEntry *te = *it;

        if (te->GUID().empty()) {
            return displayError("Found a time entry without a GUID!");
        }
        if (te->DurationInSeconds() < 0) {
            continue;
        }
        if (te->DeletedAt() > 0) {
            continue;
        }

        if (!latest || (te->Stop() > latest->Stop())) {
            latest = te;
        }
    }

    if (!latest) {
        return true;
    }

    error err = user_->Continue(latest->GUID());
    if (err != noError) {
        return displayError(err);
    }

    Settings settings;
    if (LoadSettings(&settings)) {
        if (settings.focus_on_shortcut) {
            UI()->DisplayApp();
        }
    }

    return displayError(save());
}

_Bool Context::Continue(
    const std::string GUID) {

    if (im_a_teapot_) {
        return displayError(kUnsupportedAppError);
    }

    if (!user_) {
        logger().warning("Cannot continue time entry, user logged out");
        return true;
    }

    if (GUID.empty()) {
        return displayError("Missing GUID");
    }

    error err = user_->Continue(GUID);
    if (err != noError) {
        return displayError(err);
    }

    Settings settings;
    if (LoadSettings(&settings)) {
        if (settings.focus_on_shortcut) {
            UI()->DisplayApp();
        }
    }

    err = save();
    if (err != noError) {
        return displayError(err);
    }

    DisplayTimeEntryList(true);

    return true;
}

_Bool Context::DeleteTimeEntryByGUID(const std::string GUID) {
    if (im_a_teapot_) {
        return displayError(kUnsupportedAppError);
    }
    if (!user_) {
        logger().warning("Cannot delete time entry, user logged out");
        return true;
    }
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }
    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    if (te->DeletedAt()) {
        return displayError(kCannotDeleteDeletedTimeEntry);
    }
    if (te->IsTracking()) {
        if (!Stop()) {
            return false;
        }
    }
    te->ClearValidationError();
    te->Delete();
    return displayError(save());
}

_Bool Context::SetTimeEntryDuration(
    const std::string GUID,
    const std::string duration) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot set duration, user logged out");
        return true;
    }
    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    te->SetDurationUserInput(duration);

    return displayError(save());
}

_Bool Context::SetTimeEntryProject(
    const std::string GUID,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string project_guid) {
    try {
        if (GUID.empty()) {
            return displayError("Missing GUID");
        }
        if (!user_) {
            logger().warning("Cannot set project, user logged out");
            return true;
        }

        TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return true;
        }

        Project *p = 0;
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

_Bool Context::SetTimeEntryDate(
    const std::string GUID,
    const Poco::Int64 unix_timestamp) {

    if (GUID.empty()) {
        return displayError("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot change date, user logged out");
        return true;
    }
    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
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

_Bool Context::SetTimeEntryStart(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot change start time, user logged out");
        return true;
    }
    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }

    Poco::LocalDateTime local(Poco::Timestamp::fromEpochTime(te->Start()));

    int hours(0), minutes(0);
    if (!toggl::Formatter::ParseTimeInput(value, &hours, &minutes)) {
        return false;
    }

    Poco::LocalDateTime dt(
        local.year(), local.month(), local.day(),
        hours, minutes, local.second());

    std::string s = Poco::DateTimeFormatter::format(
        dt, Poco::DateTimeFormat::ISO8601_FORMAT);

    te->SetStartUserInput(s);

    return displayError(save());
}

_Bool Context::SetTimeEntryStop(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot change stop time, user logged out");
        return true;
    }
    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }

    Poco::LocalDateTime stop(
        Poco::Timestamp::fromEpochTime(te->Stop()));

    int hours(0), minutes(0);
    if (!toggl::Formatter::ParseTimeInput(value, &hours, &minutes)) {
        return false;
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

_Bool Context::SetTimeEntryTags(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot set tags, user logged out");
        return true;
    }
    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    te->SetTags(value);

    if (te->Dirty()) {
        te->ClearValidationError();
        te->SetUIModified();
    }

    return displayError(save());
}

_Bool Context::SetTimeEntryBillable(
    const std::string GUID,
    const bool value) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot set billable, user logged out");
        return true;
    }
    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    te->SetBillable(value);

    if (te->Dirty()) {
        te->ClearValidationError();
        te->SetUIModified();
    }

    return displayError(save());
}

_Bool Context::SetTimeEntryDescription(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot set description, user logged out");
        return true;
    }
    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    te->SetDescription(value);

    if (te->Dirty()) {
        te->ClearValidationError();
        te->SetUIModified();
    }

    return displayError(save());
}

_Bool Context::Stop() {
    if (!user_) {
        logger().warning("Cannot stop tracking, user logged out");
        return true;
    }

    std::vector<TimeEntry *> stopped = user_->Stop();
    if (stopped.empty()) {
        logger().warning("No time entry was found to stop");
        return true;
    }

    Settings settings;
    if (LoadSettings(&settings)) {
        if (settings.focus_on_shortcut) {
            UI()->DisplayApp();
        }
    }

    return displayError(save());
}

_Bool Context::DiscardTimeAt(
    const std::string guid,
    const Poco::Int64 at,
    const bool split_into_new_entry) {

    if (!user_) {
        logger().warning("Cannot stop time entry, user logged out");
        return true;
    }

    TimeEntry *split = user_->DiscardTimeAt(guid, at, split_into_new_entry);
    error err = save();
    if (err != noError) {
        return displayError(err);
    }

    if (split_into_new_entry && split) {
        displayTimeEntryEditor(true, split, "");
    }

    return true;
}

_Bool Context::RunningTimeEntry(
    TimeEntry **running) const {
    if (!user_) {
        logger().warning("Cannot fetch time entry, user logged out");
        return true;
    }
    *running = user_->RunningTimeEntry();
    return true;
}

_Bool Context::ToggleTimelineRecording(const _Bool record_timeline) {
    if (!user_) {
        logger().warning("Cannot toggle timeline, user logged out");
        return true;
    }
    try {
        user_->SetRecordTimeline(record_timeline);

        error err = save();
        if (err != noError) {
            return displayError(err);
        }

        TimelineUpdateServerSettings();
        if (user_->RecordTimeline()) {
            switchTimelineOn();
        } else {
            switchTimelineOff();
        }
        return DisplaySettings(false);
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return true;
}

std::vector<TimeEntry *> Context::timeEntries(
    const bool including_running) const {
    std::vector<TimeEntry *> result;
    if (!user_) {
        return result;
    }

    // Collect visible time entries
    for (std::vector<TimeEntry *>::const_iterator it =
        user_->related.TimeEntries.begin();
            it != user_->related.TimeEntries.end(); it++) {
        TimeEntry *te = *it;

        if (te->GUID().empty()) {
            logger().error("Ignoring a time entry without a GUID!");
            continue;
        }
        if (te->DurationInSeconds() < 0 && !including_running) {
            continue;
        }
        if (te->DeletedAt() > 0) {
            continue;
        }
        result.push_back(te);
    }

    std::sort(result.begin(), result.end(), CompareTimeEntriesByStart);

    return result;
}

_Bool Context::SaveUpdateChannel(const std::string channel) {
    error err = db()->SaveUpdateChannel(channel);
    if (err != noError) {
        return displayError(err);
    }
    fetchUpdates();
    return true;
}

_Bool Context::CreateProject(
    const Poco::UInt64 workspace_id,
    const Poco::UInt64 client_id,
    const std::string project_name,
    const _Bool is_private,
    Project **result) {

    poco_check_ptr(result);

    if (!user_) {
        logger().warning("Cannot add project, user logged out");
        return true;
    }
    if (!workspace_id) {
        return displayError("Please select a workspace");
    }
    if (project_name.empty()) {
        return displayError("Project name must not be empty");
    }

    *result = user_->CreateProject(
        workspace_id, client_id, project_name, is_private);

    return displayError(save());
}

_Bool Context::CreateClient(
    const Poco::UInt64 workspace_id,
    const std::string client_name) {

    if (!user_) {
        logger().warning("Cannot create a client, user logged out");
        return true;
    }
    if (!workspace_id) {
        return displayError("Please select a workspace");
    }
    if (client_name.empty()) {
        return displayError("Client name must not be empty");
    }

    user_->CreateClient(workspace_id, client_name);

    return displayError(save());
}

void Context::SetSleep() {
    logger().debug("SetSleep");
    idle_.SetSleep();
}

_Bool Context::OpenReportsInBrowser() {
    if (im_a_teapot_) {
        return displayError(kUnsupportedAppError);
    }

    if (!user_) {
        return displayError("You must log in to view reports");
    }

    std::string response_body("");
    TogglClient client(UI());
    error err = client.Post(kAPIURL,
                            "/api/v8/desktop_login_tokens",
                            "{}",
                            user_->APIToken(),
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
    ss  << kAPIURL << "/api/v8/desktop_login"
        << "?login_token=" << login_token
        << "&goto=reports";
    UI()->DisplayURL(ss.str());

    return true;
}

void Context::SetWake() {
    logger().debug("SetWake");

    try {
        remindToTrackTime();

        scheduleSync();

        if (user_) {
            Poco::LocalDateTime now;
            if (now.year() != last_time_entry_list_render_at_.year()
                    || now.month() != last_time_entry_list_render_at_.month()
                    || now.day() != last_time_entry_list_render_at_.day()) {
                DisplayTimeEntryList(false);
            }
        }

        idle_.SetWake(user_);
    } catch(const Poco::Exception& exc) {
        logger().error(exc.displayText());
    } catch(const std::exception& ex) {
        logger().error(ex.what());
    } catch(const std::string& ex) {
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
    Settings settings;
    if (!LoadSettings(&settings)) {
        logger().error("Could not load settings");
        return;
    }

    if (!settings.reminder) {
        logger().debug("Reminder is not enabled by user");
        return;
    }

    next_reminder_at_ =
        postpone((settings.reminder_minutes * 60) * kOneSecondInMicros);
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
    Settings settings;
    if (!LoadSettings(&settings)) {
        logger().error("Could not load settings");
        return;
    }

    if (!settings.reminder) {
        logger().debug("Reminder is not enabled by user");
        return;
    }

    if (isPostponed(next_reminder_at_,
                    (settings.reminder_minutes * 60) * kOneSecondInMicros)) {
        logger().debug("onRemind postponed");
        return;
    }
    logger().debug("onRemind executing");

    if (!user_) {
        logger().warning("User logged out, cannot remind");
        return;
    }

    if (user_ && user_->RunningTimeEntry()) {
        logger().debug("User is already tracking time, no need to remind");
        return;
    }

    UI()->DisplayReminder();
}

error Context::CreateTimelineBatch(TimelineBatch *batch) {
    poco_check_ptr(batch);

    if (quit_) {
        return noError;
    }
    if (!user_) {
        return noError;
    }
    std::vector<TimelineEvent> events;
    error err = db()->SelectTimelineBatch(user_->ID(), &events);
    if (err != noError) {
        return err;
    }
    batch->SetEvents(&events);
    batch->SetUserID(user_->ID());
    batch->SetAPIToken(user_->APIToken());
    batch->SetDesktopID(db_->DesktopID());
    return noError;
}

error Context::SaveTimelineEvent(TimelineEvent *event) {
    poco_check_ptr(event);

    if (!user_) {
        return noError;
    }
    event->user_id = static_cast<unsigned int>(user_->ID());
    return db()->InsertTimelineEvent(event);
}

error Context::DeleteTimelineBatch(const std::vector<TimelineEvent> &events) {
    return db()->DeleteTimelineBatch(events);
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

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                continue;
            }
            TimeEntry *te = user_->RunningTimeEntry();
            if (!te) {
                continue;
            }
            Poco::Int64 duration = totalDurationForDate(te);
            std::string date_duration =
                Formatter::FormatDurationForDateHeader(duration);

            if (running_time != date_duration) {
                DisplayTimeEntryList(false);
            }
            running_time = date_duration;
        }
    }
}

void Context::SetLogPath(const std::string path) {
    Poco::AutoPtr<Poco::SimpleFileChannel> simpleFileChannel(
        new Poco::SimpleFileChannel);
    simpleFileChannel->setProperty("path", path);
    simpleFileChannel->setProperty("rotation", "1 M");

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
