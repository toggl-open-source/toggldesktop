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

KOPSIK_EXPORT int kopsik_is_networking_error(
  const char *error);

typedef struct {
  char *ModelType;
  char *ChangeType;
  unsigned int ModelID;
  char *GUID;
} KopsikModelChange;

typedef void (*KopsikViewItemChangeCallback)(
  kopsik_api_result result,
  const char *errmsg,
  KopsikModelChange *change);

typedef void (*KopsikResultCallback)(
  kopsik_api_result result,
  const char *errmsg);

typedef void (*KopsikErrorCallback)(
  const char *errmsg);

typedef void (*KopsikCheckUpdateCallback)(
  kopsik_api_result result,
  const char *errmsg,
  const int is_update_available,
  const char *url,
  const char *version);

// Context API.

KOPSIK_EXPORT void *kopsik_context_init(
  const char *app_name,
  const char *app_version,
  KopsikViewItemChangeCallback change_callback,
  KopsikErrorCallback on_error_callback,
  KopsikCheckUpdateCallback check_updates_callback);

KOPSIK_EXPORT void kopsik_context_shutdown(
  void *context);

KOPSIK_EXPORT void kopsik_context_clear(
  void *context);

// Configuration API

typedef struct {
  int UseProxy;
  char *ProxyHost;
  unsigned int ProxyPort;
  char *ProxyUsername;
  char *ProxyPassword;
  int UseIdleDetection;
} KopsikSettings;

KOPSIK_EXPORT kopsik_api_result kopsik_get_settings(
  void *context,
  char *errmsg,
  unsigned int errlen,
  KopsikSettings *settings);

KOPSIK_EXPORT KopsikSettings *kopsik_settings_init();

KOPSIK_EXPORT void kopsik_settings_clear(
  KopsikSettings *settings);

KOPSIK_EXPORT kopsik_api_result kopsik_set_settings(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const int use_proxy,
  const char *proxy_host,
  const unsigned int proxy_port,
  const char *proxy_username,
  const char *proxy_password,
  const int use_idle_detection);

KOPSIK_EXPORT kopsik_api_result kopsik_configure_proxy(
  void *context,
  char *errmsg,
  unsigned int errlen);

// For mock testing only
KOPSIK_EXPORT void kopsik_test_set_https_client(
  void *context,
  void *client);

KOPSIK_EXPORT kopsik_api_result kopsik_set_db_path(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *path);

KOPSIK_EXPORT void kopsik_set_log_path(
  const char *path);

KOPSIK_EXPORT void kopsik_set_log_level(
  const char *level);

KOPSIK_EXPORT void kopsik_set_api_url(
  void *context,
  const char *api_url);

KOPSIK_EXPORT void kopsik_set_websocket_url(
  void *context,
  const char *websocket_url);

// User API

typedef struct {
    unsigned int ID;
    char *Fullname;
} KopsikUser;

KOPSIK_EXPORT KopsikUser *kopsik_user_init();

KOPSIK_EXPORT void kopsik_user_clear(
  KopsikUser *user);

KOPSIK_EXPORT kopsik_api_result kopsik_set_api_token(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *api_token);

KOPSIK_EXPORT kopsik_api_result kopsik_get_api_token(
  void *context,
  char *errmsg,
  unsigned int errlen,
  char *str,
  unsigned int max_strlen);

KOPSIK_EXPORT kopsik_api_result kopsik_current_user(
  void *context,
  char *errmsg,
  unsigned int errlen,
  KopsikUser *user);

KOPSIK_EXPORT kopsik_api_result kopsik_login(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *email,
  const char *password);

KOPSIK_EXPORT kopsik_api_result kopsik_logout(
  void *context,
  char *errmsg,
  unsigned int errlen);

KOPSIK_EXPORT kopsik_api_result kopsik_clear_cache(
  void *context,
  char *errmsg,
  unsigned int errlen);

KOPSIK_EXPORT kopsik_api_result kopsik_user_has_premium_workspaces(
  void *context,
  char *errmsg,
  unsigned int errlen,
  int *has_premium_workspaces);

// Sync

typedef struct {
  unsigned int TimeEntries;
} KopsikPushableModelStats;

KOPSIK_EXPORT kopsik_api_result kopsik_pushable_models(
  void *context,
  char *errmsg,
  unsigned int errlen,
  KopsikPushableModelStats *stats);

KOPSIK_EXPORT void kopsik_sync(
  void *context);

// Autocomplete list items

#define KOPSIK_AUTOCOMPLETE_TE 0
#define KOPSIK_AUTOCOMPLETE_TASK 1
#define KOPSIK_AUTOCOMPLETE_PROJECT 2

typedef struct {
  char *Text;  // This is what is displayed to user
  char *Description;  // This is copied to description field if item is selected
  char *ProjectAndTaskLabel;  // Project label, if has a project
  char *ProjectColor;
  unsigned int TaskID;
  unsigned int ProjectID;
  unsigned int Type;
} KopsikAutocompleteItem;

typedef struct {
  KopsikAutocompleteItem **ViewItems;
  unsigned int Length;
} KopsikAutocompleteItemList;

KOPSIK_EXPORT KopsikAutocompleteItemList *
  kopsik_autocomplete_item_list_init();

KOPSIK_EXPORT void kopsik_autocomplete_item_list_clear(
  KopsikAutocompleteItemList *list);

