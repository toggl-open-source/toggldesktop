// Copyright 2014 kopsik developers

#ifndef SRC_TASKS_H_
#define SRC_TASKS_H_

#include <string>

#include "Poco/Task.h"
#include "./kopsik_api.h"
#include "./kopsik_api_private.h"
#include "./websocket_client.h"
#include "./https_client.h"
#include "./context.h"

class SyncTask : public Poco::Task {
  public:
    SyncTask(Context *ctx,
      int full_sync,
      KopsikResultCallback callback) : Task("sync"),
      ctx_(ctx),
      full_sync_(full_sync),
      callback_(callback) {}
    void runTask();

   private:
    Context *ctx_;
    int full_sync_;
    KopsikResultCallback callback_;
};

class PushTask : public Poco::Task {
  public:
    PushTask(Context *ctx,
      KopsikResultCallback callback) : Task("push"),
      ctx_(ctx),
      callback_(callback) {}
    void runTask();

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
    void runTask();
  private:
    Context *ctx_;
    kopsik::WebSocketMessageCallback websocket_callback_;
};

class WebSocketStopTask : public Poco::Task {
  public:
    explicit WebSocketStopTask(Context *ctx) :
      Task("stop_websocket"),
      ctx_(ctx) {}
    void runTask();
  private:
    Context *ctx_;
};

class TimelineStartTask : public Poco::Task {
  public:
    TimelineStartTask(Context *ctx, KopsikResultCallback callback) :
      Task("start_timeline"),
      ctx_(ctx),
      callback_(callback) {}
    void runTask();

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
    void runTask();

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
    void runTask();

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
