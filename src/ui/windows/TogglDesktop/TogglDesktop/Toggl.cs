using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Input;
using TogglDesktop.Diagnostics;
using TogglDesktop.Services;

// ReSharper disable InconsistentNaming

namespace TogglDesktop
{
public static partial class Toggl
{
    #region constants and static fields

    public const string Project = "project";
    public const string Duration = "duration";
    public const string Description = "description";

    public const string TagSeparator = "\t";

    private static readonly DateTimeOffset UnixEpoch =
        new DateTimeOffset(1970, 1, 1, 0, 0, 0, TimeSpan.Zero);

    private static IntPtr ctx = IntPtr.Zero;

    private static MainWindow mainWindow;

    // User can override some parameters when running the app
    public static string ScriptPath;
    public static string DatabasePath;
    public static string LogPath;

    public static readonly string WritableAppDirPath =
        Path.Combine(
            Environment.GetFolderPath(
            Environment.SpecialFolder.LocalApplicationData), "TogglDesktop");

    public static readonly string UpdatesPath = Path.Combine(WritableAppDirPath, "updates");

#if TOGGL_PRODUCTION_BUILD
    public static string Env = "production";
#else
    public static string Env = "development";
#endif

    #endregion

    #region enums

    public enum OnlineState
    {
        Online = kOnlineStateOnline,
        NoNetwork = kOnlineStateNoNetwork,
        BackendDown = kOnlineStateBackendDown
    }

    public enum SyncState
    {
        Idle = kSyncStateIdle,
        Syncing = kSyncStateWork
    }

    public enum DownloadStatus
    {
        Started = kDownloadStatusStarted,
        Done = kDownloadStatusDone
    }

    #endregion

    #region api calls

    public static void LoadMore()
    {
        toggl_load_more(ctx);
    }

    public static void SendObmAction(ulong experiment, string key)
    {
        toggl_add_obm_action(ctx, experiment, key, "1");
    }

    public static void Clear()
    {
        toggl_context_clear(ctx);
    }

    public static void ShowApp()
    {
        toggl_show_app(ctx);
    }

    public static void Debug(string text)
    {
        toggl_debug(text);
    }
    #region Debug overloads
    public static void Debug(string text, object arg0)
    {
        toggl_debug(string.Format(text, arg0));
    }
    public static void Debug(string text, object arg0, object arg1)
    {
        toggl_debug(string.Format(text, arg0, arg1));
    }
    public static void Debug(string text, object arg0, object arg1, object arg2)
    {
        toggl_debug(string.Format(text, arg0, arg1, arg2));
    }
    public static void Debug(string text, params object[] args)
    {
        toggl_debug(string.Format(text, args));
    }
    #endregion

    public static bool Signup(string email, string password, long country_id)
    {
        return toggl_signup(ctx, email, password, Convert.ToUInt64(country_id));
    }

    public static bool Login(string email, string password, long country_id)
    {
        return toggl_login(ctx, email, password);
    }

    public static bool GoogleSignup(string access_token, long country_id)
    {
        return toggl_google_signup(ctx, access_token, Convert.ToUInt64(country_id));
    }

    public static bool GoogleLogin(string access_token)
    {
        return toggl_google_login(ctx, access_token);
    }

    public static void PasswordForgot()
    {
        toggl_password_forgot(ctx);
    }

    public static void OpenInBrowser()
    {
        toggl_open_in_browser(ctx);
    }

    public static void AcceptToS()
    {
        toggl_accept_tos(ctx);
    }

    public static void OpenToS()
    {
        toggl_tos(ctx);
    }

    public static void OpenPrivacy()
    {
        toggl_privacy_policy(ctx);
    }

    public static bool SendFeedback(
        string topic,
        string details,
        string filename)
    {
        return toggl_feedback_send(ctx, topic, details, filename);
    }

    public static void ViewTimeEntryList()
    {
        toggl_view_time_entry_list(ctx);
    }

    public static void Edit(
        string guid,
        bool edit_running_time_entry,
        string focused_field_name)
    {
        toggl_edit(ctx, guid, edit_running_time_entry, focused_field_name);
    }

    public static void EditPreferences()
    {
        toggl_edit_preferences(ctx);
    }

    public static bool Continue(string guid)
    {
        OnUserTimeEntryStart();
        return toggl_continue(ctx, guid);
    }

    public static bool ContinueLatest(bool preventOnApp = false)
    {
        OnUserTimeEntryStart();
        return toggl_continue_latest(ctx, preventOnApp);
    }

    public static bool DeleteTimeEntry(string guid)
    {
        return toggl_delete_time_entry(ctx, guid);
    }

    #region changing time entry

    public static bool SetTimeEntryDuration(string guid, string value)
    {
        using (Performance.Measure("changing time entry duration"))
        {
            return toggl_set_time_entry_duration(ctx, guid, value);
        }
    }

    public static bool SetTimeEntryProject(
        string guid,
        UInt64 task_id,
        UInt64 project_id,
        string project_guid)
    {
        using (Performance.Measure("changing time entry project"))
        {
            return toggl_set_time_entry_project(ctx,
                                                guid, task_id, project_id, project_guid);
        }
    }

