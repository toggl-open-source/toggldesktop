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
    BaseTask(Context *ctx, const std::string task_name)
      : Task(task_name)
      , ctx_(ctx) {}

  protected:
    Context *context() { return ctx_; }

  private:
    Context *ctx_;
};

class SyncTask : public BaseTask {
  public:
    SyncTask(Context *ctx, int full_sync)
        : BaseTask(ctx, "SyncTask")
        , full_sync_(full_sync) {}
    void runTask();

   private:
    int full_sync_;
};

class WebSocketSwitchTask : public BaseTask {
  public:
    WebSocketSwitchTask(Context *ctx, const bool on)
      : BaseTask(ctx, "WebSocketSwitchTask")
      , on_(on) {}
    void runTask();
  private:
    bool on_;
};

// Start/stop timeline recording on local machine
class TimelineSwitchTask : public BaseTask {
  public:
    TimelineSwitchTask(Context *ctx, const bool on)
        : BaseTask(ctx, "TimelineSwitchTask")
        , on_(on) {}
    void runTask();
  private:
    bool on_;
};

// Enable timeline recording on server side
class TimelineUpdateServerSettingsTask : public BaseTask {
  public:
    explicit TimelineUpdateServerSettingsTask(Context *ctx)
      : BaseTask(ctx, "TimelineUpdateServerSettingsTask") {}
    void runTask();
};

// Send feedback to Toggl support
class SendFeedbackTask : public BaseTask {
 public:
  SendFeedbackTask(Context *ctx,
                   std::string subject,
                   std::string details,
                   std::string attachment_path)
  : BaseTask(ctx, "SendFeedbackTask")
  , subject_(subject)
  , details_(details)
  , attachment_path_(attachment_path) {}

  void runTask();

 private:
  std::string feedbackJSON();
  std::string filename();
  std::string base64encode_attachment();

  std::string subject_;
  std::string details_;
  std::string attachment_path_;
};

// Check if new Toggl-built version is available.
class FetchUpdatesTask : public BaseTask {
  public:
    FetchUpdatesTask(Context *ctx, const std::string update_channel)
      : BaseTask(ctx, "check_updates")
      , update_channel_(update_channel) {}
    void runTask();

  private:
    const std::string updateURL();
    const std::string osName();
    std::string update_channel_;
};

#endif  // SRC_TASKS_H_
