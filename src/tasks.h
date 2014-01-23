// Copyright 2014 kopsik developers

#ifndef SRC_TASKS_H_
#define SRC_TASKS_H_

#include <string>

#include "Poco/Task.h"
#include "./kopsik_api.h"
#include "./websocket_client.h"

class SyncTask : public Poco::Task {
  public:
    SyncTask(Context *ctx,
      int full_sync,
      KopsikResultCallback callback) : Task("sync"),
      ctx_(ctx),
      full_sync_(full_sync),
      callback_(callback) {}
    void runTask() {
      char err[KOPSIK_ERR_LEN];
      kopsik_api_result res = kopsik_sync(
        ctx_, err, KOPSIK_ERR_LEN, full_sync_);
      callback_(res, err);
    }
  private:
    Context *ctx_;
    int full_sync_;
    KopsikResultCallback callback_;
};

class PushTask : public Poco::Task {
  public:
    PushTask(Context *ctx,
      KopsikResultCallback callback) : Task("push"),
      ctx_(ctx), callback_(callback) {}
    void runTask() {
      char err[KOPSIK_ERR_LEN];
      kopsik_api_result res = kopsik_push(ctx_, err, KOPSIK_ERR_LEN);
      callback_(res, err);
    }
  private:
    Context *ctx_;
    KopsikResultCallback callback_;
};

class WebSocketStartTask : public Poco::Task {
  public:
    WebSocketStartTask(Context *ctx,
                       kopsik::WebSocketMessageCallback websocket_callback) :
      Task("start_websocket"),
      ctx_(ctx),
      websocket_callback_(websocket_callback) {}
    void runTask() {
      ctx_->ws_client->Start(
        ctx_,
        ctx_->user->APIToken(),
        websocket_callback_);
    }
  private:
    Context *ctx_;
    kopsik::WebSocketMessageCallback websocket_callback_;
};

class WebSocketStopTask : public Poco::Task {
  public:
    explicit WebSocketStopTask(Context *ctx) :
      Task("stop_websocket"),
      ctx_(ctx) {}
    void runTask() {
      ctx_->ws_client->Stop();
    }
  private:
    Context *ctx_;
};

class TimelineStartTask : public Poco::Task {
  public:
    TimelineStartTask(Context *ctx, KopsikResultCallback callback) :
      Task("start_timeline"),
      ctx_(ctx),
      callback_(callback) {}
    void runTask() {
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

  private:
    Context *ctx_;
    KopsikResultCallback callback_;
};

class TimelineStopTask : public Poco::Task {
  public:
    TimelineStopTask(Context *ctx, KopsikResultCallback callback) :
      Task("stop_timeline"),
      ctx_(ctx),
      callback_(callback) {}
    void runTask() {
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

  private:
    Context *ctx_;
    KopsikResultCallback callback_;
};

class FetchUpdatesTask : public Poco::Task {
  public:
    FetchUpdatesTask(Context *ctx,
        KopsikCheckUpdateCallback callback) :
      Task("check_updates"),
      ctx_(ctx),
      callback_(callback) {}
    void runTask() {
      std::string response_body("");
      kopsik::error err = ctx_->https_client->GetJSON(updateURL(),
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

  private:
    const std::string updateURL() {
      std::stringstream relative_url;
      relative_url << "/api/v8/updates?app=kopsik"
        << "&channel=" << channel()
        << "&platform=" << osName()
        << "&version=" << ctx_->app_version;
      return relative_url.str();
    }
    const std::string channel() {
      return std::string("dev");
    }
    const std::string osName() {
      if (POCO_OS_LINUX == POCO_OS) {
        return std::string("linux");
      }
      if (POCO_OS_WINDOWS_NT == POCO_OS) {
        return std::string("windows");
      }
      return std::string("darwin");
    }

    Context *ctx_;
    KopsikCheckUpdateCallback callback_;
};

#endif  // SRC_TASKS_H_
