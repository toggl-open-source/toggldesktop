
// Copyright 2014 Toggl Desktop developers

// No exceptions should be thrown from this class.
// If pointers to models are returned from this
// class, the ownership does not change and you
// must not delete the pointers you got.

#include "./context.h"

#include <iostream>  // NOLINT

#include "./formatter.h"
#include "./time_entry.h"
#include "./const.h"
#include "./toggl_api_private.h"
#include "./settings.h"
#include "./timeline_notifications.h"

#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Util/TimerTaskAdapter.h"
#include "Poco/Environment.h"
#include "Poco/Timestamp.h"
#include "Poco/Stopwatch.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"

namespace toggl {

Context::Context(const std::string app_name, const std::string app_version)
    : db_(0)
, user_(0)
, ws_client_(0)
, timeline_uploader_(0)
, window_change_recorder_(0)
, timeline_upload_url_("")
, feedback_("", "", "")
, next_sync_at_(0)
, next_push_changes_at_(0)
, next_fetch_updates_at_(0)
, next_update_timeline_settings_at_(0)
, next_reminder_at_(0)
, time_entry_editor_guid_("")
, environment_("production")
, last_idle_seconds_reading_(0)
, last_idle_started_(0)
, idle_minutes_(0)
, last_sync_started_(0)
, update_check_disabled_(false)
, quit_(false) {
    Poco::ErrorHandler::set(&error_handler_);
    Poco::Net::initializeSSL();

    HTTPSClientConfig::AppName = app_name;
    HTTPSClientConfig::AppVersion = app_version;

    Poco::Crypto::OpenSSLInitializer::initialize();

    startPeriodicUpdateCheck();

    startPeriodicSync();

    remindToTrackTime();
}

Context::~Context() {
    SetQuit();

    {
        Poco::Mutex::ScopedLock lock(window_change_recorder_m_);
        if (window_change_recorder_) {
            window_change_recorder_->Shutdown();
            delete window_change_recorder_;
            window_change_recorder_ = 0;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            timeline_uploader_->Shutdown();
            delete timeline_uploader_;
            timeline_uploader_ = 0;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(ws_client_m_);
        if (ws_client_) {
            ws_client_->Shutdown();
            delete ws_client_;
            ws_client_ = 0;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(db_m_);
        if (db_) {
            delete db_;
            db_ = 0;
        }
    }

    logger().debug("setUser from destructor");

    setUser(0);

    Poco::Net::uninitializeSSL();
}

_Bool Context::StartEvents() {
    logger().debug("StartEvents");

    poco_assert(!user_);

    if (HTTPSClientConfig::CACertPath.empty()) {
        return displayError(error("Missing CA cert bundle path!"),
                            "StartEvents");
    }

    // Check that UI is wired up
    error err = UI()->VerifyCallbacks();
    if (err != noError) {
        logger().error(err);
        std::cerr << err << std::endl;
        std::cout << err << std::endl;
        return false;
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
        return displayError(err, "StartEvents");
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
    displayProjectAutocomplete();
}

void Context::Shutdown() {
    {
        Poco::Mutex::ScopedLock lock(window_change_recorder_m_);
        if (window_change_recorder_) {
            window_change_recorder_->Shutdown();
        }
    }

    {
        Poco::Mutex::ScopedLock lock(ws_client_m_);
        if (ws_client_) {
            ws_client_->Shutdown();
        }
    }

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            timeline_uploader_->Shutdown();
        }
    }

    // cancel tasks but allow them finish
    {
        Poco::Mutex::ScopedLock lock(timer_m_);
        timer_.cancel(true);
    }

    Poco::ThreadPool::defaultPool().stopAll();
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

    return noError;
}

void Context::updateUI(std::vector<ModelChange> *changes) {
    bool display_time_entries(false);
    bool display_time_entry_autocomplete(false);
    bool display_project_autocomplete(false);
    bool display_client_select(false);
    bool display_tags(false);
    bool display_workspace_select(false);
    bool display_timer_state(false);
    bool display_time_entry_editor(false);
    bool open_time_entry_list(false);
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
        }
        if (ch.ModelType() != "tag" && ch.ModelType() != "user"
                && ch.ModelType() != "time_entry") {
            display_project_autocomplete = true;
        }
        if (ch.ModelType() == "client" || ch.ModelType() == "workspace") {
            display_client_select = true;
        }
        if (ch.ModelType() == "time_entry") {
            display_timer_state = true;
            // If time entry was edited, check further
            if (time_entry_editor_guid_ == ch.GUID()) {
                // If time entry was deleted, close editor and open list view
                if (ch.ChangeType() == "delete") {
                    time_entry_editor_guid_ = "";
                    open_time_entry_list = true;
                    display_time_entries = true;
                } else {
                    display_time_entry_editor = true;
                }
            }
        }
    }
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
    if (!user_) {
        return;
    }
    std::vector<AutocompleteItem> list = user_->related.AutocompleteItems(true);
    UI()->DisplayTimeEntryAutocomplete(&list);
}