    public static bool SetTimeEntryStart(string guid, string value)
    {
        using (Performance.Measure("changing time entry start time"))
        {
            return toggl_set_time_entry_start(ctx, guid, value);
        }
    }

    public static bool SetTimeEntryDate(string guid, DateTime value)
    {
        using (Performance.Measure("changing time entry date"))
        {
            return toggl_set_time_entry_date(ctx, guid, UnixFromDateTime(value));
        }
    }

    public static bool SetTimeEntryEnd(string guid, string value)
    {
        using (Performance.Measure("changing time entry end time"))
        {
            return toggl_set_time_entry_end(ctx, guid, value);
        }
    }

    public static bool SetTimeEntryTags(string guid, List<string> tags)
    {
        using (Performance.Measure("changing time entry tags, count: {0}", tags.Count))
        {
            string value = String.Join(TagSeparator, tags);
            return toggl_set_time_entry_tags(ctx, guid, value);
        }
    }

    public static bool SetTimeEntryBillable(string guid, bool billable)
    {
        using (Performance.Measure("changing time entry billable"))
        {
            return toggl_set_time_entry_billable(ctx, guid, billable);
        }
    }

    public static bool SetTimeEntryDescription(string guid, string value)
    {
        using (Performance.Measure("changing time entry description"))
        {
            return toggl_set_time_entry_description(ctx, guid, value);
        }
    }

    #endregion

    public static bool Stop(bool preventOnApp = false)
    {
        return toggl_stop(ctx, preventOnApp);
    }

    public static bool DiscardTimeAt(string guid, Int64 at, bool split)
    {
        return toggl_discard_time_at(ctx, guid, at, split);
    }

    public static bool DiscardTimeAndContinue(string guid, Int64 at, bool split)
    {
        return toggl_discard_time_and_continue(ctx, guid, at);
    }

    public static bool SetSettings(TogglSettingsView settings)
    {
        if (!toggl_set_settings_use_idle_detection(ctx,
                settings.UseIdleDetection)) {
            return false;
        }

        if (!toggl_set_settings_on_top(ctx,
                                       settings.OnTop)) {
            return false;
        }

        if (!toggl_set_settings_reminder(ctx,
                                         settings.Reminder)) {
            return false;
        }

        if (!toggl_set_settings_idle_minutes(ctx,
                                             (ulong)settings.IdleMinutes)) {
            return false;
        }

        if (!toggl_set_settings_reminder_minutes(ctx,
                (ulong)settings.ReminderMinutes)) {
            return false;
        }

        if (!toggl_set_settings_focus_on_shortcut(ctx,
                settings.FocusOnShortcut)) {
            return false;
        }

        if (!toggl_set_settings_autodetect_proxy(ctx,
                settings.AutodetectProxy))
        {
            return false;
        }

        if (!toggl_set_proxy_settings(
            ctx,
            settings.UseProxy,
            settings.ProxyHost,
            settings.ProxyPort,
            settings.ProxyUsername,
            settings.ProxyPassword))
        {
            return false;
        }
        if (!toggl_set_settings_remind_days(
            ctx,
            settings.RemindMon,
            settings.RemindTue,
            settings.RemindWed,
            settings.RemindThu,
            settings.RemindFri,
            settings.RemindSat,
            settings.RemindSun
                ))
        {
            return false;
        }
        if (!toggl_set_settings_remind_times(
            ctx,
            settings.RemindStarts,
            settings.RemindEnds
                ))
        {
            return false;
        }

        if (!toggl_set_settings_autotrack(ctx, settings.Autotrack))
        {
            return false;
        }

        if (!toggl_set_settings_pomodoro(ctx, settings.Pomodoro))
        {
            return false;
        }

        if (!toggl_set_settings_pomodoro_minutes(ctx, (ulong)settings.PomodoroMinutes))
        {
            return false;
        }

        if (!toggl_set_settings_pomodoro_break(ctx, settings.PomodoroBreak))
        {
            return false;
        }

        if (!toggl_set_settings_pomodoro_break_minutes(ctx, (ulong)settings.PomodoroBreakMinutes))
        {
            return false;
        }

        if (!toggl_set_settings_stop_entry_on_shutdown_sleep(ctx, settings.StopEntryOnShutdownSleep))
        {
            return false;
        }

        if (!toggl_set_settings_color_theme(ctx, settings.ColorTheme))
        {
            return false;
        }

        return toggl_timeline_toggle_recording(ctx, settings.RecordTimeline);
    }

    public static bool IsTimelineRecordingEnabled()
    {
        return toggl_timeline_is_recording_enabled(ctx);
    }

    public static bool Logout()
    {
        return toggl_logout(ctx);
    }

    public static bool ClearCache()
    {
        return toggl_clear_cache(ctx);
    }

    public static bool SetLoggedInUser(string json) {
        return testing_set_logged_in_user(ctx, json);
    }

