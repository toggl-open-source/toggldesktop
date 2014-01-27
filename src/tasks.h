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

class BaseTask : public Poco::Task {
  public:
    BaseTask(Context *ctx,
             const std::string task_name,
             KopsikResultCallback callback)
      : Task(task_name)
      , ctx_(ctx)
      , callback_(callback) {}

  protected:
    Context *context() { return ctx_; }
    KopsikResultCallback callback() { return callback_; }

  private:
    Context *ctx_;
    KopsikResultCallback callback_;
};

class SyncTask : public BaseTask {
  public:
    SyncTask(Context *ctx,
      int full_sync,
      KopsikResultCallback callback) : BaseTask(ctx, "sync", callback),
      full_sync_(full_sync) {}
    void runTask();

   private:
    int full_sync_;
};

class PushTask : public BaseTask {
  public:
    PushTask(Context *ctx,
      KopsikResultCallback callback) : BaseTask(ctx, "push", callback) {}
    void runTask();
};

class WebSocketStartTask : public BaseTask {
  public:
    WebSocketStartTask(Context *ctx,
                       kopsik::WebSocketMessageCallback websocket_callback)
    : BaseTask(ctx, "start_websocket", 0),
      websocket_callback_(websocket_callback) {}
    void runTask();
  private:
    kopsik::WebSocketMessageCallback websocket_callback_;
};

class WebSocketStopTask : public BaseTask {
  public:
    explicit WebSocketStopTask(Context *ctx)
      : BaseTask(ctx, "stop_websocket", 0) {}
    void runTask();
};

class TimelineStartTask : public BaseTask {
  public:
    TimelineStartTask(Context *ctx, KopsikResultCallback callback)
      : BaseTask(ctx, "start_timeline", callback) {}
    void runTask();
};

class TimelineStopTask : public BaseTask {
  public:
    TimelineStopTask(Context *ctx, KopsikResultCallback callback)
      : BaseTask(ctx, "stop_timeline", callback) {}
    void runTask();
};

class FetchUpdatesTask : public BaseTask {
  public:
    FetchUpdatesTask(Context *ctx,
        KopsikCheckUpdateCallback updates_callback)
    : BaseTask(ctx, "check_updates", 0),
      updates_callback_(updates_callback) {}
    void runTask();

  private:
    const std::string updateURL() {
      std::stringstream relative_url;
      relative_url << "/api/v8/updates?app=kopsik"
        << "&channel=" << channel()
        << "&platform=" << osName()
        << "&version=" << context()->app_version;
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

  KopsikCheckUpdateCallback updates_callback_;
};

#endif  // SRC_TASKS_H_
