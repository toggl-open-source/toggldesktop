// Copyright 2013 Tanel Lebedev

#ifndef SRC_KOPSIK_API_H_
#define SRC_KOPSIK_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stddef.h>
#include <stdio.h>

#define KOPSIK_EXPORT

#define KOPSIK_ERR_LEN 1024

typedef int kopsik_api_result;
#define KOPSIK_API_SUCCESS 0
#define KOPSIK_API_FAILURE 1

// Context API.

typedef struct {
  char *db_path;
} TogglContext;

KOPSIK_EXPORT TogglContext *kopsik_context_init();
KOPSIK_EXPORT void kopsik_context_clear(TogglContext *in_ctx);

// Configuration API

KOPSIK_EXPORT void kopsik_version(
  int *major, int *minor, int *patch);

KOPSIK_EXPORT void kopsik_set_proxy(TogglContext *in_ctx,
  const char *host, const unsigned int port,
  const char *username, const char *password);

KOPSIK_EXPORT void kopsik_set_db_path(TogglContext *in_ctx,
  const char *in_path);

KOPSIK_EXPORT void kopsik_set_log_path(TogglContext *ctx, const char *path);

// User API

typedef struct {
    unsigned int ID;
    char *Fullname;
} TogglUser;

KOPSIK_EXPORT TogglUser *kopsik_user_new();

KOPSIK_EXPORT void kopsik_user_clear(TogglUser *user);

KOPSIK_EXPORT kopsik_api_result kopsik_current_user(
  TogglContext *in_ctx,
  char *errmsg, unsigned int errlen, TogglUser *out_user);

KOPSIK_EXPORT kopsik_api_result kopsik_set_api_token(
  TogglContext *in_ctx,
  char *errmsg, unsigned int errlen, const char *in_api_token);

KOPSIK_EXPORT kopsik_api_result kopsik_login(
  TogglContext *in_ctx,
  char *errmsg, unsigned int errlen,
  const char *in_email, const char *in_password);

// Sync

typedef struct {
    int TimeEntries;
} TogglDirtyModels;

KOPSIK_EXPORT kopsik_api_result kopsik_sync(
  TogglContext *in_ctx,
  char *errmsg, unsigned int errlen);

KOPSIK_EXPORT kopsik_api_result kopsik_dirty_models(
  TogglContext *in_ctx,
  char *errmsg, unsigned int errlen, TogglDirtyModels *out_dirty_models);

// Time tracking API

typedef struct {
    char *Description;
} TogglTimeEntry;

KOPSIK_EXPORT TogglTimeEntry *kopsik_time_entry_new();

KOPSIK_EXPORT void kopsik_time_entry_clear(TogglTimeEntry *in_time_entry);

KOPSIK_EXPORT kopsik_api_result kopsik_running_time_entry(
  TogglContext *in_ctx,
  char *errmsg, unsigned int errlen,
  TogglTimeEntry *out_time_entry, int *is_tracking);

KOPSIK_EXPORT kopsik_api_result kopsik_start(
  TogglContext *in_ctx,
  char *errmsg, unsigned int errlen, TogglTimeEntry *out_time_entry);

KOPSIK_EXPORT kopsik_api_result kopsik_stop(
  TogglContext *in_ctx,
  char *errmsg, unsigned int errlen, TogglTimeEntry *out_time_entry);

typedef struct {
  TogglTimeEntry **time_entries;
  unsigned int length;
} TogglTimeEntryList;

KOPSIK_EXPORT TogglTimeEntryList *kopsik_time_entry_list_new();

KOPSIK_EXPORT void kopsik_time_entry_list_clear(
  TogglTimeEntryList *in_time_entry_list);

KOPSIK_EXPORT kopsik_api_result kopsik_time_entries(
  TogglContext *in_ctx,
  char *errmsg, unsigned int errlen, TogglTimeEntryList *out_time_entry_list);

#undef KOPSIK_EXPORT

#ifdef __cplusplus
}
#endif

#endif  // SRC_KOPSIK_API_H_
