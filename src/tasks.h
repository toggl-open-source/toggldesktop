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
             void *callback)
      : Task(task_name)
      , ctx_(ctx)
      , callback_(callback) {}

  protected:
    Context *context() { return ctx_; }
    void *callback() { return callback_; }
    KopsikResultCallback result_callback() {
      poco_assert(callback());
      return reinterpret_cast<KopsikResultCallback>(callback_);
    }

  private:
    Context *ctx_;
    void *callback_;
};

class SyncTask : public BaseTask {
  public:
    SyncTask(Context *ctx,
      int full_sync,
      KopsikResultCallback callback)
        : BaseTask(ctx, "sync", reinterpret_cast<void *>(callback)),
      full_sync_(full_sync) {}
    void runTask();

   private:
    int full_sync_;
};

class PushTask : public BaseTask {
  public:
    PushTask(Context *ctx,
      KopsikResultCallback callback)
        : BaseTask(ctx, "push", reinterpret_cast<void *>(callback)) {}
    void runTask();
};

class WebSocketStartTask : public BaseTask {
  public:
    WebSocketStartTask(Context *ctx,
                       kopsik::WebSocketMessageCallback websocket_callback)
      : BaseTask(ctx, "start_websocket",
          reinterpret_cast<void *>(websocket_callback)) {}
    void runTask();
};

class WebSocketStopTask : public BaseTask {
  public:
    explicit WebSocketStopTask(Context *ctx)
      : BaseTask(ctx, "stop_websocket", 0) {}
    void runTask();
};

class TimelineBaseTask : public BaseTask {
  public:
    TimelineBaseTask(Context *ctx,
                     const std::string task_name,
                     KopsikTimelineStateCallback callback)
      : BaseTask(ctx, task_name, reinterpret_cast<void *>(callback)) {}

  protected:
    KopsikTimelineStateCallback timeline_state_callback() {
      poco_assert(callback());
      return reinterpret_cast<KopsikTimelineStateCallback>(callback());
    }
};

// Start timeline recording on local machine
class TimelineStartTask : public TimelineBaseTask {
  public:
    TimelineStartTask(Context *ctx, KopsikTimelineStateCallback callback)
      : TimelineBaseTask(ctx, "start_timeline", callback) {}
    void runTask();
};

// Stop timeline recording on local machine
class TimelineStopTask : public TimelineBaseTask {
  public:
    TimelineStopTask(Context *ctx, KopsikTimelineStateCallback callback)
      : TimelineBaseTask(ctx, "stop_timeline", callback) {}
    void runTask();
};

// Enable timeline recording on server side and locally after that
class TimelineEnableTask : public TimelineBaseTask {
  public:
    TimelineEnableTask(Context *ctx, KopsikTimelineStateCallback callback)
      : TimelineBaseTask(ctx, "enable_timeline", callback) {}
    void runTask();
};

// Disable timeline recording on server side and locally after that
class TimelineDisableTask : public TimelineBaseTask {
  public:
    TimelineDisableTask(Context *ctx, KopsikTimelineStateCallback callback)
      : TimelineBaseTask(ctx, "disable_timeline", callback) {}
    void runTask();
};

class FetchUpdatesTask : public BaseTask {
  public:
    FetchUpdatesTask(Context *ctx,
                     KopsikCheckUpdateCallback updates_callback)
      : BaseTask(ctx, "check_updates",
          reinterpret_cast<void *>(updates_callback)) {}
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
};

#endif  // SRC_TASKS_H_
