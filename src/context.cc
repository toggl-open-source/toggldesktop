// Copyright 2014 kopsik developers

#include "./context.h"

// FIXME: dont use C API from C++ class
#include "./kopsik_api_private.h"

#include "Poco/Path.h"
#include "Poco/FileStream.h"
#include "Poco/Base64Encoder.h"
#include "Poco/StreamCopier.h"
#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Util/TimerTaskAdapter.h"

const int kRequestThrottleMicros = 2 * 1000000;

Context::Context()
  : db(0),
    user(0),
    ws_client(0),
    timeline_uploader(0),
    window_change_recorder(0),
    app_name(""),
    app_version(""),
    api_url(""),
    timeline_upload_url(""),
    update_channel(""),
    feedback_attachment_path_(""),
    feedback_subject(""),
    feedback_details(""),
    change_callback(0),
    on_error_callback(0),
    check_updates_callback(0),
    full_sync_queued_(false),
    partial_sync_queued_(false),
    fetch_updates_queued_(false),
    update_timeline_settings_queued_(false) {
  Poco::ErrorHandler::set(&error_handler);
  Poco::Net::initializeSSL();
}

Context::~Context() {
  if (window_change_recorder) {
    delete window_change_recorder;
    window_change_recorder = 0;
  }

  if (timeline_uploader) {
    delete timeline_uploader;
    timeline_uploader = 0;
  }

  if (ws_client) {
    delete ws_client;
    ws_client = 0;
  }

  if (db) {
    delete db;
    db = 0;
  }

  if (user) {
    delete user;
    user = 0;
  }

  Poco::Net::uninitializeSSL();
}

void Context::Shutdown() {
  if (window_change_recorder) {
    window_change_recorder->Stop();
  }
  if (ws_client) {
    ws_client->Stop();
  }
  if (timeline_uploader) {
    timeline_uploader->Stop();
  }

  // cancel tasks but allow them finish
  timer_.cancel(true);

  Poco::ThreadPool::defaultPool().joinAll();
}

kopsik::error Context::ConfigureProxy() {
  bool use_proxy(false);
  bool tmp(false);
  kopsik::Proxy proxy;
  kopsik::error err = db->LoadSettings(&use_proxy, &proxy, &tmp);
  if (err != kopsik::noError) {
    return err;
  }
  if (!use_proxy) {
    proxy = kopsik::Proxy();  // reset values
  }
  Poco::Mutex::ScopedLock lock(mutex);

  ws_client->SetProxy(proxy);
  return kopsik::noError;
}

kopsik::error Context::Save() {
  poco_assert(change_callback);

  try {
    std::vector<kopsik::ModelChange> changes;
    kopsik::error err = db->SaveUser(user, true, &changes);
    if (err != kopsik::noError) {
      return err;
    }
    for (std::vector<kopsik::ModelChange>::const_iterator it = changes.begin();
        it != changes.end();
        it++) {
      kopsik::ModelChange mc = *it;
      KopsikModelChange *change = model_change_init();
      model_change_to_change_item(mc, *change);
      change_callback(KOPSIK_API_SUCCESS, 0, change);
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
  kopsik::HTTPSClient https_client(api_url, app_name, app_version);
  kopsik::error err = user->Sync(&https_client, full_sync, true);
  if (err != kopsik::noError) {
    on_error_callback(err.c_str());
    return;
  }

  err = Save();
  if (err != kopsik::noError) {
    on_error_callback(err.c_str());
    return;
  }
}

void Context::FullSync() {
  logger().debug("FullSync");

  if (full_sync_queued_) {
    return;
  }

  Poco::Mutex::ScopedLock lock(mutex);

  full_sync_queued_ = true;
  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onFullSync);
  timer_.schedule(ptask, Poco::Timestamp() + kRequestThrottleMicros);
}

void Context::onFullSync(Poco::Util::TimerTask& task) {  // NOLINT
  logger().debug("onFullSync");

  full_sync_queued_ = false;

  sync(true);
}

void Context::PartialSync() {
  logger().debug("PartialSync");

  if (partial_sync_queued_) {
    return;
  }

  Poco::Mutex::ScopedLock lock(mutex);

  partial_sync_queued_ = true;
  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onPartialSync);
  timer_.schedule(ptask, Poco::Timestamp() + kRequestThrottleMicros);
}

