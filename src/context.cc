
// Copyright 2014 Toggl Desktop developers.

// No exceptions should be thrown from this class.
// If pointers to models are returned from this
// class, the ownership does not change and you
// must not delete the pointers you got.

#include "./context.h"
#include "./formatter.h"
#include "./json.h"

// FIXME: dont use C API from C++ class
#include "./kopsik_api_private.h"

#include "Poco/Path.h"
#include "Poco/FileStream.h"
#include "Poco/Base64Encoder.h"
#include "Poco/StreamCopier.h"
#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Util/TimerTaskAdapter.h"

namespace kopsik {

const int kRequestThrottleMicros = 2 * 1000000;

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
    feedback_attachment_path_(""),
    feedback_subject_(""),
    feedback_details_(""),
    change_callback_(0),
    on_error_callback_(0),
    check_updates_callback_(0),
    next_full_sync_at_(0),
    next_partial_sync_at_(0),
    next_fetch_updates_at_(0),
    next_update_timeline_settings_at_(0) {
  Poco::ErrorHandler::set(&error_handler_);
  Poco::Net::initializeSSL();
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

  if (user_) {
    Poco::Mutex::ScopedLock lock(user_m_);
    delete user_;
    user_ = 0;
  }

  Poco::Net::uninitializeSSL();
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

kopsik::error Context::ConfigureProxy() {
  bool use_proxy(false);
  bool tmp(false);
  kopsik::Proxy proxy;
  kopsik::error err = db_->LoadSettings(&use_proxy, &proxy, &tmp);
  if (err != kopsik::noError) {
    return err;
  }
  if (!use_proxy) {
    proxy = kopsik::Proxy();  // reset values
  }

  Poco::Mutex::ScopedLock lock(ws_client_m_);
  ws_client_->SetProxy(proxy);

  return kopsik::noError;
}

kopsik::error Context::save() {
  poco_assert(change_callback_);

  try {
    std::vector<kopsik::ModelChange> changes;
    kopsik::error err = db_->SaveUser(user_, true, &changes);
    if (err != kopsik::noError) {
      return err;
    }
    for (std::vector<kopsik::ModelChange>::const_iterator it = changes.begin();
        it != changes.end();
        it++) {
      kopsik::ModelChange mc = *it;
      KopsikModelChange *change = model_change_init();
      model_change_to_change_item(mc, *change);
      change_callback_(KOPSIK_API_SUCCESS, 0, change);
      model_change_clear(change);
    }
  } catch(const Poco::Exception& exc) {
    return exc.displayText();
  } catch(const std::exception& ex) {
    return ex.what();
  } catch(const std::string& ex) {
    return ex;
  }
  return kopsik::noError;
}

void Context::sync(const bool full_sync) {
  kopsik::HTTPSClient https_client(api_url_, app_name_, app_version_);
  kopsik::error err = user_->Sync(&https_client, full_sync, true);
  if (err != kopsik::noError) {
    on_error_callback_(err.c_str());
    return;
  }

  err = save();
  if (err != kopsik::noError) {
    on_error_callback_(err.c_str());
    return;
  }
}

void Context::FullSync() {
  logger().debug("FullSync");

  next_full_sync_at_ = Poco::Timestamp() + kRequestThrottleMicros;
  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onFullSync);

  Poco::Mutex::ScopedLock lock(timer_m_);
  timer_.schedule(ptask, next_full_sync_at_);
}

void Context::onFullSync(Poco::Util::TimerTask& task) {  // NOLINT
  if (next_full_sync_at_ > Poco::Timestamp()) {
    logger().debug("onFullSync postponed");
    return;
  }
  logger().debug("onFullSync executing");

  sync(true);
}

void Context::partialSync() {
  logger().debug("partialSync");

  next_partial_sync_at_ = Poco::Timestamp() + kRequestThrottleMicros;
  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onPartialSync);