    public static string Start(
        string description,
        string duration,
        UInt64 task_id,
        UInt64 project_id,
        string project_guid,
        string tags,
        bool preventOnApp = false)
    {
        OnUserTimeEntryStart();

        return toggl_start(ctx,
                           description,
                           duration,
                           task_id,
                           project_id,
                           project_guid,
                           tags,
                           preventOnApp,
                           0,
                           0);
    }

    public static string AddProject(
        string time_entry_guid,
        UInt64 workspace_id,
        UInt64 client_id,
        string client_guid,
        string project_name,
        bool is_private,
        string color)
    {
        using (Performance.Measure("adding project"))
        {
            return toggl_add_project(ctx,
                                     time_entry_guid,
                                     workspace_id,
                                     client_id,
                                     client_guid,
                                     project_name,
                                     is_private,
                                     color);
        }
    }

    public static string CreateClient(
        UInt64 workspace_id,
        string client_name)
    {
        using (Performance.Measure("adding client"))
        {
            return toggl_create_client(ctx,
                                       workspace_id,
                                       client_name);
        }
    }

    public static bool SetUpdateChannel(string channel)
    {
        return toggl_set_update_channel(ctx, channel);
    }

    public static string UpdateChannel()
    {
        return toggl_get_update_channel(ctx);
    }

    public static string UserEmail()
    {
        return toggl_get_user_email(ctx);
    }

    public static void Sync()
    {
        OnManualSync();
        toggl_fullsync(ctx);
    }

    public static void SetSleep()
    {
        toggl_set_sleep(ctx);
    }

    public static void SetLocked()
    {
        toggl_set_locked(ctx);
    }

    public static void SetUnlocked()
    {
        toggl_set_unlocked(ctx);
    }

    public static void SetOSShutdown()
    {
        toggl_os_shutdown(ctx);
    }

    public static void TrackWindowSize(Size size)
    {
        track_window_size(ctx, (ulong)size.Width, (ulong)size.Height);
    }

    public static void SetWake()
    {
        toggl_set_wake(ctx);
    }

    public static void SetIdleSeconds(UInt64 idle_seconds)
    {
        toggl_set_idle_seconds(ctx, idle_seconds);
    }

    public static string FormatDurationInSecondsHHMMSS(Int64 duration_in_seconds)
    {
        return toggl_format_tracking_time_duration(duration_in_seconds);
    }

    public static long AddAutotrackerRule(string term, ulong projectId, ulong taskId)
    {
        return toggl_autotracker_add_rule(ctx, term, projectId, taskId);
    }

    public static bool DeleteAutotrackerRule(long id)
    {
        return toggl_autotracker_delete_rule(ctx, id);
    }

    public static bool SetDefaultProject(ulong projectId, ulong taskId)
    {
        return toggl_set_default_project(ctx, projectId, taskId);
    }

    public static string GetDefaultProjectName()
    {
        return toggl_get_default_project_name(ctx);
    }

    public static ulong GetDefaultProjectId()
    {
        return toggl_get_default_project_id(ctx);
    }

    public static ulong GetDefaultTaskId()
    {
        return toggl_get_default_task_id(ctx);
    }

    public static void GetProjectColors()
    {
        toggl_get_project_colors(ctx);
    }

    public static void GetCountries()
    {
        toggl_get_countries(ctx);
    }

    public static void SetKeepEndTimeFixed(bool b)
    {
        toggl_set_keep_end_time_fixed(ctx, b);
    }

    public static bool GetKeepEndTimeFixed()
    {
        return toggl_get_keep_end_time_fixed(ctx);
    }

    public static void SetMiniTimerX(long x)
    {
        toggl_set_mini_timer_x(ctx, x);
    }

    public static long GetMiniTimerX()
    {
        return toggl_get_mini_timer_x(ctx);
    }

    public static void SetMiniTimerY(long y)
    {
        toggl_set_mini_timer_y(ctx, y);
    }

    public static long GetMiniTimerY()
    {
        return toggl_get_mini_timer_y(ctx);
    }

    public static void SetMiniTimerW(long w)
    {
        toggl_set_mini_timer_w(ctx, w);
    }

    public static long GetMiniTimerW()
    {
        return toggl_get_mini_timer_w(ctx);
    }

    public static void SetMiniTimerVisible(bool visible)
    {
        toggl_set_mini_timer_visible(ctx, visible);
    }

    public static bool GetMiniTimerVisible()
    {
        return toggl_get_mini_timer_visible(ctx);
    }

    public static void TrackClickCloseButtonInAppMessage()
    {
        toggl_iam_click(ctx, 2);
    }

    public static void TrackClickActionButtonInAppMessage()
    {
        toggl_iam_click(ctx, 3);
    }

    public static void TrackCollapseDay()
    {
        track_collapse_day(ctx);
    }

    public static void TrackExpandDay()
    {
        track_expand_day(ctx);
    }

    public static void TrackCollapseAllDays()
    {
        track_collapse_all_days(ctx);
    }

    public static void TrackExpandAllDays()
    {
        track_expand_all_days(ctx);
    }

    #endregion

    #region library events as observables

