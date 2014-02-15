// Copyright 2014 kopsik developers

#ifndef SRC_CONTEXT_H_
#define SRC_CONTEXT_H_

#include <string>

#include "./types.h"
#include "./kopsik_api.h"
#include "./database.h"
#include "./websocket_client.h"
#include "./window_change_recorder.h"
#include "./timeline_uploader.h"
#include "./CustomErrorHandler.h"

// FIXME: rename
class Context {
  public:
    Context();
    ~Context();

    void Shutdown();
    kopsik::error ConfigureProxy();
    kopsik::error Save();

    void FullSync();
    void PartialSync();
    void SwitchWebSocketOff();
    void SwitchWebSocketOn();
    void SwitchTimelineOff();
    void SwitchTimelineOn();
    void FetchUpdates();
    void TimelineUpdateServerSettings();
    void SendFeedback();

    // FIXME: make private

    kopsik::Database *db;
    kopsik::User *user;

    KopsikViewItemChangeCallback change_callback;
    KopsikResultCallback result_callback;
    kopsik::WebSocketMessageCallback websocket_callback;
    KopsikCheckUpdateCallback check_updates_callback;

    kopsik::WebSocketClient *ws_client;
    kopsik::TimelineUploader *timeline_uploader;
    kopsik::WindowChangeRecorder *window_change_recorder;

    std::string app_name;
    std::string app_version;

    std::string api_url;
    std::string timeline_upload_url;

    Poco::Mutex mutex;

    CustomErrorHandler error_handler;

    std::string update_channel;

    std::string feedback_attachment_path_;
    std::string feedback_subject;
    std::string feedback_details;

  private:
    const std::string updateURL();
    static const std::string osName();

    std::string feedbackJSON();

    Poco::Logger &logger() {
      return Poco::Logger::get("Context");
    };

    std::string feedback_filename();
    std::string base64encode_attachment();
};

#endif  // SRC_CONTEXT_H_
