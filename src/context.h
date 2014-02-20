// Copyright 2014 kopsik developers

#ifndef SRC_CONTEXT_H_
#define SRC_CONTEXT_H_

#include <string>

#include "./types.h"
#include "./database.h"
#include "./websocket_client.h"
#include "./window_change_recorder.h"
#include "./timeline_uploader.h"
#include "./CustomErrorHandler.h"

// FIXME: don't use C API from C++ class
#include "./kopsik_api.h"

#include "Poco/Util/Timer.h"

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

    // FIXME: dont use C callbacks in C++ class
    KopsikViewItemChangeCallback change_callback;
    KopsikErrorCallback on_error_callback;
    KopsikCheckUpdateCallback check_updates_callback;

  private:
    const std::string updateURL();
    static const std::string osName();
    Poco::Logger &logger() { return Poco::Logger::get("Context"); }
    std::string feedbackJSON();
    std::string feedback_filename();
    std::string base64encode_attachment();
    void sync(const bool full_sync);

    // timer_ callbacks
    void onFullSync(Poco::Util::TimerTask& task);  // NOLINT
    void onPartialSync(Poco::Util::TimerTask& task);  // NOLINT
    void onSwitchWebSocketOff(Poco::Util::TimerTask& task);  // NOLINT
    void onSwitchWebSocketOn(Poco::Util::TimerTask& task);  // NOLINT
    void onSwitchTimelineOff(Poco::Util::TimerTask& task);  // NOLINT
    void onSwitchTimelineOn(Poco::Util::TimerTask& task);  // NOLINT
    void onFetchUpdates(Poco::Util::TimerTask& task);  // NOLINT
    void onTimelineUpdateServerSettings(Poco::Util::TimerTask& task);  // NOLINT
    void onSendFeedback(Poco::Util::TimerTask& task);  // NOLINT

    // avoid same task running twice by flipping these:
    bool full_sync_queued_;
    bool partial_sync_queued_;
    bool fetch_updates_queued_;
    bool update_timeline_settings_queued_;

    // schedule background tasks using this timer:
    Poco::Util::Timer timer_;
};

#endif  // SRC_CONTEXT_H_
