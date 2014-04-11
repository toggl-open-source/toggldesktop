
// Copyright 2014 Toggl Desktop developers.

// No exceptions should be thrown from this class.
// If pointers to models are returned from this
// class, the ownership does not change and you
// must not delete the pointers you got.

#include "./context.h"

#include "./formatter.h"
#include "./json.h"
#include "./time_entry.h"
#include "./const.h"
#include "./kopsik_api_private.h"

#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Util/TimerTaskAdapter.h"
#include "Poco/Environment.h"
#include "Poco/Timestamp.h"

namespace kopsik {

Context::Context(
    const std::string app_name,
    const std::string app_version)
    : db_(0),
  user_(0),
  ws_client_(0),
  timeline_uploader_(0),
  window_change_recorder_(0),
  app_name_(app_name),
  app_version_(app_version),
  api_url_(""),
  timeline_upload_url_(""),
  update_channel_(""),
  feedback_("", "", ""),
  on_model_change_callback_(0),
  on_error_callback_(0),
  on_check_update_callback_(0),
  on_user_login_callback_(0),
  on_open_url_callback_(0),
  next_full_sync_at_(0),
  next_partial_sync_at_(0),
  next_fetch_updates_at_(0),
  next_update_timeline_settings_at_(0),
  next_reminder_at_(0) {
    Poco::ErrorHandler::set(&error_handler_);
    Poco::Net::initializeSSL();

    Poco::Crypto::OpenSSLInitializer::initialize();

    startPeriodicUpdateCheck();

    SetWake();  // obviously, we're wake
}

Context::~Context() {
    if (window_change_recorder_) {
        Poco::Mutex::ScopedLock lock(window_change_recorder_m_);
        delete window_change_recorder_;
        window_change_recorder_ = 0;
    }

    if (timeline_uploader_) {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        delete timeline_uploader_;
        timeline_uploader_ = 0;
    }

    if (ws_client_) {
        Poco::Mutex::ScopedLock lock(ws_client_m_);
        delete ws_client_;
        ws_client_ = 0;
    }

    if (db_) {
        Poco::Mutex::ScopedLock lock(db_m_);
        delete db_;
        db_ = 0;
    }

    setUser(0);

    Poco::Net::uninitializeSSL();
}

void Context::StartEvents() {
    error err = verifyCallbacks();
    if (err != noError) {
        logger().error(err);
        poco_assert(on_error_callback_);
        on_error_callback_(err.c_str());
        return;
    }

    User *user = 0;
    if (!LoadCurrentUser(&user)) {
        return;
    }
    exportUserLoginState();
}

error Context::verifyCallbacks() {
    if (!on_error_callback_) {
        return error("missing on_error_callback_");
    }
    if (!on_error_callback_) {
        return error("missing on_error_callback_");
    }
    if (!on_check_update_callback_) {
        return error("missing on on_check_update_callback_");
    }
    if (!on_online_callback_) {
        return error("missing on on_online_callback_");
    }
    if (!on_user_login_callback_) {
        return error("missing on on_user_login_callback_");
    }
    if (!on_open_url_callback_) {
        return error("missing on on_open_url_callback_");
    }
    if (!on_remind_callback_) {
        return error("missing on on_remind_callback_");
    }
    return noError;
}

void Context::exportUserLoginState() {
    if (!user_) {
        on_user_login_callback_(0, "", "");
        return;
    }
    SetWake();  // obviously, we're wake
    on_user_login_callback_(user_->ID(),
                            user_->Fullname().c_str(),
                            user_->TimeOfDayFormat().c_str());
}

void Context::Shutdown() {
    if (window_change_recorder_) {
        Poco::Mutex::ScopedLock lock(window_change_recorder_m_);
        window_change_recorder_->Stop();
    }
    if (ws_client_) {
        Poco::Mutex::ScopedLock lock(ws_client_m_);
        ws_client_->Stop();
    }
    if (timeline_uploader_) {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        timeline_uploader_->Stop();
    }

    // cancel tasks but allow them finish
    {
        Poco::Mutex::ScopedLock lock(timer_m_);
        timer_.cancel(true);
    }

    Poco::ThreadPool::defaultPool().joinAll();
}

void Context::PasswordForgot() {
    on_open_url_callback_(kLostPasswordURL);
}

_Bool Context::ConfigureProxy() {
    bool use_proxy(false);
    kopsik::Proxy proxy;
    kopsik::error err = db_->LoadProxySettings(&use_proxy, &proxy);
    if (err != kopsik::noError) {
        return exportErrorState(err);
    }
    if (!use_proxy) {
        proxy = kopsik::Proxy();  // reset values
    }

    Poco::Mutex::ScopedLock lock(ws_client_m_);
    ws_client_->SetProxy(proxy);

    return true;
}

error Context::save(const bool push_changes) {
    logger().debug("save");

    std::vector<kopsik::ModelChange> changes;
    kopsik::error err = db_->SaveUser(user_, true, &changes);
    if (err != kopsik::noError) {
        return err;
    }
    if (!on_model_change_callback_) {
        return noError;
    }
    for (std::vector<kopsik::ModelChange>::const_iterator it =
        changes.begin();
            it != changes.end();
            it++) {
        KopsikModelChange *ch = model_change_init();
        model_change_to_change_item(*it, ch);
        on_model_change_callback_(ch);
        model_change_clear(ch);
    }

    if (push_changes) {
        partialSync();
    }

    return noError;
}

void Context::FullSync() {
    logger().debug("FullSync");

    next_full_sync_at_ = postpone();
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onFullSync);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_full_sync_at_);
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

