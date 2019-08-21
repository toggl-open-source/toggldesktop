// Copyright 2019 Toggl Desktop developers.

#ifndef SRC_TOGGL_API_H_
#define SRC_TOGGL_API_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32) || defined(WIN32)
#define TOGGL_EXPORT __declspec(dllexport)
#else
#define TOGGL_EXPORT
#endif

// Constants

enum OnlineState {
    ONLINESTATE_ONLINE = 0,
    ONLINESTATE_NO_NETWORK,
    ONLINESTATE_BACKEND_DOWN
};

enum SyncState {
    SYNCSTATE_IDLE = 0,
    SYNCSTATE_WORK
};

enum DownloadStatus {
    DOWNLOADSTATUS_STARTED = 0,
    DOWNLOADSTATUS_DONE
};

enum Promotion {
    PROMOTION_JOIN_BETA_CHANNEL = 1
};

#define kOnlineStateOnline ONLINESTATE_ONLINE
#define kOnlineStateNoNetwork ONLINESTATE_NO_NETWORK
#define kOnlineStateBackendDown ONLINESTATE_BACKEND_DOWN

#define kSyncStateIdle SYNCSTATE_IDLE
#define kSyncStateWork SYNCSTATE_WORK

#define kDownloadStatusStarted DOWNLOADSTATUS_STARTED
#define kDownloadStatusDone DOWNLOADSTATUS_DONE

#define kPromotionJoinBetaChannel PROMOTION_JOIN_BETA_CHANNEL

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
    bool Billable;
    char *Tags;
    uint64_t Started;
    uint64_t Ended;
    char *StartTimeString;
    char *EndTimeString;
    uint64_t UpdatedAt;
    bool DurOnly;
    // In case it's a header
    char *DateHeader;
    char *DateDuration;
    bool IsHeader;
    // Additional fields; only when in time entry editor
    bool CanAddProjects;
    bool CanSeeBillable;
    uint64_t DefaultWID;
    char *WorkspaceName;
    // If syncing a time entry ended with an error,
    // the error is attached to the time entry
    char *Error;
    bool Locked;
    // Indicates if time entry is not synced to server
    bool Unsynced;
    // Group attributes
    bool Group;
    bool GroupOpen;
    char *GroupName;
    char *GroupDuration;
    uint64_t GroupItemCount;
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
    char *ProjectGUID;
    uint64_t TaskID;
    uint64_t ProjectID;
    uint64_t WorkspaceID;
    uint64_t Type;
    // If its a time entry or project, it can be billable
    bool Billable;
    // If its a time entry, it has tags
    char *Tags;
    char *WorkspaceName;
    uint64_t ClientID;
    void *Next;
} TogglAutocompleteView;

typedef struct {
    uint64_t ID;
    uint64_t WID;
    char *GUID;
    char *Name;
    char *WorkspaceName;
    bool Premium;
    void *Next;
} TogglGenericView;

typedef struct {
    char *Category;
    char *Name;
    char *URL;
    void *Next;
} TogglHelpArticleView;

typedef struct {
    bool UseProxy;
    char *ProxyHost;
    uint64_t ProxyPort;
    char *ProxyUsername;
    char *ProxyPassword;
    bool UseIdleDetection;
    bool MenubarTimer;
    bool MenubarProject;
    bool DockIcon;
    bool OnTop;
    bool Reminder;
    bool RecordTimeline;
    int64_t IdleMinutes;
    bool FocusOnShortcut;
    int64_t ReminderMinutes;
    bool ManualMode;
    bool AutodetectProxy;
    bool RemindMon;
    bool RemindTue;
    bool RemindWed;
    bool RemindThu;
    bool RemindFri;
    bool RemindSat;
    bool RemindSun;
    char *RemindStarts;
    char *RemindEnds;
    bool Autotrack;
    bool OpenEditorOnShortcut;
    bool Pomodoro;
    bool PomodoroBreak;
    int64_t PomodoroMinutes;
    int64_t PomodoroBreakMinutes;
    bool StopEntryOnShutdownSleep;
} TogglSettingsView;

typedef struct {
    int64_t ID;
    char *Term;
    char *ProjectAndTaskLabel;
    void *Next;
} TogglAutotrackerRuleView;

