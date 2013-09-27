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
} KopsikContext;

KOPSIK_EXPORT KopsikContext *kopsik_context_init();

KOPSIK_EXPORT void kopsik_context_clear(KopsikContext *in_ctx);

// Configuration API

KOPSIK_EXPORT void kopsik_version(
  int *major, int *minor, int *patch);

KOPSIK_EXPORT void kopsik_set_proxy(KopsikContext *in_ctx,
  const char *host, const unsigned int port,
  const char *username, const char *password);

KOPSIK_EXPORT void kopsik_set_db_path(KopsikContext *in_ctx,
  const char *in_path);

KOPSIK_EXPORT void kopsik_set_log_path(KopsikContext *in_ctx, const char *path);

// User API

typedef struct {
    unsigned int ID;
    char *Fullname;
} KopsikUser;

KOPSIK_EXPORT KopsikUser *kopsik_user_init();

KOPSIK_EXPORT void kopsik_user_clear(KopsikUser *user);

KOPSIK_EXPORT kopsik_api_result kopsik_current_user(
  KopsikContext *in_ctx,
  char *errmsg, unsigned int errlen, KopsikUser *out_user);

KOPSIK_EXPORT kopsik_api_result kopsik_set_api_token(
  KopsikContext *in_ctx,
  char *errmsg, unsigned int errlen, const char *in_api_token);

KOPSIK_EXPORT kopsik_api_result kopsik_login(
  KopsikContext *in_ctx,
  char *errmsg, unsigned int errlen,
  const char *in_email, const char *in_password);

KOPSIK_EXPORT kopsik_api_result kopsik_logout(
  KopsikContext *in_ctx,
  char *errmsg, unsigned int errlen);

// Sync

typedef struct {
  int TimeEntries;
} KopsikDirtyModels;

KOPSIK_EXPORT kopsik_api_result kopsik_sync(
  KopsikContext *in_ctx,
  char *errmsg, unsigned int errlen,
  int fetch_updates_only);

KOPSIK_EXPORT kopsik_api_result kopsik_dirty_models(
  KopsikContext *in_ctx,
  char *errmsg, unsigned int errlen, KopsikDirtyModels *out_dirty_models);

// Time entries view

typedef struct {
  int DurationInSeconds;
  char *Description;
  char *Project;
  char *Duration;
  char *Color;
} KopsikTimeEntryViewItem;

typedef struct {
  KopsikTimeEntryViewItem **ViewItems;
  unsigned int Length;
} KopsikTimeEntryViewItemList;

KOPSIK_EXPORT KopsikTimeEntryViewItem *
  kopsik_time_entry_view_item_init();

KOPSIK_EXPORT void kopsik_time_entry_view_item_clear(
  KopsikTimeEntryViewItem *in_item);

KOPSIK_EXPORT kopsik_api_result kopsik_running_time_entry_view_item(
  KopsikContext *in_ctx,
  char *errmsg, unsigned int errlen,
  KopsikTimeEntryViewItem *out_item, int *out_is_tracking);

KOPSIK_EXPORT void kopsik_format_duration_in_seconds(
  int duration_in_seconds, char *out_str, unsigned int max_strlen);

KOPSIK_EXPORT kopsik_api_result kopsik_start(
  KopsikContext *in_ctx,
  char *errmsg, unsigned int errlen,
  const char *in_description,
  KopsikTimeEntryViewItem *out_view_item);

KOPSIK_EXPORT kopsik_api_result kopsik_stop(
  KopsikContext *in_ctx,
  char *errmsg, unsigned int errlen,
  KopsikTimeEntryViewItem *out_view_item);

KOPSIK_EXPORT KopsikTimeEntryViewItemList *
  kopsik_time_entry_view_item_list_init();

KOPSIK_EXPORT void kopsik_time_entry_view_item_list_clear(
  KopsikTimeEntryViewItemList *in_list);

KOPSIK_EXPORT kopsik_api_result kopsik_time_entry_view_items(
  KopsikContext *in_ctx,
  char *errmsg, unsigned int errlen,
  KopsikTimeEntryViewItemList *out_list);

// Websocket client

KOPSIK_EXPORT kopsik_api_result kopsik_listen(
  KopsikContext *in_ctx,
  char *errmsg, unsigned int errlen);

#undef KOPSIK_EXPORT

#ifdef __cplusplus
}
#endif

#endif  // SRC_KOPSIK_API_H_
