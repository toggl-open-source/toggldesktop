// Copyright 2013 Tanel Lebedev

#include <cstring>

#include "./kopsik_api.h"
#include "./database.h"
#include "./toggl_api_client.h"
#include "./https_client.h"
#include "./version.h"

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
#include "Poco/Task.h"

#define KOPSIK_API_FAILURE 1

// Private helpers

kopsik::Database *get_db(KopsikContext *ctx) {
  poco_assert(ctx);
  poco_assert(ctx->db);
  return reinterpret_cast<kopsik::Database *>(ctx->db);
}

kopsik_api_result save(KopsikContext *ctx,
  char *errmsg, unsigned int errlen) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  kopsik::Database *db = get_db(ctx);
  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
  kopsik::error err = db->SaveUser(user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
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

  if (view_item->Duration) {
    free(view_item->Duration);
    view_item->Duration = 0;
  }
  view_item->Duration = strdup(te->DurationString().c_str());

  view_item->Started = static_cast<unsigned int>(te->Start());
  view_item->Ended = static_cast<unsigned int>(te->Stop());
  if (te->Billable()) {
    view_item->Billable = 1;
  } else {
    view_item->Billable = 0;
  }

  if (view_item->Tags) {
    free(view_item->Tags);
    view_item->Tags = 0;
  }
  if (!te->Tags().empty()) {
    view_item->Tags = strdup(te->Tags().c_str());
  }
}

// Context API.

KopsikContext *kopsik_context_init() {
  KopsikContext *ctx = new KopsikContext();
  ctx->db = 0;
  ctx->current_user = 0;
  ctx->https_client = new kopsik::HTTPSClient();
  ctx->mutex = new Poco::Mutex();
  ctx->tm = new Poco::TaskManager();
  return ctx;
}

void kopsik_context_clear(KopsikContext *ctx) {
  poco_assert(ctx);

  if (ctx->tm) {
    Poco::TaskManager *tm =
        reinterpret_cast<Poco::TaskManager *>(ctx->tm);
    tm->joinAll();
    delete tm;
    ctx->tm = 0;
  }
  if (ctx->db) {
    kopsik::Database *db = reinterpret_cast<kopsik::Database *>(ctx->db);
    delete db;
    ctx->db = 0;
  }
  if (ctx->current_user) {
    kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
    delete user;
    ctx->current_user = 0;
  }
  if (ctx->https_client) {
    kopsik::HTTPSClient *https_client =
      reinterpret_cast<kopsik::HTTPSClient *>(ctx->https_client);
    delete https_client;
    ctx->https_client = 0;
  }
  if (ctx->mutex) {
    Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
    delete mutex;
    ctx->mutex = 0;
  }
  delete ctx;
  ctx = 0;
}

// Configuration API.

void kopsik_version(int *major, int *minor, int *patch) {
  poco_assert(major);
  poco_assert(minor);
  poco_assert(patch);
  *major = kopsik::version::Major;
  *minor = kopsik::version::Minor;
  *patch = kopsik::version::Patch;
}

void kopsik_set_proxy(
    KopsikContext *ctx,
    const char *host, const unsigned int port,
    const char *username, const char *password) {
  poco_assert(ctx);
  poco_assert(host);
  poco_assert(username);
  poco_assert(password);

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  // FIXME: implement
}

void kopsik_set_db_path(KopsikContext *ctx, const char *path) {
  poco_assert(ctx);
  poco_assert(path);

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  if (ctx->db) {
    kopsik::Database *db = reinterpret_cast<kopsik::Database *>(ctx->db);
    delete db;
    ctx->db = 0;
  }
  ctx->db = new kopsik::Database(path);
}