KOPSIK_EXPORT kopsik_api_result kopsik_autocomplete_items(
  void *context,
  char *errmsg, const unsigned int errlen,
  KopsikAutocompleteItemList *list,
  const unsigned int include_time_entries,
  const unsigned int include_tasks,
  const unsigned int include_projects);

// Time entries view

typedef struct {
  int DurationInSeconds;
  char *Description;
  char *ProjectAndTaskLabel;
  unsigned int PID;
  unsigned int TID;
  char *Duration;
  char *Color;
  char *GUID;
  int Billable;
  char *Tags;
  unsigned int Started;
  unsigned int Ended;
  unsigned int UpdatedAt;
  char *DateHeader;
  char *DateDuration;
  unsigned int DurOnly;
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
  void *context,
  char *errmsg,
  unsigned int errlen,
  KopsikTimeEntryViewItem *item,
  int *is_tracking);

KOPSIK_EXPORT void kopsik_format_duration_in_seconds_hhmmss(
  int duration_in_seconds,
  char *str,
  unsigned int max_strlen);

KOPSIK_EXPORT void kopsik_format_duration_in_seconds_hhmm(
  int duration_in_seconds,
  int is_blink,
  char *str,
  unsigned int max_strlen);

KOPSIK_EXPORT int kopsik_parse_duration_string_into_seconds(
  const char *duration_string);

KOPSIK_EXPORT kopsik_api_result kopsik_start(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *description,
  const char *duration,
  const unsigned int task_id,
  const unsigned int project_id,
  KopsikTimeEntryViewItem *item);

KOPSIK_EXPORT kopsik_api_result kopsik_time_entry_view_item_by_guid(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *guid,
  KopsikTimeEntryViewItem *item,
  int *was_found);

KOPSIK_EXPORT kopsik_api_result kopsik_continue(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *guid,
  KopsikTimeEntryViewItem *item);

KOPSIK_EXPORT kopsik_api_result kopsik_continue_latest(
  void *context,
  char *errmsg,
  unsigned int errlen,
  KopsikTimeEntryViewItem *item,
  int *was_found);

KOPSIK_EXPORT kopsik_api_result kopsik_delete_time_entry(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *guid);

KOPSIK_EXPORT kopsik_api_result kopsik_set_time_entry_duration(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *guid,
  const char *value);

KOPSIK_EXPORT kopsik_api_result kopsik_set_time_entry_project(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *guid,
  const unsigned int task_id,
  const unsigned int project_id);

KOPSIK_EXPORT kopsik_api_result kopsik_set_time_entry_start_iso_8601(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *guid,
  const char *value);

KOPSIK_EXPORT kopsik_api_result kopsik_set_time_entry_end_iso_8601(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *guid,
  const char *value);

KOPSIK_EXPORT kopsik_api_result kopsik_set_time_entry_tags(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *guid,
  const char *value);

KOPSIK_EXPORT kopsik_api_result kopsik_set_time_entry_billable(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *guid,
  int value);

KOPSIK_EXPORT kopsik_api_result kopsik_set_time_entry_description(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *guid,
  const char *value);

KOPSIK_EXPORT kopsik_api_result kopsik_stop(
  void *context,
  char *errmsg,
  unsigned int errlen,
  KopsikTimeEntryViewItem *item,
  int *was_found);

KOPSIK_EXPORT kopsik_api_result kopsik_split_running_time_entry_at(
  void *context,
  char *err,
  const unsigned int errlen,
  const unsigned int at,
  KopsikTimeEntryViewItem *item,
  int *was_found);

KOPSIK_EXPORT kopsik_api_result kopsik_stop_running_time_entry_at(
  void *context,
  char *err,
  const unsigned int errlen,
  const unsigned int at,
  KopsikTimeEntryViewItem *item,
  int *was_found);

KOPSIK_EXPORT KopsikTimeEntryViewItemList *
  kopsik_time_entry_view_item_list_init();

KOPSIK_EXPORT void kopsik_time_entry_view_item_list_clear(
  KopsikTimeEntryViewItemList *list);

KOPSIK_EXPORT kopsik_api_result kopsik_time_entry_view_items(
  void *context,
  char *errmsg,
  unsigned int errlen,
  KopsikTimeEntryViewItemList *list);

KOPSIK_EXPORT kopsik_api_result kopsik_duration_for_date_header(
  void *context,
  char *err,
  const unsigned int err_len,
  const char *date,
  char *duration,
  const unsigned int duration_len);

// Websocket client

KOPSIK_EXPORT void kopsik_websocket_switch(
  void *context,
  const unsigned int on);

// Timeline

KOPSIK_EXPORT void kopsik_timeline_switch(
  void *context,
  const unsigned int on);

KOPSIK_EXPORT void kopsik_timeline_toggle_recording(
  void *context);

KOPSIK_EXPORT int kopsik_timeline_is_recording_enabled(
  void *context);

// Feedback

KOPSIK_EXPORT void kopsik_feedback_send(
  void *context,
  const char *topic,
  const char *details,
  const char *base64encoded_image);

// Updates

KOPSIK_EXPORT void kopsik_check_for_updates(
  void *context);

KOPSIK_EXPORT kopsik_api_result kopsik_set_update_channel(
  void *context,
  char *errmsg,
  unsigned int errlen,
  const char *update_channel);

KOPSIK_EXPORT kopsik_api_result kopsik_get_update_channel(
  void *context,
  char *errmsg,
  unsigned int errlen,
  char *update_channel,
  unsigned int update_channel_len);

#undef KOPSIK_EXPORT

#ifdef __cplusplus
}
#endif

#endif  // SRC_KOPSIK_API_H_