void Context::onPartialSync(Poco::Util::TimerTask& task) {  // NOLINT
  logger().debug("onPartialSync");

  partial_sync_queued_ = false;

  sync(false);
}

void Context::SwitchWebSocketOff() {
  logger().debug("PartialSync");

  Poco::Mutex::ScopedLock lock(mutex);

  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(
      *this, &Context::onSwitchWebSocketOff);
  timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchWebSocketOff(Poco::Util::TimerTask& task) {  // NOLINT
  logger().debug("onSwitchWebSocketOff");

  ws_client->Stop();
}

void on_websocket_message(
    void *context,
    std::string json) {
  poco_assert(context);
  poco_assert(!json.empty());

  Context *ctx = reinterpret_cast<Context *>(context);

  try {
    std::stringstream ss;
    ss << "on_websocket_message json=" << json;
    Poco::Logger &logger = Poco::Logger::get("Context");
    logger.debug(ss.str());

    Context *ctx = reinterpret_cast<Context *>(context);

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    ctx->user->LoadUpdateFromJSONString(json);

    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      ctx->on_error_callback(err.c_str());
      logger.error(err);
      return;
    }
  } catch(const Poco::Exception& exc) {
    ctx->on_error_callback(exc.displayText().c_str());
  } catch(const std::exception& ex) {
    ctx->on_error_callback(ex.what());
  } catch(const std::string& ex) {
    ctx->on_error_callback(ex.c_str());
  }
}

