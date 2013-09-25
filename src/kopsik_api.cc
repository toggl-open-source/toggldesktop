// Copyright 2013 Tanel Lebedev

#include <cstring>

#include "./kopsik_api.h"
#include "./database.h"
#include "./toggl_api_client.h"

#define DBNAME "kopsik.db"

#define assert(thing) {}

TogglContext *kopsik_init() {
  TogglContext *ctx = new TogglContext();
  ctx->app_path = 0;
  return ctx;
}

void kopsik_delete(TogglContext *in_ctx) {
  assert(in_ctx);
  if (in_ctx->app_path) {
    free(in_ctx->app_path);
    in_ctx->app_path = 0;
  }
  delete in_ctx;
  in_ctx = 0;
}

void time_entry_to_struct(kopsik::TimeEntry *in, TogglTimeEntry *out_te) {
  assert(in);
  assert(out_te);
  if (out_te->Description) {
    free(out_te->Description);
    out_te->Description = 0;
  }
  out_te->Description = strdup(in->Description().c_str());
}

void kopsik_version(int *major, int *minor, int *patch) {
  assert(major);
  assert(minor);
  assert(patch);
  *major = 0;
  *minor = 1;
  *patch = 0;
}

TogglUser *kopsik_user_new() {
  TogglUser *user = new TogglUser();
  user->ID = 0;
  user->Fullname = 0;
  return user;
}

void kopsik_user_delete(TogglUser *user) {
  assert(user);
  if (user->Fullname) {
    free(user->Fullname);
    user->Fullname = 0;
  }
  delete user;
  user = 0;
}

void kopsik_user_set_fullname(TogglUser *user, const char *fullname) {
  assert(user);
  assert(fullname);
  if (user->Fullname) {
    free(user->Fullname);
    user->Fullname = 0;
  }
  user->Fullname = strdup(fullname);
}

TogglTimeEntry *kopsik_time_entry_new() {
  TogglTimeEntry *te = new TogglTimeEntry();
  te->Description = 0;
  return te;
}

void kopsik_time_entry_delete(TogglTimeEntry *te) {
  assert(te);
  if (te->Description) {
    free(te->Description);
    te->Description = 0;
  }
  delete te;
  te = 0;
}

// FIXME: write tests for API

