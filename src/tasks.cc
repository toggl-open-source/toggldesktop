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

  kopsik::HTTPSClient https_client(ctx_->api_url,
                                   ctx_->app_name,
                                   ctx_->app_version);
  char errmsg[KOPSIK_ERR_LEN];
  kopsik::error err = ctx_->user->Sync(&https_client, full_sync_, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), KOPSIK_ERR_LEN);
    callback_(KOPSIK_API_FAILURE, errmsg);
    return;
  }

  kopsik_api_result res = save(ctx_, errmsg, KOPSIK_ERR_LEN);
  callback_(res, errmsg);
}

void PushTask::runTask() {
  if (isCancelled()) {
    return;
  }

  kopsik::HTTPSClient https_client(ctx_->api_url,
                                   ctx_->app_name,
                                   ctx_->app_version);
  char errmsg[KOPSIK_ERR_LEN];
  kopsik::error err = ctx_->user->Push(&https_client);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), KOPSIK_ERR_LEN);
    callback_(KOPSIK_API_FAILURE, errmsg);
    return;
  }

  kopsik_api_result res = save(ctx_, errmsg, KOPSIK_ERR_LEN);
  callback_(res, errmsg);
}

void WebSocketStartTask::runTask() {
  ctx_->ws_client->Start(
    ctx_,
    ctx_->user->APIToken(),
    websocket_callback_);
}

void WebSocketStopTask::runTask() {
  ctx_->ws_client->Stop();
}

void TimelineStartTask::runTask() {
  if (!ctx_->user) {
    callback_(KOPSIK_API_FAILURE, "Please login first");
    return;
  }

  if (!ctx_->user->RecordTimeline()) {
    callback_(KOPSIK_API_FAILURE,
      "Timeline recording is disabled in Toggl profile");
    return;
  }

  kopsik::ExplicitScopedLock("TimelineStartTask", ctx_->mutex);

  if (ctx_->timeline_uploader) {
    delete ctx_->timeline_uploader;
    ctx_->timeline_uploader = 0;
  }
  ctx_->timeline_uploader = new kopsik::TimelineUploader(
    ctx_->user->ID(),
    ctx_->user->APIToken(),
    ctx_->api_url,
    ctx_->app_name,
    ctx_->app_version);

  if (ctx_->window_change_recorder) {
    delete ctx_->window_change_recorder;
    ctx_->window_change_recorder = 0;
  }
  ctx_->window_change_recorder = new kopsik::WindowChangeRecorder(
    ctx_->user->ID());

  callback_(KOPSIK_API_SUCCESS, "");
}

void TimelineStopTask::runTask() {
  kopsik::ExplicitScopedLock("TimelineStopTask", ctx_->mutex);

  if (ctx_->window_change_recorder) {
    delete ctx_->window_change_recorder;
    ctx_->window_change_recorder = 0;
  }

  if (ctx_->timeline_uploader) {
    delete ctx_->timeline_uploader;
    ctx_->timeline_uploader = 0;
  }

  if (callback_) {
    callback_(KOPSIK_API_SUCCESS, "");
  }
}

void FetchUpdatesTask::runTask() {
  std::string response_body("");
  kopsik::HTTPSClient https_client(
    ctx_->api_url,
    ctx_->app_name,
    ctx_->app_version);
  kopsik::error err = https_client.GetJSON(updateURL(),
                                                  std::string(""),
                                                  std::string(""),
                                                  &response_body);
  if (err != kopsik::noError) {
    callback_(KOPSIK_API_FAILURE, err.c_str(), 0, 0, 0);
    return;
  }

  if ("null" == response_body) {
    callback_(KOPSIK_API_SUCCESS, 0, 0, 0, 0);
    return;
  }

  if (!json_is_valid(response_body.c_str())) {
    callback_(KOPSIK_API_FAILURE, "Invalid response JSON", 0, 0, 0);
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

  callback_(KOPSIK_API_SUCCESS, err.c_str(), 1, url.c_str(),
            version.c_str());
}