void Context::onFullSync(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_full_sync_at_)) {
        logger().debug("onFullSync postponed");
        return;
    }
    logger().debug("onFullSync executing");

    kopsik::HTTPSClient https_client = get_https_client();
    kopsik::error err = user_->FullSync(&https_client);
    if (err != kopsik::noError) {
        on_error_callback_(err.c_str());
        return;
    }

    err = save(false);
    if (err != kopsik::noError) {
        on_error_callback_(err.c_str());
        return;
    }

    on_online_callback_();
}

void Context::partialSync() {
    logger().debug("partialSync");

    next_partial_sync_at_ = postpone();
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onPartialSync);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_partial_sync_at_);
}

void Context::onPartialSync(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_partial_sync_at_)) {
        logger().debug("onPartialSync postponed");
        return;
    }
    logger().debug("onPartialSync executing");

    kopsik::HTTPSClient https_client = get_https_client();
    kopsik::error err = user_->PartialSync(&https_client);
    if (err != kopsik::noError) {
        on_error_callback_(err.c_str());
        return;
    }

    err = save(false);
    if (err != kopsik::noError) {
        on_error_callback_(err.c_str());
        return;
    }

    on_online_callback_();
}

void Context::SwitchWebSocketOff() {
    logger().debug("SwitchWebSocketOff");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchWebSocketOff);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchWebSocketOff(Poco::Util::TimerTask& task) {  // NOLINT
    logger().debug("onSwitchWebSocketOff");

    if (!ws_client_) {
        return;
    }

    Poco::Mutex::ScopedLock lock(ws_client_m_);
    ws_client_->Stop();
}

void on_websocket_message(
    void *context,
    std::string json) {
    poco_assert(context);
    poco_assert(!json.empty());

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

    LoadUserUpdateFromJSONString(user_, json);

    return exportErrorState(save());
}

void Context::SwitchWebSocketOn() {
    logger().debug("SwitchWebSocketOn");

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
void Context::SwitchTimelineOff() {
    logger().debug("SwitchTimelineOff");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchTimelineOff);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchTimelineOff(Poco::Util::TimerTask& task) {  // NOLINT
    logger().debug("onSwitchTimelineOff");

    if (window_change_recorder_) {
        Poco::Mutex::ScopedLock lock(window_change_recorder_m_);
        delete window_change_recorder_;
        window_change_recorder_ = 0;
    }

    if (timeline_uploader_) {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        delete timeline_uploader_;
        timeline_uploader_ = 0;
    }
}

void Context::SwitchTimelineOn() {
    logger().debug("SwitchTimelineOn");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchTimelineOn);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchTimelineOn(Poco::Util::TimerTask& task) {  // NOLINT
    logger().debug("onSwitchTimelineOn");

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
        timeline_uploader_ = new kopsik::TimelineUploader(
            user_->ID(),
            user_->APIToken(),
            timeline_upload_url_,
            app_name_,
            app_version_);
    }

    {
        Poco::Mutex::ScopedLock lock(window_change_recorder_m_);
        if (window_change_recorder_) {
            delete window_change_recorder_;
            window_change_recorder_ = 0;
        }
        window_change_recorder_ = new kopsik::WindowChangeRecorder(user_->ID());
    }
}

void Context::FetchUpdates() {
    logger().debug("FetchUpdates");

    next_fetch_updates_at_ = postpone();
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onFetchUpdates);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_fetch_updates_at_);
}

void Context::onFetchUpdates(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_fetch_updates_at_)) {
        logger().debug("onFetchUpdates postponed");
        return;
    }

    executeUpdateCheck();
}

void Context::startPeriodicUpdateCheck() {
    logger().debug("startPeriodicUpdateCheck");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>
    (*this, &Context::onPeriodicUpdateCheck);

    Poco::Timestamp next_periodic_check_at =
        Poco::Timestamp() + kCheckUpdateIntervalMicros;
    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_periodic_check_at);
}

void Context::onPeriodicUpdateCheck(Poco::Util::TimerTask& task) {  // NOLINT
    logger().debug("onPeriodicUpdateCheck");

    executeUpdateCheck();

    startPeriodicUpdateCheck();
}