kopsik_api_result kopsik_current_user(char *errmsg, unsigned int errlen,
    TogglUser *out_user) {
  assert(errmsg);
  assert(errlen);
  assert(out_user);
  if (!out_user) {
    strncpy(errmsg, "Invalid user pointer", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::Database db(DBNAME);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    err.copy(errmsg, errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik_user_set_fullname(out_user, user.Fullname().c_str());
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_api_token(char *errmsg, unsigned int errlen,
    const char *in_api_token) {
  assert(errmsg);
  assert(errlen);
  assert(in_api_token);
  if (!in_api_token) {
    strncpy(errmsg, "Invalid api_token pointer", errlen);
    return KOPSIK_API_FAILURE;
  }
  std::string api_token(in_api_token);
  if (api_token.empty()) {
    strncpy(errmsg, "Emtpy API token", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::Database db(DBNAME);
  kopsik::error err = db.SetCurrentAPIToken(api_token);
  if (err != kopsik::noError) {
    err.copy(errmsg, errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_login(
  char *errmsg, unsigned int errlen,
  const char *in_email, const char *in_password) {
  assert(errmsg);
  assert(errlen);
  assert(in_email);
  assert(in_password);
  if (!in_email) {
    strncpy(errmsg, "Invalid email pointer", errlen);
    return KOPSIK_API_FAILURE;
  }
  if (!in_password) {
    strncpy(errmsg, "Invalid password pointer", errlen);
    return KOPSIK_API_FAILURE;
  }
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
    err.copy(errmsg, errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::Database db(DBNAME);
  err = db.SaveUser(&user, true);
  if (err != kopsik::noError) {
    err.copy(errmsg, errlen);
    return KOPSIK_API_FAILURE;
  }
  err = db.SetCurrentAPIToken(user.APIToken());
  if (err != kopsik::noError) {
    err.copy(errmsg, errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_sync(char *errmsg, unsigned int errlen) {
  assert(errmsg);
  assert(errlen);
  kopsik::Database db(DBNAME);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    err.copy(errmsg, errlen);
    return KOPSIK_API_FAILURE;
  }
  err = user.Sync();
  if (err != kopsik::noError) {
    err.copy(errmsg, errlen);
    return KOPSIK_API_FAILURE;
  }
  err = db.SaveUser(&user, true);
  if (err != kopsik::noError) {
    err.copy(errmsg, errlen);
    return KOPSIK_API_FAILURE;
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_running_time_entry(char *errmsg, unsigned int errlen,
    TogglTimeEntry *out_time_entry, int *is_tracking) {
  assert(errmsg);
  assert(errlen);
  assert(out_time_entry);
  assert(is_tracking);
  if (!out_time_entry) {
    strncpy(errmsg, "Invalid time entry pointer", errlen);
    return KOPSIK_API_FAILURE;
  }
  if (!is_tracking) {
    strncpy(errmsg, "Invalid is_tracking pointer", errlen);
    return KOPSIK_API_FAILURE;
  }
  *is_tracking = 0;
  kopsik::Database db(DBNAME);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    err.copy(errmsg, errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::TimeEntry *te = user.RunningTimeEntry();
  if (te) {
    *is_tracking = true;
    time_entry_to_struct(te, out_time_entry);
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_dirty_models(char *errmsg, unsigned int errlen,
    TogglDirtyModels *out_dirty_models) {
  assert(errmsg);
  assert(errlen);
  assert(out_dirty_models);
  if (!out_dirty_models) {
    strncpy(errmsg, "Invalid dirty models pointer", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::Database db(DBNAME);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    err.copy(errmsg, errlen);
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

kopsik_api_result kopsik_start(char *errmsg, unsigned int errlen,
    TogglTimeEntry *out_time_entry) {
  assert(errmsg);
  assert(errlen);
  assert(out_time_entry);
  if (!out_time_entry) {
    strncpy(errmsg, "Invalid time entry pointer", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::Database db(DBNAME);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    err.copy(errmsg, errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::TimeEntry *te = user.Start();
  if (te) {
    err = db.SaveUser(&user, true);
    if (err != kopsik::noError) {
      err.copy(errmsg, errlen);
      return KOPSIK_API_FAILURE;
    }
    time_entry_to_struct(te, out_time_entry);
  }
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_stop(char *errmsg, unsigned int errlen,
    TogglTimeEntry *out_time_entry) {
  assert(errmsg);
  assert(errlen);
  assert(out_time_entry);
  if (!out_time_entry) {
    strncpy(errmsg, "Invalid time entry pointer", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::Database db(DBNAME);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    err.copy(errmsg, errlen);
    return KOPSIK_API_FAILURE;
  }
  std::vector<kopsik::TimeEntry *> stopped = user.Stop();
  if (!stopped.empty()) {
    err = db.SaveUser(&user, true);
    if (err != kopsik::noError) {
      err.copy(errmsg, errlen);
      return KOPSIK_API_FAILURE;
    }
    kopsik::TimeEntry *te = stopped[0];
    time_entry_to_struct(te, out_time_entry);
  }
  return KOPSIK_API_SUCCESS;
}

void kopsik_set_proxy(
    const char *host, const unsigned int port,
    const char *username, const char *password) {
  assert(host);
  assert(username);
  assert(password);
  // FIXME: implement
}

void kopsik_set_app_path(const char *path) {
  assert(path);
  // FIXME: implement
}

TogglTimeEntryList *kopsik_time_entry_list_new() {
  TogglTimeEntryList *result = new TogglTimeEntryList();
  result->length = 0;
  result->time_entries = 0;
  return result;
}

kopsik_api_result kopsik_time_entries(
    char *errmsg, unsigned int errlen,
    TogglTimeEntryList *out_time_entry_list) {
  assert(errmsg);
  assert(errlen);
  assert(out_time_entry_list);
  if (!out_time_entry_list) {
    strncpy(errmsg, "Invalid time entry list pointer", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::Database db(DBNAME);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    err.copy(errmsg, errlen);
    return KOPSIK_API_FAILURE;
  }
  out_time_entry_list->length = user.TimeEntries.size();
  if (!out_time_entry_list->length) {
    return KOPSIK_API_SUCCESS;
  }
  TogglTimeEntry *te = kopsik_time_entry_new();
  void *m = malloc(out_time_entry_list->length * sizeof(te));
  kopsik_time_entry_delete(te);
  assert(m);
  out_time_entry_list->time_entries = reinterpret_cast<TogglTimeEntry **>(m);
  for (unsigned int i = 0; i < user.TimeEntries.size(); i++) {
    kopsik::TimeEntry *te = user.TimeEntries[i];
    TogglTimeEntry *item = kopsik_time_entry_new();
    time_entry_to_struct(te, item);
    out_time_entry_list->time_entries[i] = item;
  }
  return KOPSIK_API_SUCCESS;
}

void kopsik_time_entry_list_delete(TogglTimeEntryList *in_time_entry_list) {
  assert(in_time_entry_list);
  for (unsigned int i = 0; i < in_time_entry_list->length; i++) {
    delete in_time_entry_list->time_entries[i];
    in_time_entry_list->time_entries[i] = 0;
  }
  if (in_time_entry_list->time_entries) {
    free(in_time_entry_list->time_entries);
  }
  delete in_time_entry_list;
  in_time_entry_list = 0;
}