typedef struct {
    int64_t ID;
    char *Title;
    char *Filename;
    int64_t StartTime;
    int64_t EndTime;
    bool Idle;
    void *Next;
} TogglTimelineEventView;

typedef struct {
    int64_t ID;
    char *Name;
    bool VatApplicable;
    char *VatRegex;
    char *VatPercentage;
    char *Code;
    void *Next;
} TogglCountryView;

typedef char * string_list_t[];

// Callbacks that need to be implemented in UI

typedef void (*TogglDisplayApp)(
    void *context,
    bool open);

typedef void (*TogglDisplaySyncState)(
    void *context,
    int64_t state);

typedef void (*TogglDisplayUnsyncedItems)(
    void *context,
    int64_t count);

typedef void (*TogglDisplayError)(
    void *context,
    const char *errmsg,
    bool user_error);

typedef void (*TogglDisplayOverlay)(
    void *context,
    int64_t type);

typedef void (*TogglDisplayOnlineState)(
    void *context,
    int64_t state);

typedef void (*TogglDisplayURL)(
    void *context,
    const char *url);

typedef void (*TogglDisplayLogin)(
    void *context,
    bool open,
    uint64_t user_id);

typedef void (*TogglDisplayReminder)(
    void *context,
    const char *title,
    const char *informative_text);

typedef void (*TogglDisplayPomodoro)(
    void *context,
    const char *title,
    const char *informative_text);

typedef void (*TogglDisplayPomodoroBreak)(
    void *context,
    const char *title,
    const char *informative_text);

typedef void (*TogglDisplayAutotrackerNotification)(
    void *context,
    const char *project_name,
    uint64_t project_id,
    uint64_t task_id);

typedef void (*TogglDisplayPromotion)(
    void *context,
    int64_t promotion_type);

typedef void (*TogglDisplayObmExperiment)(
    void *context,
    uint64_t nr,
    bool included,
    bool seen);

typedef void (*TogglDisplayTimeEntryList)(
    void *context,
    bool open,
    TogglTimeEntryView *first,
    bool show_load_more_button);

typedef void (*TogglDisplayAutocomplete)(
    void *context,
    TogglAutocompleteView *first);

typedef void (*TogglDisplayHelpArticles)(
    void *context,
    TogglHelpArticleView *first);

typedef void (*TogglDisplayViewItems)(
    void *context,
    TogglGenericView *first);

typedef void (*TogglDisplayTimeEntryEditor)(
    void *context,
    bool open,
    TogglTimeEntryView *te,
    const char *focused_field_name);

typedef void (*TogglDisplaySettings)(
    void *context,
    bool open,
    TogglSettingsView *settings);

typedef void (*TogglDisplayTimerState)(
    void *context,
    TogglTimeEntryView *te);

typedef void (*TogglDisplayIdleNotification)(
    void *context,
    const char *guid,
    const char *since,
    const char *duration,
    int64_t started,
    const char *description);

typedef void (*TogglDisplayUpdate)(
    void *context,
    const char *url);

typedef void (*TogglDisplayUpdateDownloadState)(
    void *context,
    const char *version,
    int64_t download_state);

typedef void (*TogglDisplayAutotrackerRules)(
    void *context,
    TogglAutotrackerRuleView *first,
    uint64_t title_count,
    string_list_t title_list);

typedef void (*TogglDisplayProjectColors)(
    void *context,
    string_list_t color_list,
    uint64_t color_count);

typedef void (*TogglDisplayCountries)(
    void *context,
    TogglCountryView *first);