void Context::displayProjectAutocomplete() {
    if (!user_) {
        return;
    }
    std::vector<AutocompleteItem> list =
        user_->related.AutocompleteItems(false);
    UI()->DisplayProjectAutocomplete(&list);
}

void Context::displayClientSelect() {
    std::vector<Client *> list = clients();
    UI()->DisplayClientSelect(&list);
}

void Context::displayWorkspaceSelect() {
    std::vector<Workspace *> list = workspaces();
    UI()->DisplayWorkspaceSelect(&list);
}

void Context::displayTags() {
    std::vector<std::string> list = tags();
    UI()->DisplayTags(&list);
}

Poco::Timestamp Context::postpone(
    const Poco::Timestamp::TimeDiff throttleMicros) {
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
        logger().warning("Cannot postpone task, its foo far in the future");
        return false;
    }
    return true;
}

_Bool Context::displayError(const error err, const std::string calling_method) {
    if (err.find("Request to server failed with status code: 403")
            != std::string::npos) {
        if (!user_) {
            return UI()->DisplayError("Invalid e-mail or password!", "");
        }
        setUser(0);
    }
    return UI()->DisplayError(err, calling_method);
}

void Context::scheduleSync() {
    Poco::Int64 elapsed_seconds = Poco::Int64(time(0)) - last_sync_started_;

    {
        std::stringstream ss;
        ss << "scheduleSync elapsed_seconds=" << elapsed_seconds;
        logger().debug(ss.str());
    }

    if (elapsed_seconds < kPeriodicSyncIntervalSeconds) {
        std::stringstream ss;
        ss << "Last sync attempt less than " << kPeriodicSyncIntervalSeconds
           << " seconds ago, chill";
        logger().trace(ss.str());
        return;
    }

    Sync();
}

void Context::Sync() {
    logger().debug("Sync");

    next_sync_at_ = postpone(kRequestThrottleSeconds * kOneSecondInMicros);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onSync);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_sync_at_);
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

    HTTPSClient client;
    error err = user_->PullAllUserData(&client);
    if (err != noError) {
        displayError(err, "onSync");
        return;
    }

    err = user_->PushChanges(&client);
    if (err != noError) {
        displayError(err, "onSync");
        return;
    }

    err = save(false);
    if (err != noError) {
        displayError(err, "onSync");
        return;
    }

    displayOnlineState("Sync done");
}

void Context::displayOnlineState(const std::string reason) {
    if (quit_) {
        return;
    }
    UI()->DisplayOnlineState(true, reason);
    scheduleSync();
}

void Context::pushChanges() {
    logger().debug("pushChanges");

    next_push_changes_at_ =
        postpone(kRequestThrottleSeconds * kOneSecondInMicros);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onPushChanges);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_push_changes_at_);
}

void Context::onPushChanges(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_push_changes_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger().debug("onPushChanges postponed");
        return;
    }
    logger().debug("onPushChanges executing");

    HTTPSClient client;
    error err = user_->PushChanges(&client);
    if (err != noError) {
        displayError(err, "onPushChanges");
        return;
    }

    err = save(false);
    if (err != noError) {
        displayError(err, "onPushChanges");
        return;
    }

    displayOnlineState("Changes pushed");
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
    if (!ws_client_) {
        return;
    }
    ws_client_->Shutdown();
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

