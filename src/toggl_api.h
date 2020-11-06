// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TOGGL_API_H_
#define SRC_TOGGL_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <time.h>

#if defined(_WIN32) || defined(WIN32)
#define TOGGL_EXPORT __declspec(dllexport)
#else
#define TOGGL_EXPORT
#endif

#if defined(_WIN32) || defined(WIN32)
#define char_t wchar_t
#define bool_t bool
#else
#define char_t char
#define bool_t int
#endif

// Constants

#define kOnlineStateOnline 0
#define kOnlineStateNoNetwork 1
#define kOnlineStateBackendDown 2

#define kSyncStateIdle 0
#define kSyncStateWork 1

#define kDownloadStatusStarted 0
#define kDownloadStatusDone 1

#define kPromotionJoinBetaChannel 1

// Models

    typedef struct {
        uint64_t ID;
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
        bool_t Billable;
        char_t *Tags;
        uint64_t Started;
        uint64_t Ended;
        char_t *StartTimeString;
        char_t *EndTimeString;
        uint64_t UpdatedAt;
        bool_t DurOnly;
        // In case it's a header
        char_t *DateHeader;
        char_t *DateDuration;
        bool_t IsHeader;
        // Additional fields; only when in time entry editor
        bool_t CanAddProjects;
        bool_t CanSeeBillable;
        uint64_t DefaultWID;
        char_t *WorkspaceName;
        // If syncing a time entry ended with an error,
        // the error is attached to the time entry
        char_t *Error;
        bool_t Locked;
        // Indicates if time entry is not synced to server
        bool_t Unsynced;
        // Group attributes
        bool_t Group;
        bool_t GroupOpen;
        char_t *GroupName;
        char_t *GroupDuration;
        uint64_t GroupItemCount;
        // To categorize to 15-minute batches
        uint64_t RoundedStart;
        uint64_t RoundedEnd;
        // Next in list
        void *Next;
    } TogglTimeEntryView;

    typedef struct {
        char_t *Title;
        char_t *Filename;
        int64_t Duration;
        char_t *DurationString;
        bool_t Header;
        // references subevents
        void *Event;
        // Next in list
        void *Next;
    } TogglTimelineEventView;

    typedef struct {
        uint64_t Started;
        uint64_t Ended;
        char_t *StartTimeString;
        char_t *EndTimeString;
        void *Next;
        void *FirstEvent;
    } TogglTimelineChunkView;

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
        char_t *ProjectGUID;
        uint64_t TaskID;
        uint64_t ProjectID;
        uint64_t WorkspaceID;
        uint64_t Type;
        // If its a time entry or project, it can be billable
        bool_t Billable;
        // If its a time entry, it has tags
        char_t *Tags;
        char_t *WorkspaceName;
        uint64_t ClientID;
        void *Next;
    } TogglAutocompleteView;

    typedef struct {
        uint64_t ID;
        uint64_t WID;
        char_t *GUID;
        char_t *Name;
        char_t *WorkspaceName;
        bool_t Premium;
        void *Next;
    } TogglGenericView;

    typedef struct {
        char_t *Category;
        char_t *Name;
        char_t *URL;
        void *Next;
    } TogglHelpArticleView;

    typedef struct {
        bool_t UseProxy;
        char_t *ProxyHost;
        uint64_t ProxyPort;
        char_t *ProxyUsername;
        char_t *ProxyPassword;
        bool_t UseIdleDetection;
        bool_t MenubarTimer;
        bool_t MenubarProject;
        bool_t DockIcon;
        bool_t OnTop;
        bool_t Reminder;
        bool_t RecordTimeline;
        int64_t IdleMinutes;
        bool_t FocusOnShortcut;
        int64_t ReminderMinutes;
        bool_t ManualMode;
        bool_t AutodetectProxy;
        bool_t RemindMon;
        bool_t RemindTue;
        bool_t RemindWed;
        bool_t RemindThu;
        bool_t RemindFri;
        bool_t RemindSat;
        bool_t RemindSun;
        char_t *RemindStarts;
        char_t *RemindEnds;
        bool_t Autotrack;
        bool_t OpenEditorOnShortcut;
        bool_t Pomodoro;
        bool_t PomodoroBreak;
        int64_t PomodoroMinutes;
        int64_t PomodoroBreakMinutes;
        bool_t StopEntryOnShutdownSleep;
        bool_t ShowTouchBar;
        bool_t StartAutotrackerWithoutSuggestions;
        uint8_t ActiveTab;
        uint8_t ColorTheme;
        bool_t ForceIgnoreCert;
    } TogglSettingsView;

    typedef struct {
        int64_t ID;
        char_t *Terms;
        char_t *ProjectAndTaskLabel;
        char_t *StartTime;
        char_t *EndTime;
        uint8_t DaysOfWeek;
        void *Next;
    } TogglAutotrackerRuleView;

    typedef struct {
        int64_t ID;
        char_t *Name;
        bool_t VatApplicable;
        char_t *VatRegex;
        char_t *VatPercentage;
        char_t *Code;
        void *Next;
    } TogglCountryView;

    // Range values from [0..1]
    typedef struct {
        double r;
        double g;
        double b;
    } TogglRgbColor;

    // Range values from [0..1]
    typedef struct {
        double h;
        double s;
        double v;
    } TogglHsvColor;

    typedef enum {
        AdaptiveColorShapeOnLightBackground,
        AdaptiveColorShapeOnDarkBackground,
        AdaptiveColorTextOnLightBackground,
        AdaptiveColorTextOnDarkBackground
    } TogglAdaptiveColor;

    typedef enum {
        TimelineMenuContextTypeContinueEntry,
        TimelineMenuContextTypeStartEntryFromEnd,
        TimelineMenuContextTypeDelete,
        TimelineMenuContextTypeChangeFirstEntryStopTime,
        TimelineMenuContextTypeChangeLastEntryStartTime
    } TimelineMenuContextType;

    typedef enum {
        TimerEditActionTypeDescription = 1 << 0,
        TimerEditActionTypeDuration = 1 << 1,
        TimerEditActionTypeProject = 1 << 2,
        TimerEditActionTypeTags = 1 << 3,
        TimerEditActionTypeBillable = 1 << 4
    } TimerEditActionType;

    typedef enum {
        TimerShortcutActionTypeProjectSelected,
        TimerShortcutActionTypeTagSelected,
        TimerShortcutActionTypeProjectCreated,
        TimerShortcutActionTypeTagCreated
    } TimerShortcutActionType;