void Context::executeUpdateCheck() {
    logger().debug("executeUpdateCheck");

    kopsik::error err = db_->LoadUpdateChannel(&update_channel_);
    if (err != kopsik::noError) {
        on_error_callback_(err.c_str());
        return;
    }

    std::string response_body("");
    err = get_https_client().GetJSON(updateURL(),
                                     std::string(""),
                                     std::string(""),
                                     &response_body);
    if (err != kopsik::noError) {
        on_error_callback_(err.c_str());
        return;
    }

    if ("null" == response_body) {
        on_check_update_callback_(false, "", "");
        return;
    }

    if (!IsValidJSON(response_body)) {
        on_error_callback_("Invalid response JSON");
        return;
    }

    std::string url("");
    std::string version("");

    JSONNODE *root = json_parse(response_body.c_str());
    JSONNODE_ITERATOR i = json_begin(root);
    JSONNODE_ITERATOR e = json_end(root);
    while (i != e) {
        json_char *node_name = json_name(*i);
        if (strcmp(node_name, "version") == 0) {
            version = std::string(json_as_string(*i));
        } else if (strcmp(node_name, "url") == 0) {
            url = std::string(json_as_string(*i));
        }
        ++i;
    }
    json_delete(root);

    on_check_update_callback_(true, url.c_str(), version.c_str());
}

const std::string Context::updateURL() const {
    poco_assert(!update_channel_.empty());
    poco_assert(!app_version_.empty());

    std::stringstream relative_url;
    relative_url << "/api/v8/updates?app=td"
                 << "&channel=" << update_channel_
                 << "&platform=" << osName()
                 << "&version=" << app_version_
                 << "&osname=" << Poco::Environment::osName()
                 << "&osversion=" << Poco::Environment::osVersion()
                 << "&osarch=" << Poco::Environment::osArchitecture();
    return relative_url.str();
}

const std::string Context::osName() {
    if (POCO_OS_LINUX == POCO_OS) {
        return std::string("linux");
    }
    if (POCO_OS_WINDOWS_NT == POCO_OS) {
        return std::string("windows");
    }
    return std::string("darwin");
}

void Context::TimelineUpdateServerSettings() {
    logger().debug("TimelineUpdateServerSettings");

    next_update_timeline_settings_at_ = postpone();
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this,
                &Context::onTimelineUpdateServerSettings);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_update_timeline_settings_at_);
}

const std::string kRecordTimelineEnabledJSON = "{\"record_timeline\": true}";
const std::string kRecordTimelineDisabledJSON = "{\"record_timeline\": false}";

void Context::onTimelineUpdateServerSettings(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_update_timeline_settings_at_)) {
        logger().debug("onTimelineUpdateServerSettings postponed");
        return;
    }

    logger().debug("onTimelineUpdateServerSettings executing");

    std::string json(kRecordTimelineDisabledJSON);
    if (user_->RecordTimeline()) {
        json = kRecordTimelineEnabledJSON;
    }

    std::string response_body("");
    kopsik::error err = get_https_client().PostJSON("/api/v8/timeline_settings",
                        json,
                        user_->APIToken(),
                        "api_token",
                        &response_body);
    if (err != kopsik::noError) {
        logger().warning(err);
    }
}