    private static readonly Subject<bool> _onApp = new Subject<bool>();
    public static readonly IObservable<bool> OnApp = _onApp.AsObservable();

    private static readonly Subject<long> _onOverlay = new Subject<long>();
    public static readonly IObservable<long> OnOverlay = _onOverlay.AsObservable();

    private static readonly Subject<(string, bool)> _onError = new Subject<(string,bool)>();
    public static readonly IObservable<(string errorMessage, bool isUserError)> OnError = _onError.AsObservable();

    private static readonly Subject<OnlineState> _onOnlineState = new Subject<OnlineState>();
    public static readonly IObservable<OnlineState> OnOnlineState = _onOnlineState.AsObservable();

    private static readonly Subject<(bool, ulong)> _onLogin = new Subject<(bool, ulong)>();
    public static readonly IObservable<(bool open, ulong userId)> OnLogin = _onLogin.AsObservable();

    private static readonly Subject<(string, string)> _onReminder = new Subject<(string, string)>();
    public static readonly IObservable<(string title, string informativeText)> OnReminder = _onReminder.AsObservable();

    private static readonly Subject<(bool, List<TogglTimeEntryView>, bool)> _onTimeEntryList = new Subject<(bool, List<TogglTimeEntryView>, bool)>();
    public static readonly IObservable<(bool open, List<TogglTimeEntryView> list, bool showLoadMore)> OnTimeEntryList =_onTimeEntryList.AsObservable();

    private static readonly Subject<List<TogglAutocompleteView>> _onTimeEntryAutocomplete = new Subject<List<TogglAutocompleteView>>();
    public static readonly IObservable<List<TogglAutocompleteView>> OnTimeEntryAutocomplete = _onTimeEntryAutocomplete.AsObservable();

    private static readonly Subject<List<TogglAutocompleteView>> _onMinitimerAutocomplete = new Subject<List<TogglAutocompleteView>>();
    public static readonly IObservable<List<TogglAutocompleteView>> OnMinitimerAutocomplete = _onMinitimerAutocomplete.AsObservable();

    private static readonly Subject<List<TogglAutocompleteView>> _onProjectAutocomplete = new Subject<List<TogglAutocompleteView>>();
    public static readonly IObservable<List<TogglAutocompleteView>> OnProjectAutocomplete = _onProjectAutocomplete.AsObservable();

    private static readonly Subject<(bool, TogglTimeEntryView, string)> _onTimeEntryEditor = new Subject<(bool, TogglTimeEntryView, string)>();
    public static readonly IObservable<(bool open, TogglTimeEntryView timeEntry, string focusedFieldName)>
        OnTimeEntryEditor = _onTimeEntryEditor.AsObservable();

    private static readonly Subject<List<TogglGenericView>> _onWorkspaceSelect = new Subject<List<TogglGenericView>>();
    public static readonly IObservable<List<TogglGenericView>> OnWorkspaceSelect = _onWorkspaceSelect.AsObservable();

    private static readonly Subject<List<TogglGenericView>> _onClientSelect = new Subject<List<TogglGenericView>>();
    public static readonly IObservable<List<TogglGenericView>> OnClientSelect = _onClientSelect.AsObservable();

    private static readonly Subject<List<TogglGenericView>> _onTags = new Subject<List<TogglGenericView>>();
    public static readonly IObservable<List<TogglGenericView>> OnTags = _onTags.AsObservable();

    private static readonly Subject<(bool, TogglSettingsView)> _onSettings = new Subject<(bool, TogglSettingsView)>();
    public static readonly IObservable<(bool open, TogglSettingsView settings)> OnSettings = _onSettings.AsObservable();

    private static readonly Subject<TogglTimeEntryView> _onRunningTimerState = new Subject<TogglTimeEntryView>();
    public static readonly IObservable<TogglTimeEntryView> OnRunningTimerState = _onRunningTimerState.AsObservable();

    private static readonly Subject<Unit> _onStoppedTimerState = new Subject<Unit>();
    public static readonly IObservable<Unit> OnStoppedTimerState = _onStoppedTimerState.AsObservable();

    private static readonly Subject<string> _onURL = new Subject<string>();
    public static readonly IObservable<string> OnURL = _onURL.AsObservable();

    private static readonly Subject<(string, string, string, long, string)> _onIdleNotification = new Subject<(string, string, string, long, string)>();
    public static readonly IObservable<(string, string, string, long, string)> OnIdleNotification = _onIdleNotification.AsObservable();

    private static readonly Subject<List<TogglAutotrackerRuleView>> _onAutotrackerRules = new Subject<List<TogglAutotrackerRuleView>>();
    public static readonly IObservable<List<TogglAutotrackerRuleView>> OnAutotrackerRules = _onAutotrackerRules.AsObservable();

    private static readonly Subject<(string, ulong, ulong)> _onAutotrackerNotification = new Subject<(string, ulong, ulong)>();
    public static readonly IObservable<(string projectName, ulong projectId, ulong taskId)> OnAutotrackerNotification =
        _onAutotrackerNotification.AsObservable();

