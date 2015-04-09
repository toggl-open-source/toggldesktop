// Copyright 2014 Toggl Desktop developers.

// Warning! Modifying the public API header
// will break C# definitions.
// If you change anything, you must update the C# definitions in
// src/ui/windows/TogglDesktop/TogglDesktop/Toggl.cs
// to fix this.

#ifndef SRC_TOGGL_API_H_
#define SRC_TOGGL_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
    typedef bool _Bool;
#else
#include <stdbool.h>
#endif

#ifdef _WIN32
#define TOGGL_EXPORT __declspec(dllexport)
#else
#define TOGGL_EXPORT
#endif

#ifdef _WIN32
#define char_t wchar_t
#else
#define char_t char
#endif

    // Models

    typedef struct {
        int64_t DurationInSeconds;
        char_t *Description;
        char_t *ProjectAndTaskLabel;
        char_t *TaskLabel;
        char_t *ProjectLabel;
        char_t *ClientLabel;
        uint64_t WID;
        uint64_t PID;
        uint64_t TID;
        char_t *Duration;
        char_t *Color;
        char_t *GUID;
        _Bool Billable;
        char_t *Tags;
        uint64_t Started;
        uint64_t Ended;
        char_t *StartTimeString;
        char_t *EndTimeString;
        uint64_t UpdatedAt;
        _Bool DurOnly;
        // In case it's a header
        char_t *DateHeader;
        char_t *DateDuration;
        _Bool IsHeader;
        // Additional fields; only when in time entry editor
        _Bool CanAddProjects;
        _Bool CanSeeBillable;
        uint64_t DefaultWID;
        char_t *WorkspaceName;
        // If syncing a time entry ended with an error,
        // the error is attached to the time entry
        char_t *Error;
        // Next in list
        void *Next;
    } TogglTimeEntryView;

    typedef struct {
        // This is what is displayed to user, includes project and task.
        char_t *Text;
        // This is copied to "time_entry.description" field if item is selected
        char_t *Description;
        // Project label, if has a project
        char_t *ProjectAndTaskLabel;
        char_t *TaskLabel;
        char_t *ProjectLabel;
        char_t *ClientLabel;
        char_t *ProjectColor;
        uint64_t TaskID;
        uint64_t ProjectID;
        uint64_t Type;
        void *Next;
    } TogglAutocompleteView;

    typedef struct {
        uint64_t ID;
        uint64_t WID;
        char_t *GUID;
        char_t *Name;
        void *Next;
    } TogglGenericView;

    typedef struct {
        _Bool UseProxy;
        char_t *ProxyHost;;
        uint64_t ProxyPort;
        char_t *ProxyUsername;
        char_t *ProxyPassword;
        _Bool UseIdleDetection;
        _Bool MenubarTimer;
        _Bool MenubarProject;
        _Bool DockIcon;
        _Bool OnTop;
        _Bool Reminder;
        _Bool RecordTimeline;
        uint64_t IdleMinutes;
        _Bool FocusOnShortcut;
        uint64_t ReminderMinutes;
        _Bool ManualMode;
        _Bool AutodetectProxy;
    } TogglSettingsView;

    // Callbacks that need to be implemented in UI

    typedef void (*TogglDisplayApp)(
        const _Bool open);

    typedef void (*TogglDisplaySyncState)(
        const int64_t state);

    typedef void (*TogglDisplayUnsyncedItems)(
        const int64_t count);

    typedef void (*TogglDisplayError)(
        const char_t *errmsg,
        const _Bool user_error);

    typedef void (*TogglDisplayOnlineState)(
        const int64_t state);

    typedef void(*TogglDisplayURL)(
        const char_t *url);

    typedef void (*TogglDisplayLogin)(
        const _Bool open,
        const uint64_t user_id);

    typedef void (*TogglDisplayReminder)(
        const char_t *title,
        const char_t *informative_text);

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
        const char_t *focused_field_name);

    typedef void (*TogglDisplaySettings)(
        const _Bool open,
        TogglSettingsView *settings);

    typedef void (*TogglDisplayTimerState)(
        TogglTimeEntryView *te);

    typedef void (*TogglDisplayIdleNotification)(
        const char_t *guid,
        const char_t *since,
        const char_t *duration,
        const uint64_t started,
        const char_t *description);

    typedef void (*TogglDisplayUpdate)(
        const char_t *url);

    // Initialize/destroy an instance of the app

    TOGGL_EXPORT void *toggl_context_init(
        const char_t *app_name,
        const char_t *app_version);

    TOGGL_EXPORT void toggl_context_clear(
        void *context);

    // Set environment. By default, production is assumed. Optional.

    TOGGL_EXPORT void toggl_set_environment(
        void *context,
        const char_t *environment);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_environment(
        void *context);

    // Optionally, disable update check

    TOGGL_EXPORT void toggl_disable_update_check(
        void *context);

    // CA cert bundle must be configured from UI

    TOGGL_EXPORT void toggl_set_cacert_path(
        void *context,
        const char_t *path);

    // DB path must be configured from UI

    TOGGL_EXPORT _Bool toggl_set_db_path(
        void *context,
        const char_t *path);

    // Configure update download path for silent updates
    // Need to configure only if you have
    // enabled update check and have not set the
    // display update callback
    TOGGL_EXPORT void toggl_set_update_path(
        void *context,
        const char_t *path);

    // you must free the result
    TOGGL_EXPORT char_t *toggl_update_path(
        void *context);

    // Log path must be configured from UI

    TOGGL_EXPORT void toggl_set_log_path(
        const char_t *path);

    // Log level is optional

    TOGGL_EXPORT void toggl_set_log_level(
        const char_t *level);

    // Various parts of UI can tell the app to show itself.

    TOGGL_EXPORT void toggl_show_app(
        void *context);

    // Configure the UI callbacks. Required.

    TOGGL_EXPORT void toggl_on_show_app(
        void *context,
        TogglDisplayApp);

    TOGGL_EXPORT void toggl_on_sync_state(
        void *context,
        TogglDisplaySyncState);

    TOGGL_EXPORT void toggl_on_unsynced_items(
        void *context,
        TogglDisplayUnsyncedItems);

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

    TOGGL_EXPORT void toggl_on_mini_timer_autocomplete(
        void *context,
        TogglDisplayAutocomplete);

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
        const char_t *email,
        const char_t *password);

    TOGGL_EXPORT _Bool toggl_signup(
        void *context,
        const char_t *email,
        const char_t *password);

    TOGGL_EXPORT _Bool toggl_google_login(
        void *context,
        const char_t *access_token);

    TOGGL_EXPORT void toggl_password_forgot(
        void *context);

    TOGGL_EXPORT void toggl_open_in_browser(
        void *context);

    TOGGL_EXPORT void toggl_get_support(
        void *context);

    TOGGL_EXPORT _Bool toggl_feedback_send(
        void *context,
        const char_t *topic,
        const char_t *details,
        const char_t *filename);

    TOGGL_EXPORT void toggl_view_time_entry_list(
        void *context);

    TOGGL_EXPORT void toggl_edit(
        void *context,
        const char_t *guid,
        const _Bool edit_running_time_entry,
        const char_t *focused_field_name);

    TOGGL_EXPORT void toggl_edit_preferences(
        void *context);

    TOGGL_EXPORT _Bool toggl_continue(
        void *context,
        const char_t *guid);

    TOGGL_EXPORT _Bool toggl_continue_latest(
        void *context);

    TOGGL_EXPORT _Bool toggl_delete_time_entry(
        void *context,
        const char_t *guid);

    TOGGL_EXPORT _Bool toggl_set_time_entry_duration(
        void *context,
        const char_t *guid,
        const char_t *value);

    TOGGL_EXPORT _Bool toggl_set_time_entry_project(
        void *context,
        const char_t *guid,
        const uint64_t task_id,
        const uint64_t project_id,
        const char_t *project_guid);

    TOGGL_EXPORT _Bool toggl_set_time_entry_date(
        void *context,
        const char_t *guid,
        const int64_t unix_timestamp);

    TOGGL_EXPORT _Bool toggl_set_time_entry_start(
        void *context,
        const char_t *guid,
        const char_t *value);

    TOGGL_EXPORT _Bool toggl_set_time_entry_end(
        void *context,
        const char_t *guid,
        const char_t *value);

    // value is '\t' separated tag list
    TOGGL_EXPORT _Bool toggl_set_time_entry_tags(
        void *context,
        const char_t *guid,
        const char_t *value);

    TOGGL_EXPORT _Bool toggl_set_time_entry_billable(
        void *context,
        const char_t *guid,
        _Bool);

    TOGGL_EXPORT _Bool toggl_set_time_entry_description(
        void *context,
        const char_t *guid,
        const char_t *value);

    TOGGL_EXPORT _Bool toggl_stop(
        void *context);

    TOGGL_EXPORT _Bool toggl_discard_time_at(
        void *context,
        const char_t *guid,
        const uint64_t at,
        const _Bool split_into_new_entry);

    TOGGL_EXPORT _Bool toggl_set_settings_use_idle_detection(
        void *context,
        const _Bool use_idle_detection);

    TOGGL_EXPORT _Bool toggl_set_settings_autodetect_proxy(
        void *context,
        const _Bool autodetect_proxy);

    TOGGL_EXPORT _Bool toggl_set_settings_menubar_timer(
        void *context,
        const _Bool menubar_timer);

    TOGGL_EXPORT _Bool toggl_set_settings_menubar_project(
        void *context,
        const _Bool menubar_project);

    TOGGL_EXPORT _Bool toggl_set_settings_dock_icon(
        void *context,
        const _Bool dock_icon);

    TOGGL_EXPORT _Bool toggl_set_settings_on_top(
        void *context,
        const _Bool on_top);

    TOGGL_EXPORT _Bool toggl_set_settings_reminder(
        void *context,
        const _Bool reminder);

    TOGGL_EXPORT _Bool toggl_set_settings_idle_minutes(
        void *context,
        const uint64_t idle_minutes);

    TOGGL_EXPORT _Bool toggl_set_settings_focus_on_shortcut(
        void *context,
        const _Bool focus_on_shortcut);

    TOGGL_EXPORT _Bool toggl_set_settings_reminder_minutes(
        void *context,
        const uint64_t reminder_minutes);

    TOGGL_EXPORT _Bool toggl_set_settings_manual_mode(
        void *context,
        const _Bool manual_mode);

    TOGGL_EXPORT _Bool toggl_set_proxy_settings(
        void *context,
        const _Bool use_proxy,
        const char_t *proxy_host,
        const uint64_t proxy_port,
        const char_t *proxy_username,
        const char_t *proxy_password);

    TOGGL_EXPORT _Bool toggl_set_window_settings(
        void *context,
        const int64_t window_x,
        const int64_t window_y,
        const int64_t window_height,
        const int64_t window_width);

    TOGGL_EXPORT _Bool toggl_window_settings(
        void *context,
        int64_t *window_x,
        int64_t *window_y,
        int64_t *window_height,
        int64_t *window_width);

    TOGGL_EXPORT _Bool toggl_logout(
        void *context);

    TOGGL_EXPORT _Bool toggl_clear_cache(
        void *context);

    // if char is received it should be freed
    TOGGL_EXPORT char_t *toggl_start(
        void *context,
        const char_t *description,
        const char_t *duration,
        const uint64_t task_id,
        const uint64_t project_id);

    TOGGL_EXPORT _Bool toggl_add_project(
        void *context,
        const char_t *time_entry_guid,
        const uint64_t workspace_id,
        const uint64_t client_id,
        const char_t *project_name,
        const _Bool is_private);

    TOGGL_EXPORT _Bool toggl_create_client(
        void *context,
        const uint64_t workspace_id,
        const char_t *client_name);

    TOGGL_EXPORT _Bool toggl_create_project(
        void *context,
        const uint64_t workspace_id,
        const uint64_t client_id,
        const char_t *project_name,
        const _Bool is_private);

    TOGGL_EXPORT _Bool toggl_set_update_channel(
        void *context,
        const char_t *update_channel);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_get_update_channel(
        void *context);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_get_user_fullname(
        void *context);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_get_user_email(
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

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_format_tracking_time_duration(
        const int64_t duration_in_seconds);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_format_tracked_time_duration(
        const int64_t duration_in_seconds);

    TOGGL_EXPORT int64_t toggl_parse_duration_string_into_seconds(
        const char_t *duration_string);

    // Write to the lib logger
    TOGGL_EXPORT void toggl_debug(
        const char_t *text);

    // Check if sizeof view struct matches those in UI
    // Else stuff blows up when Marshalling in C#
    // Will return false if size is invalid.
    TOGGL_EXPORT _Bool toggl_check_view_struct_size(
        const int time_entry_view_item_size,
        const int autocomplete_view_item_size,
        const int view_item_size,
        const int settings_size);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_run_script(
        void *context,
        const char* script,
        int64_t *err);

    // Testing helpers. May change any time
    void testing_sleep(
        const int seconds);

    // For testing only
    _Bool testing_set_logged_in_user(
        void *context,
        const char *json);

#undef TOGGL_EXPORT

#ifdef __cplusplus
}
#endif

#endif  // SRC_TOGGL_API_H_