_Bool Context::SendFeedback(Feedback fb) {
    if (!user_) {
        logger().warning("Cannot send feedback, user logged out");
        return true;
    }

    fb.SetAppVersion(app_version_);

    kopsik::error err = fb.Validate();
    if (err != kopsik::noError) {
        return exportErrorState(err);
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
    kopsik::error err = get_https_client().PostJSON("/api/v8/feedback",
                        feedback_.JSON(),
                        user_->APIToken(),
                        "api_token",
                        &response_body);
    if (err != kopsik::noError) {
        on_error_callback_(err.c_str());
        return;
    }
}

void Context::SetWebSocketClientURL(const std::string value) {
    Poco::Mutex::ScopedLock lock(ws_client_m_);
    if (ws_client_) {
        delete ws_client_;
    }
    ws_client_ = new kopsik::WebSocketClient(value,
            app_name_,
            app_version_);
}

_Bool Context::LoadSettings(
    bool *use_idle_settings,
    bool *menubar_timer,
    bool *dock_icon,
    bool *on_top,
    bool *reminder) const {
    return exportErrorState(db_->LoadSettings(
        use_idle_settings,
        menubar_timer,
        dock_icon,
        on_top,
        reminder));
}

_Bool Context::LoadProxySettings(
    bool *use_proxy,
    kopsik::Proxy *proxy) const {
    return exportErrorState(db_->LoadProxySettings(
        use_proxy,
        proxy));
}

_Bool Context::exportErrorState(const error err) const {
    if (err != noError) {
        on_error_callback_(err.c_str());
        return false;
    }
    return true;
}

_Bool Context::SaveSettings(
    const bool use_idle_detection,
    const bool menubar_timer,
    const bool dock_icon,
    const bool on_top,
    const bool reminder) {
    return exportErrorState(db_->SaveSettings(
        use_idle_detection, menubar_timer, dock_icon, on_top, reminder));
}

_Bool Context::SaveProxySettings(
    const bool use_proxy,
    const kopsik::Proxy *proxy) {

    kopsik::Proxy previous_proxy_settings;
    bool was_using_proxy(false);
    if (!LoadProxySettings(
        &was_using_proxy,
        &previous_proxy_settings)) {
        return false;
    };

    error err = db_->SaveProxySettings(
        use_proxy, proxy);
    if (err != kopsik::noError) {
        return exportErrorState(err);
    }

    // If proxy settings have changed, apply new settings:
    if (use_proxy != was_using_proxy
            || proxy->host != previous_proxy_settings.host
            || proxy->port != previous_proxy_settings.port
            || proxy->username != previous_proxy_settings.username
            || proxy->password != previous_proxy_settings.password) {
        if (!ConfigureProxy()) {
            return false;
        }
        if (user_) {
            FullSync();
            SwitchWebSocketOn();
        }
    }

    return true;
}

_Bool Context::SetDBPath(
    const std::string path) {
    try {
        Poco::Mutex::ScopedLock lock(db_m_);
        if (db_) {
            delete db_;
            db_ = 0;
        }
        db_ = new kopsik::Database(path);
    } catch(const Poco::Exception& exc) {
        return exportErrorState(exc.displayText());
    } catch(const std::exception& ex) {
        return exportErrorState(ex.what());
    } catch(const std::string& ex) {
        return exportErrorState(ex);
    }
    return true;
}

_Bool Context::CurrentAPIToken(std::string *token) {
    return exportErrorState(db_->CurrentAPIToken(token));
}

_Bool Context::SetCurrentAPIToken(
    const std::string token) {
    return exportErrorState(db_->SetCurrentAPIToken(token));
}

_Bool Context::LoadCurrentUser(kopsik::User **result) {
    poco_assert(!*result);

    if (user_) {
        *result = user_;
        return true;
    }

    kopsik::User *user = new kopsik::User(app_name_, app_version_);
    kopsik::error err = db_->LoadCurrentUser(user, true);
    if (err != kopsik::noError) {
        delete user;
        return exportErrorState(err);
    }

    if (!user->ID()) {
        delete user;
        return true;
    }

    setUser(user);
    *result = user_;

    return true;
}

_Bool Context::Login(
    const std::string email,
    const std::string password) {

    if (email.empty()) {
        return exportErrorState("Empty email");
    }

    if (password.empty()) {
        return exportErrorState("Empty password");
    }

    kopsik::User *logging_in = new kopsik::User(app_name_, app_version_);

    kopsik::HTTPSClient https_client = get_https_client();
    kopsik::error err = logging_in->Login(&https_client, email, password);
    if (err != kopsik::noError) {
        delete logging_in;
        return exportErrorState(err);
    }

    poco_assert(logging_in->ID() > 0);

    err = db_->LoadUserByID(logging_in->ID(), logging_in, true);
    if (err != kopsik::noError) {
        delete logging_in;
        return exportErrorState(err);
    }

    err = db_->SetCurrentAPIToken(logging_in->APIToken());
    if (err != kopsik::noError) {
        delete logging_in;
        return exportErrorState(err);
    }

    setUser(logging_in);

    return exportErrorState(save());
}

void Context::setUser(User *value) {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (user_) {
        delete user_;
    }
    user_ = value;
    exportUserLoginState();
}

_Bool Context::SetLoggedInUserFromJSON(
    const std::string json) {
    kopsik::User *import = new kopsik::User(app_name_, app_version_);

    LoadUserFromJSONString(import, json, true, true);

    kopsik::error err = db_->SetCurrentAPIToken(import->APIToken());
    if (err != kopsik::noError) {
        delete import;
        return exportErrorState(err);
    }

    setUser(import);

    err = save();
    if (err != noError) {
        return exportErrorState(err);
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

        kopsik::error err = db_->ClearCurrentAPIToken();
        if (err != kopsik::noError) {
            return exportErrorState(err);
        }

        setUser(0);
    } catch(const Poco::Exception& exc) {
        return exportErrorState(exc.displayText());
    } catch(const std::exception& ex) {
        return exportErrorState(ex.what());
    } catch(const std::string& ex) {
        return exportErrorState(ex);
    }
    return true;
}

_Bool Context::ClearCache() {
    try {
        if (!user_) {
            logger().warning("User is logged out, cannot clear cache");
            return true;
        }
        kopsik::error err = db_->DeleteUser(user_, true);
        if (err != kopsik::noError) {
            return exportErrorState(err);
        }

        return Logout();
    } catch(const Poco::Exception& exc) {
        return exportErrorState(exc.displayText());
    } catch(const std::exception& ex) {
        return exportErrorState(ex.what());
    } catch(const std::string& ex) {
        return exportErrorState(ex);
    }
    return true;
}

bool Context::CanSeeBillable(const std::string GUID) const {
    if (!user_) {
        return false;
    }
    if (!user_->HasPremiumWorkspaces()) {
        return false;
    }
    TimeEntry *te = GetTimeEntryByGUID(GUID);
    if (!te) {
        return false;
    }
    Workspace *ws = 0;
    if (te->WID()) {
        ws = user_->GetWorkspaceByID(te->WID());
    }
    if (ws && !ws->Premium()) {
        return false;
    }
    return true;
}

bool Context::CanAddProjects(const Poco::UInt64 workspace_id) const {
    if (!user_) {
        return false;
    }
    Workspace *ws = 0;
    if (workspace_id) {
        ws = user_->GetWorkspaceByID(workspace_id);
    }
    if (ws) {
        return ws->Admin() || !ws->OnlyAdminsMayCreateProjects();
    }
    return user_->CanAddProjects();
}

bool Context::UserIsLoggedIn() const {
    return (user_ && user_->ID());
}

Poco::UInt64 Context::UsersDefaultWID() const {
    if (!user_) {
        return 0;
    }
    return user_->DefaultWID();
}

void Context::CollectPushableTimeEntries(
    std::vector<kopsik::TimeEntry *> *models) const {
    poco_assert(models);

    if (!user_) {
        return;
    }
    user_->CollectPushableTimeEntries(models);
}

std::vector<std::string> Context::Tags() const {
    std::vector<std::string> tags;
    if (!user_) {
        return tags;
    }
    std::set<std::string> unique_names;
    for (std::vector<kopsik::Tag *>::const_iterator it =
        user_->related.Tags.begin();
            it != user_->related.Tags.end();
            it++) {
        kopsik::Tag *tag = *it;
        if (unique_names.find(tag->Name()) != unique_names.end()) {
            continue;
        }
        unique_names.insert(tag->Name());
        tags.push_back(tag->Name());
    }
    std::sort(tags.rbegin(), tags.rend());
    return tags;
}

std::vector<kopsik::Workspace *> Context::Workspaces() const {
    std::vector<kopsik::Workspace *> result;
    if (!user_) {
        logger().warning("User logged out, cannot fetch workspaces");
        return result;
    }
    result = user_->related.Workspaces;
    std::sort(result.rbegin(), result.rend(), CompareWorkspaceByName);
    return result;
}

std::vector<kopsik::Client *> Context::Clients(
    const Poco::UInt64 workspace_id) const {
    poco_assert(workspace_id);
    std::vector<kopsik::Client *> result;
    if (!user_) {
        logger().warning("User logged out, cannot fetch clients");
        return result;
    }
    for (std::vector<kopsik::Client *>::const_iterator it =
        user_->related.Clients.begin();
            it != user_->related.Clients.end();
            it++) {
        kopsik::Client *client = *it;
        if (client->WID() != workspace_id) {
            continue;
        }
        result.push_back(client);
    }
    std::sort(result.rbegin(), result.rend(), CompareClientByName);
    return result;
}

_Bool Context::Start(
    const std::string description,
    const std::string duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    kopsik::TimeEntry **result) {
    poco_assert(result);

    if (!user_) {
        logger().warning("Cannot start tracking, user logged out");
        return true;
    }

    *result = user_->Start(description, duration, task_id, project_id);
    poco_assert(*result);

    return exportErrorState(save());
}

_Bool Context::ContinueLatest(
    kopsik::TimeEntry **result) {
    poco_assert(result);

    if (!user_) {
        logger().warning("Cannot continue tracking, user logged out");
        return true;
    }

    kopsik::TimeEntry *latest = user_->Latest();
    if (!latest) {
        return true;
    }

    kopsik::error err = user_->Continue(latest->GUID(), result);
    if (err != kopsik::noError) {
        return exportErrorState(err);
    }

    return exportErrorState(save());
}

_Bool Context::Continue(
    const std::string GUID,
    kopsik::TimeEntry **result) {
    poco_assert(result);

    if (!user_) {
        logger().warning("Cannot continue time entry, user logged out");
        return true;
    }

    if (GUID.empty()) {
        return exportErrorState("Missing GUID");
    }


    kopsik::error err = user_->Continue(GUID, result);
    if (err != kopsik::noError) {
        return exportErrorState(err);
    }

    return exportErrorState(save());
}

_Bool Context::DeleteTimeEntryByGUID(const std::string GUID) {
    if (!user_) {
        logger().warning("Cannot delete time entry, user logged out");
        return true;
    }
    if (GUID.empty()) {
        return exportErrorState("Missing GUID");
    }
    kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    te->Delete();

    kopsik::ModelChange mc("time_entry", "delete", te->ID(), te->GUID());
    if (on_model_change_callback_) {
        KopsikModelChange *ch = model_change_init();
        model_change_to_change_item(mc, ch);
        on_model_change_callback_(ch);
        model_change_clear(ch);
    }

    return exportErrorState(save());
}

kopsik::TimeEntry *Context::GetTimeEntryByGUID(const std::string GUID) const {
    if (!user_) {
        logger().warning("Cannot get time entry, user logged out");
        return 0;
    }
    return user_->GetTimeEntryByGUID(GUID);
}

_Bool Context::SetTimeEntryDuration(
    const std::string GUID,
    const std::string duration) {
    if (GUID.empty()) {
        return exportErrorState("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot set duration, user logged out");
        return true;
    }
    kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    te->SetDurationUserInput(duration);
    if (te->Dirty()) {
        te->SetUIModified();
    }

    return exportErrorState(save());
}

_Bool Context::SetTimeEntryProject(
    const std::string GUID,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string project_guid) {
    if (GUID.empty()) {
        return exportErrorState("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot set project, user logged out");
        return true;
    }

    kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }

    kopsik::Project *p = 0;
    if (project_id) {
        p = user_->GetProjectByID(project_id);
    }
    if (!project_guid.empty()) {
        p = user_->GetProjectByGUID(project_guid);
    }

    if (p) {
        te->SetBillable(p->Billable());
        te->SetWID(p->WID());
    }
    te->SetTID(task_id);
    te->SetPID(project_id);
    te->SetProjectGUID(project_guid);

    if (te->Dirty()) {
        te->SetUIModified();
    }

    return exportErrorState(save());
}

