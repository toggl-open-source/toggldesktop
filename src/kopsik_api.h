// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_KOPSIK_API_H_
#define SRC_KOPSIK_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
#define _Bool bool
#else
#include <stdbool.h>
#endif

#ifdef _WIN32
#define KOPSIK_EXPORT __declspec(dllexport)
#else
#define KOPSIK_EXPORT
#endif

    KOPSIK_EXPORT void *kopsik_context_init(
        const char *app_name,
        const char *app_version);

    typedef void (*KopsikErrorCallback)(const char *errmsg);

    KOPSIK_EXPORT void kopsik_context_set_error_callback(
        void *context,
        KopsikErrorCallback);

    typedef void (*KopsikCheckUpdateCallback)(
        const _Bool is_update_available,
        const char *url,
        const char *version);

    KOPSIK_EXPORT void kopsik_context_set_check_update_callback(
        void *context,
        KopsikCheckUpdateCallback);

    typedef void (*KopsikOnOnlineCallback)();

    KOPSIK_EXPORT void kopsik_context_set_online_callback(
        void *context,
        KopsikOnOnlineCallback);

    typedef void(*KopsikOpenURLCallback)(const char *url);

    KOPSIK_EXPORT void kopsik_set_open_url_callback(
        void *context,
        KopsikOpenURLCallback);

    typedef void (*KopsikUserLoginCallback)(
        uint64_t id,
        const char *fullname,
        const char *timeofdayformat);

    KOPSIK_EXPORT void kopsik_context_set_user_login_callback(
        void *context,
        KopsikUserLoginCallback);

    typedef void (*KopsikRemindCallback)(
        const char *title,
        const char *informative_text);

    KOPSIK_EXPORT void kopsik_set_remind_callback(
        void *context,
        KopsikRemindCallback);

    KOPSIK_EXPORT _Bool kopsik_set_db_path(
        void *context,
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

    KOPSIK_EXPORT void kopsik_context_start_events(
        void *context);

    KOPSIK_EXPORT _Bool kopsik_login(
        void *context,
        const char *email,
        const char *password);

    KOPSIK_EXPORT void kopsik_password_forgot(
        void *context);

    KOPSIK_EXPORT void kopsik_open_in_browser(
        void *context);

    KOPSIK_EXPORT void kopsik_get_support(
        void *context);

    KOPSIK_EXPORT _Bool kopsik_feedback_send(
        void *context,
        const char *topic,
        const char *details,
        const char *filename);

    KOPSIK_EXPORT _Bool kopsik_continue(
        void *context,
        const char *guid);

    KOPSIK_EXPORT _Bool kopsik_continue_latest(
        void *context);

    KOPSIK_EXPORT _Bool kopsik_delete_time_entry(
        void *context,
        const char *guid);

    KOPSIK_EXPORT _Bool kopsik_set_time_entry_duration(
        void *context,
        const char *guid,
        const char *value);

    KOPSIK_EXPORT _Bool kopsik_set_time_entry_project(
        void *context,
        const char *guid,
        const uint64_t task_id,
        const uint64_t project_id,
        const char *project_guid);

    KOPSIK_EXPORT _Bool kopsik_set_time_entry_start_iso_8601(
        void *context,
        const char *guid,
        const char *value);

    KOPSIK_EXPORT _Bool kopsik_set_time_entry_end_iso_8601(
        void *context,
        const char *guid,
        const char *value);

    KOPSIK_EXPORT _Bool kopsik_set_time_entry_tags(
        void *context,
        const char *guid,
        const char *value);

    KOPSIK_EXPORT _Bool kopsik_set_time_entry_billable(
        void *context,
        const char *guid,
        _Bool);

    KOPSIK_EXPORT _Bool kopsik_set_time_entry_description(
        void *context,
        const char *guid,
        const char *value);

    KOPSIK_EXPORT _Bool kopsik_stop(
        void *context);

    KOPSIK_EXPORT _Bool kopsik_stop_running_time_entry_at(
        void *context,
        const uint64_t at);

    KOPSIK_EXPORT _Bool kopsik_set_settings(
        void *context,
        const _Bool use_idle_detection,
        const _Bool menubar_timer,
        const _Bool dock_icon,
        const _Bool on_top,
        const _Bool reminder);

    KOPSIK_EXPORT _Bool kopsik_logout(
        void *context);

    KOPSIK_EXPORT _Bool kopsik_clear_cache(
        void *context);

    KOPSIK_EXPORT _Bool kopsik_parse_time(
        const char *input,
        int *hours,
        int *minutes);

    KOPSIK_EXPORT _Bool kopsik_start(
        void *context,
        const char *description,
        const char *duration,
        const uint64_t task_id,
        const uint64_t project_id);

    KOPSIK_EXPORT _Bool kopsik_add_project(
        void *context,
        const char *time_entry_guid,
        const uint64_t workspace_id,
        const uint64_t client_id,
        const char *project_name,
        const _Bool is_private);

    KOPSIK_EXPORT void kopsik_format_duration_in_seconds_hhmmss(
        const int64_t duration_in_seconds,
        char *str,
        const size_t max_strlen);

    KOPSIK_EXPORT void kopsik_format_duration_in_seconds_hhmm(
        const int64_t duration_in_seconds,
        char *str,
        const size_t max_strlen);

    KOPSIK_EXPORT int64_t kopsik_parse_duration_string_into_seconds(
        const char *duration_string);

    KOPSIK_EXPORT _Bool kopsik_set_update_channel(
        void *context,
        const char *update_channel);

    KOPSIK_EXPORT _Bool kopsik_set_proxy_settings(
        void *context,
        const _Bool use_proxy,
        const char *proxy_host,
        const uint64_t proxy_port,
        const char *proxy_username,
        const char *proxy_password);

    KOPSIK_EXPORT void kopsik_sync(
        void *context);

    KOPSIK_EXPORT void kopsik_context_clear(
        void *context);

    KOPSIK_EXPORT void kopsik_timeline_toggle_recording(
        void *context);

    KOPSIK_EXPORT void kopsik_set_sleep(
        void *context);

    KOPSIK_EXPORT void kopsik_set_wake(
        void *context);

    // FIXME: stuff below should not be exported

    KOPSIK_EXPORT _Bool kopsik_users_default_wid(
        void *context,
        uint64_t *default_wid);

    KOPSIK_EXPORT _Bool kopsik_is_networking_error(
        const char *error);

    KOPSIK_EXPORT _Bool kopsik_is_user_error(
        const char *error);

    typedef struct {
        char *ModelType;
        char *ChangeType;
        uint64_t ModelID;
        char *GUID;
    } KopsikModelChange;

    typedef struct {
        uint64_t ID;
        char *GUID;
        char *Name;
        void *Next;
    } KopsikViewItem;

    KOPSIK_EXPORT void kopsik_view_item_clear(
        KopsikViewItem *first);

    typedef void (*KopsikViewItemChangeCallback)(
        KopsikModelChange *change);

    KOPSIK_EXPORT void kopsik_context_set_view_item_change_callback(
        void *context,
        KopsikViewItemChangeCallback);

    KOPSIK_EXPORT _Bool kopsik_get_settings(
        void *context,
        _Bool *use_idle_detection,
        _Bool *menubar_timer,
        _Bool *dock_icon,
        _Bool *on_top,
        _Bool *reminder);

    KOPSIK_EXPORT _Bool kopsik_get_proxy_settings(
        void *context,
        _Bool *use_proxy,
        char **proxy_host,
        uint64_t *proxy_port,
        char **proxy_username,
        char **proxy_password);

    KOPSIK_EXPORT _Bool kopsik_configure_proxy(
        void *context);

    KOPSIK_EXPORT _Bool kopsik_user_can_see_billable_flag(
        void *context,
        const char *guid,
        _Bool *can_see);

    KOPSIK_EXPORT _Bool kopsik_user_can_add_projects(
        void *context,
        const uint64_t workspace_id,
        _Bool *can_add);

    KOPSIK_EXPORT _Bool kopsik_user_is_logged_in(
        void *context,
        _Bool *is_logged_in);

    typedef struct {
        // This is what is displayed to user
        char *Text;
        // This is copied to description field if item is selected
        char *Description;
        // Project label, if has a project
        char *ProjectAndTaskLabel;
        char *ProjectColor;
        uint64_t TaskID;
        uint64_t ProjectID;
        uint64_t Type;
        void *Next;
    } KopsikAutocompleteItem;

    KOPSIK_EXPORT _Bool kopsik_autocomplete_items(
        void *context,
        KopsikAutocompleteItem **first,
        const _Bool include_time_entries,
        const _Bool include_tasks,
        const _Bool include_projects);

    KOPSIK_EXPORT void kopsik_autocomplete_item_clear(
        KopsikAutocompleteItem *item);

    KOPSIK_EXPORT _Bool kopsik_tags(
        void *context,
        KopsikViewItem **first);

    KOPSIK_EXPORT _Bool kopsik_workspaces(
        void *context,
        KopsikViewItem **first);

    KOPSIK_EXPORT _Bool kopsik_clients(
        void *context,
        const uint64_t workspace_id,
        KopsikViewItem **first);

    typedef struct {
        int64_t DurationInSeconds;
        char *Description;
        char *ProjectAndTaskLabel;
        uint64_t WID;
        uint64_t PID;
        uint64_t TID;
        char *Duration;
        char *Color;
        char *GUID;
        _Bool Billable;
        char *Tags;
        uint64_t Started;
        uint64_t Ended;
        uint64_t UpdatedAt;
        char *DateHeader;
        char *DateDuration;
        _Bool DurOnly;
        void *Next;
    } KopsikTimeEntryViewItem;

    KOPSIK_EXPORT KopsikTimeEntryViewItem *
    kopsik_time_entry_view_item_init();

    KOPSIK_EXPORT void kopsik_time_entry_view_item_clear(
        KopsikTimeEntryViewItem *item);

    KOPSIK_EXPORT _Bool kopsik_running_time_entry_view_item(
        void *context,
        KopsikTimeEntryViewItem *item,
        _Bool *is_tracking);

    KOPSIK_EXPORT _Bool kopsik_time_entry_view_item_by_guid(
        void *context,
        const char *guid,
        KopsikTimeEntryViewItem *item,
        _Bool *was_found);

    KOPSIK_EXPORT _Bool kopsik_time_entry_view_items(
        void *context,
        KopsikTimeEntryViewItem **first);

    KOPSIK_EXPORT _Bool kopsik_duration_for_date_header(
        void *context,
        const char *date,
        char *duration,
        const size_t duration_len);

    KOPSIK_EXPORT _Bool kopsik_timeline_is_recording_enabled(
        void *context);

    KOPSIK_EXPORT void kopsik_check_for_updates(
        void *context);

    KOPSIK_EXPORT _Bool kopsik_get_update_channel(
        void *context,
        char *update_channel,
        const size_t update_channel_len);

    // For testing only
    _Bool kopsik_set_api_token(
        void *context,
        const char *api_token);

    // For testing only
    _Bool kopsik_get_api_token(
        void *context,
        char *str,
        const size_t max_strlen);

    _Bool kopsik_set_logged_in_user(
        void *context,
        const char *json);

#undef KOPSIK_EXPORT

#ifdef __cplusplus
}
#endif

#endif  // SRC_KOPSIK_API_H_
