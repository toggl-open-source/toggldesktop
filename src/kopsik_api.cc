// Copyright 2013 Tanel Lebedev

#include <cstring>

#include "./kopsik_api.h"
#include "./database.h"
#include "./toggl_api_client.h"

#define DBNAME "kopsik.db"

void time_entry_to_struct(kopsik::TimeEntry *in, TogglTimeEntry *out_te) {
  if (out_te->Description) {
    free(out_te->Description);
    out_te->Description = 0;
  }
  out_te->Description = strdup(in->Description().c_str());
}

void kopsik_version(int *major, int *minor, int *patch) {
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
  if (user->Fullname) {
    free(user->Fullname);
    user->Fullname = 0;
  }
  delete user;
  user = 0;
}

void kopsik_user_set_fullname(TogglUser *user, const char *fullname) {
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
  if (!out_user) {
    strncpy(errmsg, "Invalid user pointer", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik::Database db(DBNAME);
  kopsik::User user;
  kopsik::error err = db.LoadCurrentUser(&user, true);
  if (err != kopsik::noError) {
    strncpy(errmsg, "Please log in", errlen);
    return KOPSIK_API_FAILURE;
  }
  kopsik_user_set_fullname(out_user, user.Fullname().c_str());
  return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_api_token(char *errmsg, unsigned int errlen,
  const char *in_api_token) {
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
  // FIXME: implement
}

void kopsik_set_log_path(const char *path) {
  // FIXME: implement
}

void kopsik_set_db_path(const char *path) {
  // FIXME: implement
}