_Bool Context::SetTimeEntryStartISO8601(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return exportErrorState("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot change start time, user logged out");
        return true;
    }
    kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    user_->SetLastTEDate(value);
    te->SetStartUserInput(value);
    if (te->Dirty()) {
        te->SetUIModified();
    }

    return exportErrorState(save());
}

_Bool Context::SetTimeEntryEndISO8601(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return exportErrorState("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot change end time, user logged out");
        return true;
    }
    kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    user_->SetLastTEDate(value);
    te->SetStopUserInput(value);
    if (te->Dirty()) {
        te->SetUIModified();
    }

    return exportErrorState(save());
}

_Bool Context::SetTimeEntryTags(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return exportErrorState("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot set tags, user logged out");
        return true;
    }
    kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    te->SetTags(value);
    if (te->Dirty()) {
        te->SetUIModified();
    }

    return exportErrorState(save());
}

_Bool Context::SetTimeEntryBillable(
    const std::string GUID,
    const bool value) {
    if (GUID.empty()) {
        return exportErrorState("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot set billable, user logged out");
        return true;
    }
    kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    te->SetBillable(value);
    if (te->Dirty()) {
        te->SetUIModified();
    }

    return exportErrorState(save());
}

_Bool Context::SetTimeEntryDescription(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return exportErrorState("Missing GUID");
    }
    if (!user_) {
        logger().warning("Cannot set description, user logged out");
        return true;
    }
    kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return true;
    }
    te->SetDescription(value);
    if (te->Dirty()) {
        te->SetUIModified();
    }

    return exportErrorState(save());
}