    private static readonly Subject<SyncState> _onDisplaySyncState = new Subject<SyncState>();
    public static readonly IObservable<SyncState> OnDisplaySyncState = _onDisplaySyncState.AsObservable();

    private static readonly Subject<long> _onDisplayUnsyncedItems = new Subject<long>();
    public static readonly IObservable<long> OnDisplayUnsyncedItems = _onDisplayUnsyncedItems.AsObservable();

    public static readonly Subject<UpdateStatus> OnUpdateDownloadStatus = new Subject<UpdateStatus>();

    private static readonly Subject<string[]> _onDisplayProjectColors = new Subject<string[]>();
    public static readonly IObservable<string[]> OnDisplayProjectColors = _onDisplayProjectColors.AsObservable();

    private static readonly Subject<List<TogglCountryView>> _onDisplayCountries = new Subject<List<TogglCountryView>>();
    public static readonly IObservable<List<TogglCountryView>> OnDisplayCountries = _onDisplayCountries.AsObservable();

    private static readonly Subject<long> _onDisplayPromotion = new Subject<long>();
    public static readonly IObservable<long> OnDisplayPromotion = _onDisplayPromotion.AsObservable();

    private static readonly Subject<(ulong, bool, bool)> _onDisplayObmExperiment = new Subject<(ulong, bool, bool)>();
    public static readonly IObservable<(ulong id, bool included, bool seenBefore)> OnDisplayObmExperiment =
        _onDisplayObmExperiment.AsObservable();

    private static readonly Subject<(string title, string informativeText)> _onDisplayPomodoro = new Subject<(string title, string informativeText)>();
    public static readonly IObservable<(string title, string informativeText)> OnDisplayPomodoro =
        _onDisplayPomodoro.AsObservable();

    private static readonly Subject<(string title, string informativeText)> _onDisplayPomodoroBreak = new Subject<(string title, string informativeText)>();
    public static readonly IObservable<(string title, string informativeText)> OnDisplayPomodoroBreak =
        _onDisplayPomodoroBreak.AsObservable();

    private static readonly Subject<(string title, string button, string text, string url)> _onDisplayInAppNotification =
        new Subject<(string title, string button, string text, string url)>();
    public static readonly IObservable<(string title, string button, string text, string url)> OnDisplayInAppNotification =
        _onDisplayInAppNotification.AsObservable();