_Bool Context::LoadUpdateFromJSONString(const std::string json) {
    std::stringstream ss;
    ss << "LoadUpdateFromJSONString json=" << json;
    logger().debug(ss.str());

    if (!user_) {
        return false;
    }

    error err = user_->LoadUserUpdateFromJSONString(json);
    if (err != noError) {
        return displayError(err, "LoadUpdateFromJSONString");
    }

    return displayError(save(), "LoadUpdateFromJSONString");
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

    poco_assert(!user_->APIToken().empty());

    Poco::Mutex::ScopedLock lock(ws_client_m_);
    ws_client_->Start(this, user_->APIToken(), on_websocket_message);
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
        timeline_uploader_ = new TimelineUploader(timeline_upload_url_, this);
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

    Poco::Timestamp next_periodic_sync_at_ =
        Poco::Timestamp() + (kPeriodicSyncIntervalSeconds * kOneSecondInMicros);
    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_periodic_sync_at_);
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
        return displayError(err, "UpdateChannel");
    }
    return true;
}

void Context::executeUpdateCheck() {
    logger().debug("executeUpdateCheck");

    if ("production" != environment_) {
        return;
    }

    if (update_check_disabled_) {
        return;
    }

    std::string update_channel("");
    error err = db()->LoadUpdateChannel(&update_channel);
    if (err != noError) {
        displayError(err, "executeUpdateCheck");
        return;
    }
    UI()->DisplayUpdate(false, update_channel, true, false, "", "");

    std::string response_body("");
    HTTPSClient https_client;
    err = https_client.GetJSON(updateURL(),
                               std::string(""),
                               std::string(""),
                               &response_body);
    if (err != noError) {
        displayError(err, "executeUpdateCheck");
        return;
    }

    if ("null" == response_body) {
        UI()->DisplayUpdate(false, update_channel, false, false, "", "");
        return;
    }

    Json::Value root;
    Json::Reader reader;
    bool ok = reader.parse(response_body, root);
    if (!ok) {
        displayError(error("Error parsing update check response body"),
                     "executeUpdateCheck");
        return;
    }

    std::string url = root["url"].asString();
    std::string version = root["version"].asString();

    UI()->DisplayUpdate(false, update_channel, false, true, url, version);
}

const std::string Context::updateURL() {
    std::string update_channel("");
    error err = db()->LoadUpdateChannel(&update_channel);
    if (err != noError) {
        displayError(err, "updateURL");
        return "";
    }

    poco_assert(!HTTPSClientConfig::AppVersion.empty());

    std::stringstream relative_url;
    relative_url << "/api/v8/updates?app=td"
                 << "&channel=" << update_channel
                 << "&platform=" << installerPlatform()
                 << "&version=" << HTTPSClientConfig::AppVersion
                 << "&osname=" << Poco::Environment::osName()
                 << "&osversion=" << Poco::Environment::osVersion()
                 << "&osarch=" << Poco::Environment::osArchitecture();
    return relative_url.str();
}

const std::string Context::installerPlatform() {
    if (POCO_OS_LINUX == POCO_OS) {
        return linuxPlatformName();
    }
    if (POCO_OS_WINDOWS_NT == POCO_OS) {
        return std::string("windows");
    }
    return std::string("darwin");
}

const std::string Context::linuxPlatformName() {
    Poco::File f("/opt/toggldesktop/platform");
    if (f.exists() && f.isFile() && f.canRead()) {
        Poco::FileInputStream fis(f.path());
        if (fis.good()) {
            std::string platform("");
            fis >> platform;
            return platform;
        }
    }
    std::string arch = Poco::Environment::osArchitecture();
    if (arch == "i386" || arch == "i686") {
        return std::string("linux32");
    }
    return std::string("linux");
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

    std::string json(kRecordTimelineDisabledJSON);
    if (user_->RecordTimeline()) {
        json = kRecordTimelineEnabledJSON;
    }

    std::string response_body("");
    HTTPSClient https_client;
    error err = https_client.PostJSON("/api/v8/timeline_settings",
                                      json,
                                      user_->APIToken(),
                                      "api_token",
                                      &response_body);
    if (err != noError) {
        logger().warning(err);
    }
}

_Bool Context::SendFeedback(Feedback fb) {
    if (!user_) {
        logger().warning("Cannot send feedback, user logged out");
        return true;
    }

    error err = fb.Validate();
    if (err != noError) {
        return displayError(err, "SendFeedback");
    }

    feedback_ = fb;

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSendFeedback);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());

    return true;
};