  Poco::Mutex::ScopedLock lock(timer_m_);
  timer_.schedule(ptask, next_partial_sync_at_);
}

void Context::onPartialSync(Poco::Util::TimerTask& task) {  // NOLINT
  if (next_partial_sync_at_ > Poco::Timestamp()) {
    logger().debug("onPartialSync postponed");
    return;
  }
  logger().debug("onPartialSync executing");
  sync(false);
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

void Context::LoadUpdateFromJSONString(const std::string json) {
  try {
    std::stringstream ss;
    ss << "LoadUpdateFromJSONString json=" << json;
    logger().debug(ss.str());

    if (!user_) {
      logger().warning("User is already logged out, cannot load update JSON");
      return;
    }

    LoadUserUpdateFromJSONString(user_, json);

    kopsik::error err = save();
    if (err != kopsik::noError) {
      on_error_callback_(err.c_str());
      return;
    }
  } catch(const Poco::Exception& exc) {
    on_error_callback_(exc.displayText().c_str());
  } catch(const std::exception& ex) {
    on_error_callback_(ex.what());
  } catch(const std::string& ex) {
    on_error_callback_(ex.c_str());
  }
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

  poco_assert(check_updates_callback_);

  kopsik::error err = db_->LoadUpdateChannel(&update_channel_);
  if (err != kopsik::noError) {
    // FIXME: don't use c callbacks from here
    check_updates_callback_(KOPSIK_API_FAILURE, err.c_str(), 0, 0, 0);
    return;
  }

  next_fetch_updates_at_ = Poco::Timestamp() + kRequestThrottleMicros;
  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onFetchUpdates);

  Poco::Mutex::ScopedLock lock(timer_m_);
  timer_.schedule(ptask, next_fetch_updates_at_);
}