    private static void listenToLibEvents()
    {
        toggl_on_show_app(ctx, open =>
        {
            using (Performance.Measure("Calling OnApp"))
            {
                _onApp.OnNext(open);
            }
        });

        toggl_on_overlay(ctx, type =>
        {
            using (Performance.Measure("Calling OnOverlay"))
            {
                _onOverlay.OnNext(type);
            }
        });

        toggl_on_error(ctx, (errmsg, user_error) =>
        {
            using (Performance.Measure("Calling OnError, user_error: {1}, message: {0}", errmsg, user_error))
            {
                _onError.OnNext((errmsg, user_error));
            }
        });

        toggl_on_sync_state(ctx, state =>
        {
            using (Performance.Measure("Calling OnDisplaySyncState, state: {0}", state))
            {
                _onDisplaySyncState.OnNext((SyncState)state);
            }
        });
        toggl_on_unsynced_items(ctx, count =>
        {
            using (Performance.Measure("Calling OnDisplayUnsyncedItems, count: {0}", count))
            {
                _onDisplayUnsyncedItems.OnNext(count);
            }
        });

        toggl_on_online_state(ctx, state =>
        {
            using (Performance.Measure("Calling OnOnlineState, state: {0}", state))
            {
                _onOnlineState.OnNext((OnlineState)state);
            }
        });

        toggl_on_login(ctx, (open, user_id) =>
        {
            using (Performance.Measure("Calling OnLogin"))
            {
                _onLogin.OnNext((open, user_id));
            }
        });

        toggl_on_reminder(ctx, (title, informative_text) =>
        {
            using (Performance.Measure("Calling OnReminder, title: {0}", title))
            {
                _onReminder.OnNext((title, informative_text));
            }
        });

        toggl_on_time_entry_list(ctx, (open, first, show_load_more_button) =>
        {
            using (Performance.Measure("Calling OnTimeEntryList, open: {0}", open))
            {
                _onTimeEntryList.OnNext((open, convertToTimeEntryList(first), show_load_more_button));
            }
        });

        toggl_on_time_entry_autocomplete(ctx, first =>
        {
            using (Performance.Measure("Calling OnTimeEntryAutocomplete"))
            {
                _onTimeEntryAutocomplete.OnNext(convertToAutocompleteList(first));
            }
        });

        toggl_on_mini_timer_autocomplete(ctx, first =>
        {
            using (Performance.Measure("Calling OnMinitimerAutocomplete"))
            {
                _onMinitimerAutocomplete.OnNext(convertToAutocompleteList(first));
            }
        });

        toggl_on_project_autocomplete(ctx, first =>
        {
            using (Performance.Measure("Calling OnProjectAutocomplete"))
            {
                _onProjectAutocomplete.OnNext(convertToAutocompleteList(first));
            }
        });

        toggl_on_time_entry_editor(ctx, (open, te, focused_field_name) =>
        {
            using (Performance.Measure("Calling OnTimeEntryEditor, focused field: {0}", focused_field_name))
            {
                _onTimeEntryEditor.OnNext((open, marshalStruct<TogglTimeEntryView>(te), focused_field_name));
            }
        });

        toggl_on_workspace_select(ctx, first =>
        {
            using (Performance.Measure("Calling OnWorkspaceSelect"))
            {
                _onWorkspaceSelect.OnNext(convertToViewItemList(first));
            }
        });

        toggl_on_client_select(ctx, first =>
        {
            using (Performance.Measure("Calling OnClientSelect"))
            {
                _onClientSelect.OnNext(convertToViewItemList(first));
            }
        });

        toggl_on_tags(ctx, first =>
        {
            using (Performance.Measure("Calling OnTags"))
            {
                _onTags.OnNext(convertToViewItemList(first));
            }
        });

        toggl_on_settings(ctx, (open, settings) =>
        {
            using (Performance.Measure("Calling OnSettings"))
            {
                _onSettings.OnNext((open, marshalStruct<TogglSettingsView>(settings)));
            }
        });

        toggl_on_timer_state(ctx, te =>
        {
            if (te == IntPtr.Zero)
            {
                using (Performance.Measure("Calling OnStoppedTimerState"))
                {
                    _onStoppedTimerState.OnNext(Unit.Default);
                    return;
                }
            }
            using (Performance.Measure("Calling OnRunningTimerState"))
            {
                _onRunningTimerState.OnNext(marshalStruct<TogglTimeEntryView>(te));
            }
        });

        toggl_on_url(ctx, url =>
        {
            using (Performance.Measure("Calling OnURL"))
            {
                _onURL.OnNext(url);
            }
        });

        toggl_on_idle_notification(ctx, (guid, since, duration, started, description) =>
        {
            using (Performance.Measure("Calling OnIdleNotification"))
            {
                _onIdleNotification.OnNext((guid, since, duration, started, description));
            }
        });

        toggl_on_autotracker_rules(ctx, (first, count, list) =>
        {
            using (Performance.Measure("Calling OnAutotrackerRules"))
            {
                _onAutotrackerRules.OnNext(convertToAutotrackerEntryList(first));
            }
        });

        toggl_on_autotracker_notification(ctx, (name, project_id, task_id) =>
        {
            using (Performance.Measure("Calling OnAutotrackerNotification"))
            {
                _onAutotrackerNotification.OnNext((name, project_id, task_id));
            }
        });

        toggl_on_update_download_state(ctx, (version, state) =>
        {
            using (Performance.Measure("Calling OnUpdateDownloadState, v: {0}, state: {1}", version, state))
            {
                OnUpdateDownloadStatus.OnNext(new UpdateStatus(Version.Parse(version), (DownloadStatus)state));
            }
        });

        toggl_on_project_colors(ctx, (colors, count) =>
        {
            using (Performance.Measure("Calling OnProjectColors, count: {0}", count))
            {
                _onDisplayProjectColors.OnNext(colors);
            }
        });

        toggl_on_countries(ctx, (first) =>
        {
            using (Performance.Measure("Calling OnCountries"))
            {
                _onDisplayCountries.OnNext(convertToCountryList(first));
            }
        });

        toggl_on_promotion(ctx, id =>
        {
            using (Performance.Measure("Calling OnDisplayPromotino, id: {0}", id))
            {
                _onDisplayPromotion.OnNext(id);
            }
        });

        toggl_on_obm_experiment(ctx, (id, included, seenBefore) =>
        {
            using (Performance.Measure(
                "Calling OnDisplatObmExperiment, id: {0}, included: {1}, seen: {2}",
                id, included, seenBefore))
            {
                _onDisplayObmExperiment.OnNext((id, included, seenBefore));
            }
        });
        toggl_on_pomodoro(ctx, (title, text) =>
        {
            using (Performance.Measure("Calling OnDisplayPomodoro"))
            {
                _onDisplayPomodoro.OnNext((title, text));
            }
        });
        toggl_on_pomodoro_break(ctx, (title, text) =>
        {
            using (Performance.Measure("Calling OnDisplayPomodoroBreak"))
            {
                _onDisplayPomodoroBreak.OnNext((title, text));
            }
        });
        toggl_on_message(ctx, (title, text, button, url) =>
        {
            using (Performance.Measure("Calling OnDisplayInAppNotification"))
            {
                _onDisplayInAppNotification.OnNext((title, text, button, url));
            }
        });
    }

    #endregion

    #region internal ui events

    public delegate void ManualSync();

    public static event ManualSync OnManualSync = delegate { };

    public delegate void UserTimeEntryStart();

    public static event UserTimeEntryStart OnUserTimeEntryStart = delegate { };

    #endregion

    #region startup