_Bool Context::Stop(kopsik::TimeEntry **stopped_entry) {
    *stopped_entry = 0;
    if (!user_) {
        logger().warning("Cannot stop tracking, user logged out");
        return true;
    }

    std::vector<kopsik::TimeEntry *> stopped = user_->Stop();
    if (stopped.empty()) {
        return exportErrorState("No time entry was found to stop");
    }
    *stopped_entry = stopped[0];
    return exportErrorState(save());
}

_Bool Context::StopAt(
    const Poco::Int64 at,
    kopsik::TimeEntry **result) {

    poco_assert(result);

    if (!user_) {
        logger().warning("Cannot stop time entry, user logged out");
        return true;
    }

    TimeEntry *stopped = user_->StopAt(at);
    if (!stopped) {
        logger().warning("Time entry not found");
        return true;
    }

    *result = stopped;

    return exportErrorState(save());
}

_Bool Context::RunningTimeEntry(
    kopsik::TimeEntry **running) const {
    if (!user_) {
        logger().warning("Cannot fetch time entry, user logged out");
        return true;
    }
    *running = user_->RunningTimeEntry();
    return true;
}

_Bool Context::ToggleTimelineRecording() {
    if (!user_) {
        logger().warning("Cannot toggle timeline, user logged out");
        return true;
    }
    try {
        user_->SetRecordTimeline(!user_->RecordTimeline());

        kopsik::error err = save();
        if (err != kopsik::noError) {
            return exportErrorState(err);
        }

        TimelineUpdateServerSettings();
        if (user_->RecordTimeline()) {
            SwitchTimelineOn();
            return true;
        }
        SwitchTimelineOff();
    } catch(const Poco::Exception& exc) {
        return exportErrorState(exc.displayText());
    } catch(const std::exception& ex) {
        return exportErrorState(ex.what());
    } catch(const std::string& ex) {
        return exportErrorState(ex);
    }
    return true;
}

_Bool Context::TimeEntries(
    std::map<std::string, Poco::Int64> *date_durations,
    std::vector<kopsik::TimeEntry *> *visible) const {
    if (!user_) {
        return true;
    }

    for (std::vector<kopsik::TimeEntry *>::const_iterator it =
        user_->related.TimeEntries.begin();
            it != user_->related.TimeEntries.end(); it++) {
        kopsik::TimeEntry *te = *it;
        poco_assert(!te->GUID().empty());
        if (te->DurationInSeconds() < 0) {
            continue;
        }
        if (te->DeletedAt() > 0) {
            continue;
        }
        visible->push_back(te);

        std::string date_header = te->DateHeaderString();
        Poco::Int64 duration = (*date_durations)[date_header];
        duration += te->DurationInSeconds();
        (*date_durations)[date_header] = duration;
    }

    std::sort(visible->begin(), visible->end(), CompareTimeEntriesByStart);

    return true;
}

