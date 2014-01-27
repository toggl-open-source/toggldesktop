// Copyright 2014 kopsik developers

#include <string>

#include "Poco/Task.h"
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
    callback()(KOPSIK_API_FAILURE, err.c_str());
    return;
  }

  char errmsg[KOPSIK_ERR_LEN];
  kopsik_api_result res = save(context(), errmsg, KOPSIK_ERR_LEN);
  callback()(res, errmsg);
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
    callback()(KOPSIK_API_FAILURE, err.c_str());
    return;
  }

  char errmsg[KOPSIK_ERR_LEN];
  kopsik_api_result res = save(context(), errmsg, KOPSIK_ERR_LEN);
  callback()(res, errmsg);
}

void WebSocketStartTask::runTask() {
  context()->ws_client->Start(
    context(),
    context()->user->APIToken(),
    websocket_callback_);
}

void WebSocketStopTask::runTask() {
  context()->ws_client->Stop();
}

void TimelineStartTask::runTask() {
  if (!context()->user) {
    callback()(KOPSIK_API_FAILURE, "Please login first");
    return;
  }

  if (!context()->user->RecordTimeline()) {
    callback()(KOPSIK_API_FAILURE,
      "Timeline recording is disabled in Toggl profile");
    return;
  }

  kopsik::ExplicitScopedLock("TimelineStartTask", context()->mutex);

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

  callback()(KOPSIK_API_SUCCESS, "");
}

void TimelineStopTask::runTask() {
  kopsik::ExplicitScopedLock("TimelineStopTask", context()->mutex);

  if (context()->window_change_recorder) {
    delete context()->window_change_recorder;
    context()->window_change_recorder = 0;
  }

  if (context()->timeline_uploader) {
    delete context()->timeline_uploader;
    context()->timeline_uploader = 0;
  }

  if (callback()) {
    callback()(KOPSIK_API_SUCCESS, "");
  }
}

void TimelineEnableTask::runTask() {
  kopsik::ExplicitScopedLock("TimelineEnableTask", context()->mutex);

  kopsik::HTTPSClient https_client(context()->api_url,
                                   context()->app_name,
                                   context()->app_version);
  Poco::Net::HTMLForm form;
  form.set("record", "true");
  form.set("enable", "true");

  std::string response_body("");

  kopsik::error err = https_client.PostForm("/api/v8/toggle_timeline",
                                            &form,
                                            context()->user->APIToken(),
                                            "api_token",
                                            &response_body);
  if (err != kopsik::noError) {
    callback()(KOPSIK_API_FAILURE, err.c_str());
    return;
  }

  getOwner()->start(new TimelineStartTask(context(), callback()));
}

void TimelineDisableTask::runTask() {
  kopsik::ExplicitScopedLock("TimelineDisableTask", context()->mutex);

  kopsik::HTTPSClient https_client(context()->api_url,
                                   context()->app_name,
                                   context()->app_version);
  Poco::Net::HTMLForm form;
  form.set("record", "false");
  form.set("enable", "false");

  std::string response_body("");

  kopsik::error err = https_client.PostForm("/api/v8/toggle_timeline",
                                            &form,
                                            context()->user->APIToken(),
                                            "api_token",
                                            &response_body);
  if (err != kopsik::noError) {
    callback()(KOPSIK_API_FAILURE, err.c_str());
    return;
  }

  getOwner()->start(new TimelineStopTask(context(), callback()));
}

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
  if (err != kopsik::noError) {
    updates_callback_(KOPSIK_API_FAILURE, err.c_str(), 0, 0, 0);
    return;
  }

  if ("null" == response_body) {
    updates_callback_(KOPSIK_API_SUCCESS, 0, 0, 0, 0);
    return;
  }

  if (!json_is_valid(response_body.c_str())) {
    updates_callback_(KOPSIK_API_FAILURE, "Invalid response JSON", 0, 0, 0);
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

  updates_callback_(KOPSIK_API_SUCCESS, err.c_str(), 1, url.c_str(),
            version.c_str());
}