typedef struct {
    TogglDisplayApp App;
    TogglDisplaySyncState SyncState;
    TogglDisplayUnsyncedItems UnsyncedItems;
    TogglDisplayError Error;
    TogglDisplayOverlay Overlay;
    TogglDisplayOnlineState OnlineState;
    TogglDisplayURL URL;
    TogglDisplayLogin Login;
    TogglDisplayReminder Reminder;
    TogglDisplayPomodoro Pomodoro;
    TogglDisplayPomodoroBreak PomodoroBreak;
    TogglDisplayAutotrackerNotification AutotrackerNotification;
    TogglDisplayPromotion Promotion;
    TogglDisplayObmExperiment ObmExperiment;
    TogglDisplayTimeEntryList TimeEntryList;
    TogglDisplayAutocomplete TimeEntryAutocomplete;
    TogglDisplayAutocomplete ProjectAutocomplete;
    TogglDisplayAutocomplete MiniTimerAutocomplete;
    TogglDisplayHelpArticles HelpArticles;
    TogglDisplayViewItems WorkspaceSelect;
    TogglDisplayViewItems ClientSelect;
    TogglDisplayViewItems Tags;
    TogglDisplayTimeEntryEditor TimeEntryEditor;
    TogglDisplaySettings Settings;
    TogglDisplayTimerState TimerState;
    TogglDisplayIdleNotification IdleNotification;
    TogglDisplayUpdate Update;
    TogglDisplayUpdateDownloadState UpdateDownloadState;
    TogglDisplayAutotrackerRules AutotrackerRules;
    TogglDisplayProjectColors ProjectColors;
    TogglDisplayCountries Countries;
} TogglCallbacks;

// Initialize/destroy an instance of the app

TOGGL_EXPORT void *toggl_context_init(
    const char *app_name,
    const char *app_version,
    bool production,
    bool check_for_updates);

TOGGL_EXPORT void toggl_context_clear(
    void *context);

// CA cert bundle must be configured from UI

TOGGL_EXPORT void toggl_set_cacert_path(
    void *context,
    const char *path);

// DB path must be configured from UI

TOGGL_EXPORT bool toggl_set_db_path(
    void *context,
    const char *path);

// Configure update download path for silent updates
// Need to configure only if you have
// enabled update check and have not set the
// display update callback
TOGGL_EXPORT void toggl_set_update_path(
    void *context,
    const char *path);

// you must free the result
TOGGL_EXPORT char *toggl_update_path(
    void *context);

// Log path must be configured from UI

TOGGL_EXPORT void toggl_set_log_path(
    void *context,
    const char *path);

// Log level is optional

TOGGL_EXPORT void toggl_set_log_level(
    void *context,
    const char *level);

// Various parts of UI can tell the app to show itself.

TOGGL_EXPORT void toggl_show_app(
    void *context);

// Configure the UI callbacks. Required.

TOGGL_EXPORT void toggl_register_callbacks(
    void *context,
    TogglCallbacks callbacks);

// After UI callbacks are configured, start pumping UI events

TOGGL_EXPORT bool toggl_ui_start(
    void *context);

// User interaction with the app

TOGGL_EXPORT bool toggl_login(
    void *context,
    const char *email,
    const char *password);

TOGGL_EXPORT bool toggl_signup(
    void *context,
    const char *email,
    const char *password,
    uint64_t country_id);

TOGGL_EXPORT bool toggl_google_login(
    void *context,
    const char *access_token);

TOGGL_EXPORT void toggl_password_forgot(
    void *context);

TOGGL_EXPORT void toggl_tos(
    void *context);

TOGGL_EXPORT void toggl_privacy_policy(
    void *context);

TOGGL_EXPORT void toggl_open_in_browser(
    void *context);

TOGGL_EXPORT bool toggl_accept_tos(
    void *context);

TOGGL_EXPORT void toggl_get_support(
    void *context,
    int32_t type);

TOGGL_EXPORT bool toggl_feedback_send(
    void *context,
    const char *topic,
    const char *details,
    const char *filename);

TOGGL_EXPORT void toggl_search_help_articles(
    void *context,
    const char *keywords);

TOGGL_EXPORT void toggl_view_time_entry_list(
    void *context);

TOGGL_EXPORT void toggl_edit(
    void *context,
    const char *guid,
    bool edit_running_time_entry,
    const char *focused_field_name);

TOGGL_EXPORT void toggl_edit_preferences(
    void *context);

TOGGL_EXPORT bool toggl_continue_time_entry(
    void *context,
    const char *guid);

TOGGL_EXPORT bool toggl_continue_latest_time_entry(
    void *context,
    bool prevent_on_app);

TOGGL_EXPORT bool toggl_delete_time_entry(
    void *context,
    const char *guid);

