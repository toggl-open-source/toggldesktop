// Copyright 2014 kopsik developers

#include <string>

#include "Poco/Task.h"
#include "Poco/Base64Encoder.h"
#include "Poco/FileStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"

#include "./kopsik_api.h"
#include "./websocket_client.h"
#include "./https_client.h"
#include "./tasks.h"

void SyncTask::runTask() {
  if (isCancelled()) {
    return;
  }

  kopsik::HTTPSClient https_client(context()->api_url,
                                   context()->app_name,
                                   context()->app_version);
  kopsik::error err = context()->user->Sync(&https_client, full_sync_, true);
  if (err != kopsik::noError) {
    result_callback()(KOPSIK_API_FAILURE, err.c_str());
    return;
  }

  err = context()->Save();
  if (err != kopsik::noError) {
    result_callback()(KOPSIK_API_FAILURE, err.c_str());
    return;
  }

  result_callback()(KOPSIK_API_SUCCESS, 0);
}

void PushTask::runTask() {
  if (isCancelled()) {
    return;
  }

  kopsik::HTTPSClient https_client(context()->api_url,
                                   context()->app_name,
                                   context()->app_version);
  kopsik::error err = context()->user->Push(&https_client);
  if (err != kopsik::noError) {
    result_callback()(KOPSIK_API_FAILURE, err.c_str());
    return;
  }

  err = context()->Save();
  if (err != kopsik::noError) {
    result_callback()(KOPSIK_API_FAILURE, err.c_str());
    return;
  }

  result_callback()(KOPSIK_API_SUCCESS, 0);
}

void WebSocketStartTask::runTask() {
  kopsik::WebSocketMessageCallback cb =
    reinterpret_cast<kopsik::WebSocketMessageCallback>(callback());
  context()->ws_client->Start(
    context(),
    context()->user->APIToken(),
    cb);
}

void WebSocketStopTask::runTask() {
  context()->ws_client->Stop();
}

void TimelineStartTask::runTask() {
  if (!context()->user) {
    result_callback()(KOPSIK_API_FAILURE, "Please login to start timeline");
    return;
  }

  if (!context()->user->RecordTimeline()) {
    result_callback()(KOPSIK_API_SUCCESS, 0);
    return;
  }

  Poco::Mutex::ScopedLock lock(context()->mutex);

  if (context()->timeline_uploader) {
    delete context()->timeline_uploader;
    context()->timeline_uploader = 0;
  }
  context()->timeline_uploader = new kopsik::TimelineUploader(
    context()->user->ID(),
    context()->user->APIToken(),
    context()->api_url,
    context()->app_name,
    context()->app_version);

  if (context()->window_change_recorder) {
    delete context()->window_change_recorder;
    context()->window_change_recorder = 0;
  }
  context()->window_change_recorder = new kopsik::WindowChangeRecorder(
    context()->user->ID());

  result_callback()(KOPSIK_API_SUCCESS, 0);
}

void TimelineStopTask::runTask() {
  Poco::Mutex::ScopedLock lock(context()->mutex);

  if (context()->window_change_recorder) {
    delete context()->window_change_recorder;
    context()->window_change_recorder = 0;
  }

  if (context()->timeline_uploader) {
    delete context()->timeline_uploader;
    context()->timeline_uploader = 0;
  }

  result_callback()(KOPSIK_API_SUCCESS, 0);
}

void TimelineUpdateServerSettingsTask::runTask() {
  kopsik::HTTPSClient https_client(context()->api_url,
                                   context()->app_name,
                                   context()->app_version);

  std::string json("{\"record_timeline\": false}");
  if (context()->user->RecordTimeline()) {
    json = "{\"record_timeline\": true}";
  }

  Poco::Logger &logger = Poco::Logger::get("TimelineEnableTask");
  logger.debug(json);

  std::string response_body("");
  kopsik::error err = https_client.PostJSON("/api/v8/timeline_settings",
                                            json,
                                            context()->user->APIToken(),
                                            "api_token",
                                            &response_body);
  if (err != kopsik::noError) {
    // Network can be down at times, carry on
    logger.warning(err);
  }
}

std::string SendFeedbackTask::feedbackJSON() {
  JSONNODE *root = json_new(JSON_NODE);
  json_push_back(root, json_new_b("desktop", true));
  json_push_back(root, json_new_a("toggl_version",
    context()->app_version.c_str()));
  json_push_back(root, json_new_a("details", details_.c_str()));
  json_push_back(root, json_new_a("subject", subject_.c_str()));
  if (!attachment_path_.empty()) {
    json_push_back(root, json_new_a("base64_encoded_attachment",
                                    base64encode_attachment().c_str()));
    json_push_back(root, json_new_a("attachment_name",
                                    filename().c_str()));
  }
  json_char *jc = json_write_formatted(root);
  std::string json(jc);
  json_free(jc);
  json_delete(root);
  return json;
}

std::string SendFeedbackTask::filename() {
  Poco::Path p(true);
  bool ok = p.tryParse(attachment_path_);
  if (!ok) {
    return "";
  }
  return p.getFileName();
}

std::string SendFeedbackTask::base64encode_attachment() {
  std::ostringstream oss;
  Poco::FileInputStream fis(attachment_path_);
  if (!fis.good()) {
    Poco::Logger &logger = Poco::Logger::get("SendFeedbackTask");
    logger.error("Failed to load attached image");
    return "";
  }
  Poco::Base64Encoder encoder(oss);
  encoder.rdbuf()->setLineLength(0);  // disable line feeds in output
  Poco::StreamCopier::copyStream(fis, encoder);
  encoder.close();
  return oss.str();
}

void SendFeedbackTask::runTask() {
  kopsik::HTTPSClient https_client(context()->api_url,
                                   context()->app_name,
                                   context()->app_version);
  std::string response_body("");
  kopsik::error err = https_client.PostJSON("/api/v8/feedback",
                                            feedbackJSON(),
                                            context()->user->APIToken(),
                                            "api_token",
                                            &response_body);
  if (err != kopsik::noError) {
    result_callback()(KOPSIK_API_FAILURE, err.c_str());
    return;
  }

  result_callback()(KOPSIK_API_SUCCESS, 0);
};

void FetchUpdatesTask::runTask() {
  std::string response_body("");
  kopsik::HTTPSClient https_client(
    context()->api_url,
    context()->app_name,
    context()->app_version);
  kopsik::error err = https_client.GetJSON(updateURL(),
                                                  std::string(""),
                                                  std::string(""),
                                                  &response_body);
  KopsikCheckUpdateCallback updates_callback =
    reinterpret_cast<KopsikCheckUpdateCallback>(callback());

  if (err != kopsik::noError) {
    updates_callback(KOPSIK_API_FAILURE, err.c_str(), 0, 0, 0);
    return;
  }

  if ("null" == response_body) {
    updates_callback(KOPSIK_API_SUCCESS, 0, 0, 0, 0);
    return;
  }

  if (!json_is_valid(response_body.c_str())) {
    updates_callback(KOPSIK_API_FAILURE, "Invalid response JSON", 0, 0, 0);
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

  updates_callback(KOPSIK_API_SUCCESS, err.c_str(), 1, url.c_str(),
            version.c_str());
}
