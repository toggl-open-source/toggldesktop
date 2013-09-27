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
  ctx->db_path = 0;
  return ctx;
}

void kopsik_context_clear(KopsikContext *in_ctx) {
  poco_assert(in_ctx);
  if (in_ctx->db_path) {
    free(in_ctx->db_path);
    in_ctx->db_path = 0;
  }
  delete in_ctx;
  in_ctx = 0;
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
    KopsikContext *in_ctx,
    const char *host, const unsigned int port,
    const char *username, const char *password) {
  poco_assert(in_ctx);
  poco_assert(host);
  poco_assert(username);
  poco_assert(password);
  // FIXME: implement
}

void kopsik_set_db_path(KopsikContext *in_ctx, const char *path) {
  poco_assert(in_ctx);
  poco_assert(path);
  if (in_ctx->db_path) {
    free(in_ctx->db_path);
    in_ctx->db_path = 0;
  }
  in_ctx->db_path = strdup(path);
}

void kopsik_set_log_path(KopsikContext *in_ctx, const char *path) {
  poco_assert(in_ctx);
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
  if (user->Fullname) {
    free(user->Fullname);
    user->Fullname = 0;
  }
  delete user;
  user = 0;
}

kopsik_api_result kopsik_current_user(
    KopsikContext *in_ctx,
    char *errmsg, unsigned int errlen,
    KopsikUser *out_user) {
  poco_assert(in_ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(out_user);
  poco_assert(in_ctx->db_path);
  kopsik::Database db(in_ctx->db_path);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }

  if (out_user->Fullname) {
    free(out_user->Fullname);
    out_user->Fullname = 0;
  }
  out_user->Fullname = strdup(user.Fullname().c_str());

  out_user->ID = (unsigned int)user.ID();
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_api_token(
    KopsikContext *in_ctx,
    char *errmsg, unsigned int errlen,
    const char *in_api_token) {
  poco_assert(in_ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(in_api_token);
  std::string api_token(in_api_token);
  if (api_token.empty()) {
    strncpy(errmsg, "Emtpy API token", errlen);
    return KOPSIK_API_FAILURE;
  }
  poco_assert(in_ctx->db_path);
  kopsik::Database db(in_ctx->db_path);
  kopsik::error err = db.SetCurrentAPIToken(api_token);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_login(
    KopsikContext *in_ctx,
    char *errmsg, unsigned int errlen,
    const char *in_email, const char *in_password) {
  poco_assert(in_ctx);
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
  kopsik::User user;
  kopsik::error err = user.Login(email, password);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  poco_assert(in_ctx->db_path);
  kopsik::Database db(in_ctx->db_path);
  err = db.SaveUser(&user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  err = db.SetCurrentAPIToken(user.APIToken());
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_logout(
    KopsikContext *in_ctx,
    char *errmsg, unsigned int errlen) {
  poco_assert(in_ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(in_ctx->db_path);
  kopsik::Database db(in_ctx->db_path);
  kopsik::error err = db.ClearCurrentAPIToken();
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

// Sync

kopsik_api_result kopsik_sync(
    KopsikContext *in_ctx,
    char *errmsg, unsigned int errlen,
    int fetch_updates_only) {
  poco_assert(in_ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(in_ctx->db_path);
  kopsik::Database db(in_ctx->db_path);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  err = user.Sync();
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  err = db.SaveUser(&user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_dirty_models(
    KopsikContext *in_ctx,
    char *errmsg, unsigned int errlen,
    KopsikDirtyModels *out_dirty_models) {
  poco_assert(in_ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(out_dirty_models);
  poco_assert(in_ctx->db_path);
  kopsik::Database db(in_ctx->db_path);

  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  std::vector<kopsik::TimeEntry *> dirty;
  user.CollectDirtyObjects(&dirty);
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
  delete item;
  item = 0;
}

void kopsik_time_entry_to_toggl_time_entry_view_item_struct(
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
    KopsikContext *in_ctx,
    char *errmsg, unsigned int errlen,
    const char *in_description,
    KopsikTimeEntryViewItem *out_view_item) {
  poco_assert(in_ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(in_description);
  poco_assert(out_view_item);
  poco_assert(in_ctx->db_path);
  std::string description(in_description);
  if (description.empty()) {
    strncpy(errmsg, "Missing description", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::Database db(in_ctx->db_path);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::TimeEntry *te = user.Start(description);
  if (te) {
    err = db.SaveUser(&user, true);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
    kopsik_time_entry_to_toggl_time_entry_view_item_struct(
      te, &user, out_view_item);
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_stop(
    KopsikContext *in_ctx,
    char *errmsg, unsigned int errlen,
    KopsikTimeEntryViewItem *out_view_item) {
  poco_assert(in_ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(out_view_item);
  poco_assert(in_ctx->db_path);
  kopsik::Database db(in_ctx->db_path);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  std::vector<kopsik::TimeEntry *> stopped = user.Stop();
  if (!stopped.empty()) {
    err = db.SaveUser(&user, true);
    if (err != kopsik::noError) {
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
    kopsik::TimeEntry *te = stopped[0];
    kopsik_time_entry_to_toggl_time_entry_view_item_struct(
      te, &user, out_view_item);
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_running_time_entry_view_item(
    KopsikContext *in_ctx,
    char *errmsg, unsigned int errlen,
    KopsikTimeEntryViewItem *out_item, int *out_is_tracking) {
  poco_assert(in_ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(out_item);
  poco_assert(out_is_tracking);
  *out_is_tracking = 0;
  poco_assert(in_ctx->db_path);
  kopsik::Database db(in_ctx->db_path);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::TimeEntry *te = user.RunningTimeEntry();
  if (te) {
    *out_is_tracking = true;
    kopsik_time_entry_to_toggl_time_entry_view_item_struct(
      te, &user, out_item);
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
    KopsikContext *in_ctx,
    char *errmsg, unsigned int errlen,
    KopsikTimeEntryViewItemList *out_list) {
  poco_assert(in_ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(out_list);
  poco_assert(in_ctx->db_path);

  kopsik::Database db(in_ctx->db_path);
  kopsik::User user;

  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }

  user.SortTimeEntriesByStart();

  std::vector<kopsik::TimeEntry *>stopped;
  for (std::vector<kopsik::TimeEntry *>::const_iterator it =
      user.TimeEntries.begin(); it != user.TimeEntries.end(); it++) {
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
    kopsik_time_entry_to_toggl_time_entry_view_item_struct(
      te, &user, view_item);
    out_list->ViewItems[i] = view_item;
    out_list->Length++;
  }
  return KOPSIK_API_SUCCESS;
}

// Websocket client

kopsik_api_result kopsik_listen(
    KopsikContext *in_ctx,
    char *errmsg, unsigned int errlen) {
  poco_assert(in_ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  kopsik::Database db(in_ctx->db_path);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  err = user.Listen();
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}