void Context::SwitchWebSocketOn() {
  logger().debug("SwitchWebSocketOn");

  Poco::Mutex::ScopedLock lock(mutex);

  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(
      *this, &Context::onSwitchWebSocketOn);
  timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchWebSocketOn(Poco::Util::TimerTask& task) {  // NOLINT
  logger().debug("onSwitchWebSocketOn");

  poco_assert(!user->APIToken().empty());
  ws_client->Start(this, user->APIToken(), on_websocket_message);
}

// Start/stop timeline recording on local machine
void Context::SwitchTimelineOff() {
  logger().debug("SwitchTimelineOff");

  Poco::Mutex::ScopedLock lock(mutex);

  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(
      *this, &Context::onSwitchTimelineOff);
  timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchTimelineOff(Poco::Util::TimerTask& task) {  // NOLINT
  logger().debug("onSwitchTimelineOff");

  Poco::Mutex::ScopedLock lock(mutex);

  if (window_change_recorder) {
    delete window_change_recorder;
    window_change_recorder = 0;
  }

  if (timeline_uploader) {
    delete timeline_uploader;
    timeline_uploader = 0;
  }
}

void Context::SwitchTimelineOn() {
  logger().debug("SwitchTimelineOn");

  Poco::Mutex::ScopedLock lock(mutex);

  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(
      *this, &Context::onSwitchTimelineOn);
  timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchTimelineOn(Poco::Util::TimerTask& task) {  // NOLINT
  logger().debug("onSwitchTimelineOn");

  Poco::Mutex::ScopedLock lock(mutex);

  if (!user) {
    return;
  }

  if (!user->RecordTimeline()) {
    return;
  }

  if (timeline_uploader) {
    delete timeline_uploader;
    timeline_uploader = 0;
  }
  timeline_uploader = new kopsik::TimelineUploader(
    user->ID(),
    user->APIToken(),
    timeline_upload_url,
    app_name,
    app_version);

  if (window_change_recorder) {
    delete window_change_recorder;
    window_change_recorder = 0;
  }
  window_change_recorder = new kopsik::WindowChangeRecorder(user->ID());
}

void Context::FetchUpdates() {
  logger().debug("FetchUpdates");

  poco_assert(check_updates_callback);

  if (fetch_updates_queued_) {
    return;
  }

  Poco::Mutex::ScopedLock lock(mutex);

  fetch_updates_queued_ = true;
  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onFetchUpdates);
  timer_.schedule(ptask, Poco::Timestamp() + kRequestThrottleMicros);
}

void Context::onFetchUpdates(Poco::Util::TimerTask& task) {  // NOLINT
  logger().debug("onFetchUpdates");

  fetch_updates_queued_ = false;

  std::string response_body("");
  kopsik::HTTPSClient https_client(api_url, app_name, app_version);
  kopsik::error err = https_client.GetJSON(updateURL(),
                                            std::string(""),
                                            std::string(""),
                                            &response_body);
  if (err != kopsik::noError) {
    check_updates_callback(KOPSIK_API_FAILURE, err.c_str(), 0, 0, 0);
    return;
  }

  if ("null" == response_body) {
    check_updates_callback(KOPSIK_API_SUCCESS, 0, 0, 0, 0);
    return;
  }

  if (!json_is_valid(response_body.c_str())) {
    check_updates_callback(
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

  check_updates_callback(
    KOPSIK_API_SUCCESS,
    err.c_str(),
    1,
    url.c_str(),
    version.c_str());
}

const std::string Context::updateURL() {
  poco_assert(!update_channel.empty());
  poco_assert(!app_version.empty());

  std::stringstream relative_url;
  relative_url << "/api/v8/updates?app=kopsik"
    << "&channel=" << update_channel
    << "&platform=" << osName()
    << "&version=" << app_version;
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

  if (update_timeline_settings_queued_) {
    return;
  }

  Poco::Mutex::ScopedLock lock(mutex);

  update_timeline_settings_queued_ = true;

  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(*this,
        &Context::onTimelineUpdateServerSettings);
  timer_.schedule(ptask, Poco::Timestamp() + kRequestThrottleMicros);
}

void Context::onTimelineUpdateServerSettings(Poco::Util::TimerTask& task) {  // NOLINT
  logger().debug("onTimelineUpdateServerSettings");

  update_timeline_settings_queued_ = false;

  kopsik::HTTPSClient https_client(api_url, app_name, app_version);

  std::string json("{\"record_timeline\": false}");
  if (user->RecordTimeline()) {
    json = "{\"record_timeline\": true}";
  }

  std::string response_body("");
  kopsik::error err = https_client.PostJSON("/api/v8/timeline_settings",
                                            json,
                                            user->APIToken(),
                                            "api_token",
                                            &response_body);
  if (err != kopsik::noError) {
    logger().warning(err);
  }
}

std::string Context::feedbackJSON() {
  JSONNODE *root = json_new(JSON_NODE);
  json_push_back(root, json_new_b("desktop", true));
  json_push_back(root, json_new_a("toggl_version", app_version.c_str()));
  json_push_back(root, json_new_a("details", feedback_details.c_str()));
  json_push_back(root, json_new_a("subject", feedback_subject.c_str()));
  if (!feedback_attachment_path_.empty()) {
    json_push_back(root, json_new_a("base64_encoded_attachment",
                                    base64encode_attachment().c_str()));
    json_push_back(root, json_new_a("attachment_name",
                                    feedback_filename().c_str()));
  }
  json_char *jc = json_write_formatted(root);
  std::string json(jc);
  json_free(jc);
  json_delete(root);
  return json;
}

std::string Context::feedback_filename() {
  Poco::Path p(true);
  bool ok = p.tryParse(feedback_attachment_path_);
  if (!ok) {
    return "";
  }
  return p.getFileName();
}

std::string Context::base64encode_attachment() {
  std::ostringstream oss;
  Poco::FileInputStream fis(feedback_attachment_path_);
  if (!fis.good()) {
    logger().error("Failed to load attached image");
    return "";
  }
  Poco::Base64Encoder encoder(oss);
  encoder.rdbuf()->setLineLength(0);  // disable line feeds in output
  Poco::StreamCopier::copyStream(fis, encoder);
  encoder.close();
  return oss.str();
}

void Context::SendFeedback() {
  logger().debug("SendFeedback");

  Poco::Mutex::ScopedLock lock(mutex);

  Poco::Util::TimerTask::Ptr ptask =
    new Poco::Util::TimerTaskAdapter<Context>(
      *this, &Context::onSendFeedback);
  timer_.schedule(ptask, Poco::Timestamp());
};

void Context::onSendFeedback(Poco::Util::TimerTask& task) {  // NOLINT
  logger().debug("onSendFeedback");

  kopsik::HTTPSClient https_client(api_url, app_name, app_version);
  std::string response_body("");
  kopsik::error err = https_client.PostJSON("/api/v8/feedback",
                                            feedbackJSON(),
                                            user->APIToken(),
                                            "api_token",
                                            &response_body);
  if (err != kopsik::noError) {
    on_error_callback(err.c_str());
    return;
  }
}