_Bool Context::TrackedPerDateHeader(
    const std::string date_header,
    int *sum) const {
    if (!user_) {
        logger().warning("Cannot access time entries, user logged out");
        return true;
    }
    for (std::vector<kopsik::TimeEntry *>::const_iterator it =
        user_->related.TimeEntries.begin();
            it != user_->related.TimeEntries.end(); it++) {
        kopsik::TimeEntry *te = *it;
        if (te->DurationInSeconds() >= 0 && !te->DeletedAt() &&
                te->DateHeaderString() == date_header) {
            sum += te->DurationInSeconds();
        }
    }
    return true;
}

bool Context::RecordTimeline() const {
    return user_ && user_->RecordTimeline();
}

_Bool Context::SaveUpdateChannel(
    const std::string channel) {
    update_channel_ = channel;
    return exportErrorState(db_->SaveUpdateChannel(std::string(channel)));
}

_Bool Context::LoadUpdateChannel(std::string *channel) {
    return exportErrorState(db_->LoadUpdateChannel(channel));
}

void Context::ProjectLabelAndColorCode(
    kopsik::TimeEntry *te,
    std::string *project_and_task_label,
    std::string *color_code) const {
    poco_assert(te);
    poco_assert(project_and_task_label);
    poco_assert(color_code);

    if (!user_) {
        return;
    }

    kopsik::Task *t = 0;
    if (te->TID()) {
        t = user_->GetTaskByID(te->TID());
    }

    kopsik::Project *p = 0;
    if (t) {
        p = user_->GetProjectByID(t->PID());
    }
    if (!p && te->PID()) {
        p = user_->GetProjectByID(te->PID());
    }
    if (!p && !te->ProjectGUID().empty()) {
        p = user_->GetProjectByGUID(te->ProjectGUID());
    }

    kopsik::Client *c = 0;
    if (p && p->CID()) {
        c = user_->GetClientByID(p->CID());
    }

    *project_and_task_label = Formatter::JoinTaskName(t, p, c);

    if (p) {
        *color_code = p->ColorCode();
    }
}

bool CompareAutocompleteItems(
    AutocompleteItem a,
    AutocompleteItem b) {

    // Time entries first
    if (a.IsTimeEntry() && !b.IsTimeEntry()) {
        return true;
    }
    if (b.IsTimeEntry() && !(a.IsTimeEntry())) {
        return false;
    }

    // Then tasks
    if (a.IsTask() && !b.IsTask()) {
        return true;
    }
    if (b.IsTask() && !a.IsTask()) {
        return false;
    }

    // Then projects
    if (a.IsProject() && !b.IsProject()) {
        return true;
    }
    if (b.IsProject() && !a.IsProject()) {
        return false;
    }

    return (strcmp(a.Text.c_str(), b.Text.c_str()) < 0);
}

// Add time entries, in format:
// Description - Task. Project. Client
void Context::getTimeEntryAutocompleteItems(
    std::vector<AutocompleteItem> *list) const {
    poco_assert(list);

    if (!user_) {
        logger().warning("User logged out, cannot fetch autocomplete items");
        return;
    }

    for (std::vector<kopsik::TimeEntry *>::const_iterator it =
        user_->related.TimeEntries.begin();
            it != user_->related.TimeEntries.end(); it++) {
        kopsik::TimeEntry *te = *it;

        if (te->DeletedAt() || te->IsMarkedAsDeletedOnServer()
                || te->Description().empty()) {
            continue;
        }

        kopsik::Task *t = 0;
        if (te->TID()) {
            t = user_->GetTaskByID(te->TID());
        }

        kopsik::Project *p = 0;
        if (t && t->PID()) {
            p = user_->GetProjectByID(t->PID());
        } else if (te->PID()) {
            p = user_->GetProjectByID(te->PID());
        }

        if (p && !p->Active()) {
            continue;
        }

        kopsik::Client *c = 0;
        if (p && p->CID()) {
            c = user_->GetClientByID(p->CID());
        }

        std::string project_label = Formatter::JoinTaskNameReverse(t, p, c);

        std::stringstream search_parts;
        search_parts << te->Description();
        std::string description = search_parts.str();
        if (!project_label.empty()) {
            search_parts << " - " << project_label;
        }

        std::string text = search_parts.str();
        if (text.empty()) {
            continue;
        }

        AutocompleteItem autocomplete_item;
        autocomplete_item.Description = description;
        autocomplete_item.Text = text;
        autocomplete_item.ProjectAndTaskLabel = project_label;
        if (p) {
            autocomplete_item.ProjectColor = p->ColorCode();
            autocomplete_item.ProjectID = p->ID();
        }
        if (t) {
            autocomplete_item.TaskID = t->ID();
        }
        autocomplete_item.Type = kAutocompleteItemTE;
        list->push_back(autocomplete_item);
    }
}

