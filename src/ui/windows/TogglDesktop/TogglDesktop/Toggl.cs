using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows;
using TogglDesktop.Diagnostics;

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
    private static string updatePath;

    // User can override some parameters when running the app
    public static string ScriptPath;
    public static string DatabasePath;
    public static string LogPath;

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

    #region callback delegates

    public delegate void DisplayApp(
        bool open);

    public delegate void DisplayOverlay(
        Int64 type);

    public delegate void DisplayError(
        string errmsg,
        bool user_error);

    public delegate void DisplaySyncState(
        SyncState state);

    public delegate void DisplayUnsyncedItems(
        Int64 count);

    public delegate void DisplayOnlineState(
        OnlineState state);

    public delegate void DisplayURL(
        string url);

    public delegate void DisplayLogin(
        bool open,
        UInt64 user_id);

    public delegate void DisplayReminder(
        string title,
        string informative_text);

    public delegate void DisplayTimeEntryList(
        bool open,
        List<TogglTimeEntryView> list,
        bool show_load_more_button);

    public delegate void DisplayAutocomplete(
        List<TogglAutocompleteView> list);

    public delegate void DisplayViewItems(
        List<TogglGenericView> list);

    public delegate void DisplayTimeEntryEditor(
        bool open,
        TogglTimeEntryView te,
        string focused_field_name);

    public delegate void DisplaySettings(
        bool open,
        TogglSettingsView settings);

    public delegate void DisplayRunningTimerState(
        TogglTimeEntryView te);

    public delegate void DisplayStoppedTimerState();

    public delegate void DisplayIdleNotification(
        string guid,
        string since,
        string duration,
        UInt64 started,
        string description);

    public delegate void DisplayAutotrackerRules(
        List<TogglAutotrackerRuleView> rules, string[] terms);

    public delegate void DisplayAutotrackerNotification(
        string projectName, ulong projectId, ulong taskId);

    public delegate void DisplayUpdateDownloadState(
        string url, DownloadStatus status);

    public delegate void DisplayProjectColors(
        string[] colors, ulong count);

    public delegate void DisplayCountries(
        List<TogglCountryView> list);

    public delegate void DisplayPromotion(
        long id);

    public delegate void DisplayObmExperiment(
        ulong id, bool included, bool seenBefore);

    public delegate void DisplayPomodoro(
        string title, string informativeText);

    public delegate void DisplayPomodoroBreak(
        string title, string informativeText);

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

    public static bool DiscardTimeAt(string guid, UInt64 at, bool split)
    {
        return toggl_discard_time_at(ctx, guid, at, split);
    }

    public static bool DiscardTimeAndContinue(string guid, UInt64 at, bool split)
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
                                             settings.IdleMinutes)) {
            return false;
        }

        if (!toggl_set_settings_reminder_minutes(ctx,
                settings.ReminderMinutes)) {
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

        if (!toggl_set_settings_pomodoro_minutes(ctx, settings.PomodoroMinutes))
        {
            return false;
        }

        if (!toggl_set_settings_pomodoro_break(ctx, settings.PomodoroBreak))
        {
            return false;
        }

        if (!toggl_set_settings_pomodoro_break_minutes(ctx, settings.PomodoroBreakMinutes))
        {
            return false;
        }

        if (!toggl_set_settings_stop_entry_on_shutdown_sleep(ctx, settings.StopEntryOnShutdownSleep))
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
                           preventOnApp);
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

    public static void FullSync()
    {
        toggl_fullsync(ctx);
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

    public static string RunScript(string script, ref Int64 err)
    {
        return toggl_run_script(ctx, script, ref err);
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
    #endregion

    #region callback events

    public static event DisplayApp OnApp = delegate { };
    public static event DisplayOverlay OnOverlay = delegate { };
    public static event DisplayError OnError = delegate { };
    public static event DisplayOnlineState OnOnlineState = delegate { };
    public static event DisplayLogin OnLogin = delegate { };
    public static event DisplayReminder OnReminder = delegate { };
    public static event DisplayTimeEntryList OnTimeEntryList = delegate { };
    public static event DisplayAutocomplete OnTimeEntryAutocomplete = delegate { };
    public static event DisplayAutocomplete OnMinitimerAutocomplete = delegate { };
    public static event DisplayAutocomplete OnProjectAutocomplete = delegate { };
    public static event DisplayTimeEntryEditor OnTimeEntryEditor = delegate { };
    public static event DisplayViewItems OnWorkspaceSelect = delegate { };
    public static event DisplayViewItems OnClientSelect = delegate { };
    public static event DisplayViewItems OnTags = delegate { };
    public static event DisplaySettings OnSettings = delegate { };
    public static event DisplayRunningTimerState OnRunningTimerState = delegate { };
    public static event DisplayStoppedTimerState OnStoppedTimerState = delegate { };
    public static event DisplayURL OnURL = delegate { };
    public static event DisplayIdleNotification OnIdleNotification = delegate { };
    public static event DisplayAutotrackerRules OnAutotrackerRules = delegate { };
    public static event DisplayAutotrackerNotification OnAutotrackerNotification = delegate { };
    public static event DisplaySyncState OnDisplaySyncState = delegate { };
    public static event DisplayUnsyncedItems OnDisplayUnsyncedItems = delegate { };
    public static event DisplayUpdateDownloadState OnDisplayUpdateDownloadState = delegate { };
    public static event DisplayProjectColors OnDisplayProjectColors = delegate { };
    public static event DisplayCountries OnDisplayCountries = delegate { };
    public static event DisplayPromotion OnDisplayPromotion = delegate { };
    public static event DisplayObmExperiment OnDisplayObmExperiment = delegate { };
    public static event DisplayPomodoro OnDisplayPomodoro = delegate { };
    public static event DisplayPomodoroBreak OnDisplayPomodoroBreak = delegate { };

    private static void listenToLibEvents()
    {
        toggl_on_show_app(ctx, open =>
        {
            using (Performance.Measure("Calling OnApp"))
            {
                OnApp(open);
            }
        });

        toggl_on_overlay(ctx, type =>
        {
            using (Performance.Measure("Calling OnOverlay"))
            {
                OnOverlay(type);
            }
        });

        toggl_on_error(ctx, (errmsg, user_error) =>
        {
            using (Performance.Measure("Calling OnError, user_error: {1}, message: {0}", errmsg, user_error))
            {
                OnError(errmsg, user_error);
            }
        });

        toggl_on_sync_state(ctx, state =>
        {
            using (Performance.Measure("Calling OnDisplaySyncState, state: {0}", state))
            {
                OnDisplaySyncState((SyncState)state);
            }
        });
        toggl_on_unsynced_items(ctx, count =>
        {
            using (Performance.Measure("Calling OnDisplayUnsyncedItems, count: {0}", count))
            {
                OnDisplayUnsyncedItems(count);
            }
        });

        toggl_on_online_state(ctx, state =>
        {
            using (Performance.Measure("Calling OnOnlineState, state: {0}", state))
            {
                OnOnlineState((OnlineState)state);
            }
        });

        toggl_on_login(ctx, (open, user_id) =>
        {
            using (Performance.Measure("Calling OnLogin"))
            {
                OnLogin(open, user_id);
            }
        });

        toggl_on_reminder(ctx, (title, informative_text) =>
        {
            using (Performance.Measure("Calling OnReminder, title: {0}", title))
            {
                OnReminder(title, informative_text);
            }
        });

        toggl_on_time_entry_list(ctx, (open, first, show_load_more_button) =>
        {
            using (Performance.Measure("Calling OnTimeEntryList, open: {0}", open))
            {
                OnTimeEntryList(open, convertToTimeEntryList(first), show_load_more_button);
            }
        });

        toggl_on_time_entry_autocomplete(ctx, first =>
        {
            using (Performance.Measure("Calling OnTimeEntryAutocomplete"))
            {
                OnTimeEntryAutocomplete(convertToAutocompleteList(first));
            }
        });

        toggl_on_mini_timer_autocomplete(ctx, first =>
        {
            using (Performance.Measure("Calling OnMinitimerAutocomplete"))
            {
                OnMinitimerAutocomplete(convertToAutocompleteList(first));
            }
        });

        toggl_on_project_autocomplete(ctx, first =>
        {
            using (Performance.Measure("Calling OnProjectAutocomplete"))
            {
                OnProjectAutocomplete(convertToAutocompleteList(first));
            }
        });

        toggl_on_time_entry_editor(ctx, (open, te, focused_field_name) =>
        {
            using (Performance.Measure("Calling OnTimeEntryEditor, focused field: {0}", focused_field_name))
            {
                OnTimeEntryEditor(open, marshalStruct<TogglTimeEntryView>(te), focused_field_name);
            }
        });

        toggl_on_workspace_select(ctx, first =>
        {
            using (Performance.Measure("Calling OnWorkspaceSelect"))
            {
                OnWorkspaceSelect(convertToViewItemList(first));
            }
        });

        toggl_on_client_select(ctx, first =>
        {
            using (Performance.Measure("Calling OnClientSelect"))
            {
                OnClientSelect(convertToViewItemList(first));
            }
        });

        toggl_on_tags(ctx, first =>
        {
            using (Performance.Measure("Calling OnTags"))
            {
                OnTags(convertToViewItemList(first));
            }
        });

        toggl_on_settings(ctx, (open, settings) =>
        {
            using (Performance.Measure("Calling OnSettings"))
            {
                OnSettings(open, marshalStruct<TogglSettingsView>(settings));
            }
        });

        toggl_on_timer_state(ctx, te =>
        {
            if (te == IntPtr.Zero)
            {
                using (Performance.Measure("Calling OnStoppedTimerState"))
                {
                    OnStoppedTimerState();
                    return;
                }
            }
            using (Performance.Measure("Calling OnRunningTimerState"))
            {
                OnRunningTimerState(marshalStruct<TogglTimeEntryView>(te));
            }
        });

        toggl_on_url(ctx, url =>
        {
            using (Performance.Measure("Calling OnURL"))
            {
                OnURL(url);
            }
        });

        toggl_on_idle_notification(ctx, (guid, since, duration, started, description) =>
        {
            using (Performance.Measure("Calling OnIdleNotification"))
            {
                OnIdleNotification(guid, since, duration, started, description);
            }
        });

        toggl_on_autotracker_rules(ctx, (first, count, list) =>
        {
            using (Performance.Measure("Calling OnAutotrackerRules"))
            {
                OnAutotrackerRules(convertToAutotrackerEntryList(first), list);
            }
        });

        toggl_on_autotracker_notification(ctx, (name, project_id, task_id) =>
        {
            using (Performance.Measure("Calling OnAutotrackerNotification"))
            {
                OnAutotrackerNotification(name, project_id, task_id);
            }
        });

        toggl_on_update_download_state(ctx, (version, state) =>
        {
            using (Performance.Measure("Calling OnUpdateDownloadState, v: {0}, state: {1}", version, state))
            {
                OnDisplayUpdateDownloadState(version, (DownloadStatus)state);
            }
        });

        toggl_on_project_colors(ctx, (colors, count) =>
        {
            using (Performance.Measure("Calling OnProjectColors, count: {0}", count))
            {
                OnDisplayProjectColors(colors, count);
            }
        });

        toggl_on_countries(ctx, (first) =>
        {
            using (Performance.Measure("Calling OnCountries"))
            {
                OnDisplayCountries(convertToCountryList(first));
            }
        });

        toggl_on_promotion(ctx, id =>
        {
            using (Performance.Measure("Calling OnDisplayPromotino, id: {0}", id))
            {
                OnDisplayPromotion(id);
            }
        });

        toggl_on_obm_experiment(ctx, (id, included, seenBefore) =>
        {
            using (Performance.Measure(
                "Calling OnDisplatObmExperiment, id: {0}, included: {1}, seen: {2}",
                id, included, seenBefore))
            {
                OnDisplayObmExperiment(id, included, seenBefore);
            }
        });
        toggl_on_pomodoro(ctx, (title, text) =>
        {
            using (Performance.Measure("Calling OnDisplayPomodoro"))
            {
                OnDisplayPomodoro(title, text);
            }
        });
        toggl_on_pomodoro_break(ctx, (title, text) =>
        {
            using (Performance.Measure("Calling OnDisplayPomodoroBreak"))
            {
                OnDisplayPomodoroBreak(title, text);
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

        if (IsUpdateCheckDisabled())
        {
            toggl_disable_update_check(ctx);
        }

        // Move "old" format app data folder, if it exists
        string oldpath = Path.Combine(Environment.GetFolderPath(
            Environment.SpecialFolder.ApplicationData), "Kopsik");
        string path = Path.Combine(Environment.GetFolderPath(
            Environment.SpecialFolder.LocalApplicationData), "TogglDesktop");
        if (Directory.Exists(oldpath) && !Directory.Exists(path))
        {
            Directory.Move(oldpath, path);
        }

        updatePath = Path.Combine(path, "updates");

#if TOGGL_ALLOW_UPDATE_CHECK
        installPendingUpdates();
#endif

        // Configure log, db path
        Directory.CreateDirectory(path);

        if (null == DatabasePath)
        {
            DatabasePath = Path.Combine(path, "toggldesktop.db");
        }

        // Rename database file, if not done yet
        string olddatabasepath = Path.Combine(path, "kopsik.db");
        if (File.Exists(olddatabasepath) && !File.Exists(DatabasePath))
        {
            File.Move(olddatabasepath, DatabasePath);
        }

        if (!toggl_set_db_path(ctx, DatabasePath))
        {
            throw new System.Exception("Failed to initialize database at " + DatabasePath);
        }

        toggl_set_update_path(ctx, updatePath);

        // Start pumping UI events
        return toggl_ui_start(ctx);
    }

    // ReSharper disable once UnusedMember.Local
    // (updates are disabled in Release_VS configuration to allow for proper debugging)
    private static void installPendingUpdates()
    {
        if (Environment.GetCommandLineArgs().Contains("--updated"))
        {
            // --updated means we've just been silently updated and started by the installer
            // so we just clean up the installer files and continue
            var di = new DirectoryInfo(updatePath);
            foreach (var file in di.GetFiles("TogglDesktopInstaller*.exe", SearchOption.TopDirectoryOnly))
            {
                file.Delete();
            }

            return;
        }

        var update = createUpdateAction();

        if (update == null)
            return;

        update();
    }

    private static Action createUpdateAction()
    {
        if (!Directory.Exists(updatePath))
        {
            return null;
        }

        var di = new DirectoryInfo(updatePath);
        var files = di.GetFiles("TogglDesktopInstaller*.exe", SearchOption.TopDirectoryOnly);
        if (files.Length > 1)
        {
            Debug("Multiple update installers found. Deleting.");
            foreach (var file in files)
            {
                file.Delete();
            }
            return null;
        }

        if (files.Length < 1)
        {
            return null;
        }

        var installerFullPath = files[0].FullName;

        return () =>
        {
            var process = Process.Start(installerFullPath, "/S /U");
            if (process != null && !process.HasExited && process.Id != 0)
            {
                // Update has started. Quit, installer will restart me.
                Environment.Exit(0);
            }

            Debug("Failed to start installer process");
        };
    }

    public static bool IsUpdateCheckDisabled()
    {
        // On Windows platform, system admin can disable
        // automatic update check via registry key.
        object value = Microsoft.Win32.Registry.GetValue(
            "HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Toggl\\TogglDesktop",
            "UpdateCheckDisabled",
            false);
        if (value == null)
        {
            return false;
        }
        return Convert.ToBoolean(value);
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
    public static string GetKeyStart()
    {
        return toggl_get_key_start(ctx);
    }
    public static void SetKeyShow(string key)
    {
        toggl_set_key_show(ctx, key);
    }
    public static string GetKeyShow()
    {
        return toggl_get_key_show(ctx);
    }
    public static void SetKeyModifierShow(ModifierKeys mods)
    {
        toggl_set_key_modifier_show(ctx, mods.ToString());
    }
    public static ModifierKeys GetKeyModifierShow()
    {
        var s = toggl_get_key_modifier_show(ctx);
        if (string.IsNullOrWhiteSpace(s))
            return ModifierKeys.None;
        return (ModifierKeys)Enum.Parse(typeof(ModifierKeys), s, true);
    }
    public static void SetKeyModifierStart(ModifierKeys mods)
    {
        toggl_set_key_modifier_start(ctx, mods.ToString());
    }
    public static ModifierKeys GetKeyModifierStart()
    {
        var s = toggl_get_key_modifier_start(ctx);
        if(string.IsNullOrWhiteSpace(s))
            return ModifierKeys.None;
        return (ModifierKeys)Enum.Parse(typeof(ModifierKeys), s, true);
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

    public static void RestartAndUpdate()
    {
        var update = createUpdateAction();

        if (update == null)
        {
            return;
        }

        mainWindow.PrepareShutdown(true);

        Clear();

        update();
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
        OnError(errmsg, user_error);
    }

    public static bool AskToDeleteEntry(string guid)
    {
        var result = MessageBox.Show(mainWindow, "Deleted time entries cannot be restored.", "Delete time entry?",
                                     MessageBoxButton.OKCancel, "DELETE ENTRY");

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
