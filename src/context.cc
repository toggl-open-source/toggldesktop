// Copyright 2014 kopsik developers

#include "./context.h"
#include "./kopsik_api_private.h"

#include "Poco/Path.h"
#include "Poco/FileStream.h"
#include "Poco/Base64Encoder.h"
#include "Poco/StreamCopier.h"

Context::Context()
  : db(0),
    user(0),
    change_callback(0),
    on_error_callback(0),
    check_updates_callback(0),
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
    feedback_details("") {
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

void Context::FullSync() {
  kopsik::HTTPSClient https_client(api_url, app_name, app_version);
  kopsik::error err = user->Sync(&https_client, 1, true);
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

void Context::PartialSync() {
  kopsik::HTTPSClient https_client(api_url, app_name, app_version);
  kopsik::error err = user->Sync(&https_client, 0, true);
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

void Context::SwitchWebSocketOff() {
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
  poco_assert(!user->APIToken().empty());
  ws_client->Start(this, user->APIToken(), on_websocket_message);
}

// Start/stop timeline recording on local machine
void Context::SwitchTimelineOff() {
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
  if (!user) {
    return;
  }

  if (!user->RecordTimeline()) {
    return;
  }

  Poco::Mutex::ScopedLock lock(mutex);

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
  poco_assert(check_updates_callback);

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
};