// Add tasks, in format:
// Task. Project. Client
void Context::getTaskAutocompleteItems(
    std::vector<AutocompleteItem> *list) const {
    poco_assert(list);

    if (!user_) {
        logger().warning("User logged out, cannot fetch autocomplete items");
        return;
    }

    for (std::vector<kopsik::Task *>::const_iterator it =
        user_->related.Tasks.begin();
            it != user_->related.Tasks.end(); it++) {
        kopsik::Task *t = *it;

        if (t->IsMarkedAsDeletedOnServer()) {
            continue;
        }

        kopsik::Project *p = 0;
        if (t->PID()) {
            p = user_->GetProjectByID(t->PID());
        }

        if (p && !p->Active()) {
            continue;
        }

        kopsik::Client *c = 0;
        if (p && p->CID()) {
            c = user_->GetClientByID(p->CID());
        }

        std::string text = Formatter::JoinTaskNameReverse(t, p, c);
        if (text.empty()) {
            continue;
        }

        AutocompleteItem autocomplete_item;
        autocomplete_item.Text = text;
        autocomplete_item.ProjectAndTaskLabel = text;
        autocomplete_item.TaskID = t->ID();
        if (p) {
            autocomplete_item.ProjectColor = p->ColorCode();
            autocomplete_item.ProjectID = p->ID();
        }
        autocomplete_item.Type = kAutocompleteItemTask;
        list->push_back(autocomplete_item);
    }
}

// Add projects, in format:
// Project. Client
void Context::getProjectAutocompleteItems(
    std::vector<AutocompleteItem> *list) const {
    poco_assert(list);

    if (!user_) {
        logger().warning("User logged out, cannot fetch autocomplete items");
        return;
    }

    for (std::vector<kopsik::Project *>::const_iterator it =
        user_->related.Projects.begin();
            it != user_->related.Projects.end(); it++) {
        kopsik::Project *p = *it;

        if (!p->Active()) {
            continue;
        }

        kopsik::Client *c = 0;
        if (p->CID()) {
            c = user_->GetClientByID(p->CID());
        }

        std::string text = Formatter::JoinTaskNameReverse(0, p, c);
        if (text.empty()) {
            continue;
        }

        AutocompleteItem autocomplete_item;
        autocomplete_item.Text = text;
        autocomplete_item.ProjectAndTaskLabel = text;
        autocomplete_item.ProjectID = p->ID();
        autocomplete_item.ProjectColor = p->ColorCode();
        autocomplete_item.Type = kAutocompleteItemProject;
        list->push_back(autocomplete_item);
    }
}

void Context::AutocompleteItems(
    std::vector<AutocompleteItem> *list,
    const bool include_time_entries,
    const bool include_tasks,
    const bool include_projects) const {
    poco_assert(list);
    if (!user_) {
        return;
    }

    if (include_time_entries) {
        getTimeEntryAutocompleteItems(list);
    }

    if (include_tasks) {
        getTaskAutocompleteItems(list);
    }

    if (include_projects) {
        getProjectAutocompleteItems(list);
    }

    std::sort(list->begin(), list->end(), CompareAutocompleteItems);
}

_Bool Context::AddProject(
    const Poco::UInt64 workspace_id,
    const Poco::UInt64 client_id,
    const std::string project_name,
    const _Bool is_private,
    Project **result) {
    poco_assert(result);

    if (!user_) {
        logger().warning("Cannot add project, user logged out");
        return true;
    }
    if (!workspace_id) {
        return exportErrorState("Please select a workspace");
    }
    if (project_name.empty()) {
        return exportErrorState("Project name must not be empty");
    }

    *result = user_->AddProject(
        workspace_id, client_id, project_name, is_private);
    poco_assert(*result);

    return exportErrorState(save());
}

kopsik::HTTPSClient Context::get_https_client() {
    kopsik::HTTPSClient result(api_url_, app_name_, app_version_);
    bool use_proxy(false);
    kopsik::Proxy proxy;
    poco_assert(noError == db_->LoadProxySettings(&use_proxy, &proxy));
    if (use_proxy) {
        result.SetProxy(proxy);
        logger().debug("Using proxy to connect: " + proxy.String());
    }
    return result;
}

void Context::SetSleep() {
    logger().debug("SetSleep");
}

void Context::SetWake() {
    logger().debug("SetWake");

    next_reminder_at_ = postpone(kReminderThrottleMicros);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onRemind);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_reminder_at_);
}

void Context::onRemind(Poco::Util::TimerTask& task) {  // NOLINT
    if (isPostponed(next_reminder_at_, kReminderThrottleMicros)) {
        logger().debug("onRemind postponed");
        return;
    }
    logger().debug("onRemind executing");

    if (!user_) {
        logger().warning("User logged out, cannot remind");
    }

    bool use_idle_settings(false),
         menubar_timer(false),
         dock_icon(false),
         on_top(false),
         reminder(false);
    if (!LoadSettings(&use_idle_settings,
                      &menubar_timer,
                      &dock_icon,
                      &on_top,
                      &reminder)) {
        logger().error("Could not load settings");
        return;
    }

    if (!reminder) {
        logger().debug("Reminder is not enabled by user");
        return;
    }

    if (user_->RunningTimeEntry()) {
        logger().debug("User is already tracking time, no need to remind");
        return;
    }

    if (user_->HasTrackedTimeToday()) {
        logger().debug("Already tracked time today, no need to remind");
        return;
    }

    on_remind_callback_();
}

}  // namespace kopsik
