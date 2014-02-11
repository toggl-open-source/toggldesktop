// Copyright 2013 Tanel Lebedev

#include <cstring>

#include "./kopsik_api.h"
#include "./kopsik_api_private.h"
#include "./database.h"
#include "./toggl_api_client.h"
#include "./https_client.h"
#include "./websocket_client.h"
#include "./version.h"
#include "./timeline_uploader.h"
#include "./window_change_recorder.h"
#include "./CustomErrorHandler.h"
#include "./proxy.h"
#include "./context.h"
#include "./tasks.h"

#include "Poco/Bugcheck.h"
#include "Poco/Path.h"
#include "Poco/Logger.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/ScopedLock.h"
#include "Poco/Mutex.h"
#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include "Poco/TaskManager.h"

Poco::Logger &logger() {
  return Poco::Logger::get("kopsik_api");
}

Poco::Logger &rootLogger() {
  return Poco::Logger::get("");
}

void kopsik_set_change_callback(
    void *context,
    KopsikViewItemChangeCallback callback) {
  poco_assert(callback);

  logger().debug("kopsik_set_change_callback");

  Context *ctx = reinterpret_cast<Context *>(context);

  Poco::Mutex::ScopedLock lock(ctx->mutex);

  ctx->change_callback = callback;
}

int kopsik_is_networking_error(
    const char *error) {
  std::string value(error);
  if (value.find("Host not found") != std::string::npos) {
    return 1;
  }
  if (value.find("Cannot upgrade to WebSocket connection")
      != std::string::npos) { // NOLINT
    return 1;
  }
  if (value.find("No message received") != std::string::npos) {
    return 1;
  }
  if (value.find("Connection refused") != std::string::npos) {
    return 1;
  }
  if (value.find("Connection timed out") != std::string::npos) {
    return 1;
  }
  if (value.find("connect timed out") != std::string::npos) {
    return 1;
  }
  if (value.find("SSL connection unexpectedly closed") != std::string::npos) {
    return 1;
  }
  if (value.find("Network is down") != std::string::npos) {
    return 1;
  }
  return 0;
}

// Context API.

void *kopsik_context_init(
    const char *app_name,
    const char *app_version) {
  poco_assert(app_name);
  poco_assert(app_version);

  Context *ctx = new Context();

  ctx->app_name = std::string(app_name);
  ctx->app_version = std::string(app_version);

  ctx->api_url = "https://www.toggl.com";
  ctx->timeline_upload_url = "https://timeline.toggl.com";

  ctx->ws_client = new kopsik::WebSocketClient("https://stream.toggl.com",
    ctx->app_name, ctx->app_version);

  return ctx;
}

void kopsik_context_shutdown(
    void *context) {
  poco_assert(context);

  Context *ctx = reinterpret_cast<Context *>(context);
  ctx->Shutdown();
}

void kopsik_context_clear(
    void *context) {
  poco_assert(context);

  Context *ctx = reinterpret_cast<Context *>(context);
  delete ctx;
}

// Configuration API.

KopsikSettings *kopsik_settings_init() {
  KopsikSettings *settings = new KopsikSettings();
  settings->UseProxy = 0;
  settings->ProxyHost = 0;
  settings->ProxyPort = 0;
  settings->ProxyUsername = 0;
  settings->ProxyPassword = 0;
  settings->UseIdleDetection = 0;
  return settings;
}

void kopsik_settings_clear(
    KopsikSettings *settings) {
  if (settings->ProxyHost) {
    free(settings->ProxyHost);
    settings->ProxyHost = 0;
  }
  if (settings->ProxyUsername) {
    free(settings->ProxyUsername);
    settings->ProxyUsername = 0;
  }
  if (settings->ProxyPassword) {
    free(settings->ProxyPassword);
    settings->ProxyPassword = 0;
  }
  delete settings;
}

kopsik_api_result kopsik_get_settings(
    void *context,
    char *errmsg,
    unsigned int errlen,
    KopsikSettings *settings) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(settings);

    Context *ctx = reinterpret_cast<Context *>(context);

    bool use_proxy(false);
    bool use_idle_detection(false);
    kopsik::Proxy proxy;
    kopsik::error err = ctx->db->LoadSettings(&use_proxy,
                                              &proxy,
                                              &use_idle_detection);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }

    settings->UseProxy = 0;
    if (use_proxy) {
      settings->UseProxy = 1;
    }

    poco_assert(!settings->ProxyHost);
    settings->ProxyHost = strdup(proxy.host.c_str());
    settings->ProxyPort = proxy.port;
    poco_assert(!settings->ProxyUsername);
    settings->ProxyUsername = strdup(proxy.username.c_str());
    poco_assert(!settings->ProxyPassword);
    settings->ProxyPassword = strdup(proxy.password.c_str());

    settings->UseIdleDetection = 0;
    if (use_idle_detection) {
      settings->UseIdleDetection = 1;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_settings(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const int use_proxy,
    const char *proxy_host,
    const unsigned int proxy_port,
    const char *proxy_username,
    const char *proxy_password,
    const int use_idle_detection) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(proxy_host);
    poco_assert(proxy_username);
    poco_assert(proxy_password);

    Context *ctx = reinterpret_cast<Context *>(context);

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::Proxy proxy;
    proxy.host = std::string(proxy_host);
    proxy.port = proxy_port;
    proxy.username = std::string(proxy_username);
    proxy.password = std::string(proxy_password);

    kopsik::error err = ctx->db->SaveSettings(use_proxy,
                                              &proxy,
                                              use_idle_detection);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }

    err = ctx->ConfigureProxy();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_configure_proxy(
    void *context,
    char *errmsg,
    unsigned int errlen) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);

    Context *ctx = reinterpret_cast<Context *>(context);

    kopsik::error err = ctx->ConfigureProxy();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