void Context::onSendFeedback(Poco::Util::TimerTask& task) {  // NOLINT
    logger().debug("onSendFeedback");

    std::string response_body("");
    HTTPSClient https_client;
    error err = https_client.PostJSON("/api/v8/feedback",
                                      feedback_.JSON(),
                                      user_->APIToken(),
                                      "api_token",
                                      &response_body);
    if (err != noError) {
        displayError(err, "onSendFeedback");
        return;
    }
}

void Context::SetWebSocketClientURL(const std::string value) {
    Poco::Mutex::ScopedLock lock(ws_client_m_);
    if (ws_client_) {
        delete ws_client_;
    }
    ws_client_ = new WebSocketClient(value);
}

_Bool Context::LoadSettings(Settings *settings) {
    poco_check_ptr(settings);
    return displayError(db()->LoadSettings(settings), "LoadSettings");
}

_Bool Context::SetSettings(const Settings settings) {
    error err = db()->SaveSettings(settings);
    if (err != noError) {
        return displayError(err, "SetSettings");
    }
    return DisplaySettings(false);
}

_Bool Context::SetProxySettings(
    const _Bool use_proxy,
    const Proxy proxy) {

    _Bool was_using_proxy(false);
    Proxy previous_proxy_settings;
    error err = db()->LoadProxySettings(&was_using_proxy,
                                        &previous_proxy_settings);
    if (err != noError) {
        return displayError(err, "SetProxySettings");
    }

    err = db()->SaveProxySettings(use_proxy, proxy);
    if (err != noError) {
        return displayError(err, "SetProxySettings");
    }

    if (!DisplaySettings(false)) {
        return false;
    }

    if (!user_) {
        return true;
    }

    if (use_proxy != was_using_proxy
            || proxy.host != previous_proxy_settings.host
            || proxy.port != previous_proxy_settings.port
            || proxy.username != previous_proxy_settings.username
            || proxy.password != previous_proxy_settings.password) {
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

    std::string project_and_task_label("");
    std::string task_label("");
    std::string project_label("");
    std::string client_label("");
    std::string color("");
    projectLabelAndColorCode(te, &project_and_task_label,
                             &task_label, &project_label,
                             &client_label, &color);

    Poco::Int64 duration = totalDurationForDate(te);
    std::string date_duration = Formatter::FormatDuration(
        duration, true, Formatter::DurationFormat);

    return time_entry_view_item_init(te,
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
        return displayError(err, "DisplaySettings");
    }

    bool use_proxy(false);
    Proxy proxy;
    err = db()->LoadProxySettings(&use_proxy, &proxy);
    if (err != noError) {
        setUser(0);
        return displayError(err, "DisplaySettings");
    }

    bool record_timeline(false);
    if (user_) {
        record_timeline = user_->RecordTimeline();
    }

    idle_minutes_ = settings.idle_minutes;

    HTTPSClientConfig::UseProxy = use_proxy;
    HTTPSClientConfig::IgnoreCert = false;
    HTTPSClientConfig::ProxySettings = proxy;

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
        return displayError(exc.displayText(), "SetDBPath");
    } catch(const std::exception& ex) {
        return displayError(ex.what(), "SetDBPath");
    } catch(const std::string& ex) {
        return displayError(ex, "SetDBPath");
    }
    return true;
}

void Context::SetEnvironment(const std::string value) {
    poco_assert("production" == value ||
                "development" == value ||
                "test" == value);
    environment_ = value;
}

Database *Context::db() const {
    poco_check_ptr(db_);
    return db_;
}

_Bool Context::GoogleLogin(const std::string access_token) {
    return Login(access_token, "google_access_token");
}

_Bool Context::Login(
    const std::string email,
    const std::string password) {

    if (email.empty()) {
        return displayError("Empty email", "Login");
    }

    if (password.empty()) {
        return displayError("Empty password", "Login");
    }

    HTTPSClient client;
    std::string user_data_json("");
    error err = User::Me(&client, email, password, &user_data_json);
    if (err != noError) {
        return displayError(err, "Login");
    }

    if (user_data_json.empty()) {
        return false;
    }

    Poco::UInt64 userID(0);
    err = User::UserID(user_data_json, &userID);
    if (err != noError) {
        return displayError(err, "Login");
    }

    if (!userID) {
        return false;
    }

    User *user = new User();
    err = db()->LoadUserByID(userID, user);
    if (err != noError) {
        delete user;
        return displayError(err, "Login");
    }

    err = user->LoadUserAndRelatedDataFromJSONString(user_data_json);
    if (err != noError) {
        return displayError(err, "Login");
    }

    err = db()->SetCurrentAPIToken(user->APIToken());
    if (err != noError) {
        delete user;
        return displayError(err, "Login");
    }

    logger().debug("setUser from Login");

    setUser(user, true);

    displayUI();

    return displayError(save(), "Login");
}

