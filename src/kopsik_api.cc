// Copyright 2014 Toggl Desktop developers.

// No exceptions should be thrown from this library.

#include <cstring>
#include <set>

#include "./kopsik_api.h"
#include "./kopsik_api_private.h"
#include "./database.h"
#include "./user.h"
#include "./https_client.h"
#include "./websocket_client.h"
#include "./version.h"
#include "./timeline_uploader.h"
#include "./window_change_recorder.h"
#include "./CustomErrorHandler.h"
#include "./proxy.h"
#include "./context.h"
#include "./formatter.h"

#include "Poco/Bugcheck.h"
#include "Poco/Path.h"
#include "Poco/Logger.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"

inline Poco::Logger &logger() {
  return Poco::Logger::get("kopsik_api");
}

inline Poco::Logger &rootLogger() {
  return Poco::Logger::get("");
}

inline kopsik::Context *app(void *context) {
  poco_assert(context);
  return reinterpret_cast<kopsik::Context *>(context);
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
    const char *app_version,
    KopsikViewItemChangeCallback change_callback,
    KopsikErrorCallback on_error_callback,
    KopsikCheckUpdateCallback check_updates_callback) {
  poco_assert(app_name);
  poco_assert(app_version);

  kopsik::Context *ctx =
    new kopsik::Context(std::string(app_name), std::string(app_version));

  ctx->SetChangeCallback(change_callback);
  ctx->SetOnErrorCallback(on_error_callback);
  ctx->SetCheckUpdatesCallback(check_updates_callback);

  ctx->SetAPIURL("https://www.toggl.com");
  ctx->SetTimelineUploadURL("https://timeline.toggl.com");
  ctx->SetWebSocketClientURL("https://stream.toggl.com");

  return ctx;
}

void kopsik_context_shutdown(void *context) {
  app(context)->Shutdown();
}

