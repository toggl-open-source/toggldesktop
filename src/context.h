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

#include "Poco/TaskManager.h"
#include "Poco/ErrorHandler.h"

class Context {
  public:
    Context();
    ~Context();
    void Shutdown();
    kopsik::error ConfigureProxy();
    kopsik::error Save();

    kopsik::Database *db;
    kopsik::User *user;
    kopsik::WebSocketClient *ws_client;
    KopsikViewItemChangeCallback change_callback;
    kopsik::TimelineUploader *timeline_uploader;
    kopsik::WindowChangeRecorder *window_change_recorder;
    std::string app_name;
    std::string app_version;
    std::string api_url;
    Poco::Mutex mutex;
    Poco::TaskManager tm;
    CustomErrorHandler error_handler;
};

#endif  // SRC_CONTEXT_H_