void Context::setUser(User *value, const bool user_logged_in) {
    logger().debug("setUser");

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
}

_Bool Context::SetLoggedInUserFromJSON(
    const std::string json) {

    User *user = new User();

    user->LoadUserAndRelatedDataFromJSONString(json);

    error err = db()->SetCurrentAPIToken(user->APIToken());
    if (err != noError) {
        delete user;
        return displayError(err, "SetLoggedInUserFromJSON");
    }

    logger().debug("setUser from SetLoggedInUserFromJSON");

    setUser(user);

    err = save();
    if (err != noError) {
        return displayError(err, "SetLoggedInUserFromJSON");
    }

    return true;
}

_Bool Context::Logout() {
    try {
        if (!user_) {
            logger().warning("User is logged out, cannot logout again");
            return true;
        }

        Shutdown();

        error err = db()->ClearCurrentAPIToken();
        if (err != noError) {
            return displayError(err, "Logout");
        }

        logger().debug("setUser from Logout");

        setUser(0);

        UI()->DisplayApp();
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText(), "Logout");
    } catch(const std::exception& ex) {
        return displayError(ex.what(), "Logout");
    } catch(const std::string& ex) {
        return displayError(ex, "Logout");
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
            return displayError(err, "ClearCache");
        }

        return Logout();
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText(), "ClearCache");
    } catch(const std::exception& ex) {
        return displayError(ex.what(), "ClearCache");
    } catch(const std::string& ex) {
        return displayError(ex, "ClearCache");
    }
    return true;
}

bool Context::canSeeBillable(
    TimeEntry *time_entry,
    Workspace *ws) const {
    if (!user_->HasPremiumWorkspaces()) {
        return false;
    }
    if (ws && !ws->Premium()) {
        return false;
    }
    return true;
}


void Context::CollectPushableTimeEntries(
    std::vector<TimeEntry *> *models) const {

    poco_check_ptr(models);

    if (!user_) {
        return;
    }
    user_->CollectPushableTimeEntries(models);
}

std::vector<std::string> Context::tags() const {
    std::vector<std::string> tags;
    if (!user_) {
        return tags;
    }
    std::set<std::string> unique_names;
    for (std::vector<Tag *>::const_iterator it =
        user_->related.Tags.begin();
            it != user_->related.Tags.end();
            it++) {
        Tag *tag = *it;
        if (unique_names.find(tag->Name()) != unique_names.end()) {
            continue;
        }
        unique_names.insert(tag->Name());
        tags.push_back(tag->Name());
    }
    std::sort(tags.rbegin(), tags.rend());
    return tags;
}

std::vector<Workspace *> Context::workspaces() const {
    std::vector<Workspace *> result;
    if (!user_) {
        logger().warning("User logged out, cannot fetch workspaces");
        return result;
    }
    result = user_->related.Workspaces;
    std::sort(result.rbegin(), result.rend(), CompareWorkspaceByName);
    return result;
}

std::vector<Client *> Context::clients() const {
    std::vector<Client *> result;
    if (!user_) {
        logger().warning("User logged out, cannot fetch clients");
        return result;
    }
    result = user_->related.Clients;
    std::sort(result.rbegin(), result.rend(), CompareClientByName);
    return result;
}