void kopsik_context_clear(void *context) {
  delete app(context);
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(settings);

    bool use_proxy(false);
    bool use_idle_detection(false);
    kopsik::Proxy proxy;
    kopsik::error err = app(context)->LoadSettings(&use_proxy,
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

    settings->ProxyHost = strdup(proxy.host.c_str());
    settings->ProxyPort = proxy.port;
    settings->ProxyUsername = strdup(proxy.username.c_str());
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(proxy_host);
    poco_assert(proxy_username);
    poco_assert(proxy_password);

    kopsik::Proxy proxy;
    proxy.host = std::string(proxy_host);
    proxy.port = proxy_port;
    proxy.username = std::string(proxy_username);
    proxy.password = std::string(proxy_password);

    kopsik::error err = app(context)->SaveSettings(
      use_proxy, &proxy, use_idle_detection);
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
    poco_assert(errmsg);
    poco_assert(errlen);

    kopsik::error err = app(context)->ConfigureProxy();
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

kopsik_api_result kopsik_set_db_path(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *path) {
  kopsik::error err = kopsik::noError;
  try {
    poco_assert(path);

    std::stringstream ss;
    ss << "kopsik_set_db_path path=" << path;
    logger().debug(ss.str());

    app(context)->SetDBPath(std::string(path));
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

void kopsik_set_log_path(const char *path) {
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

void kopsik_set_log_level(const char *level) {
  poco_assert(level);

  rootLogger().setLevel(level);
}

void kopsik_set_api_url(
    void *context,
    const char *api_url) {
  poco_assert(api_url);

  app(context)->SetAPIURL(std::string(api_url));
}

void kopsik_set_websocket_url(
    void *context,
    const char *websocket_url) {
  poco_assert(websocket_url);

  app(context)->SetWebSocketClientURL(websocket_url);
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_user);

    logger().debug("kopsik_current_user");

    kopsik::User *current_user = 0;
    kopsik::error err = app(context)->CurrentUser(&current_user);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }

    if (!current_user) {
      return KOPSIK_API_SUCCESS;
    }
    if (out_user->Fullname) {
      free(out_user->Fullname);
      out_user->Fullname = 0;
    }
    out_user->Fullname = strdup(current_user->Fullname().c_str());
    out_user->ID = (unsigned int)current_user->ID();
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(api_token);

    std::stringstream ss;
    ss << "kopsik_set_api_token api_token=" << api_token;
    logger().debug(ss.str());

    kopsik::error err = app(context)->SetCurrentAPIToken(api_token);
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(str);
    poco_assert(max_strlen);

    std::string token("");
    kopsik::error err = app(context)->CurrentAPIToken(&token);
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

kopsik_api_result kopsik_set_logged_in_user(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *json) {
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(json);

  try {
    logger().debug("kopsik_set_logged_in_user");

    kopsik::error err =
      app(context)->SetLoggedInUserFromJSON(std::string(json));
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

kopsik_api_result kopsik_login(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *in_email,
    const char *in_password) {
  try {
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

    kopsik::error err = app(context)->Login(email, password);
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
  poco_assert(errmsg);
  poco_assert(errlen);

  logger().debug("kopsik_logout");

  kopsik::error err = app(context)->Logout();
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }

  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_clear_cache(
    void *context,
    char *errmsg,
    unsigned int errlen) {
  poco_assert(errmsg);
  poco_assert(errlen);

  logger().debug("kopsik_clear_cache");

  kopsik::error err = app(context)->ClearCache();
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_user_has_premium_workspaces(
    void *context,
    char *errmsg,
    unsigned int errlen,
    int *has_premium_workspaces) {
  try {
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(has_premium_workspaces);

    *has_premium_workspaces = 0;
    if (app(context)->UserHasPremiumWorkspaces()) {
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(stats);

    logger().debug("kopsik_pushable_models");

    std::vector<kopsik::TimeEntry *> pushable;
    app(context)->CollectPushableTimeEntries(&pushable);

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

void kopsik_sync(void *context) {
  logger().debug("kopsik_sync");
  app(context)->FullSync();
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(list);

    logger().debug("kopsik_autocomplete_items");

    /* FIXME: collect autocomplete items

    if (!ctx->user) {
      // User is already logged out
      return KOPSIK_API_SUCCESS;
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

        if (!p->Active()) {
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
        autocomplete_item->ProjectColor = strdup(p->ColorCode().c_str());
        autocomplete_item->Type = KOPSIK_AUTOCOMPLETE_PROJECT;
        autocomplete_items.push_back(autocomplete_item);
      }
    }

    std::sort(autocomplete_items.begin(), autocomplete_items.end(),
      compareAutocompleteItems);

    list->Length = 0;

    size_t list_size = autocomplete_items.size();

    // FIXME: get rid of malloc, use a list
    KopsikAutocompleteItem *tmp = autocomplete_item_init();
    void *m = malloc(list_size * sizeof(tmp));
    autocomplete_item_clear(tmp);
    poco_assert(m);

    list->ViewItems = reinterpret_cast<KopsikAutocompleteItem **>(m);

    for (unsigned int i = 0; i < autocomplete_items.size(); i++) {
      list->ViewItems[list->Length] = autocomplete_items[i];
      list->Length++;
    }
     
    */
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

// Tags

kopsik_api_result kopsik_tags(
    void *context,
    char *errmsg,
    unsigned int errlen,
    KopsikTagViewItem **first) {
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(first);
  poco_assert(!*first);

  std::vector<std::string> tags = app(context)->Tags();

  *first = 0;
  for (std::vector<std::string>::const_iterator it = tags.begin();
       it != tags.end();
       it++) {
    std::string name = *it;
    KopsikTagViewItem *item = new KopsikTagViewItem();
    item->Name = strdup(name.c_str());
    item->Next = *first;
    *first = item;
  }

  return KOPSIK_API_SUCCESS;
}

void kopsik_tags_clear(
    KopsikTagViewItem *first) {
  if (!first) {
    return;
  }
  if (first->Name) {
    free(first->Name);
    first->Name = 0;
  }
  if (first->Next) {
    KopsikTagViewItem *next =
      reinterpret_cast<KopsikTagViewItem *>(first->Next);
    kopsik_tags_clear(next);
  }
  delete first;
  first = 0;
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_view_item);

    logger().debug("kopsik_start");

    std::string desc("");
    if (description) {
      desc = std::string(description);
    }

    std::string dur("");
    if (duration) {
      dur = std::string(duration);
    }

    kopsik::TimeEntry *te = app(context)->Start(desc, dur, task_id, project_id);
    if (te) {
      std::string project_label("");
      std::string color_code("");
      app(context)->ProjectLabelAndColorCode(te, &project_label, &color_code);
      time_entry_to_view_item(te, project_label, color_code, out_view_item, "");
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

kopsik_api_result kopsik_time_entry_view_item_by_guid(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *guid,
    KopsikTimeEntryViewItem *view_item,
    int *was_found) {
  try {
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(view_item);
    poco_assert(was_found);

    std::stringstream ss;
    ss << "kopsik_time_entry_view_item_by_guid guid=" << guid;
    logger().trace(ss.str());

    std::string GUID(guid);
    poco_assert(!GUID.empty());

    kopsik::TimeEntry *te = app(context)->GetTimeEntryByGUID(GUID);
    if (!te) {
      *was_found = 0;
      return KOPSIK_API_SUCCESS;
    }

    *was_found = 1;

    std::string project_label("");
    std::string color_code("");
    app(context)->ProjectLabelAndColorCode(te, &project_label, &color_code);

    time_entry_to_view_item(te, project_label, color_code, view_item, "");
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

    kopsik::TimeEntry *te = app(context)->Continue(GUID);
    if (te) {
      std::string project_label("");
      std::string color_code("");
      app(context)->ProjectLabelAndColorCode(te, &project_label, &color_code);
      time_entry_to_view_item(te, project_label, color_code, view_item, "");
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

kopsik_api_result kopsik_continue_latest(
    void *context,
    char *errmsg,
    unsigned int errlen,
    KopsikTimeEntryViewItem *view_item,
    int *was_found) {
  try {
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(view_item);
    poco_assert(was_found);

    *was_found = 0;

    logger().debug("kopsik_continue_latest");

    kopsik::TimeEntry *te = app(context)->ContinueLatest();
    if (te) {
      *was_found = 1;
      std::string project_label("");
      std::string color_code("");
      app(context)->ProjectLabelAndColorCode(te, &project_label, &color_code);
      time_entry_to_view_item(te, project_label, color_code, view_item, "");
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

kopsik_api_result kopsik_delete_time_entry(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *guid) {
  try {
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

    kopsik::error err = app(context)->DeleteTimeEntryByGUID(GUID);
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_duration guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    kopsik::error err = app(context)->SetTimeEntryDuration(std::string(guid),
                                                           std::string(value));
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    kopsik::error err = app(context)->SetTimeEntryProject(std::string(guid),
                                                          task_id,
                                                          project_id);
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_start_iso_8601 guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    kopsik::error err =
      app(context)->SetTimeEntryStartISO8601(std::string(guid),
                                             std::string(value));
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_end_iso_8601 guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    kopsik::error err = app(context)->SetTimeEntryEndISO8601(
      std::string(guid),
      std::string(value));
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_tags guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    kopsik::error err = app(context)->SetTimeEntryTags(std::string(guid),
                                                       std::string(value));
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_billable guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    kopsik::error err =
      app(context)->SetTimeEntryBillable(std::string(guid), value);
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_description guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    kopsik::error err =
      app(context)->SetTimeEntryDescription(std::string(guid),
                                            std::string(value));
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_view_item);
    poco_assert(was_found);

    logger().debug("kopsik_stop");

    *was_found = 0;

    kopsik::TimeEntry *te = 0;
    kopsik::error err = app(context)->Stop(&te);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
    if (te) {
      *was_found = 1;
      std::string project_label("");
      std::string color_code("");
      app(context)->ProjectLabelAndColorCode(te, &project_label, &color_code);
      time_entry_to_view_item(te, project_label, color_code, out_view_item, "");
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

kopsik_api_result kopsik_split_running_time_entry_at(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const unsigned int at,
    KopsikTimeEntryViewItem *out_view_item,
    int *was_found) {
  try {
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_view_item);
    poco_assert(was_found);
    poco_assert(at);

    logger().debug("kopsik_stop");

    *was_found = 0;
    kopsik::TimeEntry *te = 0;
    kopsik::error err = app(context)->SplitAt(at, &te);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
    if (te) {
      *was_found = 1;
      std::string project_label("");
      std::string color_code("");
      app(context)->ProjectLabelAndColorCode(te, &project_label, &color_code);
      time_entry_to_view_item(te,
                              project_label,
                              color_code,
                              out_view_item,
                              "");
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_view_item);
    poco_assert(was_found);
    poco_assert(at);

    logger().debug("kopsik_stop");

    *was_found = 0;
    kopsik::TimeEntry *te = 0;
    kopsik::error err = app(context)->StopAt(at, &te);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
    if (te) {
      *was_found = 1;
      std::string project_label("");
      std::string color_code("");
      app(context)->ProjectLabelAndColorCode(te, &project_label, &color_code);
      time_entry_to_view_item(te,
                              project_label,
                              color_code,
                              out_view_item,
                              "");
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_item);
    poco_assert(out_is_tracking);

    logger().debug("kopsik_running_time_entry_view_item");

    *out_is_tracking = 0;
    kopsik::TimeEntry *te = 0;
    kopsik::error err = app(context)->RunningTimeEntry(&te);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
    if (te) {
      *out_is_tracking = true;
      std::string project_label("");
      std::string color_code("");
      app(context)->ProjectLabelAndColorCode(te, &project_label, &color_code);
      time_entry_to_view_item(te,
                              project_label,
                              color_code,
                              out_item,
                              "");
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(out_list);

    logger().debug("kopsik_time_entry_view_items");

    std::map<std::string, Poco::Int64> date_durations;
    std::vector<kopsik::TimeEntry *> visible;

    kopsik::error err = app(context)->TimeEntries(&date_durations,
      &visible);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }

    if (visible.empty()) {
      return KOPSIK_API_SUCCESS;
    }

    out_list->Length = 0;

    // FIXME: get rid of malloc, use a list
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

      std::string project_label("");
      std::string color_code("");
      app(context)->ProjectLabelAndColorCode(te, &project_label, &color_code);
      time_entry_to_view_item(te,
                              project_label,
                              color_code,
                              view_item,
                              formatted);

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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(duration);
    poco_assert(duration_len);
    poco_assert(date);

    logger().debug("kopsik_duration_for_date_header");

    int sum(0);
    kopsik::error err =
      app(context)->TrackedPerDateHeader(std::string(date), &sum);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
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

void kopsik_websocket_switch(
    void *context,
    const unsigned int on) {
  std::stringstream ss;
  ss << "kopsik_websocket_switch on=" << on;
  logger().debug(ss.str());

  if (on) {
    app(context)->SwitchWebSocketOn();
    return;
  }
  app(context)->SwitchWebSocketOff();
}

// Timeline

void kopsik_timeline_switch(
    void *context,
    const unsigned int on) {
  std::stringstream ss;
  ss << "kopsik_timeline_switch on=" << on;
  logger().debug(ss.str());

  if (on) {
    app(context)->SwitchTimelineOn();
    return;
  }

  app(context)->SwitchTimelineOff();
}

void kopsik_timeline_toggle_recording(
    void *context) {
  logger().debug("kopsik_timeline_toggle_recording");
  app(context)->ToggleTimelineRecording();
}

int kopsik_timeline_is_recording_enabled(
    void *context) {
  return app(context)->RecordTimeline();
}

// Feedback

kopsik_api_result kopsik_feedback_send(
                                       void *context,
                                       char *errmsg,
                                       const unsigned int errlen,
                                       const char *topic,
                                       const char *details,
                                       const char *filename) {
    std::stringstream ss;
    ss << "kopsik_feedback_send topic=" << topic << " details=" << details;
    logger().debug(ss.str());

    kopsik::error err =
      app(context)->SendFeedback(std::string(topic),
                                 std::string(details),
                                 std::string(filename));
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

// Updates

void kopsik_check_for_updates(
    void *context) {
  logger().debug("kopsik_check_for_updates");

  app(context)->FetchUpdates();
}

kopsik_api_result kopsik_set_update_channel(
    void *context,
    char *errmsg,
    unsigned int errlen,
    const char *update_channel) {
  try {
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(update_channel);

    kopsik::error err =
      app(context)->SaveUpdateChannel(std::string(update_channel));
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
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(update_channel);
    poco_assert(update_channel_len);

    std::string s("");
    kopsik::error err = app(context)->LoadUpdateChannel(&s);
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

int kopsik_parse_duration_string_into_seconds(const char *duration_string) {
  if (!duration_string) {
    return 0;
  }
  return kopsik::Formatter::ParseDurationString(std::string(duration_string));
}