void kopsik_test_set_https_client(
    void *context,
    void *client) {
  poco_assert(context);
  poco_assert(client);
  // FIXME:
}

kopsik_api_result kopsik_set_db_path(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *path) {
  kopsik::error err = kopsik::noError;
  try {
    poco_assert(context);
    poco_assert(path);

    std::stringstream ss;
    ss << "kopsik_set_db_path path=" << path;
    logger().debug(ss.str());

    Context *ctx = reinterpret_cast<Context *>(context);

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    if (ctx->db) {
      delete ctx->db;
      ctx->db = 0;
    }
    ctx->db = new kopsik::Database(path);
  } catch(const Poco::Exception& exc) {
    err = exc.displayText();
  } catch(const std::exception& ex) {
    err = ex.what();
  } catch(const std::string& ex) {
    err = ex;
  }
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

void kopsik_set_log_path(
    const char *path) {
  poco_assert(path);

  Poco::AutoPtr<Poco::SimpleFileChannel> simpleFileChannel(
    new Poco::SimpleFileChannel);
  simpleFileChannel->setProperty("path", path);
  simpleFileChannel->setProperty("rotation", "1 M");

  Poco::AutoPtr<Poco::FormattingChannel> formattingChannel(
      new Poco::FormattingChannel(
        new Poco::PatternFormatter("%Y-%m-%d %H:%M:%S.%i [%P %I]:%s:%q:%t")));
  formattingChannel->setChannel(simpleFileChannel);

  rootLogger().setChannel(formattingChannel);
  rootLogger().setLevel(Poco::Message::PRIO_DEBUG);
}

void kopsik_set_log_level(
    const char *level) {
  poco_assert(level);

  rootLogger().setLevel(level);
}

void kopsik_set_api_url(
    void *context,
    const char *api_url) {
  poco_assert(context);
  poco_assert(api_url);

  Context *ctx = reinterpret_cast<Context *>(context);

  Poco::Mutex::ScopedLock lock(ctx->mutex);

  ctx->api_url = api_url;
}

void kopsik_set_websocket_url(
    void *context,
    const char *websocket_url) {
  poco_assert(context);
  poco_assert(websocket_url);

  Context *ctx = reinterpret_cast<Context *>(context);

  Poco::Mutex::ScopedLock lock(ctx->mutex);

  ctx->ws_client->SetWebsocketURL(websocket_url);
}

// User API.

KopsikUser *kopsik_user_init() {
  KopsikUser *user = new KopsikUser();
  user->ID = 0;
  user->Fullname = 0;
  return user;
}

void kopsik_user_clear(
    KopsikUser *user) {
  poco_assert(user);
  user->ID = 0;
  if (user->Fullname) {
    free(user->Fullname);
    user->Fullname = 0;
  }
  delete user;
}

kopsik_api_result kopsik_current_user(
    void *context,
    char *errmsg,
    unsigned int errlen,
    KopsikUser *out_user) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_user);

    logger().debug("kopsik_current_user");

    Context *ctx = reinterpret_cast<Context *>(context);

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    if (!ctx->user) {
      kopsik::User *user = new kopsik::User(ctx->app_name, ctx->app_version);
      kopsik::error err = ctx->db->LoadCurrentUser(user, true);
      if (err != kopsik::noError) {
        delete user;
        strncpy(errmsg, err.c_str(), errlen);
        return KOPSIK_API_FAILURE;
      }
      ctx->user = user;
    }
    poco_assert(ctx->user);
    if (out_user->Fullname) {
      free(out_user->Fullname);
      out_user->Fullname = 0;
    }
    out_user->Fullname = strdup(ctx->user->Fullname().c_str());
    out_user->ID = (unsigned int)ctx->user->ID();
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_api_token(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *api_token) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(api_token);

    std::stringstream ss;
    ss << "kopsik_set_api_token api_token=" << api_token;
    logger().debug(ss.str());

    Context *ctx = reinterpret_cast<Context *>(context);

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::error err = ctx->db->SetCurrentAPIToken(api_token);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_get_api_token(
    void *context,
    char *errmsg,
    unsigned int errlen,
    char *str,
    unsigned int max_strlen) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(str);
    poco_assert(max_strlen);

    Context *ctx = reinterpret_cast<Context *>(context);

    std::string token("");
    kopsik::error err = ctx->db->CurrentAPIToken(&token);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
    strncpy(str, token.c_str(), max_strlen);
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_login(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *in_email,
    const char *in_password) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(in_email);
    poco_assert(in_password);

    std::stringstream ss;
    ss << "kopik_login email=" << in_email;
    logger().debug(ss.str());

    std::string email(in_email);
    std::string password(in_password);
    if (email.empty()) {
      strncpy(errmsg, "Empty email", errlen);
      return KOPSIK_API_FAILURE;
    }
    if (password.empty()) {
      strncpy(errmsg, "Empty password", errlen);
      return KOPSIK_API_FAILURE;
    }

    Context *ctx = reinterpret_cast<Context *>(context);

    kopsik::User *user = new kopsik::User(ctx->app_name, ctx->app_version);

    kopsik::HTTPSClient https_client(ctx->api_url,
                                     ctx->app_name,
                                     ctx->app_version);
    kopsik::error err = user->Login(&https_client, email, password);
    if (err != kopsik::noError) {
      delete user;
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }

    poco_assert(user->ID() > 0);

    err = ctx->db->LoadUserByID(user->ID(), user, true);
    if (err != kopsik::noError) {
      delete user;
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    err = ctx->db->SetCurrentAPIToken(user->APIToken());
    if (err != kopsik::noError) {
      delete user;
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }

    if (ctx->user) {
      delete ctx->user;
      ctx->user = 0;
    }

    ctx->user = user;

    err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_logout(
    void *context,
    char *errmsg,
    unsigned int errlen) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);

    logger().debug("kopsik_logout");

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      return KOPSIK_API_SUCCESS;
    }

    ctx->Shutdown();

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::error err = ctx->db->ClearCurrentAPIToken();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }

    if (ctx->user) {
      delete ctx->user;
      ctx->user = 0;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_clear_cache(
    void *context,
    char *errmsg,
    unsigned int errlen) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);

    logger().debug("kopsik_logout");

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      return KOPSIK_API_SUCCESS;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::error err = ctx->db->DeleteUser(ctx->user, true);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return kopsik_logout(context, errmsg, errlen);
}