TimeEntry *Context::Start(
    const std::string description,
    const std::string duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id) {

    if (!user_) {
        logger().warning("Cannot start tracking, user logged out");
        return 0;
    }

    TimeEntry *te = user_->Start(description, duration, task_id, project_id);

    UI()->DisplayApp();

	error err = save();
	if (err != noError) {
		displayError(err, "Start");
		return 0;
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

        std::string project_and_task_label("");
        std::string task_label("");
        std::string project_label("");
        std::string client_label("");
        std::string color("");
        projectLabelAndColorCode(te, &project_and_task_label,
                                 &task_label, &project_label,
                                 &client_label, &color);

        Poco::Int64 duration = date_durations[te->DateHeaderString()];
        std::string date_duration = Formatter::FormatDuration(
            duration, false, Formatter::DurationFormat);

        TogglTimeEntryView *item =
            time_entry_view_item_init(te,
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
    poco_assert(!GUID.empty() || edit_running_entry);

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

void Context::About() {
    std::string update_channel("");
    error err = db()->LoadUpdateChannel(&update_channel);
    if (err != noError) {
        displayError(err, "About");
        return;
    }
    UI()->DisplayUpdate(true, update_channel, true, false, "", "");
    fetchUpdates();
}

void Context::displayTimeEntryEditor(const _Bool open,
                                     TimeEntry *te,
                                     const std::string focused_field_name) {
    poco_check_ptr(te);
    time_entry_editor_guid_ = te->GUID();
    TogglTimeEntryView *view = timeEntryViewItem(te);

    Workspace *ws = 0;
    if (te->WID()) {
        ws = user_->related.WorkspaceByID(te->WID());
    }
    view->CanSeeBillable = canSeeBillable(te, ws);
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
    if (!user_) {
        logger().warning("Cannot continue tracking, user logged out");
        return true;
    }

    TimeEntry *latest = 0;
    std::vector<TimeEntry*> list = timeEntries(false);
    if (list.empty()) {
        return true;
    }
    latest = list.back();

    error err = user_->Continue(latest->GUID());
    if (err != noError) {
        return displayError(err, "ContinueLatest");
    }

    UI()->DisplayApp();

    return displayError(save(), "ContinueLatest");
}

_Bool Context::Continue(
    const std::string GUID) {

    if (!user_) {
        logger().warning("Cannot continue time entry, user logged out");
        return true;
    }

    if (GUID.empty()) {
        return displayError("Missing GUID", "Continue");
    }

    error err = user_->Continue(GUID);
    if (err != noError) {
        return displayError(err, "Continue");
    }

    UI()->DisplayApp();

    err = save();
    if (err != noError) {
        return displayError(err, "Continue");
    }

    DisplayTimeEntryList(true);

    return true;
}

_Bool Context::DeleteTimeEntryByGUID(const std::string GUID) {
    if (!user_) {
        logger().warning("Cannot delete time entry, user logged out");
        return true;
    }
    if (GUID.empty()) {
        return displayError("Missing GUID", "DeleteTimeEntryByGUID");
    }
    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    if (te->IsTracking()) {
        if (!Stop()) {
            return false;
        }
    }
    te->Delete();
    return displayError(save(), "DeleteTimeEntryByGUID");
}

_Bool Context::SetTimeEntryDuration(
    const std::string GUID,
    const std::string duration) {
    if (GUID.empty()) {
        return displayError("Missing GUID", "SetTimeEntryDuration");
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

    return displayError(save(), "SetTimeEntryDuration");
}

_Bool Context::SetTimeEntryProject(
    const std::string GUID,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string project_guid) {
    if (GUID.empty()) {
        return displayError("Missing GUID", "SetTimeEntryProject");
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
        // flag any more. (User selected billable project, unchecked billable,
        // then selected the same project again).
        if (p->ID() != te->PID()) {
            te->SetBillable(p->Billable());
        }
        te->SetWID(p->WID());
    }
    te->SetTID(task_id);
    te->SetPID(project_id);
    te->SetProjectGUID(project_guid);

    if (te->Dirty()) {
        te->SetUIModified();
    }

    return displayError(save(), "SetTimeEntryProject");
}

_Bool Context::SetTimeEntryDate(
    const std::string GUID,
    const Poco::Int64 unix_timestamp) {

    if (GUID.empty()) {
        return displayError("Missing GUID", "SetTimeEntryDate");
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

    Poco::DateTime date_part(
        Poco::Timestamp::fromEpochTime(unix_timestamp));
    Poco::DateTime time_part(
        Poco::Timestamp::fromEpochTime(te->Start()));

    Poco::DateTime dt(
        date_part.year(), date_part.month(), date_part.day(),
        time_part.hour(), time_part.minute(), time_part.second());

    std::string s = Formatter::Format8601(
        dt.timestamp().epochTime());

    te->SetStartUserInput(s);

    return displayError(save(), "SetTimeEntryDate");
}

_Bool Context::SetTimeEntryStart(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID", "SetTimeEntryStart");
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
        local.tzd(),
        local.year(), local.month(), local.day(),
        hours, minutes, local.second(), 0, 0);

    std::string s = Poco::DateTimeFormatter::format(
        dt, Poco::DateTimeFormat::ISO8601_FORMAT);

    te->SetStartUserInput(s);

    return displayError(save(), "SetTimeEntryStart");
}

_Bool Context::SetTimeEntryStop(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID", "SetTimeEntryStop");
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

    Poco::LocalDateTime local(Poco::Timestamp::fromEpochTime(te->Stop()));

    int hours(0), minutes(0);
    if (!toggl::Formatter::ParseTimeInput(value, &hours, &minutes)) {
        return false;
    }

    Poco::LocalDateTime dt(
        local.tzd(),
        local.year(), local.month(), local.day(),
        hours, minutes, local.second(), 0, 0);

    std::string s = Poco::DateTimeFormatter::format(
        dt, Poco::DateTimeFormat::ISO8601_FORMAT);

    te->SetStopUserInput(s);

    return displayError(save(), "SetTimeEntryStop");
}

_Bool Context::SetTimeEntryTags(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID", "SetTimeEntryTags");
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
        te->SetUIModified();
    }

    return displayError(save(), "SetTimeEntryTags");
}

_Bool Context::SetTimeEntryBillable(
    const std::string GUID,
    const bool value) {
    if (GUID.empty()) {
        return displayError("Missing GUID", "SetTimeEntryBillable");
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
        te->SetUIModified();
    }

    return displayError(save(), "SetTimeEntryBillable");
}

_Bool Context::SetTimeEntryDescription(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError("Missing GUID", "SetTimeEntryDescription");
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
        te->SetUIModified();
    }

    return displayError(save(), "SetTimeEntryDescription");
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

    UI()->DisplayApp();

    return displayError(save(), "Stop");
}

_Bool Context::DiscardTimeAt(
    const std::string guid,
    const Poco::Int64 at,
    const bool split_into_new_entry) {

    if (!user_) {
        logger().warning("Cannot stop time entry, user logged out");
        return true;
    }

    TimeEntry *stopped = user_->DiscardTimeAt(guid, at, split_into_new_entry);
    if (!stopped) {
        logger().warning("Time entry not found");
        return true;
    }

    UI()->DisplayApp();

    return displayError(save(), "DiscardTimeAt");
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
            return displayError(err, "ToggleTimelineRecording");
        }

        TimelineUpdateServerSettings();
        if (user_->RecordTimeline()) {
            switchTimelineOn();
        } else {
            switchTimelineOff();
        }
        return DisplaySettings(false);
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText(), "ToggleTimelineRecording");
    } catch(const std::exception& ex) {
        return displayError(ex.what(), "ToggleTimelineRecording");
    } catch(const std::string& ex) {
        return displayError(ex, "ToggleTimelineRecording");
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

        poco_assert(!te->GUID().empty());

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
        return displayError(err, "SaveUpdateChannel");
    }
    UI()->DisplayUpdate(false, channel, true, false, "", "");
    fetchUpdates();
    return true;
}