TOGGL_EXPORT bool toggl_set_time_entry_duration(
    void *context,
    const char *guid,
    const char *value);

TOGGL_EXPORT bool toggl_set_time_entry_project(
    void *context,
    const char *guid,
    uint64_t task_id,
    uint64_t project_id,
    const char *project_guid);

TOGGL_EXPORT bool toggl_set_time_entry_date(
    void *context,
    const char *guid,
    int64_t unix_timestamp);

TOGGL_EXPORT bool toggl_set_time_entry_start(
    void *context,
    const char *guid,
    const char *value);

TOGGL_EXPORT bool toggl_set_time_entry_end(
    void *context,
    const char *guid,
    const char *value);

// value is '\t' separated tag list
TOGGL_EXPORT bool toggl_set_time_entry_tags(
    void *context,
    const char *guid,
    const char *value);

TOGGL_EXPORT bool toggl_set_time_entry_billable(
    void *context,
    const char *guid,
    bool value);

TOGGL_EXPORT bool toggl_set_time_entry_description(
    void *context,
    const char *guid,
    const char *value);

TOGGL_EXPORT bool toggl_stop(
    void *context,
    bool prevent_on_app);

TOGGL_EXPORT bool toggl_discard_time_at(
    void *context,
    const char *guid,
    int64_t at,
    bool split_into_new_entry);

TOGGL_EXPORT bool toggl_discard_time_and_continue(
    void *context,
    const char *guid,
    int64_t at);

TOGGL_EXPORT bool toggl_set_settings_remind_days(
    void *context,
    bool remind_mon,
    bool remind_tue,
    bool remind_wed,
    bool remind_thu,
    bool remind_fri,
    bool remind_sat,
    bool remind_sun);

TOGGL_EXPORT bool toggl_set_settings_remind_times(
    void *context,
    const char *remind_starts,
    const char *remind_ends);

TOGGL_EXPORT bool toggl_set_settings_use_idle_detection(
    void *context,
    bool use_idle_detection);

TOGGL_EXPORT bool toggl_set_settings_autotrack(
    void *context,
    bool value);

TOGGL_EXPORT bool toggl_set_settings_open_editor_on_shortcut(
    void *context,
    bool value);

TOGGL_EXPORT bool toggl_set_settings_autodetect_proxy(
    void *context,
    bool autodetect_proxy);

TOGGL_EXPORT bool toggl_set_settings_menubar_timer(
    void *context,
    bool menubar_timer);

TOGGL_EXPORT bool toggl_set_settings_menubar_project(
    void *context,
    bool menubar_project);

TOGGL_EXPORT bool toggl_set_settings_dock_icon(
    void *context,
    bool dock_icon);

TOGGL_EXPORT bool toggl_set_settings_on_top(
    void *context,
    bool on_top);

TOGGL_EXPORT bool toggl_set_settings_reminder(
    void *context,
    bool reminder);

TOGGL_EXPORT bool toggl_set_settings_pomodoro(
    void *context,
    bool pomodoro);

TOGGL_EXPORT bool toggl_set_settings_pomodoro_break(
    void *context,
    bool pomodoro_break);

TOGGL_EXPORT bool toggl_set_settings_stop_entry_on_shutdown_sleep(
    void *context,
    bool stop_entry);

TOGGL_EXPORT bool toggl_set_settings_idle_minutes(
    void *context,
    uint64_t idle_minutes);

TOGGL_EXPORT bool toggl_set_settings_focus_on_shortcut(
    void *context,
    bool focus_on_shortcut);

TOGGL_EXPORT bool toggl_set_settings_reminder_minutes(
    void *context,
    uint64_t reminder_minutes);

TOGGL_EXPORT bool toggl_set_settings_pomodoro_minutes(
    void *context,
    uint64_t pomodoro_minutes);

TOGGL_EXPORT bool toggl_set_settings_pomodoro_break_minutes(
    void *context,
    uint64_t pomodoro_break_minutes);

TOGGL_EXPORT bool toggl_set_settings_manual_mode(
    void *context,
    bool manual_mode);

