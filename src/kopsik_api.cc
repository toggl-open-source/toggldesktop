// Copyright 2013 Tanel Lebedev

#include <cstring>

#include "./kopsik_api.h"
#include "./database.h"
#include "./toggl_api_client.h"

#include "Poco/Bugcheck.h"
#include "Poco/Path.h"
#include "Poco/Logger.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"

// Context API.

KopsikContext *kopsik_context_init() {
  KopsikContext *ctx = new KopsikContext();
  ctx->db = 0;
  return ctx;
}

void kopsik_context_db_clear(KopsikContext *ctx) {
  poco_assert(ctx);
  if (ctx->db) {
    kopsik::Database *db = reinterpret_cast<kopsik::Database *>(ctx->db);
    delete db;
    ctx->db = 0;
  }
}

void kopsik_context_user_clear(KopsikContext *ctx) {
  poco_assert(ctx);
  if (ctx->user) {
    kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->user);
    delete user;
    ctx->user = 0;
  }
}

void kopsik_context_clear(KopsikContext *ctx) {
  poco_assert(ctx);
  kopsik_context_db_clear(ctx);
  kopsik_context_user_clear(ctx);
  delete ctx;
  ctx = 0;
}

// Configuration API.

void kopsik_version(int *major, int *minor, int *patch) {
  poco_assert(major);
  poco_assert(minor);
  poco_assert(patch);
  *major = 0;
  *minor = 1;
  *patch = 0;
}

void kopsik_set_proxy(
    KopsikContext *ctx,
    const char *host, const unsigned int port,
    const char *username, const char *password) {
  poco_assert(ctx);
  poco_assert(host);
  poco_assert(username);
  poco_assert(password);
  // FIXME: implement
}

void kopsik_set_db_path(KopsikContext *ctx, const char *path) {
  poco_assert(ctx);
  poco_assert(path);
  kopsik_context_db_clear(ctx);
  ctx->db = new kopsik::Database(path);
}

void kopsik_set_log_path(KopsikContext *ctx, const char *path) {
  poco_assert(ctx);
  poco_assert(path);

  Poco::AutoPtr<Poco::SimpleFileChannel> simpleFileChannel(
    new Poco::SimpleFileChannel);
  simpleFileChannel->setProperty("path", path);
  simpleFileChannel->setProperty("rotation", "1 M");

  Poco::AutoPtr<Poco::FormattingChannel> formattingChannel(
      new Poco::FormattingChannel(
        new Poco::PatternFormatter("%Y-%m-%d %H:%M:%S.%c [%P]:%s:%q:%t")));
  formattingChannel->setChannel(simpleFileChannel);

  Poco::Logger &logger = Poco::Logger::get("");
  logger.setChannel(formattingChannel);
  logger.setLevel(Poco::Message::PRIO_DEBUG);
}

// User API.

KopsikUser *kopsik_user_init() {
  KopsikUser *user = new KopsikUser();
  user->ID = 0;
  user->Fullname = 0;
  return user;
}

void kopsik_user_clear(KopsikUser *user) {
  poco_assert(user);
  user->ID = 0;
  if (user->Fullname) {
    free(user->Fullname);
    user->Fullname = 0;
  }
  delete user;
  user = 0;
}

kopsik::Database *get_db(KopsikContext *ctx) {
  poco_assert(ctx);
  poco_assert(ctx->db);
  return reinterpret_cast<kopsik::Database *>(ctx->db);
}

kopsik::User *get_user(KopsikContext *ctx) {
  poco_assert(ctx);
  poco_assert(ctx->user);
  return reinterpret_cast<kopsik::User *>(ctx->user);
}