    private static void parseCommandlineParams()
    {
        string[] args = Environment.GetCommandLineArgs();
        for (int i = 0; i < args.Length; i++)
        {
            if (args[i].Contains("script") && args[i].Contains("path"))
            {
                ScriptPath = args[i + 1];
                Console.WriteLine("ScriptPath = {0}", ScriptPath);
            }
            else if (args[i].Contains("log") && args[i].Contains("path"))
            {
                LogPath = args[i + 1];
                Console.WriteLine("LogPath = {0}", LogPath);
            }
            else if (args[i].Contains("db") && args[i].Contains("path"))
            {
                DatabasePath = args[i + 1];
                Console.WriteLine("DatabasePath = {0}", DatabasePath);
            }
            else if (args[i].Contains("environment"))
            {
                Env = args[i + 1];
                Console.WriteLine("Environment = {0}", Env);
            }
        }
    }

    public static void InitialiseLog()
    {
        string path = Path.Combine(Environment.GetFolderPath(
            Environment.SpecialFolder.LocalApplicationData), "TogglDesktop");
        Directory.CreateDirectory(path);

        if (null == LogPath)
        {
            LogPath = Path.Combine(path, "toggldesktop.log");
        }
        toggl_set_log_path(LogPath);
        toggl_set_log_level("debug");
    }

    public static bool StartUI(string version, IEnumerable<ulong> experimentIds)
    {
        parseCommandlineParams();

        foreach (var id in experimentIds)
        {
            toggl_add_obm_experiment_nr(id);
        }

        ctx = toggl_context_init("windows_native_app", version);

        toggl_set_environment(ctx, Env);

        string cacert_path = Path.Combine(
            AppDomain.CurrentDomain.BaseDirectory,
            "cacert.pem");
        toggl_set_cacert_path(ctx, cacert_path);

        string err = toggl_check_view_struct_size(
            Marshal.SizeOf(new TogglTimeEntryView()),
            Marshal.SizeOf(new TogglAutocompleteView()),
            Marshal.SizeOf(new TogglGenericView()),
            Marshal.SizeOf(new TogglSettingsView()),
            Marshal.SizeOf(new TogglAutotrackerRuleView()));
        if (null != err) {
            throw new System.InvalidOperationException(err);
        }

        listenToLibEvents();

        if (Utils.GetIsUpdateCheckDisabledFromRegistry())
        {
            toggl_disable_update_check(ctx);
        }

        string path = Path.Combine(Environment.GetFolderPath(
            Environment.SpecialFolder.LocalApplicationData), "TogglDesktop");

#if TOGGL_ALLOW_UPDATE_CHECK
        installPendingUpdates();
#endif

        // Configure log, db path
        Directory.CreateDirectory(path);

        if (null == DatabasePath)
        {
            DatabasePath = Path.Combine(path, "toggldesktop.db");
        }

        if (!toggl_set_db_path(ctx, DatabasePath))
        {
            throw new System.Exception("Failed to initialize database at " + DatabasePath);
        }

        toggl_set_update_path(ctx, UpdatesPath);

        // Start pumping UI events
        return toggl_ui_start(ctx);
    }

    // ReSharper disable once UnusedMember.Local
    // (updates are disabled in Debug configuration to allow for proper debugging)
    private static void installPendingUpdates()
    {
        Directory.CreateDirectory(UpdatesPath); // make sure the directory exists
        var di = new DirectoryInfo(UpdatesPath);
        foreach (var file in di.GetFiles("TogglDesktopInstaller*.exe", SearchOption.TopDirectoryOnly))
        {
            try
            {
                Utils.DeleteFile(file.FullName);
            }
            catch (Exception e)
            {
                BugsnagService.NotifyBugsnag(e);
                _onError.OnNext(($"Unable to delete the file: {file.FullName}. Delete this file manually.", false));
            }
        }

        var aboutWindowViewModel = mainWindow.GetWindow<AboutWindow>().ViewModel;
        if (aboutWindowViewModel.InstallPendingUpdate())
        {
            // quit, updater will restart the app
            Environment.Exit(0);
        }
    }

    public static bool IsUpdateCheckDisabled()
    {
#if TOGGL_ALLOW_UPDATE_CHECK
        return Utils.GetIsUpdateCheckDisabledFromRegistry();
#else
        return true;
#endif
    }
    #endregion

    #region converting data

    #region high level

    private static List<TogglGenericView> convertToViewItemList(IntPtr first)
    {
        return marshalList<TogglGenericView>(first, n => n.Next);
    }

    private static List<TogglAutocompleteView> convertToAutocompleteList(IntPtr first)
    {
        return marshalList<TogglAutocompleteView>(first, n => n.Next);
    }

    private static List<TogglTimeEntryView> convertToTimeEntryList(IntPtr first)
    {
        return marshalList<TogglTimeEntryView>(first, n => n.Next);
    }

    private static List<TogglAutotrackerRuleView> convertToAutotrackerEntryList(IntPtr first)
    {
        return marshalList<TogglAutotrackerRuleView>(first, n => n.Next);
    }

    private static List<TogglCountryView> convertToCountryList(IntPtr first)
    {
        return marshalList<TogglCountryView>(first, n => n.Next);
    }

