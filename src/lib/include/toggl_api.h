// Copyright 2014 Toggl Desktop developers.

// Warning! Modifying the public API header
// will break C# definitions.
// If you change anything, you must update the C# definitions in
// src/ui/windows/TogglDesktop/TogglDesktop/Toggl.cs
// to fix this.

#ifndef SRC_LIB_INCLUDE_TOGGL_API_H_
#define SRC_LIB_INCLUDE_TOGGL_API_H_

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
#define TOGGL_EXPORT __declspec(dllexport)
#else
#define TOGGL_EXPORT
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
    } TogglTimeEntryView;

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
    } TogglAutocompleteView;

    typedef struct {
        uint64_t ID;
        uint64_t WID;
        char *GUID;
        char *Name;
        void *Next;
    } TogglGenericView;

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
    } TogglSettingsView;

    typedef struct {
        char *UpdateChannel;
        _Bool IsChecking;
        _Bool IsUpdateAvailable;
        char *URL;
        char *Version;
    } TogglUpdateView;

    // Callbacks that need to be implemented in UI

    typedef void (*TogglDisplayApp)(
        const _Bool open);

    typedef void (*TogglDisplayError)(
        const char *errmsg,
        const _Bool user_error);

    typedef void (*TogglDisplayUpdate)(
        const _Bool open,
        TogglUpdateView *update);

    typedef void (*TogglDisplayOnlineState)(
        const _Bool is_online,
        const char *reason);

    typedef void(*TogglDisplayURL)(
        const char *url);

    typedef void (*TogglDisplayLogin)(
        const _Bool open,
        const uint64_t user_id);

    typedef void (*TogglDisplayReminder)(
        const char *title,
        const char *informative_text);

    typedef void (*TogglDisplayTimeEntryList)(
        const _Bool open,
        TogglTimeEntryView *first);

    typedef void (*TogglDisplayAutocomplete)(
        TogglAutocompleteView *first);

    typedef void (*TogglDisplayViewItems)(
        TogglGenericView *first);

    typedef void (*TogglDisplayTimeEntryEditor)(
        const _Bool open,
        TogglTimeEntryView *te,
        const char *focused_field_name);

    typedef void (*TogglDisplaySettings)(
        const _Bool open,
        TogglSettingsView *settings);

    typedef void (*TogglDisplayTimerState)(
        TogglTimeEntryView *te);

    typedef void (*TogglDisplayIdleNotification)(
        const char *guid,
        const char *since,
        const char *duration,
        const uint64_t started);

    // Initialize/destroy an instance of the app

    TOGGL_EXPORT void *toggl_context_init(
        const char *app_name,
        const char *app_version);

    TOGGL_EXPORT void toggl_context_clear(
        void *context);

    // Set environment. By default, production is assumed. Optional.

    TOGGL_EXPORT void toggl_set_environment(
        void *context,
        const char *environment);

    // Optionally, disable update check

    TOGGL_EXPORT void toggl_disable_update_check(
        void *context);

    // CA cert bundle must be configured from UI

    TOGGL_EXPORT void toggl_set_cacert_path(
        void *context,
        const char *path);

    TOGGL_EXPORT void toggl_set_cacert_path_utf16(
        void *context,
        const wchar_t *path);

    // DB path must be configured from UI

    TOGGL_EXPORT _Bool toggl_set_db_path(
        void *context,
        const char *path);

    TOGGL_EXPORT _Bool toggl_set_db_path_utf16(
        void *context,
        const wchar_t *path);

    // Log path must be configured from UI

    TOGGL_EXPORT void toggl_set_log_path(
        const char *path);

    TOGGL_EXPORT void toggl_set_log_path_utf16(
        const wchar_t *path);

    // Log level is optional

    TOGGL_EXPORT void toggl_set_log_level(
        const char *level);

    // API URL can be overriden from UI. Optional

    TOGGL_EXPORT void toggl_set_api_url(
        void *context,
        const char *api_url);

    // WebSocket URL can be overriden from UI. Optional

    TOGGL_EXPORT void toggl_set_websocket_url(
        void *context,
        const char *websocket_url);

    // Configure the UI callbacks. Required.

    TOGGL_EXPORT void toggl_on_show_app(
        void *context,
        TogglDisplayApp);

    TOGGL_EXPORT void toggl_on_error(
        void *context,
        TogglDisplayError);

    TOGGL_EXPORT void toggl_on_update(
        void *context,
        TogglDisplayUpdate);

    TOGGL_EXPORT void toggl_on_online_state(
        void *context,
        TogglDisplayOnlineState);

    TOGGL_EXPORT void toggl_on_url(
        void *context,
        TogglDisplayURL);

    TOGGL_EXPORT void toggl_on_login(
        void *context,
        TogglDisplayLogin);

    TOGGL_EXPORT void toggl_on_reminder(
        void *context,
        TogglDisplayReminder);

    TOGGL_EXPORT void toggl_on_time_entry_list(
        void *context,
        TogglDisplayTimeEntryList);

    TOGGL_EXPORT void toggl_on_time_entry_autocomplete(
        void *context,
        TogglDisplayAutocomplete);

    TOGGL_EXPORT void toggl_on_project_autocomplete(
        void *context,
        TogglDisplayAutocomplete);

    TOGGL_EXPORT void toggl_on_workspace_select(
        void *context,
        TogglDisplayViewItems);

    TOGGL_EXPORT void toggl_on_client_select(
        void *context,
        TogglDisplayViewItems);

    TOGGL_EXPORT void toggl_on_tags(
        void *context,
        TogglDisplayViewItems);

    TOGGL_EXPORT void toggl_on_time_entry_editor(
        void *context,
        TogglDisplayTimeEntryEditor);

    TOGGL_EXPORT void toggl_on_settings(
        void *context,
        TogglDisplaySettings);

    TOGGL_EXPORT void toggl_on_timer_state(
        void *context,
        TogglDisplayTimerState);

    TOGGL_EXPORT void toggl_on_idle_notification(
        void *context,
        TogglDisplayIdleNotification);

    // After UI callbacks are configured, start pumping UI events

    TOGGL_EXPORT _Bool toggl_ui_start(
        void *context);

    // User interaction with the app

    TOGGL_EXPORT _Bool toggl_login(
        void *context,
        const char *email,
        const char *password);

    TOGGL_EXPORT _Bool toggl_google_login(
        void *context,
        const char *access_token);

    TOGGL_EXPORT void toggl_password_forgot(
        void *context);

    TOGGL_EXPORT void toggl_open_in_browser(
        void *context);

    TOGGL_EXPORT void toggl_get_support(
        void *context);

    TOGGL_EXPORT _Bool toggl_feedback_send(
        void *context,
        const char *topic,
        const char *details,
        const char *filename);

    TOGGL_EXPORT void toggl_about(
        void *context);

    TOGGL_EXPORT void toggl_view_time_entry_list(
        void *context);

    TOGGL_EXPORT void toggl_edit(
        void *context,
        const char *guid,
        const _Bool edit_running_time_entry,
        const char *focused_field_name);

    TOGGL_EXPORT void toggl_edit_preferences(
        void *context);

    TOGGL_EXPORT _Bool toggl_continue(
        void *context,
        const char *guid);

    TOGGL_EXPORT _Bool toggl_continue_latest(
        void *context);

    TOGGL_EXPORT _Bool toggl_delete_time_entry(
        void *context,
        const char *guid);

    TOGGL_EXPORT _Bool toggl_set_time_entry_duration(
        void *context,
        const char *guid,
        const char *value);

    TOGGL_EXPORT _Bool toggl_set_time_entry_project(
        void *context,
        const char *guid,
        const uint64_t task_id,
        const uint64_t project_id,
        const char *project_guid);

    TOGGL_EXPORT _Bool toggl_set_time_entry_start_iso_8601(
        void *context,
        const char *guid,
        const char *value);

    TOGGL_EXPORT _Bool toggl_set_time_entry_end_iso_8601(
        void *context,
        const char *guid,
        const char *value);

    TOGGL_EXPORT _Bool toggl_set_time_entry_tags(
        void *context,
        const char *guid,
        const char *value);

    TOGGL_EXPORT _Bool toggl_set_time_entry_billable(
        void *context,
        const char *guid,
        _Bool);

    TOGGL_EXPORT _Bool toggl_set_time_entry_description(
        void *context,
        const char *guid,
        const char *value);

    TOGGL_EXPORT _Bool toggl_stop(
        void *context);

    TOGGL_EXPORT _Bool toggl_discard_time_at(
        void *context,
        const char *guid,
        const uint64_t at);

    TOGGL_EXPORT _Bool toggl_set_settings(
        void *context,
        const _Bool use_idle_detection,
        const _Bool menubar_timer,
        const _Bool dock_icon,
        const _Bool on_top,
        const _Bool reminder);

    TOGGL_EXPORT _Bool toggl_set_proxy_settings(
        void *context,
        const _Bool use_proxy,
        const char *proxy_host,
        const uint64_t proxy_port,
        const char *proxy_username,
        const char *proxy_password);

    TOGGL_EXPORT _Bool toggl_logout(
        void *context);

    TOGGL_EXPORT _Bool toggl_clear_cache(
        void *context);

    TOGGL_EXPORT _Bool toggl_start(
        void *context,
        const char *description,
        const char *duration,
        const uint64_t task_id,
        const uint64_t project_id);

    TOGGL_EXPORT _Bool toggl_add_project(
        void *context,
        const char *time_entry_guid,
        const uint64_t workspace_id,
        const uint64_t client_id,
        const char *project_name,
        const _Bool is_private);

    TOGGL_EXPORT _Bool toggl_set_update_channel(
        void *context,
        const char *update_channel);

    // You must free() the result
    TOGGL_EXPORT char *toggl_get_update_channel(
        void *context);

    TOGGL_EXPORT void toggl_sync(
        void *context);

    TOGGL_EXPORT _Bool toggl_timeline_toggle_recording(
        void *context,
        const _Bool record_timeline);

    TOGGL_EXPORT _Bool toggl_timeline_is_recording_enabled(
        void *context);

    TOGGL_EXPORT void toggl_set_sleep(
        void *context);

    TOGGL_EXPORT void toggl_set_wake(
        void *context);

    // Notify lib that client is online again.
    TOGGL_EXPORT void toggl_set_online(
        void *context);

    TOGGL_EXPORT void toggl_set_idle_seconds(
        void *context,
        const uint64_t idle_seconds);

    // Shared helpers

    TOGGL_EXPORT _Bool toggl_parse_time(
        const char *input,
        int *hours,
        int *minutes);

    // You must free() the result
    TOGGL_EXPORT char *toggl_format_duration_in_seconds_hhmmss(
        const int64_t duration_in_seconds);

    // You must free() the result
    TOGGL_EXPORT char *toggl_format_duration_in_seconds_hhmm(
        const int64_t duration_in_seconds);

    TOGGL_EXPORT int64_t toggl_parse_duration_string_into_seconds(
        const char *duration_string);

    // Write to the lib logger
    TOGGL_EXPORT void toggl_debug(
        const char *text);

    // Check if sizeof view struct matches those in UI
    // Else stuff blows up when Marshalling in C#
    // Will crash if size is invalid.
    TOGGL_EXPORT void toggl_check_view_struct_size(
        const int time_entry_view_item_size,
        const int autocomplete_view_item_size,
        const int view_item_size,
        const int settings_size);

#undef TOGGL_EXPORT

#ifdef __cplusplus
}
#endif

#endif  // SRC_LIB_INCLUDE_TOGGL_API_H_
