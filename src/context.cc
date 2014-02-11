// Copyright 2014 kopsik developers

#include "./context.h"
#include "./kopsik_api_private.h"

Context::Context()
  : db(0),
    user(0),
    ws_client(0),
    change_callback(0),
    timeline_uploader(0),
    window_change_recorder(0),
    app_name(""),
    app_version(""),
    api_url(""),
    timeline_upload_url("") {
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
  if (window_change_recorder) {
    window_change_recorder->Stop();
  }
  if (ws_client) {
    ws_client->Stop();
  }
  if (timeline_uploader) {
    timeline_uploader->Stop();
  }

  tm.joinAll();
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
  Poco::Mutex::ScopedLock lock(mutex);

  ws_client->SetProxy(proxy);
  return kopsik::noError;
}

kopsik::error Context::Save() {
  try {
    std::vector<kopsik::ModelChange> changes;
    kopsik::error err = db->SaveUser(user, true, &changes);
    if (err != kopsik::noError) {
      return err;
    }
    for (std::vector<kopsik::ModelChange>::const_iterator it = changes.begin();
        it != changes.end();
        it++) {
      kopsik::ModelChange mc = *it;
      KopsikModelChange *change = model_change_init();
      model_change_to_change_item(mc, *change);
      change_callback(KOPSIK_API_SUCCESS, 0, change);
      model_change_clear(change);
    }
  } catch(const Poco::Exception& exc) {
    return exc.displayText();
  } catch(const std::exception& ex) {
    return ex.what();
  } catch(const std::string& ex) {
    return ex;
  }
  return kopsik::noError;
}
