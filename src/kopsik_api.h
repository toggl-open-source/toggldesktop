// Copyright 2013 Tanel Lebedev

#ifndef SRC_KOPSIK_API_H_
#define SRC_KOPSIK_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdio.h>

#define KOPSIK_EXPORT

#define KOPSIK_ERR_LEN 1024

typedef int kopsik_api_result;
#define KOPSIK_API_SUCCESS 0
#define KOPSIK_API_FAILURE 1

// Context API.

typedef struct {
  void *db;
  void *current_user;
  void *https_client;
  void *mutex;
  void *tm;
} KopsikContext;

KOPSIK_EXPORT KopsikContext *kopsik_context_init();

KOPSIK_EXPORT void kopsik_context_clear(KopsikContext *ctx);

// Configuration API

KOPSIK_EXPORT void kopsik_version(
  int *major, int *minor, int *patch);

KOPSIK_EXPORT void kopsik_set_proxy(KopsikContext *ctx,
  const char *host, const unsigned int port,
  const char *username, const char *password);

KOPSIK_EXPORT void kopsik_set_db_path(KopsikContext *ctx,
  const char *path);

KOPSIK_EXPORT void kopsik_set_log_path(KopsikContext *ctx,
  const char *path);

// User API

typedef struct {
    unsigned int ID;
    char *Fullname;
} KopsikUser;

KOPSIK_EXPORT KopsikUser *kopsik_user_init();

KOPSIK_EXPORT void kopsik_user_clear(KopsikUser *user);

KOPSIK_EXPORT kopsik_api_result kopsik_set_api_token(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen,
  const char *api_token);

KOPSIK_EXPORT kopsik_api_result kopsik_get_api_token(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen,
  char *str, unsigned int max_strlen);

KOPSIK_EXPORT kopsik_api_result kopsik_current_user(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen,
  KopsikUser *user);

KOPSIK_EXPORT kopsik_api_result kopsik_login(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen,
  const char *email, const char *password);

KOPSIK_EXPORT kopsik_api_result kopsik_logout(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen);

// Sync

typedef struct {
  unsigned int TimeEntries;
} KopsikDirtyModels;

KOPSIK_EXPORT kopsik_api_result kopsik_sync(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen,
  int full_sync);

KOPSIK_EXPORT kopsik_api_result kopsik_push(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen);

KOPSIK_EXPORT kopsik_api_result kopsik_pushable_models(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen,
  KopsikDirtyModels *dirty_models);

// Async API

typedef void (*kopsik_callback)(
  kopsik_api_result result,
  // NB! you need to free() the memory yourself
  char *errmsg,
  // Length of the returned error string.
  unsigned int errlen);

KOPSIK_EXPORT void kopsik_sync_async(
  KopsikContext *ctx,
  int full_sync,
  kopsik_callback callback);

KOPSIK_EXPORT void kopsik_push_async(
  KopsikContext *ctx,
  kopsik_callback callback);

// Time entries view

typedef struct {
  int DurationInSeconds;
  char *Description;
  char *Project;
  char *Duration;
  char *Color;
  char *GUID;
  int Billable;
  char *Tags;
  time_t Started;
  time_t Ended;
} KopsikTimeEntryViewItem;

typedef struct {
  KopsikTimeEntryViewItem **ViewItems;
  unsigned int Length;
} KopsikTimeEntryViewItemList;

KOPSIK_EXPORT KopsikTimeEntryViewItem *
  kopsik_time_entry_view_item_init();

KOPSIK_EXPORT void kopsik_time_entry_view_item_clear(
  KopsikTimeEntryViewItem *item);

KOPSIK_EXPORT kopsik_api_result kopsik_running_time_entry_view_item(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen,
  KopsikTimeEntryViewItem *item,
  int *is_tracking);

KOPSIK_EXPORT void kopsik_format_duration_in_seconds(
  int duration_in_seconds, char *str, unsigned int max_strlen);

KOPSIK_EXPORT kopsik_api_result kopsik_start(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen,
  const char *description,
  KopsikTimeEntryViewItem *item);

KOPSIK_EXPORT kopsik_api_result kopsik_time_entry_view_item_by_guid(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen,
  const char *guid,
  KopsikTimeEntryViewItem *item,
  int *was_found);

KOPSIK_EXPORT kopsik_api_result kopsik_continue(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen,
  const char *guid,
  KopsikTimeEntryViewItem *item);

KOPSIK_EXPORT kopsik_api_result kopsik_stop(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen,
  KopsikTimeEntryViewItem *item);

KOPSIK_EXPORT KopsikTimeEntryViewItemList *
  kopsik_time_entry_view_item_list_init();

KOPSIK_EXPORT void kopsik_time_entry_view_item_list_clear(
  KopsikTimeEntryViewItemList *list);

KOPSIK_EXPORT kopsik_api_result kopsik_time_entry_view_items(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen,
  KopsikTimeEntryViewItemList *list);

// Websocket client

KOPSIK_EXPORT kopsik_api_result kopsik_listen(
  KopsikContext *ctx,
  char *errmsg, unsigned int errlen);

#undef KOPSIK_EXPORT

#ifdef __cplusplus
}
#endif

#endif  // SRC_KOPSIK_API_H_