kopsik_api_result kopsik_user_has_premium_workspaces(
    void *context,
    char *errmsg,
    unsigned int errlen,
    int *has_premium_workspaces) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(has_premium_workspaces);

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to check if you have premium workspaces",
              errlen);
      return KOPSIK_API_FAILURE;
    }

    *has_premium_workspaces = 0;
    if (ctx->user->HasPremiumWorkspaces()) {
      *has_premium_workspaces = 1;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

// Sync

kopsik_api_result kopsik_pushable_models(
    void *context,
    char *errmsg,
    unsigned int errlen,
    KopsikPushableModelStats *stats) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(stats);

    logger().debug("kopsik_pushable_models");

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to check if sync is required", errlen);
      return KOPSIK_API_FAILURE;
    }
    std::vector<kopsik::TimeEntry *> pushable;
    ctx->user->CollectPushableObjects(&pushable);
    stats->TimeEntries = 0;
    for (std::vector<kopsik::TimeEntry *>::const_iterator it = pushable.begin();
      it != pushable.end();
      it++) {
      stats->TimeEntries++;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

void kopsik_sync(
    void *context,
    int full_sync,
    KopsikResultCallback callback) {
  poco_assert(context);
  poco_assert(callback);

  logger().debug("kopsik_sync");

  Context *ctx = reinterpret_cast<Context *>(context);

  ctx->tm.start(new SyncTask(ctx, full_sync, callback));
}

// Autocomplete list

KopsikAutocompleteItemList *
    kopsik_autocomplete_item_list_init() {
  KopsikAutocompleteItemList *result = new KopsikAutocompleteItemList();
  result->Length = 0;
  result->ViewItems = 0;
  return result;
}

void kopsik_autocomplete_item_list_clear(
    KopsikAutocompleteItemList *list) {
  poco_assert(list);
  for (unsigned int i = 0; i < list->Length; i++) {
    KopsikAutocompleteItem *item = list->ViewItems[i];
    poco_assert(item);
    autocomplete_item_clear(item);
    list->ViewItems[i] = 0;
  }
  if (list->ViewItems) {
    free(list->ViewItems);
  }
  delete list;
}

kopsik_api_result kopsik_autocomplete_items(
    void *context,
    char *errmsg,
    unsigned int errlen,
    KopsikAutocompleteItemList *list,
    const unsigned int include_time_entries,
    const unsigned int include_tasks,
    const unsigned int include_projects) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(list);

    logger().debug("kopsik_autocomplete_items");

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to fill autocomplete", errlen);
      return KOPSIK_API_FAILURE;
    }

    std::vector<KopsikAutocompleteItem *> autocomplete_items;

    // Add time entries, in format:
    // Description - Task. Project. Client
    if (include_time_entries) {
      for (std::vector<kopsik::TimeEntry *>::const_iterator it =
          ctx->user->related.TimeEntries.begin();
          it != ctx->user->related.TimeEntries.end(); it++) {
        kopsik::TimeEntry *te = *it;

        if (te->DeletedAt() || te->IsMarkedAsDeletedOnServer()
            || te->Description().empty()) {
          continue;
        }

        kopsik::Task *t = 0;
        if (te->TID()) {
          t = ctx->user->GetTaskByID(te->TID());
        }

        kopsik::Project *p = 0;
        if (t && t->PID()) {
          p = ctx->user->GetProjectByID(t->PID());
        } else if (te->PID()) {
          p = ctx->user->GetProjectByID(te->PID());
        }

        if (p && !p->Active()) {
          continue;
        }

        kopsik::Client *c = 0;
        if (p && p->CID()) {
          c = ctx->user->GetClientByID(p->CID());
        }

        std::string project_label = ctx->user->JoinTaskName(t, p, c);

        std::stringstream search_parts;
        search_parts << te->Description();
        std::string description = search_parts.str();
        if (!project_label.empty()) {
          search_parts << " - " << project_label;
        }

        std::string text = search_parts.str();
        if (text.empty()) {
          continue;
        }

        KopsikAutocompleteItem *autocomplete_item = autocomplete_item_init();
        autocomplete_item->Description = strdup(description.c_str());
        autocomplete_item->Text = strdup(text.c_str());
        autocomplete_item->ProjectAndTaskLabel = strdup(project_label.c_str());
        if (p) {
          autocomplete_item->ProjectColor = strdup(p->ColorCode().c_str());
          autocomplete_item->ProjectID = p->ID();
        }
        if (t) {
          autocomplete_item->TaskID = t->ID();
        }
        autocomplete_item->Type = KOPSIK_AUTOCOMPLETE_TE;
        autocomplete_items.push_back(autocomplete_item);
      }
    }

    // Add tasks, in format:
    // Task. Project. Client
    if (include_tasks) {
      for (std::vector<kopsik::Task *>::const_iterator it =
           ctx->user->related.Tasks.begin();
           it != ctx->user->related.Tasks.end(); it++) {
        kopsik::Task *t = *it;

        if (t->IsMarkedAsDeletedOnServer()) {
          continue;
        }

        kopsik::Project *p = 0;
        if (t->PID()) {
          p = ctx->user->GetProjectByID(t->PID());
        }

        if (p && !p->Active()) {
          continue;
        }

        kopsik::Client *c = 0;
        if (p && p->CID()) {
          c = ctx->user->GetClientByID(p->CID());
        }

        std::string text = ctx->user->JoinTaskName(t, p, c);
        if (text.empty()) {
          continue;
        }

        KopsikAutocompleteItem *autocomplete_item = autocomplete_item_init();
        autocomplete_item->Text = strdup(text.c_str());
        autocomplete_item->ProjectAndTaskLabel = strdup(text.c_str());
        autocomplete_item->TaskID = t->ID();
        if (p) {
          autocomplete_item->ProjectColor = strdup(p->ColorCode().c_str());
          autocomplete_item->ProjectID = p->ID();
        }
        autocomplete_item->Type = KOPSIK_AUTOCOMPLETE_TASK;
        autocomplete_items.push_back(autocomplete_item);
      }
    }

    // Add projects, in format:
    // Project. Client
    if (include_projects) {
      for (std::vector<kopsik::Project *>::const_iterator it =
           ctx->user->related.Projects.begin();
           it != ctx->user->related.Projects.end(); it++) {
        kopsik::Project *p = *it;

        if (p && !p->Active()) {
          continue;
        }

        kopsik::Client *c = 0;
        if (p->CID()) {
          c = ctx->user->GetClientByID(p->CID());
        }

        std::string text = ctx->user->JoinTaskName(0, p, c);
        if (text.empty()) {
          continue;
        }

        KopsikAutocompleteItem *autocomplete_item = autocomplete_item_init();
        autocomplete_item->Text = strdup(text.c_str());
        autocomplete_item->ProjectAndTaskLabel = strdup(text.c_str());
        autocomplete_item->ProjectID = p->ID();
        if (p) {
          autocomplete_item->ProjectColor = strdup(p->ColorCode().c_str());
        }
        autocomplete_item->Type = KOPSIK_AUTOCOMPLETE_PROJECT;
        autocomplete_items.push_back(autocomplete_item);
      }
    }

    std::sort(autocomplete_items.begin(), autocomplete_items.end(),
      compareAutocompleteItems);

    list->Length = 0;

    size_t list_size = autocomplete_items.size();

    KopsikAutocompleteItem *tmp = autocomplete_item_init();
    void *m = malloc(list_size * sizeof(tmp));
    autocomplete_item_clear(tmp);
    poco_assert(m);

    list->ViewItems = reinterpret_cast<KopsikAutocompleteItem **>(m);

    for (unsigned int i = 0; i < autocomplete_items.size(); i++) {
      list->ViewItems[list->Length] = autocomplete_items[i];
      list->Length++;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

// Time entries view API

KopsikTimeEntryViewItem *kopsik_time_entry_view_item_init() {
  KopsikTimeEntryViewItem *item = new KopsikTimeEntryViewItem();
  item->DurationInSeconds = 0;
  item->Description = 0;
  item->ProjectAndTaskLabel = 0;
  item->PID = 0;
  item->TID = 0;
  item->Duration = 0;
  item->Color = 0;
  item->GUID = 0;
  item->Billable = 0;
  item->Tags = 0;
  item->Started = 0;
  item->Ended = 0;
  item->UpdatedAt = 0;
  item->DateHeader = 0;
  item->DurOnly = 0;
  return item;
}

void kopsik_time_entry_view_item_clear(
    KopsikTimeEntryViewItem *item) {
  poco_assert(item);
  if (item->Description) {
    free(item->Description);
    item->Description = 0;
  }
  if (item->ProjectAndTaskLabel) {
    free(item->ProjectAndTaskLabel);
    item->ProjectAndTaskLabel = 0;
  }
  if (item->Duration) {
    free(item->Duration);
    item->Duration = 0;
  }
  if (item->Color) {
    free(item->Color);
    item->Color = 0;
  }
  if (item->GUID) {
    free(item->GUID);
    item->GUID = 0;
  }
  if (item->Tags) {
    free(item->Tags);
    item->Tags = 0;
  }
  if (item->UpdatedAt) {
    free(item->UpdatedAt);
    item->UpdatedAt = 0;
  }
  if (item->DateHeader) {
    free(item->DateHeader);
    item->DateHeader = 0;
  }
  if (item->DateDuration) {
    free(item->DateDuration);
    item->DateDuration = 0;
  }
  delete item;
}

void kopsik_format_duration_in_seconds_hhmmss(
    int duration_in_seconds,
    char *out_str,
    unsigned int max_strlen) {
  poco_assert(out_str);
  poco_assert(max_strlen);
  std::string formatted =
    kopsik::Formatter::FormatDurationInSecondsHHMMSS(duration_in_seconds);
  strncpy(out_str, formatted.c_str(), max_strlen);
}

void kopsik_format_duration_in_seconds_hhmm(
    int duration_in_seconds,
    int type,
    char *out_str,
    unsigned int max_strlen) {
  poco_assert(out_str);
  poco_assert(max_strlen);
  std::string formatted = kopsik::Formatter::FormatDurationInSecondsHHMM(
    duration_in_seconds, type);
  strncpy(out_str, formatted.c_str(), max_strlen);
}

kopsik_api_result kopsik_start(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *description,
    const char *duration,
    const unsigned int task_id,
    const unsigned int project_id,
    KopsikTimeEntryViewItem *out_view_item) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_view_item);

    logger().debug("kopsik_start");

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to start time tracking", errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    std::string desc("");
    if (description) {
      desc = std::string(description);
    }

    std::string dur("");
    if (duration) {
      dur = std::string(duration);
    }

    kopsik::TimeEntry *te = ctx->user->Start(desc, dur, task_id, project_id);
    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }

    time_entry_to_view_item(te, ctx->user, out_view_item, "");
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_time_entry_view_item_by_guid(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *guid,
    KopsikTimeEntryViewItem *view_item,
    int *was_found) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(view_item);
    poco_assert(was_found);

    std::stringstream ss;
    ss << "kopsik_time_entry_view_item_by_guid guid=" << guid;
    logger().trace(ss.str());

    std::string GUID(guid);
    if (GUID.empty()) {
      strncpy(errmsg, "Missing GUID", errlen);
      return KOPSIK_API_FAILURE;
    }

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to load time entry data", errlen);
      return KOPSIK_API_FAILURE;
    }
    kopsik::TimeEntry *te = ctx->user->GetTimeEntryByGUID(GUID);
    if (te) {
      *was_found = 1;
      time_entry_to_view_item(te, ctx->user, view_item, "");
    } else {
      *was_found = 0;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_continue(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *guid,
    KopsikTimeEntryViewItem *view_item) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(view_item);

    std::stringstream ss;
    ss << "kopsik_continue guid=" << guid;
    logger().debug(ss.str());

    std::string GUID(guid);
    if (GUID.empty()) {
      strncpy(errmsg, "Missing GUID", errlen);
      return KOPSIK_API_FAILURE;
    }

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to continue time tracking", errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::TimeEntry *te = ctx->user->Continue(GUID);
    poco_assert(te);
    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }

    time_entry_to_view_item(te, ctx->user, view_item, "");
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_continue_latest(
    void *context,
    char *errmsg,
    unsigned int errlen,
    KopsikTimeEntryViewItem *view_item,
    int *was_found) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(view_item);
    poco_assert(was_found);

    logger().debug("kopsik_continue_latest");

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to continue tracking last time entry",
              errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);
    ctx->user->SortTimeEntriesByStart();

    kopsik::TimeEntry *latest = ctx->user->Latest();
    if (!latest) {
      *was_found = 0;
      return KOPSIK_API_SUCCESS;
    }

    kopsik::TimeEntry *te = ctx->user->Continue(latest->GUID());
    poco_assert(te);
    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_SUCCESS;
    }
    *was_found = 1;
    time_entry_to_view_item(te, ctx->user, view_item, "");
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_delete_time_entry(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *guid) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);

    std::stringstream ss;
    ss << "kopsik_delete_time_entry guid=" << guid;
    logger().debug(ss.str());

    std::string GUID(guid);
    if (GUID.empty()) {
      strncpy(errmsg, "Missing GUID", errlen);
      return KOPSIK_API_FAILURE;
    }

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to delete time entry", errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::TimeEntry *te = ctx->user->GetTimeEntryByGUID(GUID);
    poco_assert(te);
    te->SetDeletedAt(time(0));
    te->SetUIModifiedAt(time(0));

    kopsik::ModelChange mc("time_entry", "delete", te->ID(), te->GUID());
    KopsikModelChange *change = model_change_init();
    model_change_to_change_item(mc, *change);
    ctx->change_callback(KOPSIK_API_SUCCESS, 0, change);
    model_change_clear(change);

    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_duration(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *guid,
    const char *value) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_duration guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    std::string GUID(guid);
    if (GUID.empty()) {
      strncpy(errmsg, "Missing GUID", errlen);
      return KOPSIK_API_FAILURE;
    }

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to set time entry duration", errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::TimeEntry *te = ctx->user->GetTimeEntryByGUID(GUID);
    poco_assert(te);
    te->SetDurationString(std::string(value));

    if (te->Dirty()) {
      te->SetUIModifiedAt(time(0));
    }

    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_project(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *guid,
    const unsigned int task_id,
    const unsigned int project_id) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);

    std::string GUID(guid);
    if (GUID.empty()) {
      strncpy(errmsg, "Missing GUID", errlen);
      return KOPSIK_API_FAILURE;
    }

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to select project", errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::TimeEntry *te = ctx->user->GetTimeEntryByGUID(GUID);
    poco_assert(te);

    if (project_id) {
      kopsik::Project *p = ctx->user->GetProjectByID(project_id);
      if (p) {
        te->SetBillable(p->Billable());
      }
    }

    te->SetTID(task_id);
    te->SetPID(project_id);

    if (te->Dirty()) {
      te->SetUIModifiedAt(time(0));
    }

    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_start_iso_8601(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *guid,
    const char *value) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_start_iso_8601 guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    std::string GUID(guid);
    if (GUID.empty()) {
      strncpy(errmsg, "Missing GUID", errlen);
      return KOPSIK_API_FAILURE;
    }

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to change time entry start time", errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::TimeEntry *te = ctx->user->GetTimeEntryByGUID(GUID);
    poco_assert(te);
    te->SetStartString(std::string(value));

    if (te->Dirty()) {
      te->SetUIModifiedAt(time(0));
    }

    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_end_iso_8601(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *guid,
    const char *value) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_end_iso_8601 guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    std::string GUID(guid);
    if (GUID.empty()) {
      strncpy(errmsg, "Missing GUID", errlen);
      return KOPSIK_API_FAILURE;
    }

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to change time entry end time", errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::TimeEntry *te = ctx->user->GetTimeEntryByGUID(GUID);
    poco_assert(te);
    te->SetStopString(std::string(value));
    if (te->Dirty()) {
      te->SetUIModifiedAt(time(0));
    }

    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_tags(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *guid,
    const char *value) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_tags guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    std::string GUID(guid);
    if (GUID.empty()) {
      strncpy(errmsg, "Missing GUID", errlen);
      return KOPSIK_API_FAILURE;
    }

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to change time entry tags", errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::TimeEntry *te = ctx->user->GetTimeEntryByGUID(GUID);
    poco_assert(te);
    te->SetTags(std::string(value));
    if (te->Dirty()) {
      te->SetUIModifiedAt(time(0));
    }

    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_billable(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *guid,
    int value) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_billable guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    std::string GUID(guid);
    if (GUID.empty()) {
      strncpy(errmsg, "Missing GUID", errlen);
      return KOPSIK_API_FAILURE;
    }

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to change time entry billable state",
              errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::TimeEntry *te = ctx->user->GetTimeEntryByGUID(GUID);
    poco_assert(te);
    if (value) {
      te->SetBillable(true);
    } else {
      te->SetBillable(false);
    }
    if (te->Dirty()) {
      te->SetUIModifiedAt(time(0));
    }

    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_description(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *guid,
    const char *value) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_description guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    std::string GUID(guid);
    if (GUID.empty()) {
      strncpy(errmsg, "Missing GUID", errlen);
      return KOPSIK_API_FAILURE;
    }

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to change time entry description", errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::TimeEntry *te = ctx->user->GetTimeEntryByGUID(GUID);
    poco_assert(te);
    te->SetDescription(std::string(value));
    if (te->Dirty()) {
      te->SetUIModifiedAt(time(0));
    }

    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_stop(
    void *context,
    char *errmsg,
    unsigned int errlen,
    KopsikTimeEntryViewItem *out_view_item,
    int *was_found) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_view_item);
    poco_assert(was_found);

    logger().debug("kopsik_stop");

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to stop time tracking", errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    std::vector<kopsik::TimeEntry *> stopped = ctx->user->Stop();
    if (stopped.empty()) {
      *was_found = 0;
      return KOPSIK_API_SUCCESS;
    }
    *was_found = 1;

    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }

    kopsik::TimeEntry *te = stopped[0];
    time_entry_to_view_item(te, ctx->user, out_view_item, "");
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_split_running_time_entry_at(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const unsigned int at,
    KopsikTimeEntryViewItem *out_view_item,
    int *was_found) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_view_item);
    poco_assert(was_found);
    poco_assert(at);

    logger().debug("kopsik_stop");

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to split time entry", errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    *was_found = 0;
    kopsik::TimeEntry *running = ctx->user->SplitAt(at);
    if (running) {
      *was_found = 1;
      time_entry_to_view_item(running, ctx->user, out_view_item, "");
    }

    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_stop_running_time_entry_at(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const unsigned int at,
    KopsikTimeEntryViewItem *out_view_item,
    int *was_found) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_view_item);
    poco_assert(was_found);
    poco_assert(at);

    logger().debug("kopsik_stop");

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to stop running time entry", errlen);
      return KOPSIK_API_FAILURE;
    }

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    *was_found = 0;
    kopsik::TimeEntry *stopped = ctx->user->StopAt(at);
    if (stopped) {
      *was_found = 1;
      time_entry_to_view_item(stopped, ctx->user, out_view_item, "");
    }

    kopsik::error err = ctx->Save();
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_running_time_entry_view_item(
    void *context,
    char *errmsg,
    unsigned int errlen,
    KopsikTimeEntryViewItem *out_item,
    int *out_is_tracking) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_item);
    poco_assert(out_is_tracking);

    logger().debug("kopsik_running_time_entry_view_item");

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to access tracking time entry", errlen);
      return KOPSIK_API_FAILURE;
    }

    *out_is_tracking = 0;

    kopsik::TimeEntry *te = ctx->user->RunningTimeEntry();
    if (te) {
      *out_is_tracking = true;
      time_entry_to_view_item(te, ctx->user, out_item, "");
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

KopsikTimeEntryViewItemList *kopsik_time_entry_view_item_list_init() {
  KopsikTimeEntryViewItemList *result = new KopsikTimeEntryViewItemList();
  result->Length = 0;
  result->ViewItems = 0;
  return result;
}

void kopsik_time_entry_view_item_list_clear(
    KopsikTimeEntryViewItemList *in_list) {
  poco_assert(in_list);
  for (unsigned int i = 0; i < in_list->Length; i++) {
    kopsik_time_entry_view_item_clear(in_list->ViewItems[i]);
    in_list->ViewItems[i] = 0;
  }
  if (in_list->ViewItems) {
    free(in_list->ViewItems);
  }
  delete in_list;
}

kopsik_api_result kopsik_time_entry_view_items(
    void *context,
    char *errmsg,
    unsigned int errlen,
    KopsikTimeEntryViewItemList *out_list) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_list);

    logger().debug("kopsik_time_entry_view_items");

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to access time entries", errlen);
      return KOPSIK_API_FAILURE;
    }

    {
      Poco::Mutex::ScopedLock lock(ctx->mutex);
      ctx->user->SortTimeEntriesByStart();
    }

    std::map<std::string, Poco::Int64> date_durations;

    std::vector<kopsik::TimeEntry *>visible;
    for (std::vector<kopsik::TimeEntry *>::const_iterator it =
        ctx->user->related.TimeEntries.begin();
        it != ctx->user->related.TimeEntries.end(); it++) {
      kopsik::TimeEntry *te = *it;
      poco_assert(!te->GUID().empty());
      if (te->DurationInSeconds() < 0) {
        continue;
      }
      if (te->DeletedAt() > 0) {
        continue;
      }
      visible.push_back(te);

      std::string date_header = te->DateHeaderString();
      Poco::Int64 duration = date_durations[date_header];
      duration += te->DurationInSeconds();
      date_durations[date_header] = duration;
    }

    if (visible.empty()) {
      return KOPSIK_API_SUCCESS;
    }

    out_list->Length = 0;

    KopsikTimeEntryViewItem *tmp = kopsik_time_entry_view_item_init();
    void *m = malloc(visible.size() * sizeof(tmp));
    kopsik_time_entry_view_item_clear(tmp);
    poco_assert(m);
    out_list->ViewItems =
      reinterpret_cast<KopsikTimeEntryViewItem **>(m);
    for (unsigned int i = 0; i < visible.size(); i++) {
      kopsik::TimeEntry *te = visible[i];
      KopsikTimeEntryViewItem *view_item = kopsik_time_entry_view_item_init();

      Poco::Int64 duration = date_durations[te->DateHeaderString()];
      std::string formatted =
        kopsik::Formatter::FormatDurationInSecondsHHMM(duration, 2);
      time_entry_to_view_item(te, ctx->user, view_item, formatted);

      out_list->ViewItems[i] = view_item;
      out_list->Length++;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_duration_for_date_header(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *date,
    char *duration,
    const unsigned int duration_len) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(duration);
    poco_assert(duration_len);
    poco_assert(date);

    logger().debug("kopsik_duration_for_date_header");

    Context *ctx = reinterpret_cast<Context *>(context);

    if (!ctx->user) {
      strncpy(errmsg, "Please login to access time entry", errlen);
      return KOPSIK_API_FAILURE;
    }

    int sum(0);
    std::string date_header(date);
    for (std::vector<kopsik::TimeEntry *>::const_iterator it =
        ctx->user->related.TimeEntries.begin();
        it != ctx->user->related.TimeEntries.end(); it++) {
      kopsik::TimeEntry *te = *it;
      if (te->DurationInSeconds() >= 0 && !te->DeletedAt() &&
          te->DateHeaderString() == date_header) {
        sum += te->DurationInSeconds();
      }
    }

    kopsik_format_duration_in_seconds_hhmm(sum, 0, duration, duration_len);
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

// Websocket client

void on_websocket_message(
    void *context,
    std::string json) {
  poco_assert(context);
  poco_assert(!json.empty());

  std::stringstream ss;
  ss << "on_websocket_message json=" << json;
  logger().debug(ss.str());

  Context *ctx = reinterpret_cast<Context *>(context);

  Poco::Mutex::ScopedLock lock(ctx->mutex);

  ctx->user->LoadUpdateFromJSONString(json);

  kopsik::error err = ctx->Save();
  if (err != kopsik::noError) {
    // FIXME: error handling
  }
}

void kopsik_websocket_start(
    void *context) {
  poco_assert(context);

  logger().debug("kopsik_websocket_start");

  Context *ctx = reinterpret_cast<Context *>(context);

  ctx->tm.start(new WebSocketStartTask(ctx, on_websocket_message));
}

void kopsik_websocket_stop(
    void *context) {
  poco_assert(context);

  logger().debug("kopsik_websocket_stop");

  Context *ctx = reinterpret_cast<Context *>(context);

  ctx->tm.start(new WebSocketStopTask(ctx));
}

// Timeline

void kopsik_timeline_start(
    void *context,
    KopsikResultCallback callback) {
  poco_assert(context);
  poco_assert(callback);

  logger().debug("kopsik_timeline_start");

  Context *ctx = reinterpret_cast<Context *>(context);
  ctx->tm.start(new TimelineStartTask(ctx, callback));
}

void kopsik_timeline_stop(
    void *context,
    KopsikResultCallback callback) {
  poco_assert(context);

  logger().debug("kopsik_timeline_stop");

  Context *ctx = reinterpret_cast<Context *>(context);
  ctx->tm.start(new TimelineStopTask(ctx, callback));
}

void kopsik_timeline_toggle_recording(
    void *context,
    KopsikResultCallback callback) {
  poco_assert(context);

  logger().debug("kopsik_timeline_toggle_recording");

  Context *ctx = reinterpret_cast<Context *>(context);

  Poco::Mutex::ScopedLock lock(ctx->mutex);

  ctx->user->SetRecordTimeline(!ctx->user->RecordTimeline());
  kopsik::error err = ctx->Save();
  if (err != kopsik::noError) {
    callback(KOPSIK_API_FAILURE, err.c_str());
    return;
  }

  ctx->tm.start(new TimelineUpdateServerSettingsTask(ctx, callback));
  if (ctx->user->RecordTimeline()) {
    ctx->tm.start(new TimelineStartTask(ctx, callback));
    return;
  }
  ctx->tm.start(new TimelineStopTask(ctx, callback));
}

int kopsik_timeline_is_recording_enabled(void *context) {
  if (!context) {
    return 0;
  }
  Context *ctx = reinterpret_cast<Context *>(context);
  if (!ctx->user) {
    return 0;
  }
  return ctx->user->RecordTimeline();
}

// Feedback

void kopsik_feedback_send(void *context,
                          const char *topic,
                          const char *details,
                          const char *base64encoded_image,
                          KopsikResultCallback callback) {
  poco_assert(context);
  poco_assert(callback);

  logger().debug("kopsik_feedback_send");

  Context *ctx = reinterpret_cast<Context *>(context);
  if (!ctx->user) {
    callback(KOPSIK_API_FAILURE, "Please login to send feedback");
    return;
  }

  if (!topic || !strlen(topic)) {
    callback(KOPSIK_API_FAILURE, "Missing topic");
    return;
  }

  if (!details || !strlen(details)) {
    callback(KOPSIK_API_FAILURE, "Missing details");
    return;
  }

  std::string image("");
  if (base64encoded_image && strlen(base64encoded_image)) {
    image = std::string(base64encoded_image);
  }

  ctx->tm.start(new SendFeedbackTask(ctx,
                                     std::string(topic),
                                     std::string(details),
                                     image,
                                     callback));
}

// Updates

void kopsik_check_for_updates(
    void *context,
    KopsikCheckUpdateCallback callback) {
  poco_assert(context);

  logger().debug("kopsik_check_for_updates");

  Context *ctx = reinterpret_cast<Context *>(context);

  std::string update_channel("");
  kopsik::error err = ctx->db->LoadUpdateChannel(&update_channel);
  if (err != kopsik::noError) {
    callback(KOPSIK_API_FAILURE, err.c_str(), 0, 0, 0);
    return;
  }

  ctx->tm.start(new FetchUpdatesTask(ctx, callback, update_channel));
}

kopsik_api_result kopsik_set_update_channel(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *update_channel) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(update_channel);

    Context *ctx = reinterpret_cast<Context *>(context);

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    kopsik::error err =
      ctx->db->SaveUpdateChannel(std::string(update_channel));
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_get_update_channel(
    void *context,
    char *errmsg,
    unsigned int errlen,
    char *update_channel,
    unsigned int update_channel_len) {
  try {
    poco_assert(context);
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(update_channel);
    poco_assert(update_channel_len);

    Context *ctx = reinterpret_cast<Context *>(context);

    Poco::Mutex::ScopedLock lock(ctx->mutex);

    std::string s("");
    kopsik::error err = ctx->db->LoadUpdateChannel(&s);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }

    strncpy(update_channel, s.c_str(), update_channel_len);
  } catch(const Poco::Exception& exc) {
      strncpy(errmsg, exc.displayText().c_str(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::exception& ex) {
      strncpy(errmsg, ex.what(), errlen);
      return KOPSIK_API_FAILURE;
  } catch(const std::string& ex) {
      strncpy(errmsg, ex.c_str(), errlen);
      return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}