void kopsik_set_log_path(KopsikContext *ctx, const char *path) {
  poco_assert(ctx);
  poco_assert(path);

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

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

kopsik_api_result kopsik_current_user(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    KopsikUser *out_user) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(out_user);

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  if (!ctx->current_user) {
    kopsik::Database *db = get_db(ctx);
    kopsik::User *user = new kopsik::User();
    kopsik::error err = db->LoadCurrentUser(user, true);
    if (err != kopsik::noError) {
      delete user;
      strncpy(errmsg, err.c_str(), errlen);
      return KOPSIK_API_FAILURE;
    }
    ctx->current_user = user;
  }
  poco_assert(ctx->current_user);
  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
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

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

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

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

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

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

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
  if (ctx->current_user) {
    kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
    delete user;
    ctx->current_user = 0;
  }
  kopsik::User *user = new kopsik::User();
  kopsik::HTTPSClient *https_client =
    reinterpret_cast<kopsik::HTTPSClient *>(ctx->https_client);
  kopsik::error err = user->Login(https_client, email, password);
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
  ctx->current_user = user;
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_logout(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  kopsik::Database *db = get_db(ctx);
  kopsik::error err = db->ClearCurrentAPIToken();
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

// Sync

kopsik_api_result kopsik_sync(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    int full_sync) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);

  if (!ctx->current_user) {
    strncpy(errmsg, "Please login first", errlen);
    return KOPSIK_API_FAILURE;
  }

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
  kopsik::HTTPSClient *https_client =
    reinterpret_cast<kopsik::HTTPSClient *>(ctx->https_client);
  kopsik::error err = user->Sync(https_client, full_sync);
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

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  if (!ctx->current_user) {
    strncpy(errmsg, "Please login first", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
  kopsik::HTTPSClient *https_client =
    reinterpret_cast<kopsik::HTTPSClient *>(ctx->https_client);
  kopsik::error err = user->Push(https_client);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return save(ctx, errmsg, errlen);
}

kopsik_api_result kopsik_pushable_models(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    KopsikPushableModelStats *stats) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(stats);

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  if (!ctx->current_user) {
    strncpy(errmsg, "Please login first", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
  std::vector<kopsik::TimeEntry *> pushable;
  user->CollectPushableObjects(&pushable);
  stats->TimeEntries = 0;
  for (std::vector<kopsik::TimeEntry *>::const_iterator it = pushable.begin();
    it != pushable.end();
    it++) {
    stats->TimeEntries++;
  }
  return KOPSIK_API_SUCCESS;
}

// Async API

class SyncTask : public Poco::Task {
  public:
    SyncTask(KopsikContext *ctx,
      int full_sync,
      kopsik_callback callback) : Task("sync"),
      ctx_(ctx),
      full_sync_(full_sync),
      callback_(callback) {}
    void runTask() {
      char err[KOPSIK_ERR_LEN];
      kopsik_api_result res = kopsik_sync(
        ctx_, err, KOPSIK_ERR_LEN, full_sync_);
      char *result_str = 0;
      unsigned int result_len = 0;
      if (res != KOPSIK_API_SUCCESS) {
        result_str = strdup(err);
        result_len = static_cast<int>(strlen(err));
      }
      callback_(res, result_str, result_len);
    }
  private:
    KopsikContext *ctx_;
    int full_sync_;
    kopsik_callback callback_;
};

void kopsik_sync_async(
    KopsikContext *ctx,
    int full_sync,
    kopsik_callback callback) {
  poco_assert(ctx);
  poco_assert(callback);
  Poco::TaskManager *tm = reinterpret_cast<Poco::TaskManager *>(ctx->tm);
  tm->start(new SyncTask(ctx, full_sync, callback));
}

class PushTask : public Poco::Task {
  public:
    PushTask(KopsikContext *ctx,
      kopsik_callback callback) : Task("push"),
      ctx_(ctx), callback_(callback) {}
    void runTask() {
      char err[KOPSIK_ERR_LEN];
      kopsik_api_result res = kopsik_push(ctx_, err, KOPSIK_ERR_LEN);
      char *result_str = 0;
      unsigned int result_len = 0;
      if (res != KOPSIK_API_SUCCESS) {
        result_str = strdup(err);
        result_len = static_cast<int>(strlen(err));
      }
      callback_(res, result_str, result_len);
    }
  private:
    KopsikContext *ctx_;
    kopsik_callback callback_;
};

void kopsik_push_async(
    KopsikContext *ctx,
    kopsik_callback callback) {
  poco_assert(ctx);
  poco_assert(callback);

  Poco::TaskManager *tm = reinterpret_cast<Poco::TaskManager *>(ctx->tm);
  tm->start(new PushTask(ctx, callback));
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
  item->Billable = 0;
  item->Tags = 0;
  item->Started = 0;
  item->Ended = 0;
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
  if (item->Tags) {
    free(item->Tags);
    item->Tags = 0;
  }
  delete item;
  item = 0;
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

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  std::string description(in_description);
  if (description.empty()) {
    strncpy(errmsg, "Missing description", errlen);
    return KOPSIK_API_FAILURE;
  }
  if (!ctx->current_user) {
    strncpy(errmsg, "Please login first", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
  kopsik::TimeEntry *te = user->Start(description);
  kopsik_api_result res = save(ctx, errmsg, errlen);
  if (KOPSIK_API_SUCCESS != res) {
    return res;
  }
  time_entry_to_view_item(te, user, out_view_item);
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_time_entry_view_item_by_guid(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *guid,
    KopsikTimeEntryViewItem *view_item,
    int *was_found) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(guid);
  poco_assert(view_item);
  poco_assert(was_found);

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  std::string GUID(guid);
  if (GUID.empty()) {
    strncpy(errmsg, "Missing GUID", errlen);
    return KOPSIK_API_FAILURE;
  }
  if (!ctx->current_user) {
    strncpy(errmsg, "Please login first", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
  kopsik::TimeEntry *te = user->GetTimeEntryByGUID(GUID);
  if (te) {
    *was_found = 1;
    time_entry_to_view_item(te, user, view_item);
  } else {
    *was_found = 0;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_continue(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *guid,
    KopsikTimeEntryViewItem *view_item) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(guid);
  poco_assert(view_item);

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  std::string GUID(guid);
  if (GUID.empty()) {
    strncpy(errmsg, "Missing GUID", errlen);
    return KOPSIK_API_FAILURE;
  }
  if (!ctx->current_user) {
    strncpy(errmsg, "Please login first", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
  kopsik::TimeEntry *te = user->Continue(GUID);
  poco_assert(te);
  kopsik_api_result res = save(ctx, errmsg, errlen);
  if (KOPSIK_API_SUCCESS != res) {
    return res;
  }
  time_entry_to_view_item(te, user, view_item);
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_delete_time_entry(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *guid) {
  poco_assert(ctx);
  poco_assert(errmsg);
  poco_assert(errlen);
  poco_assert(guid);

  std::string GUID(guid);
  if (GUID.empty()) {
    strncpy(errmsg, "Missing GUID", errlen);
    return KOPSIK_API_FAILURE;
  }
  if (!ctx->current_user) {
    strncpy(errmsg, "Please login first", errlen);
    return KOPSIK_API_FAILURE;
  }

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
  user->DeleteTimeEntry(GUID);
  return save(ctx, errmsg, errlen);
}

kopsik_api_result kopsik_set_time_entry_duration(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *guid,
    const char *value) {
  // FIXME:
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_project(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *guid,
    const char *value) {
  // FIXME:
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_start_time(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *guid,
    const char *value) {
  // FIXME:
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_end_time(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *guid,
    const char *value) {
  // FIXME:
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_start_date(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *guid,
    const char *value) {
  // FIXME:
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_tags(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *guid,
    const char *value) {
  // FIXME:
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_billable(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *guid,
    int value) {
  // FIXME:
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_description(
    KopsikContext *ctx,
    char *errmsg, unsigned int errlen,
    const char *guid,
    const char *value) {
  // FIXME:
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

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  if (!ctx->current_user) {
    strncpy(errmsg, "Please login first", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
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

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  *out_is_tracking = 0;
  if (!ctx->current_user) {
    strncpy(errmsg, "Please login first", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
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

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  if (!ctx->current_user) {
    strncpy(errmsg, "Please login first", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
  user->SortTimeEntriesByStart();

  std::vector<kopsik::TimeEntry *>visible;
  for (std::vector<kopsik::TimeEntry *>::const_iterator it =
      user->related.TimeEntries.begin();
      it != user->related.TimeEntries.end(); it++) {
    kopsik::TimeEntry *te = *it;
    poco_assert(!te->GUID().empty());
    if (te->DurationInSeconds() < 0) {
      continue;
    }
    if (te->DeletedAt() > 0) {
      continue;
    }
    visible.push_back(te);
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

  Poco::Mutex *mutex = reinterpret_cast<Poco::Mutex *>(ctx->mutex);
  Poco::Mutex::ScopedLock lock(*mutex);

  if (!ctx->current_user) {
    strncpy(errmsg, "Please login first", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::User *user = reinterpret_cast<kopsik::User *>(ctx->current_user);
  kopsik::HTTPSClient *https_client =
    reinterpret_cast<kopsik::HTTPSClient *>(ctx->https_client);
  kopsik::error err = user->ListenToWebsocket(https_client);
  if (err != kopsik::noError) {
    strncpy(errmsg, err.c_str(), errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}