TOGGL_EXPORT bool toggl_set_proxy_settings(
    void *context,
    bool use_proxy,
    const char *proxy_host,
    uint64_t proxy_port,
    const char *proxy_username,
    const char *proxy_password);

TOGGL_EXPORT bool toggl_set_window_settings(
    void *context,
    int64_t window_x,
    int64_t window_y,
    int64_t window_height,
    int64_t window_width);

TOGGL_EXPORT bool toggl_window_settings(
    void *context,
    int64_t *window_x,
    int64_t *window_y,
    int64_t *window_height,
    int64_t *window_width);

TOGGL_EXPORT void toggl_set_window_maximized(
    void *context,
    bool value);

TOGGL_EXPORT bool toggl_get_window_maximized(
    void *context);

TOGGL_EXPORT void toggl_set_window_minimized(
    void *context,
    bool value);

TOGGL_EXPORT bool toggl_get_window_minimized(
    void *context);

TOGGL_EXPORT void toggl_set_window_edit_size_height(
    void *context,
    int64_t value);

TOGGL_EXPORT int64_t toggl_get_window_edit_size_height(
    void *context);

TOGGL_EXPORT void toggl_set_window_edit_size_width(
    void *context,
    int64_t value);

TOGGL_EXPORT int64_t toggl_get_window_edit_size_width(
    void *context);

TOGGL_EXPORT void toggl_set_key_start(
    void *context,
    const char *value);

// You must free() the result
TOGGL_EXPORT char *toggl_get_key_start(
    void *context);

TOGGL_EXPORT void toggl_set_key_show(
    void *context,
    const char *value);

// You must free() the result
TOGGL_EXPORT char *toggl_get_key_show(
    void *context);

TOGGL_EXPORT void toggl_set_key_modifier_show(
    void *context,
    const char *value);

// You must free() the result
TOGGL_EXPORT char *toggl_get_key_modifier_show(
    void *context);

TOGGL_EXPORT void toggl_set_key_modifier_start(
    void *context,
    const char *value);

// You must free() the result
TOGGL_EXPORT char *toggl_get_key_modifier_start(
    void *context);

TOGGL_EXPORT bool toggl_logout(
    void *context);

TOGGL_EXPORT bool toggl_clear_cache(
    void *context);

// returns GUID of the started time entry. you must free() the result
TOGGL_EXPORT char *toggl_start(
    void *context,
    const char *description,
    const char *duration,
    uint64_t task_id,
    uint64_t project_id,
    const char *project_guid,
    const char *tags,
    bool prevent_on_app);

// returns GUID of the new project. you must free() the result
TOGGL_EXPORT char *toggl_add_project(
    void *context,
    const char *time_entry_guid,
    uint64_t workspace_id,
    uint64_t client_id,
    const char *client_guid,
    const char *project_name,
    bool is_private,
    const char *project_color);

// returns GUID of the new client. you must free() the result
TOGGL_EXPORT char *toggl_create_client(
    void *context,
    uint64_t workspace_id,
    const char *client_name);

TOGGL_EXPORT bool toggl_add_obm_action(
    void *context,
    uint64_t experiment_id,
    const char *key,
    const char *value);

TOGGL_EXPORT void toggl_add_obm_experiment_nr(
    void *context,
    uint64_t nr);

TOGGL_EXPORT bool toggl_set_default_project(
    void *context,
    uint64_t pid,
    uint64_t tid);

TOGGL_EXPORT void toggl_get_project_colors(
    void *context);

TOGGL_EXPORT void toggl_get_countries(
    void *context);

// You must free() the result
TOGGL_EXPORT char *toggl_get_default_project_name(
    void *context);

TOGGL_EXPORT uint64_t toggl_get_default_project_id(
    void *context);

TOGGL_EXPORT uint64_t toggl_get_default_task_id(
    void *context);

TOGGL_EXPORT bool toggl_set_update_channel(
    void *context,
    const char *update_channel);

// You must free() the result
TOGGL_EXPORT char *toggl_get_update_channel(
    void *context);

// You must free() the result
TOGGL_EXPORT char *toggl_get_user_fullname(
    void *context);

// You must free() the result
TOGGL_EXPORT char *toggl_get_user_email(
    void *context);