void Context::projectLabelAndColorCode(
    TimeEntry *te,
    std::string *project_and_task_label,
    std::string *task_label,
    std::string *project_label,
    std::string *client_label,
    std::string *color_code) const {

    poco_check_ptr(te);
    poco_check_ptr(project_and_task_label);
    poco_check_ptr(task_label);
    poco_check_ptr(project_label);
    poco_check_ptr(client_label);
    poco_check_ptr(color_code);

    if (!user_) {
        return;
    }

    Task *t = 0;
    if (te->TID()) {
        t = user_->related.TaskByID(te->TID());
    }

    Project *p = 0;
    if (t) {
        p = user_->related.ProjectByID(t->PID());
        *task_label = t->Name();
    }
    if (!p && te->PID()) {
        p = user_->related.ProjectByID(te->PID());
    }
    if (!p && !te->ProjectGUID().empty()) {
        p = user_->related.ProjectByGUID(te->ProjectGUID());
    }

    Client *c = 0;
    if (p && p->CID()) {
        c = user_->related.ClientByID(p->CID());
    }

    *project_and_task_label = Formatter::JoinTaskName(t, p, c);

    if (p) {
        *color_code = p->ColorCode();
        *project_label = p->Name();
    }

    if (c) {
        *client_label = c->Name();
    }
}

