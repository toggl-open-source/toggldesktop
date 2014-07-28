// Copyright 2014 Toggl Desktop developers.

// Warning! Modifying the public API header
// will break clients, including C# definitions.
// You must update the C# definitions in
// src/ui/windows/TogglDesktop/TogglDesktop/KopsikApi.cs
// to fix this.

#ifndef SRC_LIBKOPSIK_INCLUDE_KOPSIK_API_H_
#define SRC_LIBKOPSIK_INCLUDE_KOPSIK_API_H_

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
        char *TaskLabel;
        char *ProjectLabel;
        char *ClientLabel;
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
        _Bool DurOnly;
        // In case it's a header
        char *DateHeader;
        char *DateDuration;
        _Bool IsHeader;
        // Additional fields; only when in time entry editor
        _Bool CanAddProjects;
        _Bool CanSeeBillable;
        uint64_t DefaultWID;
        // Next in list
        void *Next;
    } KopsikTimeEntryViewItem;

    typedef struct {
        // This is what is displayed to user, includes project and task.
        char *Text;
        // This is copied to "time_entry.description" field if item is selected
        char *Description;
        // Project label, if has a project
        char *ProjectAndTaskLabel;
        char *TaskLabel;
        char *ProjectLabel;
        char *ClientLabel;
        char *ProjectColor;
        uint64_t TaskID;
        uint64_t ProjectID;
        uint64_t Type;
        void *Next;
    } KopsikAutocompleteItem;

    typedef struct {
        uint64_t ID;
        uint64_t WID;
        char *GUID;
        char *Name;
        void *Next;
    } KopsikViewItem;

    typedef struct {
        _Bool UseProxy;
        char *ProxyHost;;
        uint64_t ProxyPort;
        char *ProxyUsername;
        char *ProxyPassword;
        _Bool UseIdleDetection;
        _Bool MenubarTimer;
        _Bool DockIcon;
        _Bool OnTop;
        _Bool Reminder;
        _Bool RecordTimeline;
    } KopsikSettingsViewItem;

    typedef struct {
        char *UpdateChannel;
        _Bool IsChecking;
        _Bool IsUpdateAvailable;
        char *URL;
        char *Version;
    } KopsikUpdateViewItem;

    // Callbacks that need to be implemented in UI

    typedef void (*KopsikDisplayApp)(
        const _Bool open);

    typedef void (*KopsikDisplayError)(
        const char *errmsg,
        const _Bool user_error);

    typedef void (*KopsikDisplayUpdate)(
        const _Bool open,
        KopsikUpdateViewItem *update);

    typedef void (*KopsikDisplayOnlineState)(
        const _Bool is_online,
        const char *reason);

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

    typedef void (*KopsikDisplaySettings)(
        const _Bool open,
        KopsikSettingsViewItem *settings);

    typedef void (*KopsikDisplayTimerState)(
        KopsikTimeEntryViewItem *te);

    typedef void (*KopsikDisplayIdleNotification)(
        const char *guid,
        const char *since,
        const char *duration,
        const uint64_t started);

    // Initialize/destroy an instance of the app

    KOPSIK_EXPORT void *kopsik_context_init(
        const char *app_name,
        const char *app_version);

    KOPSIK_EXPORT void kopsik_context_clear(
        void *context);

    // Set environment. By default, production is assumed. Optional.

    KOPSIK_EXPORT void kopsik_set_environment(
        void *context,
        const char *environment);

    // Optionally, disable update check

    KOPSIK_EXPORT void kopsik_disable_update_check(
        void *context);

    // CA cert bundle must be configured from UI

    KOPSIK_EXPORT void kopsik_set_cacert_path(
        void *context,
        const char *path);

    KOPSIK_EXPORT void kopsik_set_cacert_path_utf16(
        void *context,
        const wchar_t *path);

    // DB path must be configured from UI

    KOPSIK_EXPORT _Bool kopsik_set_db_path(
        void *context,
        const char *path);

    KOPSIK_EXPORT _Bool kopsik_set_db_path_utf16(
        void *context,
        const wchar_t *path);

    // Log path must be configured from UI

    KOPSIK_EXPORT void kopsik_set_log_path(
        const char *path);

    KOPSIK_EXPORT void kopsik_set_log_path_utf16(
        const wchar_t *path);

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

    KOPSIK_EXPORT void kopsik_on_app(
        void *context,
        KopsikDisplayApp);

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

    KOPSIK_EXPORT void kopsik_on_time_entry_autocomplete(
        void *context,
        KopsikDisplayAutocomplete);

    KOPSIK_EXPORT void kopsik_on_project_autocomplete(
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

    KOPSIK_EXPORT void kopsik_on_idle_notification(
        void *context,
        KopsikDisplayIdleNotification);

    // After UI callbacks are configured, start pumping UI events

    KOPSIK_EXPORT _Bool kopsik_context_start_events(
        void *context);

    // User interaction with the app

    KOPSIK_EXPORT _Bool kopsik_login(
        void *context,
        const char *email,
        const char *password);

    KOPSIK_EXPORT _Bool kopsik_google_login(
        void *context,
        const char *access_token);

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

    KOPSIK_EXPORT void kopsik_about(
        void *context);

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

    KOPSIK_EXPORT _Bool kopsik_discard_time_at(
        void *context,
        const char *guid,
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

    KOPSIK_EXPORT _Bool kopsik_set_update_channel(
        void *context,
        const char *update_channel);

    KOPSIK_EXPORT void kopsik_sync(
        void *context);

    KOPSIK_EXPORT void kopsik_timeline_toggle_recording(
        void *context,
        const _Bool record_timeline);

    KOPSIK_EXPORT void kopsik_set_sleep(
        void *context);

    KOPSIK_EXPORT void kopsik_set_wake(
        void *context);

    KOPSIK_EXPORT void kopsik_set_idle_seconds(
        void *context,
        const uint64_t idle_seconds);

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

    // Write to the lib logger
    KOPSIK_EXPORT void kopsik_debug(
        const char *text);

    // Check if sizeof view struct matches those in UI
    // Else stuff blows up when Marshalling in C#
    // Will crash if size is invalid.
    KOPSIK_EXPORT void kopsik_check_view_item_size(
        const int time_entry_view_item_size,
        const int autocomplete_view_item_size,
        const int view_item_size,
        const int settings_size,
        const int update_view_item_size);

#undef KOPSIK_EXPORT

#ifdef __cplusplus
}
#endif

#endif  // SRC_LIBKOPSIK_INCLUDE_KOPSIK_API_H_