kopsik_api_result kopsik_current_user(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    KopsikUser *out_user) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(out_user);

  if (!ctx->user) {
    kopsik::Database *db = get_db(ctx);
    kopsik::User *user = new kopsik::User();
    kopsik::error err = db->LoadCurrentUser(user, true);
    if (err != kopsik::noError) {
      delete user;
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
    ctx->user = user;
  }
  poco_assert(ctx->user);
  kopsik::User *user = get_user(ctx);
  if (out_user->Fullname) {
    free(out_user->Fullname);
    out_user->Fullname = 0;
  }
  out_user->Fullname = strdup(user->Fullname().c_str());
  out_user->ID = (unsigned int)user->ID();
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_api_token(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *api_token) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(api_token);
  kopsik::Database *db = get_db(ctx);
  kopsik::error err = db->SetCurrentAPIToken(api_token);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_get_api_token(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    char *str, unsigned int max_strlen) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(str);
  poco_assert(max_strlen);
  kopsik::Database *db = get_db(ctx);
  std::string token("");
  kopsik::error err = db->CurrentAPIToken(&token);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  strncpy(str, token.c_str(), max_strlen);
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_login(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *in_email, const char *in_password) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(in_email);
  poco_assert(in_password);
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
  kopsik_context_user_clear(ctx);
  kopsik::User *user = new kopsik::User();
  kopsik::error err = user->Login(email, password);
  if (err != kopsik::noError) {
    delete user;
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::Database *db = get_db(ctx);
  err = db->SaveUser(user, true);
  if (err != kopsik::noError) {
    delete user;
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  err = db->SetCurrentAPIToken(user->APIToken());
  if (err != kopsik::noError) {
    delete user;
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  ctx->user = user;
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_logout(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  kopsik::Database *db = get_db(ctx);
  kopsik::error err = db->ClearCurrentAPIToken();
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

// Sync

kopsik_api_result save(KopsikContext *ctx,
  char *errmsg, unsigned int errlen) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  kopsik::Database *db = get_db(ctx);
  kopsik::User *user = get_user(ctx);
  kopsik::error err = db->SaveUser(user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_sync(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    int full_sync) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  kopsik::User *user = get_user(ctx);
  kopsik::error err = user->Sync(full_sync);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return save(ctx, errmsg, errlen);
}

kopsik_api_result kopsik_push(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  kopsik::User *user = get_user(ctx);
  kopsik::error err = user->Push();
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return save(ctx, errmsg, errlen);
}

kopsik_api_result kopsik_dirty_models(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    KopsikDirtyModels *out_dirty_models) {
  poco_assert(out_dirty_models);
  kopsik::User *user = get_user(ctx);
  std::vector<kopsik::TimeEntry *> dirty;
  user->CollectDirtyObjects(&dirty);
  out_dirty_models->TimeEntries = 0;
  for (std::vector<kopsik::TimeEntry *>::const_iterator it = dirty.begin();
    it != dirty.end();
    it++) {
    out_dirty_models->TimeEntries++;
  }
  return KOPSIK_API_SUCCESS;
}

// Time entries view API

KopsikTimeEntryViewItem *kopsik_time_entry_view_item_init() {
  KopsikTimeEntryViewItem *item = new KopsikTimeEntryViewItem();
  item->DurationInSeconds = 0;
  item->Description = 0;
  item->Project = 0;
  item->Duration = 0;
  item->Color = 0;
  item->GUID = 0;
  return item;
}

void kopsik_time_entry_view_item_clear(KopsikTimeEntryViewItem *item) {
  poco_assert(item);
  if (item->Description) {
    free(item->Description);
    item->Description = 0;
  }
  if (item->Project) {
    free(item->Project);
    item->Project = 0;
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
  delete item;
  item = 0;
}

void time_entry_to_view_item(
    kopsik::TimeEntry *te,
    kopsik::User *user,
    KopsikTimeEntryViewItem *view_item) {
  poco_assert(te);
  poco_assert(user);
  poco_assert(view_item);
  view_item->DurationInSeconds = static_cast<int>(te->DurationInSeconds());
  if (view_item->Description) {
    free(view_item->Description);
    view_item->Description = 0;
  }
  view_item->Description = strdup(te->Description().c_str());
  if (view_item->GUID) {
    free(view_item->GUID);
    view_item->GUID = 0;
  }
  view_item->GUID = strdup(te->GUID().c_str());
  if (te->PID()) {
    kopsik::Project *p = user->GetProjectByID(te->PID());
    if (p) {
      if (view_item->Project) {
        free(view_item->Project);
        view_item->Project = 0;
      }
      view_item->Project = strdup(p->Name().c_str());

      if (view_item->Color) {
        free(view_item->Color);
        view_item->Color = 0;
      }
      view_item->Color = strdup(p->ColorCode().c_str());
    }
  }
  view_item->Duration = strdup(te->DurationString().c_str());
}

void kopsik_format_duration_in_seconds(
    int duration_in_seconds, char *out_str, unsigned int max_strlen) {
  poco_assert(out_str);
  poco_assert(max_strlen);
  std::string formatted =
    kopsik::Formatter::FormatDurationInSeconds(duration_in_seconds);
  strncpy(out_str, formatted.c_str(), max_strlen);
}

kopsik_api_result kopsik_start(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *in_description,
    KopsikTimeEntryViewItem *out_view_item) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(in_description);
  poco_assert(out_view_item);
  std::string description(in_description);
  if (description.empty()) {
    strncpy(errmsg, "Missing description", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::User *user = get_user(ctx);
  kopsik::TimeEntry *te = user->Start(description);
  kopsik_api_result res = save(ctx, errmsg, errlen);
  if (KOPSIK_API_SUCCESS != res) {
    return res;
  }
  time_entry_to_view_item(te, user, out_view_item);
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_continue(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *in_guid,
    KopsikTimeEntryViewItem *out_view_item) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(in_guid);
  poco_assert(out_view_item);
  std::string GUID(in_guid);
  if (GUID.empty()) {
    strncpy(errmsg, "Missing GUID", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::User *user = get_user(ctx);
  kopsik::TimeEntry *te = user->Continue(GUID);
  kopsik_api_result res = save(ctx, errmsg, errlen);
  if (KOPSIK_API_SUCCESS != res) {
    return res;
  }
  time_entry_to_view_item(te, user, out_view_item);
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_stop(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    KopsikTimeEntryViewItem *out_view_item) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(out_view_item);
  kopsik::User *user = get_user(ctx);
  std::vector<kopsik::TimeEntry *> stopped = user->Stop();
  if (!stopped.empty()) {
    kopsik_api_result res = save(ctx, errmsg, errlen);
    if (res != KOPSIK_API_SUCCESS) {
      return res;
    }
    kopsik::TimeEntry *te = stopped[0];
    time_entry_to_view_item(te, user, out_view_item);
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_running_time_entry_view_item(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    KopsikTimeEntryViewItem *out_item, int *out_is_tracking) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(out_item);
  poco_assert(out_is_tracking);
  *out_is_tracking = 0;
  kopsik::User *user = get_user(ctx);
  kopsik::TimeEntry *te = user->RunningTimeEntry();
  if (te) {
    *out_is_tracking = true;
    time_entry_to_view_item(te, user, out_item);
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
  in_list = 0;
}

kopsik_api_result kopsik_time_entry_view_items(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    KopsikTimeEntryViewItemList *out_list) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(out_list);

  kopsik::User *user = get_user(ctx);
  user->SortTimeEntriesByStart();

  std::vector<kopsik::TimeEntry *>stopped;
  for (std::vector<kopsik::TimeEntry *>::const_iterator it =
      user->TimeEntries.begin(); it != user->TimeEntries.end(); it++) {
    kopsik::TimeEntry *te = *it;
    if (te->DurationInSeconds() >= 0) {
      stopped.push_back(te);
    }
  }

  if (stopped.empty()) {
    return KOPSIK_API_SUCCESS;
  }

  out_list->Length = 0;

  KopsikTimeEntryViewItem *tmp = kopsik_time_entry_view_item_init();
  void *m = malloc(stopped.size() * sizeof(tmp));
  kopsik_time_entry_view_item_clear(tmp);
  poco_assert(m);
  out_list->ViewItems =
    reinterpret_cast<KopsikTimeEntryViewItem **>(m);
  for (unsigned int i = 0; i < stopped.size(); i++) {
    kopsik::TimeEntry *te = stopped[i];
    KopsikTimeEntryViewItem *view_item = kopsik_time_entry_view_item_init();
    time_entry_to_view_item(te, user, view_item);
    out_list->ViewItems[i] = view_item;
    out_list->Length++;
  }
  return KOPSIK_API_SUCCESS;
}

// Websocket client

kopsik_api_result kopsik_listen(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  kopsik::User *user = get_user(ctx);
  kopsik::error err = user->Listen();
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}