    #endregion

    #region low level

    private static List<T> marshalList<T>(IntPtr node, Func<T, IntPtr> getNext)
    where T : struct
    {
        var list = new List<T>();

        while (node != IntPtr.Zero)
        {
            var t = (T)Marshal.PtrToStructure(node, typeof(T));
            list.Add(t);
            node = getNext(t);
        }

        return list;
    }

    private static T marshalStruct<T>(IntPtr pointer)
    where T : struct
    {
        return (T)Marshal.PtrToStructure(pointer, typeof(T));
    }

    #endregion

    #endregion

    #region getting/setting global shortcuts

    public static void SetKeyStart(string key)
    {
        toggl_set_key_start(ctx, key);
    }
    public static Key GetKeyStart()
    {
        var keyCode = toggl_get_key_start(ctx);
        return getKey(keyCode);
    }
    public static void SetKeyShow(string key)
    {
        toggl_set_key_show(ctx, key);
    }
    public static Key GetKeyShow()
    {
        var keyCode = toggl_get_key_show(ctx);
        return getKey(keyCode);
    }
    public static void SetKeyModifierShow(ModifierKeys mods)
    {
        toggl_set_key_modifier_show(ctx, mods.ToString());
    }
    public static ModifierKeys GetKeyModifierShow()
    {
        var s = toggl_get_key_modifier_show(ctx);
        if (string.IsNullOrWhiteSpace(s) || !Enum.TryParse(s, true, out ModifierKeys modifierKeys))
            return ModifierKeys.None;
        return modifierKeys;
    }
    public static void SetKeyModifierStart(ModifierKeys mods)
    {
        toggl_set_key_modifier_start(ctx, mods.ToString());
    }
    public static ModifierKeys GetKeyModifierStart()
    {
        var s = toggl_get_key_modifier_start(ctx);
        if (string.IsNullOrWhiteSpace(s) || !Enum.TryParse(s, true, out ModifierKeys modifierKeys))
            return ModifierKeys.None;
        return modifierKeys;
    }

    private static Key getKey(string keyCode)
    {
        if (string.IsNullOrEmpty(keyCode) || !Enum.TryParse(keyCode, out Key key))
        {
            return Key.None;
        }

        return key;
    }

    #endregion

    #region getting/setting window settings

    public static void SetWindowMaximized(bool maximised)
    {
        toggl_set_window_maximized(ctx, maximised);
    }
    public static bool GetWindowMaximized()
    {
        return toggl_get_window_maximized(ctx);
    }
    public static void SetWindowMinimized(bool minimized)
    {
        toggl_set_window_minimized(ctx, minimized);
    }
    public static bool GetWindowMinimized()
    {
        return toggl_get_window_minimized(ctx);
    }

    public static void SetEditViewWidth(long width)
    {
        toggl_set_window_edit_size_width(ctx, width);
    }
    public static long GetEditViewWidth()
    {
        return toggl_get_window_edit_size_width(ctx);
    }

    public static bool SetWindowSettings(
        long x, long y, long h, long w)
    {
        return toggl_set_window_settings(ctx, x, y, h, w);
    }


    public static bool WindowSettings(
        ref long x, ref long y, ref long h, ref long w)
    {
        return toggl_window_settings(ctx, ref x, ref y, ref h, ref w);
    }

    #endregion

    #region various

    public static void PrepareShutdown()
    {
        mainWindow.PrepareShutdown(true);
    }

    public static void SetManualMode(bool manualMode)
    {
        toggl_set_settings_manual_mode(ctx, manualMode);
    }

    public static DateTime DateTimeFromUnix(UInt64 unix_seconds)
    {
        return UnixEpoch.AddSeconds(unix_seconds).ToLocalTime().DateTime;
    }

    public static Int64 UnixFromDateTime(DateTime value)
    {
        var span = new DateTimeOffset(value) - UnixEpoch;
        return (Int64)span.TotalSeconds;
    }

    public static void NewError(string errmsg, bool user_error)
    {
        _onError.OnNext((errmsg, user_error));
    }

    public static void OpenInBrowser(string url)
    {
        _onURL.OnNext(url);
    }

    public static void ShowErrorAndNotify(string errmsg, Exception ex)
    {
        NewError(errmsg, true);
        BugsnagService.NotifyBugsnag(ex);
    }

    public static bool AskToDeleteEntry(string guid)
    {
        var result = MessageBox.Show(mainWindow, "Deleted time entries cannot be restored.", "Delete time entry?",
                                     MessageBoxButton.OKCancel, "Delete entry");

        if (result == MessageBoxResult.OK)
        {
            return DeleteTimeEntry(guid);
        }
        return false;
    }

    public static void RegisterMainWindow(MainWindow window)
    {
        if (mainWindow != null)
            throw new Exception("Can only register main window once!");

        mainWindow = window;
    }

    #endregion


    public static void ToggleEntriesGroup(string groupName)
    {
        toggl_toggle_entries_group(ctx, groupName);
    }
}
}