void Context::onFetchUpdates(Poco::Util::TimerTask& task) {  // NOLINT
  if (next_fetch_updates_at_ > Poco::Timestamp()) {
    logger().debug("onFetchUpdates postponed");
    return;
  }

  logger().debug("onFetchUpdates executing");

  std::string response_body("");
  kopsik::HTTPSClient https_client(api_url_, app_name_, app_version_);
  kopsik::error err = https_client.GetJSON(updateURL(),
                                            std::string(""),
                                            std::string(""),
                                            &response_body);
  if (err != kopsik::noError) {
    check_updates_callback_(KOPSIK_API_FAILURE, err.c_str(), 0, 0, 0);
    return;
  }

  if ("null" == response_body) {
    check_updates_callback_(KOPSIK_API_SUCCESS, 0, 0, 0, 0);
    return;
  }

  if (!IsValidJSON(response_body)) {
    check_updates_callback_(
      KOPSIK_API_FAILURE, "Invalid response JSON", 0, 0, 0);
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

  check_updates_callback_(
    KOPSIK_API_SUCCESS,
    err.c_str(),
    1,
    url.c_str(),
    version.c_str());
}

const std::string Context::updateURL() const {
  poco_assert(!update_channel_.empty());
  poco_assert(!app_version_.empty());

  std::stringstream relative_url;
  relative_url << "/api/v8/updates?app=kopsik"
    << "&channel=" << update_channel_
    << "&platform=" << osName()
    << "&version=" << app_version_;
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

  next_update_timeline_settings_at_ =
    Poco::Timestamp() + kRequestThrottleMicros;
  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(*this,
        &Context::onTimelineUpdateServerSettings);

  Poco::Mutex::ScopedLock lock(timer_m_);
  timer_.schedule(ptask, next_update_timeline_settings_at_);
}

const std::string kRecordTimelineEnabledJSON = "{\"record_timeline\": true}";
const std::string kRecordTimelineDisabledJSON = "{\"record_timeline\": false}";

void Context::onTimelineUpdateServerSettings(Poco::Util::TimerTask& task) {  // NOLINT
  if (next_update_timeline_settings_at_ > Poco::Timestamp()) {
    logger().debug("onTimelineUpdateServerSettings postponed");
    return;
  }

  logger().debug("onTimelineUpdateServerSettings executing");

  kopsik::HTTPSClient https_client(api_url_, app_name_, app_version_);

  std::string json(kRecordTimelineDisabledJSON);
  if (user_->RecordTimeline()) {
    json = kRecordTimelineEnabledJSON;
  }

  std::string response_body("");
  kopsik::error err = https_client.PostJSON("/api/v8/timeline_settings",
                                            json,
                                            user_->APIToken(),
                                            "api_token",
                                            &response_body);
  if (err != kopsik::noError) {
    logger().warning(err);
  }
}

const std::string Context::feedbackJSON() const {
  JSONNODE *root = json_new(JSON_NODE);
  json_push_back(root, json_new_b("desktop", true));
  json_push_back(root, json_new_a("toggl_version", app_version_.c_str()));
  json_push_back(root, json_new_a("details",
    Formatter::EscapeJSONString(feedback_details_).c_str()));
  json_push_back(root, json_new_a("subject",
    Formatter::EscapeJSONString(feedback_subject_).c_str()));
  if (!feedback_attachment_path_.empty()) {
    json_push_back(root, json_new_a("base64_encoded_attachment",
      base64encode_attachment().c_str()));
    json_push_back(root, json_new_a("attachment_name",
      Formatter::EscapeJSONString(feedback_filename()).c_str()));
  }
  json_char *jc = json_write_formatted(root);
  std::string json(jc);
  json_free(jc);
  json_delete(root);
  return json;
}

const std::string Context::feedback_filename() const {
  Poco::Path p(true);
  bool ok = p.tryParse(feedback_attachment_path_);
  if (!ok) {
    return "";
  }
  return p.getFileName();
}

const std::string Context::base64encode_attachment() const {
  std::ostringstream oss;
  Poco::FileInputStream fis(feedback_attachment_path_);
  if (!fis.good()) {
    return "";
  }
  Poco::Base64Encoder encoder(oss);
  encoder.rdbuf()->setLineLength(0);  // disable line feeds in output
  Poco::StreamCopier::copyStream(fis, encoder);
  encoder.close();
  return oss.str();
}

kopsik::error Context::SendFeedback(
    const std::string topic,
    const std::string details,
    const std::string filename) {
  if (!user_) {
    return kopsik::error("Please login to send feedback");
  }
  if (topic.empty()) {
    return kopsik::error("Missing topic");
  }
  if (details.empty()) {
    return kopsik::error("Missing details");
  }
  feedback_subject_ = topic;
  feedback_details_ = details;
  feedback_attachment_path_ = filename;

  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(
      *this, &Context::onSendFeedback);

  Poco::Mutex::ScopedLock lock(timer_m_);
  timer_.schedule(ptask, Poco::Timestamp());

  return kopsik::noError;
};

void Context::onSendFeedback(Poco::Util::TimerTask& task) {  // NOLINT
  logger().debug("onSendFeedback");

  kopsik::HTTPSClient https_client(api_url_, app_name_, app_version_);
  std::string response_body("");
  kopsik::error err = https_client.PostJSON("/api/v8/feedback",
                                            feedbackJSON(),
                                            user_->APIToken(),
                                            "api_token",
                                            &response_body);
  if (err != kopsik::noError) {
    on_error_callback_(err.c_str());
    return;
  }
}

void Context::SetChangeCallback(KopsikViewItemChangeCallback cb) {
    change_callback_ = cb;
}
void Context::SetOnErrorCallback(KopsikErrorCallback cb) {
    on_error_callback_ = cb;
}
void Context::SetCheckUpdatesCallback(KopsikCheckUpdateCallback cb) {
    check_updates_callback_ = cb;
}

void Context::SetAPIURL(const std::string value) {
  api_url_ = value;
}

void Context::SetTimelineUploadURL(const std::string value) {
    timeline_upload_url_ = value;
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

kopsik::error Context::LoadSettings(
    bool *use_proxy,
    kopsik::Proxy *proxy,
    bool *use_idle_settings) const {
  return db_->LoadSettings(use_proxy, proxy, use_idle_settings);
}

kopsik::error Context::SaveSettings(
    const bool use_proxy,
    const kopsik::Proxy *proxy,
    const bool use_idle_detection) {

  kopsik::Proxy previous_proxy_settings;
  bool was_using_proxy(false);
  {
    bool was_using_idle_detection(false);
    kopsik::error err = LoadSettings(&was_using_proxy,
                                     &previous_proxy_settings,
                                     &was_using_idle_detection);
    if (err != kopsik::noError) {
      return err;
    }
  }

  kopsik::error err = db_->SaveSettings(use_proxy, proxy, use_idle_detection);
  if (err != kopsik::noError) {
    return err;
  }

  // If proxy settings have changed, apply new settings:
  if (use_proxy != was_using_proxy
      || proxy->host != previous_proxy_settings.host
      || proxy->port != previous_proxy_settings.port
      || proxy->username != previous_proxy_settings.username
      || proxy->password != previous_proxy_settings.password) {
    kopsik::error err = ConfigureProxy();
    if (err != kopsik::noError) {
      return err;
    }
    if (user_) {
      FullSync();
      SwitchWebSocketOn();
    }
  }

  return kopsik::noError;
}

void Context::SetDBPath(
    const std::string path) {

  Poco::Mutex::ScopedLock lock(db_m_);
  if (db_) {
    delete db_;
    db_ = 0;
  }
  db_ = new kopsik::Database(path);
}

kopsik::error Context::CurrentAPIToken(std::string *token) {
    return db_->CurrentAPIToken(token);
}

kopsik::error Context::SetCurrentAPIToken(
    const std::string token) {
  return db_->SetCurrentAPIToken(token);
}

kopsik::error Context::CurrentUser(kopsik::User **result) {
  poco_assert(!*result);

  if (user_) {
    *result = user_;
    return kopsik::noError;
  }

  kopsik::User *user = new kopsik::User(app_name_, app_version_);
  kopsik::error err = db_->LoadCurrentUser(user, true);
  if (err != kopsik::noError) {
    delete user;
    return err;
  }

  Poco::Mutex::ScopedLock lock(user_m_);
  user_ = user;

  *result = user_;
  return kopsik::noError;
}

kopsik::error Context::Login(
    const std::string email,
    const std::string password) {
  kopsik::User *logging_in = new kopsik::User(app_name_, app_version_);

  kopsik::HTTPSClient https_client(api_url_,
                                   app_name_,
                                   app_version_);
  kopsik::error err = logging_in->Login(&https_client, email, password);
  if (err != kopsik::noError) {
    delete logging_in;
    return err;
  }

  poco_assert(logging_in->ID() > 0);

  err = db_->LoadUserByID(logging_in->ID(), logging_in, true);
  if (err != kopsik::noError) {
    delete logging_in;
    return err;
  }

  err = db_->SetCurrentAPIToken(logging_in->APIToken());
  if (err != kopsik::noError) {
    delete logging_in;
    return err;
  }

  Poco::Mutex::ScopedLock lock(user_m_);
  if (user_) {
    delete user_;
  }
  user_ = logging_in;

  return save();
}

kopsik::error Context::SetLoggedInUserFromJSON(
    const std::string json) {
  kopsik::User *import = new kopsik::User(app_name_, app_version_);

  LoadUserFromJSONString(import, json, true, true);

  kopsik::error err = db_->SetCurrentAPIToken(import->APIToken());
  if (err != kopsik::noError) {
    delete import;
    return err;
  }

  Poco::Mutex::ScopedLock lock(user_m_);
  if (user_) {
    delete user_;
  }
  user_ = import;

  return save();
}

kopsik::error Context::Logout() {
  try {
    if (!user_) {
      logger().warning("User is logged out, cannot logout again");
      return kopsik::noError;
    }

    Shutdown();

    kopsik::error err = db_->ClearCurrentAPIToken();
    if (err != kopsik::noError) {
      return err;
    }

    if (user_) {
      Poco::Mutex::ScopedLock lock(user_m_);
      delete user_;
      user_ = 0;
    }
  } catch(const Poco::Exception& exc) {
    return exc.displayText();
  } catch(const std::exception& ex) {
    return ex.what();
  } catch(const std::string& ex) {
    return ex;
  }
  return kopsik::noError;
}

kopsik::error Context::ClearCache() {
  try {
    if (!user_) {
      logger().warning("User is logged out, cannot clear cache");
      return kopsik::noError;
    }
    kopsik::error err = db_->DeleteUser(user_, true);
    if (err != kopsik::noError) {
      return err;
    }

    return Logout();
  } catch(const Poco::Exception& exc) {
      return exc.displayText();
  } catch(const std::exception& ex) {
      return ex.what();
  } catch(const std::string& ex) {
      return ex;
  }
  return kopsik::noError;
}

bool Context::UserHasPremiumWorkspaces() const {
  return (user_ && user_->HasPremiumWorkspaces());
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

kopsik::TimeEntry *Context::Start(
    const std::string description,
    const std::string duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id) {
  if (!user_) {
    return 0;
  }
  kopsik::TimeEntry *te =
    user_->Start(description, duration, task_id, project_id);
  if (!te) {
    return 0;
  }
  save();
  if (te->NeedsPush()) {
    partialSync();
  }
  return te;
}

kopsik::TimeEntry *Context::ContinueLatest() {
  if (!user_) {
    return 0;
  }
  user_->SortTimeEntriesByStart();
  kopsik::TimeEntry *latest = user_->Latest();
  if (!latest) {
    return 0;
  }
  kopsik::TimeEntry *te = user_->Continue(latest->GUID());
  if (!te) {
    return 0;
  }
  save();
  if (te->NeedsPush()) {
    partialSync();
  }
  return te;
}

kopsik::TimeEntry *Context::Continue(const std::string GUID) {
  if (!user_) {
    return 0;
  }
  kopsik::TimeEntry *te = user_->Continue(GUID);
  if (!te) {
    return 0;
  }
  save();
  if (te->NeedsPush()) {
    partialSync();
  }
  return te;
}

kopsik::error Context::DeleteTimeEntryByGUID(const std::string GUID) {
  if (!user_) {
    return kopsik::error("Please login to delete time entry");
  }
  kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
  if (!te) {
    return kopsik::error("Time entry not found");
  }
  te->SetDeletedAt(time(0));
  te->SetUIModifiedAt(time(0));

/* FIXME: callback
  kopsik::ModelChange mc("time_entry", "delete", te->ID(), te->GUID());
  KopsikModelChange *change = model_change_init();
  model_change_to_change_item(mc, *change);
  ctx->change_callback(KOPSIK_API_SUCCESS, 0, change);
  model_change_clear(change);
*/

  save();
  partialSync();
  return kopsik::noError;
}

kopsik::TimeEntry *Context::GetTimeEntryByGUID(const std::string GUID) const {
  if (!user_) {
    return 0;
  }
  return user_->GetTimeEntryByGUID(GUID);
}

kopsik::error Context::SetTimeEntryDuration(
    const std::string GUID,
    const std::string duration) {
  if (GUID.empty()) {
    return kopsik::error("Missing GUID");
  }
  if (!user_) {
    return kopsik::error("Log in to set time entry duration");
  }
  kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
  if (!te) {
    return kopsik::error("Time entry not found");
  }
  te->SetDurationString(duration);
  if (te->Dirty()) {
    te->SetUIModifiedAt(time(0));
  }
  save();
  if (te->NeedsPush()) {
    partialSync();
  }
  return kopsik::noError;
}

kopsik::error Context::SetTimeEntryProject(
    const std::string GUID,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id) {
  if (GUID.empty()) {
    return kopsik::error("Missing GUID");
  }
  if (!user_) {
    return kopsik::error("Please login to select project");
  }
  kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
  if (!te) {
    return kopsik::error("Time entry not found");
  }
  if (project_id) {
    kopsik::Project *p = user_->GetProjectByID(project_id);
    if (p) {
      te->SetBillable(p->Billable());
    }
  }
  te->SetTID(task_id);
  te->SetPID(project_id);
  if (te->Dirty()) {
    te->SetUIModifiedAt(time(0));
  }
  save();
  if (te->NeedsPush()) {
    partialSync();
  }
  return kopsik::noError;
}

kopsik::error Context::SetTimeEntryStartISO8601(
    const std::string GUID,
    const std::string value) {
  if (GUID.empty()) {
    return kopsik::error("Missing GUID");
  }
  if (!user_) {
    return kopsik::error("Please login to change time entry start time");
  }
  kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
  if (!te) {
    return kopsik::error("Time entry not found");
  }
  te->SetStartString(value);
  if (te->Dirty()) {
    te->SetUIModifiedAt(time(0));
  }
  save();
  if (te->NeedsPush()) {
    partialSync();
  }
  return kopsik::noError;
}

kopsik::error Context::SetTimeEntryEndISO8601(
    const std::string GUID,
    const std::string value) {
  if (GUID.empty()) {
    return kopsik::error("Missing GUID");
  }
  if (!user_) {
    return kopsik::error("Please login to change time entry end time");
  }
  kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
  if (!te) {
    return kopsik::error("Time entry not found");
  }
  te->SetStopString(value);
  if (te->Dirty()) {
    te->SetUIModifiedAt(time(0));
  }
  save();
  if (te->NeedsPush()) {
    partialSync();
  }
  return kopsik::noError;
}

kopsik::error Context::SetTimeEntryTags(
    const std::string GUID,
    const std::string value) {
  if (GUID.empty()) {
    return kopsik::error("Missing GUID");
  }
  if (!user_) {
    return kopsik::error("Please login to change time entry tags");
  }
  kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
  if (!te) {
    return kopsik::error("Time entry not found");
  }
  te->SetTags(value);
  if (te->Dirty()) {
    te->SetUIModifiedAt(time(0));
  }
  save();
  if (te->NeedsPush()) {
    partialSync();
  }
  return kopsik::noError;
}

kopsik::error Context::SetTimeEntryBillable(
    const std::string GUID,
    const bool value) {
  if (GUID.empty()) {
    return kopsik::error("Missing GUID");
  }
  if (!user_) {
    return kopsik::error("Please login to change time entry billable state");
  }
  kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
  if (!te) {
    return kopsik::error("Time entry not found");
  }
  te->SetBillable(value);
  if (te->Dirty()) {
    te->SetUIModifiedAt(time(0));
  }
  save();
  if (te->NeedsPush()) {
    partialSync();
  }
  return kopsik::noError;
}

kopsik::error Context::SetTimeEntryDescription(
    const std::string GUID,
    const std::string value) {
  if (GUID.empty()) {
    return kopsik::error("Missing GUID");
  }
  if (!user_) {
    return kopsik::error("Please login to change time entry description");
  }
  kopsik::TimeEntry *te = user_->GetTimeEntryByGUID(GUID);
  if (!te) {
    return kopsik::error("Time entry not found");
  }
  te->SetDescription(value);
  if (te->Dirty()) {
    te->SetUIModifiedAt(time(0));
  }
  save();
  if (te->NeedsPush()) {
    partialSync();
  }
  return kopsik::noError;
}

kopsik::error Context::Stop(kopsik::TimeEntry **stopped_entry) {
  *stopped_entry = 0;
  if (!user_) {
    return kopsik::error("Please login to stop time tracking");
  }

  std::vector<kopsik::TimeEntry *> stopped = user_->Stop();
  if (stopped.empty()) {
    return kopsik::error("No time entry was found to stop");
  }
  *stopped_entry = stopped[0];
  save();
  if ((*stopped_entry)->NeedsPush()) {
    partialSync();
  }
  return kopsik::noError;
}

kopsik::error Context::SplitAt(
    const Poco::Int64 at,
    kopsik::TimeEntry **new_running_entry) {
  *new_running_entry = 0;
  if (!user_) {
    return kopsik::error("Pleae login to split time entry");
  }

  *new_running_entry = user_->SplitAt(at);
  if (!*new_running_entry) {
    return kopsik::error("Failed to split tracking time entry");
  }
  if ((*new_running_entry)->NeedsPush()) {
    partialSync();
  }
  return kopsik::noError;
}

kopsik::error Context::StopAt(
    const Poco::Int64 at,
    kopsik::TimeEntry **stopped) {
  if (!user_) {
    return kopsik::error("Please login to stop running time entry");
  }

  *stopped = user_->StopAt(at);
  if (!stopped) {
    return kopsik::error("Time entry not found to stop");
  }
  save();
  if ((*stopped)->NeedsPush()) {
    partialSync();
  }
  return kopsik::noError;
}

kopsik::error Context::RunningTimeEntry(
    kopsik::TimeEntry **running) {
  if (!user_) {
    return kopsik::error("Please login to access tracking time entry");
  }
  *running = user_->RunningTimeEntry();
  return kopsik::noError;
}

kopsik::error Context::ToggleTimelineRecording() {
  if (!user_) {
    return kopsik::error("Please login to change timeline settings");
  }
  try {
    user_->SetRecordTimeline(!user_->RecordTimeline());
    save();
    TimelineUpdateServerSettings();
    if (user_->RecordTimeline()) {
      SwitchTimelineOn();
      return kopsik::noError;
    }
    SwitchTimelineOff();
  } catch(const Poco::Exception& exc) {
    return exc.displayText();
  } catch(const std::exception& ex) {
    return ex.what();
  } catch(const std::string& ex) {
    return ex;
  }
  return kopsik::noError;
}

kopsik::error Context::TimeEntries(
    std::map<std::string, Poco::Int64> *date_durations,
    std::vector<kopsik::TimeEntry *> *visible) {
  if (!user_) {
    logger().warning("User is already logged out, cannot fetch time entries");
    return kopsik::noError;
  }

  user_->SortTimeEntriesByStart();
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
  return kopsik::noError;
}

kopsik::error Context::TrackedPerDateHeader(
    const std::string date_header,
    int *sum) {
  if (!user_) {
    return kopsik::error("Please login to access time entries");
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
  return kopsik::noError;
}

bool Context::RecordTimeline() {
  return user_ && user_->RecordTimeline();
}

kopsik::error Context::SaveUpdateChannel(
    const std::string channel) {
  update_channel_ = channel;
  return db_->SaveUpdateChannel(std::string(channel));
}

kopsik::error Context::LoadUpdateChannel(std::string *channel) {
  return db_->LoadUpdateChannel(channel);
}

void Context::ProjectLabelAndColorCode(
    kopsik::TimeEntry *te,
    std::string *project_and_task_label,
    std::string *color_code) {
  if (!user_) {
    logger().warning("User is already logged out, cannot fetch project info");
    return;
  }

  kopsik::Task *t = 0;
  if (te->TID()) {
    t = user_->GetTaskByID(te->TID());
  }

  kopsik::Project *p = 0;
  if (t) {
    p = user_->GetProjectByID(t->PID());
  } else if (te->PID()) {
    p = user_->GetProjectByID(te->PID());
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

}  // namespace kopsik