typedef enum {
        TogglServerStaging = 0,
        TogglServerProduction
    } TogglServerType;

    // Callbacks that need to be implemented in UI

    typedef void (*TogglDisplayApp)(
        const bool_t open);

    typedef void (*TogglDisplaySyncState)(
        const int64_t state);

    typedef void (*TogglDisplayUnsyncedItems)(
        const int64_t count);

    typedef void (*TogglDisplayError)(
        const char_t *errmsg,
        const bool_t user_error);

    typedef void (*TogglDisplayOverlay)(
        const int64_t type);

    typedef void (*TogglDisplayOnlineState)(
        const int64_t state);

    typedef void (*TogglDisplayURL)(
        const char_t *url);

    typedef void (*TogglDisplayLogin)(
        const bool_t open,
        const uint64_t user_id);

    typedef void (*TogglDisplayReminder)(
        const char_t *title,
        const char_t *informative_text);

    typedef void (*TogglDisplayPomodoro)(
        const char_t *title,
        const char_t *informative_text);

    typedef void (*TogglDisplayPomodoroBreak)(
        const char_t *title,
        const char_t *informative_text);

    typedef void (*TogglDisplayAutotrackerNotification)(
        const char_t *project_name,
        const uint64_t project_id,
        const uint64_t task_id);

    typedef void (*TogglDisplayPromotion)(
        const int64_t promotion_type);

    typedef void (*TogglDisplayTimeEntryList)(
        const bool_t open,
        TogglTimeEntryView *first,
        const bool_t show_load_more_button);

    typedef void (*TogglDisplayTimeline)(
        const bool_t open,
        const char_t *date,
        TogglTimelineChunkView *first,
        TogglTimeEntryView *first_entry,
        const uint64_t start_day,
        const uint64_t end_day);

    typedef void (*TogglDisplayAutocomplete)(
        TogglAutocompleteView *first);

    typedef void (*TogglDisplayHelpArticles)(
        TogglHelpArticleView *first);

    typedef void (*TogglDisplayViewItems)(
        TogglGenericView *first);

    typedef void (*TogglDisplayTimeEntryEditor)(
        const bool_t open,
        TogglTimeEntryView *te,
        const char_t *focused_field_name);

    typedef void (*TogglDisplaySettings)(
        const bool_t open,
        TogglSettingsView *settings);

    typedef void (*TogglDisplayTimerState)(
        TogglTimeEntryView *te);

    typedef void (*TogglContinueSignIn)(
    );

    typedef void (*TogglDisplayLoginSSO)(
        const char_t *sso_url);

    typedef void (*TogglDisplayIdleNotification)(
        const char_t *guid,
        const char_t *since,
        const char_t *duration,
        const int64_t started,
        const char_t *description,
        const char_t *project,
        const char_t *task,
        const char_t *projectColor);

    typedef void (*TogglDisplayUpdate)(
        const char_t *url);

    typedef void (*TogglDisplayUpdateDownloadState)(
        const char_t *version,
        const int64_t download_state);

    typedef void (*TogglDisplayMessage)(
        const char_t *title,
        const char_t *text,
        const char_t *button,
        const char_t *url);

    typedef char_t * string_list_t[];

    typedef void (*TogglDisplayAutotrackerRules)(
        TogglAutotrackerRuleView *first,
        const uint64_t title_count,
        string_list_t title_list);

    typedef void (*TogglDisplayProjectColors)(
        string_list_t color_list,
        const uint64_t color_count);

    typedef void (*TogglDisplayCountries)(
        TogglCountryView *first);

    typedef void (*TogglDisplayOnboarding)(
        const int64_t onboarding_type);

    typedef void (*TogglDisplayTimelineUI)(
        const bool_t isEnabled);

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

    TOGGL_EXPORT bool_t toggl_set_db_path(
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

    // Allow overriding the server in production

    TOGGL_EXPORT void toggl_set_staging_override(
        bool_t value);

    // To request what server is set up in the library

    TOGGL_EXPORT TogglServerType toggl_get_server_type();

    // Ignoring SSL verification can be turned on in the UI
    TOGGL_EXPORT bool_t toggl_set_settings_ignore_cert(
        void *context,
        const bool_t ignore);

    // Various parts of UI can tell the app to show itself.

    TOGGL_EXPORT void toggl_show_app(
        void *context);

    // Configure the UI callbacks. Required.

    TOGGL_EXPORT void toggl_on_show_app(
        void *context,
        TogglDisplayApp cb);

    TOGGL_EXPORT void toggl_on_sync_state(
        void *context,
        TogglDisplaySyncState cb);

    TOGGL_EXPORT void toggl_on_unsynced_items(
        void *context,
        TogglDisplayUnsyncedItems cb);

    TOGGL_EXPORT void toggl_on_error(
        void *context,
        TogglDisplayError cb);

    TOGGL_EXPORT void toggl_on_overlay(
        void *context,
        TogglDisplayOverlay cb);

    TOGGL_EXPORT void toggl_on_update(
        void *context,
        TogglDisplayUpdate cb);

    TOGGL_EXPORT void toggl_on_update_download_state(
        void *context,
        TogglDisplayUpdateDownloadState cb);

    TOGGL_EXPORT void toggl_on_message(
        void *context,
        TogglDisplayMessage cb);

    TOGGL_EXPORT void toggl_on_online_state(
        void *context,
        TogglDisplayOnlineState cb);

    TOGGL_EXPORT void toggl_on_url(
        void *context,
        TogglDisplayURL cb);

    TOGGL_EXPORT void toggl_on_login(
        void *context,
        TogglDisplayLogin cb);

    TOGGL_EXPORT void toggl_on_reminder(
        void *context,
        TogglDisplayReminder cb);

    TOGGL_EXPORT void toggl_on_pomodoro(
        void *context,
        TogglDisplayPomodoro cb);

    TOGGL_EXPORT void toggl_on_pomodoro_break(
        void *context,
        TogglDisplayPomodoroBreak cb);

    TOGGL_EXPORT void toggl_on_autotracker_notification(
        void *context,
        TogglDisplayAutotrackerNotification cb);

    TOGGL_EXPORT void toggl_on_time_entry_list(
        void *context,
        TogglDisplayTimeEntryList cb);

    TOGGL_EXPORT void toggl_toggle_entries_group(
        void *context,
        const char_t *name);

    TOGGL_EXPORT void toggl_on_timeline(
        void *context,
        TogglDisplayTimeline cb);

    TOGGL_EXPORT void toggl_on_mini_timer_autocomplete(
        void *context,
        TogglDisplayAutocomplete cb);

    TOGGL_EXPORT void toggl_on_help_articles(
        void *context,
        TogglDisplayHelpArticles cb);

    TOGGL_EXPORT void toggl_on_time_entry_autocomplete(
        void *context,
        TogglDisplayAutocomplete cb);

    TOGGL_EXPORT void toggl_on_project_autocomplete(
        void *context,
        TogglDisplayAutocomplete cb);

    TOGGL_EXPORT void toggl_on_workspace_select(
        void *context,
        TogglDisplayViewItems cb);

    TOGGL_EXPORT void toggl_on_client_select(
        void *context,
        TogglDisplayViewItems cb);

    TOGGL_EXPORT void toggl_on_tags(
        void *context,
        TogglDisplayViewItems cb);

    TOGGL_EXPORT void toggl_on_time_entry_editor(
        void *context,
        TogglDisplayTimeEntryEditor cb);

    TOGGL_EXPORT void toggl_on_settings(
        void *context,
        TogglDisplaySettings cb);

    TOGGL_EXPORT void toggl_on_timer_state(
        void *context,
        TogglDisplayTimerState cb);

    TOGGL_EXPORT void toggl_on_idle_notification(
        void *context,
        TogglDisplayIdleNotification cb);

    TOGGL_EXPORT void toggl_on_autotracker_rules(
        void *context,
        TogglDisplayAutotrackerRules cb);

    TOGGL_EXPORT void toggl_on_project_colors(
        void *context,
        TogglDisplayProjectColors cb);

    TOGGL_EXPORT void toggl_on_countries(
        void *context,
        TogglDisplayCountries cb);

    TOGGL_EXPORT void toggl_on_promotion(
        void *context,
        TogglDisplayPromotion cb);

    // After UI callbacks are configured, start pumping UI events

    TOGGL_EXPORT bool_t toggl_ui_start(
        void *context);

    // User interaction with the app

    TOGGL_EXPORT bool_t toggl_login(
        void *context,
        const char_t *email,
        const char_t *password);

    TOGGL_EXPORT bool_t toggl_login_async(
        void *context,
        const char_t *email,
        const char_t *password);

    TOGGL_EXPORT bool_t toggl_signup(
        void *context,
        const char_t *email,
        const char_t *password,
        const uint64_t country_id);

    TOGGL_EXPORT bool_t toggl_signup_async(
        void *context,
        const char_t *email,
        const char_t *password,
        const uint64_t country_id);

    TOGGL_EXPORT bool_t toggl_google_signup(
        void *context,
        const char_t *access_token,
        const uint64_t country_id);

    TOGGL_EXPORT bool_t toggl_google_signup_async(
        void *context,
        const char_t *access_token,
        const uint64_t country_id);

    TOGGL_EXPORT bool_t toggl_google_login(
        void *context,
        const char_t *access_token);

    TOGGL_EXPORT bool_t toggl_google_login_async(
        void *context,
        const char_t *access_token);

    TOGGL_EXPORT bool_t toggl_apple_login(
        void *context,
        const char_t *access_token);

    TOGGL_EXPORT bool_t toggl_apple_login_async(
        void *context,
        const char_t *access_token);

    TOGGL_EXPORT bool_t toggl_apple_signup(
        void *context,
        const char_t *access_token,
        const uint64_t country_id,
        const char_t *full_name);

    TOGGL_EXPORT bool_t toggl_apple_signup_async(
        void *context,
        const char_t *access_token,
        const uint64_t country_id,
        const char_t *full_name);

    TOGGL_EXPORT void toggl_password_forgot(
        void *context);

    TOGGL_EXPORT void toggl_tos(
        void *context);

    TOGGL_EXPORT void toggl_privacy_policy(
        void *context);

    TOGGL_EXPORT void toggl_open_in_browser(
        void *context);

    TOGGL_EXPORT bool_t toggl_accept_tos(
        void *context);

    TOGGL_EXPORT void toggl_get_support(
        void *context,
        const int type);

    TOGGL_EXPORT bool_t toggl_feedback_send(
        void *context,
        const char_t *topic,
        const char_t *details,
        const char_t *filename);

    TOGGL_EXPORT void toggl_search_help_articles(
        void *context,
        const char_t *keywords);

    TOGGL_EXPORT void toggl_view_time_entry_list(
        void *context);

    TOGGL_EXPORT void toggl_view_timeline_data(
        void *context);

    TOGGL_EXPORT void toggl_view_timeline_prev_day(
        void *context);

    TOGGL_EXPORT void toggl_view_timeline_next_day(
        void *context);

    TOGGL_EXPORT void toggl_view_timeline_current_day(
        void *context);

    TOGGL_EXPORT void toggl_view_timeline_set_day(
        void *context,
        const int64_t unix_timestamp);

    TOGGL_EXPORT void toggl_edit(
        void *context,
        const char_t *guid,
        const bool_t edit_running_time_entry,
        const char_t *focused_field_name);

    TOGGL_EXPORT void toggl_edit_preferences(
        void *context);

    TOGGL_EXPORT bool_t toggl_continue(
        void *context,
        const char_t *guid);

    TOGGL_EXPORT bool_t toggl_continue_latest(
        void *context,
        const bool_t prevent_on_app);

    TOGGL_EXPORT bool_t toggl_delete_time_entry(
        void *context,
        const char_t *guid);

    TOGGL_EXPORT bool_t toggl_set_time_entry_duration(
        void *context,
        const char_t *guid,
        const char_t *value);

    TOGGL_EXPORT bool_t toggl_set_time_entry_project(
        void *context,
        const char_t *guid,
        const uint64_t task_id,
        const uint64_t project_id,
        const char_t *project_guid);

    TOGGL_EXPORT bool_t toggl_set_time_entry_date(
        void *context,
        const char_t *guid,
        const int64_t unix_timestamp);

    TOGGL_EXPORT bool_t toggl_set_time_entry_start(
        void *context,
        const char_t *guid,
        const char_t *value);

    TOGGL_EXPORT bool_t toggl_set_time_entry_start_timestamp(
        void *context,
        const char_t *guid,
        const int64_t start);

    TOGGL_EXPORT bool_t toggl_set_time_entry_start_timestamp_with_option(
        void *context,
        const char_t *guid,
        const int64_t start,
        const bool_t keep_end_time_fixed);

    TOGGL_EXPORT bool_t toggl_set_time_entry_end(
        void *context,
        const char_t *guid,
        const char_t *value);

    TOGGL_EXPORT bool_t toggl_set_time_entry_end_timestamp(
        void *context,
        const char_t *guid,
        const int64_t end);

    // value is '\t' separated tag list
    TOGGL_EXPORT bool_t toggl_set_time_entry_tags(
        void *context,
        const char_t *guid,
        const char_t *value);

    TOGGL_EXPORT bool_t toggl_set_time_entry_billable(
        void *context,
        const char_t *guid,
        bool_t value);

    TOGGL_EXPORT bool_t toggl_set_time_entry_description(
        void *context,
        const char_t *guid,
        const char_t *value);

    TOGGL_EXPORT bool_t toggl_stop(
        void *context,
        const bool_t prevent_on_app);

    TOGGL_EXPORT bool_t toggl_discard_time_at(
        void *context,
        const char_t *guid,
        const int64_t at,
        const bool_t split_into_new_entry);

    TOGGL_EXPORT bool_t toggl_discard_time_and_continue(
        void *context,
        const char_t *guid,
        const int64_t at);

    TOGGL_EXPORT bool_t toggl_can_see_billable(
        void *context,
        const int64_t workspaceID);

    TOGGL_EXPORT void toggl_fetch_tags(
        void *context,
        const int64_t workspaceID);

    TOGGL_EXPORT bool_t toggl_set_settings_remind_days(
        void *context,
        const bool_t remind_mon,
        const bool_t remind_tue,
        const bool_t remind_wed,
        const bool_t remind_thu,
        const bool_t remind_fri,
        const bool_t remind_sat,
        const bool_t remind_sun);

    TOGGL_EXPORT bool_t toggl_set_settings_remind_times(
        void *context,
        const char_t *remind_starts,
        const char_t *remind_ends);

    TOGGL_EXPORT bool_t toggl_set_settings_use_idle_detection(
        void *context,
        const bool_t use_idle_detection);

    TOGGL_EXPORT bool_t toggl_set_settings_autotrack(
        void *context,
        const bool_t value);

    TOGGL_EXPORT bool_t toggl_set_settings_open_editor_on_shortcut(
        void *context,
        const bool_t value);

    TOGGL_EXPORT bool_t toggl_set_settings_autodetect_proxy(
        void *context,
        const bool_t autodetect_proxy);

    TOGGL_EXPORT bool_t toggl_set_settings_menubar_timer(
        void *context,
        const bool_t menubar_timer);

    TOGGL_EXPORT bool_t toggl_set_settings_menubar_project(
        void *context,
        const bool_t menubar_project);

    TOGGL_EXPORT bool_t toggl_set_settings_dock_icon(
        void *context,
        const bool_t dock_icon);

    TOGGL_EXPORT bool_t toggl_set_settings_on_top(
        void *context,
        const bool_t on_top);

    TOGGL_EXPORT bool_t toggl_set_settings_reminder(
        void *context,
        const bool_t reminder);

    TOGGL_EXPORT bool_t toggl_set_settings_pomodoro(
        void *context,
        const bool_t pomodoro);

    TOGGL_EXPORT bool_t toggl_set_settings_pomodoro_break(
        void *context,
        const bool_t pomodoro_break);

    TOGGL_EXPORT bool_t toggl_set_settings_stop_entry_on_shutdown_sleep(
        void *context,
        const bool_t stop_entry);

    TOGGL_EXPORT bool_t toggl_set_settings_show_touch_bar(
        void *context,
        const bool_t show_touch_bar);

    TOGGL_EXPORT bool_t toggl_set_settings_start_autotracker_without_suggestions(
        void *context,
        const bool_t start_autotracker_without_suggestions);

    TOGGL_EXPORT bool_t toggl_set_settings_active_tab(
        void *context,
        const uint8_t active_tab);

    TOGGL_EXPORT bool_t toggl_set_settings_color_theme(
        void *context,
        const uint8_t color_theme);

    TOGGL_EXPORT bool_t toggl_set_settings_idle_minutes(
        void *context,
        const uint64_t idle_minutes);

    TOGGL_EXPORT bool_t toggl_set_settings_focus_on_shortcut(
        void *context,
        const bool_t focus_on_shortcut);

    TOGGL_EXPORT bool_t toggl_set_settings_reminder_minutes(
        void *context,
        const uint64_t reminder_minutes);

    TOGGL_EXPORT bool_t toggl_set_settings_pomodoro_minutes(
        void *context,
        const uint64_t pomodoro_minutes);

    TOGGL_EXPORT bool_t toggl_set_settings_pomodoro_break_minutes(
        void *context,
        const uint64_t pomodoro_break_minutes);

    TOGGL_EXPORT bool_t toggl_set_settings_manual_mode(
        void *context,
        const bool_t manual_mode);

    TOGGL_EXPORT bool_t toggl_set_proxy_settings(
        void *context,
        const bool_t use_proxy,
        const char_t *proxy_host,
        const uint64_t proxy_port,
        const char_t *proxy_username,
        const char_t *proxy_password);

    TOGGL_EXPORT bool_t toggl_set_window_settings(
        void *context,
        const int64_t window_x,
        const int64_t window_y,
        const int64_t window_height,
        const int64_t window_width);

    TOGGL_EXPORT bool_t toggl_window_settings(
        void *context,
        int64_t *window_x,
        int64_t *window_y,
        int64_t *window_height,
        int64_t *window_width);

    TOGGL_EXPORT void toggl_set_window_maximized(
        void *context,
        const bool_t value);

    TOGGL_EXPORT bool_t toggl_get_window_maximized(
        void *context);

    TOGGL_EXPORT void toggl_set_window_minimized(
        void *context,
        const bool_t value);

    TOGGL_EXPORT bool_t toggl_get_window_minimized(
        void *context);

    TOGGL_EXPORT void toggl_set_window_edit_size_height(
        void *context,
        const int64_t value);

    TOGGL_EXPORT int64_t toggl_get_window_edit_size_height(
        void *context);

    TOGGL_EXPORT void toggl_set_window_edit_size_width(
        void *context,
        const int64_t value);

    TOGGL_EXPORT int64_t toggl_get_window_edit_size_width(
        void *context);

    TOGGL_EXPORT void toggl_set_key_start(
        void *context,
        const char_t *value);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_get_key_start(
        void *context);

    TOGGL_EXPORT void toggl_set_key_show(
        void *context,
        const char_t *value);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_get_key_show(
        void *context);

    TOGGL_EXPORT void toggl_set_key_modifier_show(
        void *context,
        const char_t *value);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_get_key_modifier_show(
        void *context);

    TOGGL_EXPORT void toggl_set_key_modifier_start(
        void *context,
        const char_t *value);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_get_key_modifier_start(
        void *context);

    TOGGL_EXPORT bool_t toggl_logout(
        void *context);

    TOGGL_EXPORT bool_t toggl_clear_cache(
        void *context);

    // returns GUID of the started time entry. you must free() the result
    TOGGL_EXPORT char_t *toggl_start_with_current_running(
        void *context,
        const char_t *description,
        const char_t *duration,
        const uint64_t task_id,
        const uint64_t project_id,
        const char_t *project_guid,
        const char_t *tags,
        const bool_t prevent_on_app,
        const uint64_t started,
        const uint64_t ended,
        const bool_t stop_current_running);

    // returns GUID of the started time entry. you must free() the result
    TOGGL_EXPORT char_t *toggl_start(
        void *context,
        const char_t *description,
        const char_t *duration,
        const uint64_t task_id,
        const uint64_t project_id,
        const char_t *project_guid,
        const char_t *tags,
        const bool_t prevent_on_app,
        const uint64_t started,
        const uint64_t ended);

    // Create an Empty Time Entry without stopping the running TE
    TOGGL_EXPORT char_t *toggl_create_empty_time_entry(
        void *context,
        const uint64_t started,
        const uint64_t ended);

    // returns GUID of the new project. you must free() the result
    TOGGL_EXPORT char_t *toggl_add_project(
        void *context,
        const char_t *time_entry_guid,
        const uint64_t workspace_id,
        const uint64_t client_id,
        const char_t *client_guid,
        const char_t *project_name,
        const bool_t is_private,
        const char_t *project_color);

    // returns GUID of the new client. you must free() the result
    TOGGL_EXPORT char_t *toggl_create_client(
        void *context,
        const uint64_t workspace_id,
        const char_t *client_name);

    TOGGL_EXPORT bool_t toggl_set_default_project(
        void *context,
        const uint64_t pid,
        const uint64_t tid);

    TOGGL_EXPORT void toggl_get_project_colors(
        void *context);

    TOGGL_EXPORT void toggl_get_countries(
        void *context);

    TOGGL_EXPORT void toggl_get_countries_async(
        void *context);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_get_default_project_name(
        void *context);

    TOGGL_EXPORT uint64_t toggl_get_default_project_id(
        void *context);

    TOGGL_EXPORT uint64_t toggl_get_default_task_id(
        void *context);

    TOGGL_EXPORT uint64_t toggl_get_default_or_first_workspace_id(
        void *context);

    TOGGL_EXPORT bool_t toggl_set_update_channel(
        void *context,
        const char_t *update_channel);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_get_update_channel(
        void *context);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_get_user_fullname(
        void *context);

    TOGGL_EXPORT uint8_t toggl_get_user_beginning_of_week(
        void *context);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_get_user_email(
        void *context);

    TOGGL_EXPORT void toggl_sync(
        void *context);

    TOGGL_EXPORT void toggl_fullsync(
        void *context);

    TOGGL_EXPORT bool_t toggl_timeline_toggle_recording(
        void *context,
        const bool_t record_timeline);

    TOGGL_EXPORT bool_t toggl_timeline_is_recording_enabled(
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
        const uint64_t idle_seconds);

    TOGGL_EXPORT bool_t toggl_set_promotion_response(
        void *context,
        const int64_t promotion_type,
        const int64_t promotion_response);

    // Shared helpers

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_format_tracking_time_duration(
        const int64_t duration_in_seconds);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_format_tracked_time_duration(
        const int64_t duration_in_seconds);

    // You must free() the result
    TOGGL_EXPORT char_t *toggl_format_time(
        const int64_t time);

    TOGGL_EXPORT int64_t toggl_timestamp_from_time_string(
        const char_t *time);

    TOGGL_EXPORT int64_t toggl_parse_duration_string_into_seconds(
        const char_t *duration_string);

    TOGGL_EXPORT uint8_t toggl_days_of_week_into_uint8(
        const bool_t sun,
        const bool_t mon,
        const bool_t tue,
        const bool_t wed,
        const bool_t thu,
        const bool_t fri,
        const bool_t sat);

    // Write to the lib logger
    TOGGL_EXPORT void toggl_debug(
        const char_t *text);

    // Check if sizeof view struct matches those in UI
    // Else stuff blows up when Marshalling in C#
    // Will return error string if size is invalid,
    // you must free() the result
    TOGGL_EXPORT char_t *toggl_check_view_struct_size(
        const int time_entry_view_item_size,
        const int autocomplete_view_item_size,
        const int view_item_size,
        const int settings_size,
        const int autotracker_view_item_size);

    TOGGL_EXPORT int64_t toggl_autotracker_add_rule(
        void *context,
        const char_t *terms,
        const uint64_t project_id,
        const uint64_t task_id,
        const char_t *start_time,
        const char_t *end_time,
        const uint8_t days_of_week);

    TOGGL_EXPORT bool_t toggl_autotracker_update_rule(
        void *context,
        const int64_t rule_id,
        const char_t *terms,
        const uint64_t project_id,
        const uint64_t task_id,
        const char_t *start_time,
        const char_t *end_time,
        const uint8_t days_of_week);

    TOGGL_EXPORT bool_t toggl_autotracker_delete_rule(
        void *context,
        const int64_t id);

    // Testing helpers. May change any time
    TOGGL_EXPORT void testing_sleep(
        const int seconds);

    // For testing only
    TOGGL_EXPORT bool_t testing_set_logged_in_user(
        void *context,
        const char *json);

    TOGGL_EXPORT void toggl_set_keep_end_time_fixed(
        void *context,
        const bool_t value);

    TOGGL_EXPORT bool_t toggl_get_keep_end_time_fixed(
        void *context);

    TOGGL_EXPORT bool_t toggl_get_show_touch_bar(
        void *context);

    TOGGL_EXPORT uint8_t toggl_get_active_tab(
        void *context);

    TOGGL_EXPORT void toggl_set_mini_timer_x(
        void *context,
        const int64_t value);

    TOGGL_EXPORT int64_t toggl_get_mini_timer_x(
        void *context);

    TOGGL_EXPORT void toggl_set_mini_timer_y(
        void *context,
        const int64_t value);

    TOGGL_EXPORT int64_t toggl_get_mini_timer_y(
        void *context);

    TOGGL_EXPORT void toggl_set_mini_timer_w(
        void *context,
        const int64_t value);

    TOGGL_EXPORT int64_t toggl_get_mini_timer_w(
        void *context);

    TOGGL_EXPORT void toggl_set_mini_timer_visible(
        void *context,
        const bool_t value);

    TOGGL_EXPORT bool_t toggl_get_mini_timer_visible(
        void *context);

    TOGGL_EXPORT void toggl_load_more(
        void *context);

    TOGGL_EXPORT void track_window_size(
        void *context,
        const uint64_t width,
        const uint64_t height);

    TOGGL_EXPORT void track_edit_size(
        void *context,
        const uint64_t width,
        const uint64_t height);

    TOGGL_EXPORT void toggl_iam_click(
        void *context,
        const uint64_t type);

    TOGGL_EXPORT char_t *toggl_format_duration_time(
        void *context,
        const uint64_t timestamp);

    TOGGL_EXPORT void track_collapse_day(
        void *context);

    TOGGL_EXPORT void track_expand_day(
        void *context);

    TOGGL_EXPORT void track_collapse_all_days(
        void *context);

    TOGGL_EXPORT void track_expand_all_days(
        void *context);

    TOGGL_EXPORT void track_timer_edit(
        void *context,
        TimerEditActionType action);

    TOGGL_EXPORT void track_timer_start(
        void *context,
        TimerEditActionType actions);

    TOGGL_EXPORT void track_timer_shortcut(
        void *context,
        TimerShortcutActionType action);

    TOGGL_EXPORT bool_t toggl_update_time_entry(
        void *context,
        const char_t *guid,
        const char_t *description,
        const uint64_t task_id,
        const uint64_t project_id,
        const char_t *project_guid,
        const char_t *tags,
        const bool_t billable);

    TOGGL_EXPORT void toggl_on_onboarding(
        void *context,
        TogglDisplayOnboarding cb);

    TOGGL_EXPORT void toggl_user_did_click_on_timeline_tab(
        void *context);

    TOGGL_EXPORT void toggl_user_did_turn_on_record_activity(
        void *context);

    TOGGL_EXPORT void toggl_user_did_edit_add_timeentry_on_timeline_view(
        void *context);

    TOGGL_EXPORT void toggl_on_continue_sign_in(
        void *context,
        TogglContinueSignIn cb);

    TOGGL_EXPORT void toggl_on_display_login_sso(
        void *context,
        TogglDisplayLoginSSO cb);

    TOGGL_EXPORT TogglHsvColor toggl_get_adaptive_hsv_color(
       TogglRgbColor rgbColor,
       TogglAdaptiveColor type);

    TOGGL_EXPORT bool_t toggl_get_identity_provider_sso(
        void *context,
        const char_t *email);

    TOGGL_EXPORT void toggl_set_need_enable_SSO(
        void *context,
        const char_t *code);

    TOGGL_EXPORT void toggl_reset_enable_SSO(
        void *context);

    TOGGL_EXPORT void toggl_login_sso(
        void *context,
        const char_t *api_token);
    
    TOGGL_EXPORT void toggl_track_timeline_menu_context(
        void *context,
        TimelineMenuContextType menuType);

    TOGGL_EXPORT TogglRgbColor toggl_get_adaptive_rgb_color_from_hex(
        const char_t *hexColor,
        TogglAdaptiveColor type);

    TOGGL_EXPORT void toggl_on_timeline_ui_enabled(
        void *context,
        TogglDisplayTimelineUI cb);

#undef TOGGL_EXPORT

#ifdef __cplusplus
}
#endif

#endif  // SRC_TOGGL_API_H_
