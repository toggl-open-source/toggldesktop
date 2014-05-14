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

    // Models

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
        char *StartTimeString;
        char *EndTimeString;
        uint64_t UpdatedAt;
        char *DateHeader;
        char *DateDuration;
        _Bool DurOnly;
        _Bool IsHeader;
        void *Next;
    } KopsikTimeEntryViewItem;

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

    typedef struct {
        uint64_t ID;
        char *GUID;
        char *Name;
        void *Next;
    } KopsikViewItem;

    typedef struct {
        _Bool use_proxy;
        char *proxy_host;
        uint64_t proxy_port;
        char *proxy_username;
        char *proxy_password;
        _Bool use_idle_detection;
        _Bool menubar_timer;
        _Bool dock_icon;
        _Bool on_top;
        _Bool reminder;
        _Bool record_timeline;
    } KopsikSettingsViewItem;

    // Callbacks that need to be implemented in UI

    typedef void (*KopsikDisplayError)(
        const char *errmsg,
        const _Bool user_error);

    typedef void (*KopsikDisplayUpdate)(
        const _Bool is_update_available,
        const char *url,
        const char *version);

    typedef void (*KopsikDisplayOnlineState)(
        const _Bool is_online);

    typedef void(*KopsikDisplayURL)(
        const char *url);

    typedef void (*KopsikDisplayLogin)(
        const _Bool open,
        const uint64_t user_id);

    typedef void (*KopsikDisplayReminder)(
        const char *title,
        const char *informative_text);

    typedef void (*KopsikDisplayTimeEntryList)(
        const _Bool open,
        KopsikTimeEntryViewItem *first);

    typedef void (*KopsikDisplayAutocomplete)(
        KopsikAutocompleteItem *first);

    typedef void (*KopsikDisplayViewItems)(
        KopsikViewItem *first);

    typedef void (*KopsikDisplayTimeEntryEditor)(
        const _Bool open,
        KopsikTimeEntryViewItem *te,
        const char *focused_field_name);

    typedef void (*KopsikApplySettings)(
        KopsikSettingsViewItem *settings);

    typedef void (*KopsikDisplaySettings)(
        const _Bool open,
        KopsikSettingsViewItem *settings);

    typedef void (*KopsikDisplayTimerState)(
        KopsikTimeEntryViewItem *te);

    // Initialize/destroy an instance of the app

    KOPSIK_EXPORT void *kopsik_context_init(
        const char *app_name,
        const char *app_version);

    KOPSIK_EXPORT void kopsik_context_clear(
        void *context);

    // DB path must be configured from UI

    KOPSIK_EXPORT _Bool kopsik_set_db_path(
        void *context,
        const char *path);

    // Log path must be configured from UI

    KOPSIK_EXPORT void kopsik_set_log_path(
        const char *path);

    // Log level is optional

    KOPSIK_EXPORT void kopsik_set_log_level(
        const char *level);

    // API URL can be overriden from UI. Optional

    KOPSIK_EXPORT void kopsik_set_api_url(
        void *context,
        const char *api_url);

    // WebSocket URL can be overriden from UI. Optional

    KOPSIK_EXPORT void kopsik_set_websocket_url(
        void *context,
        const char *websocket_url);

    // Configure the UI callbacks. Required.

    KOPSIK_EXPORT void kopsik_on_error(
        void *context,
        KopsikDisplayError);

    KOPSIK_EXPORT void kopsik_on_update(
        void *context,
        KopsikDisplayUpdate);

    KOPSIK_EXPORT void kopsik_on_online_state(
        void *context,
        KopsikDisplayOnlineState);

    KOPSIK_EXPORT void kopsik_on_url(
        void *context,
        KopsikDisplayURL);

    KOPSIK_EXPORT void kopsik_on_login(
        void *context,
        KopsikDisplayLogin);

    KOPSIK_EXPORT void kopsik_on_reminder(
        void *context,
        KopsikDisplayReminder);

    KOPSIK_EXPORT void kopsik_on_time_entry_list(
        void *context,
        KopsikDisplayTimeEntryList);

    KOPSIK_EXPORT void kopsik_on_autocomplete(
        void *context,
        KopsikDisplayAutocomplete);

    KOPSIK_EXPORT void kopsik_on_workspace_select(
        void *context,
        KopsikDisplayViewItems);

    KOPSIK_EXPORT void kopsik_on_client_select(
        void *context,
        KopsikDisplayViewItems);

    KOPSIK_EXPORT void kopsik_on_tags(
        void *context,
        KopsikDisplayViewItems);

    KOPSIK_EXPORT void kopsik_on_time_entry_editor(
        void *context,
        KopsikDisplayTimeEntryEditor);

    KOPSIK_EXPORT void kopsik_on_settings(
        void *context,
        KopsikDisplaySettings);

    KOPSIK_EXPORT void kopsik_on_timer_state(
        void *context,
        KopsikDisplayTimerState);

    // After UI callbacks are configured, start pumping UI events

    KOPSIK_EXPORT _Bool kopsik_context_start_events(
        void *context);

    // User interaction with the app

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

    KOPSIK_EXPORT void kopsik_view_time_entry_list(
        void *context);

    KOPSIK_EXPORT void kopsik_edit(
        void *context,
        const char *guid,
        const _Bool edit_running_time_entry,
        const char *focused_field_name);

    KOPSIK_EXPORT void kopsik_edit_preferences(
        void *context);

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

    KOPSIK_EXPORT _Bool kopsik_set_proxy_settings(
        void *context,
        const _Bool use_proxy,
        const char *proxy_host,
        const uint64_t proxy_port,
        const char *proxy_username,
        const char *proxy_password);

    KOPSIK_EXPORT _Bool kopsik_logout(
        void *context);

    KOPSIK_EXPORT _Bool kopsik_clear_cache(
        void *context);

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

    KOPSIK_EXPORT void kopsik_check_for_updates(
        void *context);

    KOPSIK_EXPORT _Bool kopsik_set_update_channel(
        void *context,
        const char *update_channel);

    KOPSIK_EXPORT void kopsik_sync(
        void *context);

    KOPSIK_EXPORT void kopsik_timeline_toggle_recording(
        void *context);

    KOPSIK_EXPORT void kopsik_set_sleep(
        void *context);

    KOPSIK_EXPORT void kopsik_set_wake(
        void *context);

    // Shared helpers

    KOPSIK_EXPORT _Bool kopsik_parse_time(
        const char *input,
        int *hours,
        int *minutes);

    KOPSIK_EXPORT void kopsik_format_duration_in_seconds_hhmmss(
        const int64_t duration_in_seconds,
        char *str,
        const size_t max_strlen);

    KOPSIK_EXPORT void kopsik_format_duration_in_seconds_hhmm(
        const int64_t duration_in_seconds,
        char *str,
        const size_t max_strlen);

    KOPSIK_EXPORT void kopsik_format_duration_in_seconds_pretty_hhmm(
        const int64_t duration_in_seconds,
        char *str,
        const size_t max_strlen);

    KOPSIK_EXPORT int64_t kopsik_parse_duration_string_into_seconds(
        const char *duration_string);

    // FIXME: should be invoked inside lib instead
    KOPSIK_EXPORT _Bool kopsik_users_default_wid(
        void *context,
        uint64_t *default_wid);

    // FIXME: should not be exported from lib
    KOPSIK_EXPORT _Bool kopsik_user_can_see_billable_flag(
        void *context,
        const char *guid,
        _Bool *can_see);

    // FIXME: should not be exported from lib
    KOPSIK_EXPORT _Bool kopsik_user_can_add_projects(
        void *context,
        const uint64_t workspace_id,
        _Bool *can_add);

    // FIXME: should not be exported from lib
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

    // For testing only
    _Bool kopsik_set_logged_in_user(
        void *context,
        const char *json);

#undef KOPSIK_EXPORT

#ifdef __cplusplus
}
#endif

#endif  // SRC_KOPSIK_API_H_