_Bool Context::AddProject(
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
        return displayError("Please select a workspace", "AddProject");
    }
    if (project_name.empty()) {
        return displayError("Project name must not be empty", "AddProject");
    }

    *result = user_->AddProject(
        workspace_id, client_id, project_name, is_private);

    return displayError(save(), "AddProject");
}

void Context::SetSleep() {
    logger().debug("SetSleep");
}

_Bool Context::OpenReportsInBrowser() {
    if (!user_) {
        return displayError("You must log in to view reports",
                            "OpenReportsInBrowser");
    }

    std::string response_body("");
    HTTPSClient https_client;
    error err = https_client.PostJSON("/api/v8/desktop_login_tokens",
                                      "{}",
                                      user_->APIToken(),
                                      "api_token",
                                      &response_body);
    if (err != noError) {
        return displayError(err, "OpenReportsInBrowser");
    }
    if (response_body.empty()) {
        return displayError("Unexpected empty response from API",
                            "OpenReportsInBrowser");
    }

    std::string login_token("");
    err = User::LoginToken(response_body, &login_token);
    if (err != noError) {
        return displayError(err, "OpenReportsInBrowser");
    }

    if (login_token.empty()) {
        return displayError("Could not extract login token from JSON",
                            "OpenReportsInBrowser");
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
}

void Context::remindToTrackTime() {
    next_reminder_at_ =
        postpone(kReminderThrottleSeconds * kOneSecondInMicros);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onRemind);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_reminder_at_);
}

void Context::onRemind(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_reminder_at_,
                    kReminderThrottleSeconds * kOneSecondInMicros)) {
        logger().debug("onRemind postponed");
        return;
    }
    logger().debug("onRemind executing");

    if (!user_) {
        logger().warning("User logged out, cannot remind");
        return;
    }

    Settings settings;
    if (!LoadSettings(&settings)) {
        logger().error("Could not load settings");
        return;
    }

    if (!settings.reminder) {
        logger().debug("Reminder is not enabled by user");
        return;
    }

    if (user_ && user_->RunningTimeEntry()) {
        logger().debug("User is already tracking time, no need to remind");
        return;
    }

    if (user_ && user_->HasTrackedTimeToday()) {
        logger().debug("Already tracked time today, no need to remind");
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

void Context::SetIdleSeconds(const Poco::UInt64 idle_seconds) {
    /*
    {
        std::stringstream ss;
        ss << "SetIdleSeconds idle_seconds=" << idle_seconds;
        logger().debug(ss.str());
    }
    */

    if (user_) {
        computeIdleState(idle_seconds);
    }

    last_idle_seconds_reading_ = idle_seconds;
}

void Context::computeIdleState(const Poco::UInt64 idle_seconds) {
    if (idle_minutes_ &&
            (idle_seconds >= (idle_minutes_*60)) &&
            !last_idle_started_) {
        last_idle_started_ = time(0) - idle_seconds;

        std::stringstream ss;
        ss << "User is idle since " << last_idle_started_;
        logger().debug(ss.str());

        return;
    }

    if (last_idle_started_ &&
            idle_seconds < last_idle_seconds_reading_) {
        time_t now = time(0);

        TimeEntry *te = user_->RunningTimeEntry();
        if (!te) {
            logger().warning("Time entry is not tracking, ignoring idleness");
        } else if (TimeEntry::AbsDuration(te->DurationInSeconds())
                   < last_idle_seconds_reading_) {
            logger().warning("Time entry duration is less than idle, ignoring");
        } else {
            Settings settings;
            error err = db()->LoadSettings(&settings);
            if (err != noError) {
                displayError(err, "SetIdleSeconds");
            }
            if (settings.use_idle_detection) {
                std::stringstream since;
                since << "You have been idle since "
                      << Formatter::FormatTimeForTimeEntryEditor(
                          last_idle_started_);

                int minutes = static_cast<int>(last_idle_seconds_reading_ / 60);
                std::stringstream duration;
                duration << "(" << minutes << " minute";
                if (minutes != 1) {
                    duration << "s";
                }
                duration << ")";

                UI()->DisplayIdleNotification(te->GUID(),
                                              since.str(),
                                              duration.str(),
                                              last_idle_started_);
            }
        }

        std::stringstream ss;
        ss << "User is not idle since " << now;
        logger().debug(ss.str());

        last_idle_started_ = 0;
    }
}

}  // namespace toggl
