// Copyright 2014 kopsik developers

#include "./context.h"

Context::Context()
  : db(0),
    user(0),
    https_client(0),
    ws_client(0),
    change_callback(0),
    timeline_uploader(0),
    window_change_recorder(0),
    app_name(""),
    app_version(""),
    api_url("") {
  Poco::ErrorHandler::set(&error_handler);
  Poco::Net::initializeSSL();
}

Context::~Context() {
  if (window_change_recorder) {
    delete window_change_recorder;
    window_change_recorder = 0;
  }

  if (timeline_uploader) {
    delete timeline_uploader;
    timeline_uploader = 0;
  }

  if (ws_client) {
    delete ws_client;
    ws_client = 0;
  }

  if (https_client) {
    delete https_client;
    https_client = 0;
  }

  if (db) {
    delete db;
    db = 0;
  }

  if (user) {
    delete user;
    user = 0;
  }

  Poco::Net::uninitializeSSL();
}

void Context::Shutdown() {
  tm.joinAll();

  if (window_change_recorder) {
    window_change_recorder->Stop();
  }
  if (ws_client) {
    ws_client->Stop();
  }
  if (timeline_uploader) {
    timeline_uploader->Stop();
  }
}

kopsik::error Context::ConfigureProxy() {
  bool use_proxy(false);
  bool tmp(false);
  kopsik::Proxy proxy;
  kopsik::error err = db->LoadSettings(&use_proxy, &proxy, &tmp);
  if (err != kopsik::noError) {
    return err;
  }
  if (!use_proxy) {
    proxy = kopsik::Proxy();  // reset values
  }
  kopsik::ExplicitScopedLock("Context::ConfigureProxy", mutex);

  https_client->SetProxy(proxy);
  ws_client->SetProxy(proxy);
  return kopsik::noError;
}