TOGGL_EXPORT void toggl_sync(
    void *context);

TOGGL_EXPORT void toggl_fullsync(
    void *context);

TOGGL_EXPORT bool toggl_timeline_toggle_recording(
    void *context,
    bool record_timeline);

TOGGL_EXPORT bool toggl_timeline_is_recording_enabled(
    void *context);

TOGGL_EXPORT void toggl_set_sleep(
    void *context);

TOGGL_EXPORT void toggl_set_wake(
    void *context);

TOGGL_EXPORT void toggl_set_locked(
    void *context);

TOGGL_EXPORT void toggl_set_unlocked(
    void *context);

TOGGL_EXPORT void toggl_os_shutdown(
    void *context);

// Notify lib that client is online again.
TOGGL_EXPORT void toggl_set_online(
    void *context);

TOGGL_EXPORT void toggl_set_idle_seconds(
    void *context,
    uint64_t idle_seconds);

TOGGL_EXPORT bool toggl_set_promotion_response(
    void *context,
    int64_t promotion_type,
    int64_t promotion_response);

// Shared helpers

// You must free() the result
TOGGL_EXPORT char *toggl_format_tracking_time_duration(
    void *context,
    int64_t duration_in_seconds);

// You must free() the result
TOGGL_EXPORT char *toggl_format_tracked_time_duration(
    void *context,
    int64_t duration_in_seconds);

TOGGL_EXPORT int64_t toggl_parse_duration_string_into_seconds(
    void *context,
    const char *duration_string);

// Write to the lib logger
TOGGL_EXPORT void toggl_debug(
    void *context,
    const char *text);

// Check if sizeof view struct matches those in UI
// Else stuff blows up when Marshalling in C#
// Will return error string if size is invalid,
// you must free() the result
TOGGL_EXPORT char *toggl_check_view_struct_size(
    void *context,
    int32_t time_entry_view_item_size,
    int32_t autocomplete_view_item_size,
    int32_t view_item_size,
    int32_t settings_size,
    int32_t autotracker_view_item_size);

// You must free() the result
TOGGL_EXPORT char *toggl_run_script(
    void *context,
    const char *script,
    int64_t *err);

TOGGL_EXPORT int64_t toggl_autotracker_add_rule(
    void *context,
    const char *term,
    uint64_t project_id,
    uint64_t task_id);

TOGGL_EXPORT bool toggl_autotracker_delete_rule(
    void *context,
    int64_t id);

// Testing helpers. May change any time
TOGGL_EXPORT void testing_sleep(
    void *context,
    int32_t seconds);

// For testing only
TOGGL_EXPORT bool testing_set_logged_in_user(
    void *context,
    const char *json);

TOGGL_EXPORT void toggl_set_compact_mode(
    void *context,
    bool value);

TOGGL_EXPORT bool toggl_get_compact_mode(
    void *context);

TOGGL_EXPORT void toggl_set_keep_end_time_fixed(
    void *context,
    bool value);

TOGGL_EXPORT bool toggl_get_keep_end_time_fixed(
    void *context);


TOGGL_EXPORT void toggl_set_mini_timer_x(
    void *context,
    int64_t value);

TOGGL_EXPORT int64_t toggl_get_mini_timer_x(
    void *context);

TOGGL_EXPORT void toggl_set_mini_timer_y(
    void *context,
    int64_t value);

TOGGL_EXPORT int64_t toggl_get_mini_timer_y(
    void *context);

TOGGL_EXPORT void toggl_set_mini_timer_w(
    void *context,
    int64_t value);

TOGGL_EXPORT int64_t toggl_get_mini_timer_w(
    void *context);

TOGGL_EXPORT void toggl_set_mini_timer_visible(
    void *context,
    bool value);

TOGGL_EXPORT bool toggl_get_mini_timer_visible(
    void *context);

TOGGL_EXPORT void toggl_load_more(
    void *context);

TOGGL_EXPORT void track_window_size(
    void *context,
    uint64_t width,
    uint64_t height);

TOGGL_EXPORT void track_edit_size(
    void *context,
    uint64_t width,
    uint64_t height);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // SRC_TOGGL_API_H_
