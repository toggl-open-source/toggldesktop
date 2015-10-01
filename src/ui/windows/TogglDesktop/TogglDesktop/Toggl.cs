using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using TogglDesktop.Diagnostics;
using TogglDesktop.WPF;
using MessageBox = TogglDesktop.WPF.MessageBox;

// ReSharper disable InconsistentNaming

namespace TogglDesktop
{
public static partial class Toggl
{
    private static IntPtr ctx = IntPtr.Zero;

    public const string Project = "project";
    public const string Duration = "duration";
    public const string Description = "description";

    public const string TagSeparator = "\t";

    // User can override some parameters when running the app
    public static string ScriptPath;
    public static string DatabasePath;
    public static string LogPath;
    public static string Env = "production";

    private static Window mainWindow;

    // Callbacks

    public delegate void DisplayApp(
        bool open);

    public delegate void DisplayError(
        string errmsg,
        bool user_error);

    public delegate void DisplayOnlineState(
        Int64 state);

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
        List<TogglTimeEntryView> list);

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

    public static bool Signup(string email, string password)
    {
        return toggl_signup(ctx, email, password);
    }

    public static bool Login(string email, string password)
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
        return toggl_continue(ctx, guid);
    }

    public static bool ContinueLatest()
    {
        return toggl_continue_latest(ctx);
    }

    public static bool DeleteTimeEntry(string guid)
    {
        return toggl_delete_time_entry(ctx, guid);
    }


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

    public static bool Stop()
    {
        return toggl_stop(ctx);
    }

    public static bool DiscardTimeAt(string guid, UInt64 at, bool split)
    {
        return toggl_discard_time_at(ctx, guid, at, split);
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
        string tags)
    {
        return toggl_start(ctx,
                           description,
                           duration,
                           task_id,
                           project_id,
                           project_guid,
                           tags);
    }


    public static string AddProject(
        string time_entry_guid,
        UInt64 workspace_id,
        UInt64 client_id,
        string client_guid,
        string project_name,
        bool is_private)
    {
        using (Performance.Measure("adding project"))
        {
            return toggl_add_project(ctx,
                                     time_entry_guid,
                                     workspace_id,
                                     client_id,
                                     client_guid,
                                     project_name,
                                     is_private);
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
        toggl_sync(ctx);
    }


    public static void SetSleep()
    {
        toggl_set_sleep(ctx);
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

    // Events for C#

    public static event DisplayApp OnApp = delegate { };
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

    private static void listenToLibEvents()
    {
        toggl_on_show_app(ctx, delegate(bool open)
        {
            using (Performance.Measure("Calling OnApp"))
            {
                OnApp(open);
            }
        });

        toggl_on_error(ctx, delegate(string errmsg, bool user_error)
        {
            using (Performance.Measure("Calling OnError, user_error: {1}, message: {0}", errmsg, user_error))
            {
                OnError(errmsg, user_error);
            }
        });

        toggl_on_online_state(ctx, delegate(Int64 state)
        {
            using (Performance.Measure("Calling OnOnlineState, state: {0}", state))
            {
                OnOnlineState(state);
            }
        });

        toggl_on_login(ctx, delegate(bool open, UInt64 user_id)
        {
            using (Performance.Measure("Calling OnLogin"))
            {
                OnLogin(open, user_id);
            }
        });

        toggl_on_reminder(ctx, delegate(string title, string informative_text)
        {
            using (Performance.Measure("Calling OnReminder, title: {0}", title))
            {
                OnReminder(title, informative_text);
            }
        });

        toggl_on_time_entry_list(ctx, delegate(bool open, IntPtr first)
        {
            using (Performance.Measure("Calling OnTimeEntryList"))
            {
                var list = ConvertToTimeEntryList(first);
                OnTimeEntryList(open, list);
            }
        });

        toggl_on_time_entry_autocomplete(ctx, delegate(IntPtr first)
        {
            using (Performance.Measure("Calling OnTimeEntryAutocomplete"))
            {
                var list = ConvertToAutocompleteList(first);
                OnTimeEntryAutocomplete(list);
            }
        });

        toggl_on_mini_timer_autocomplete(ctx, delegate(IntPtr first)
        {
            using (Performance.Measure("Calling OnMinitimerAutocomplete"))
            {
                var list = ConvertToAutocompleteList(first);
                OnMinitimerAutocomplete(list);
            }
        });

        toggl_on_project_autocomplete(ctx, delegate(IntPtr first)
        {
            using (Performance.Measure("Calling OnProjectAutocomplete"))
            {
                var list = ConvertToAutocompleteList(first);
                OnProjectAutocomplete(list);
            }
        });

        toggl_on_time_entry_editor(ctx, delegate(
            bool open,
            IntPtr te,
            string focused_field_name)
        {
            using (Performance.Measure("Calling OnTimeEntryEditor, focused field: {0}", focused_field_name))
            {
                TogglTimeEntryView model = (TogglTimeEntryView)Marshal.PtrToStructure(
                    te, typeof(TogglTimeEntryView));
                OnTimeEntryEditor(open, model, focused_field_name);
            }
        });

        toggl_on_workspace_select(ctx, delegate(IntPtr first)
        {
            using (Performance.Measure("Calling OnWorkspaceSelect"))
            {
                var list = ConvertToViewItemList(first);
                OnWorkspaceSelect(list);
            }
        });

        toggl_on_client_select(ctx, delegate(IntPtr first)
        {
            using (Performance.Measure("Calling OnClientSelect"))
            {
                var list = ConvertToViewItemList(first);
                OnClientSelect(list);
            }
        });

        toggl_on_tags(ctx, delegate(IntPtr first)
        {
            using (Performance.Measure("Calling OnTags"))
            {
                var list = ConvertToViewItemList(first);
                OnTags(list);
            }
        });

        toggl_on_settings(ctx, delegate(bool open, IntPtr settings)
        {
            using (Performance.Measure("Calling OnSettings"))
            {
                TogglSettingsView model = (TogglSettingsView)Marshal.PtrToStructure(
                    settings, typeof(TogglSettingsView));
                OnSettings(open, model);
            }
        });

        toggl_on_timer_state(ctx, delegate(IntPtr te)
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
                TogglTimeEntryView view =
                    (TogglTimeEntryView)Marshal.PtrToStructure(
                        te, typeof(TogglTimeEntryView));
                OnRunningTimerState(view);
            }
        });

        toggl_on_url(ctx, delegate(string url)
        {
            using (Performance.Measure("Calling OnURL"))
            {
                OnURL(url);
            }
        });

        toggl_on_idle_notification(ctx, delegate(
            string guid,
            string since,
            string duration,
            UInt64 started,
            string description)
        {
            using (Performance.Measure("Calling OnIdleNotification"))
            {
                OnIdleNotification(guid, since, duration, started, description);
            }
        });
    }

    // Start UI

    public static bool StartUI(string version)
    {
        parseCommandlineParams();

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

        string updatePath = Path.Combine(path, "updates");

#if !INVS
        installPendingUpdates(updatePath);
#endif

        // Configure log, db path
        Directory.CreateDirectory(path);

        if (null == LogPath)
        {
            LogPath = Path.Combine(path, "toggldesktop.log");
        }
        toggl_set_log_path(LogPath);
        toggl_set_log_level("debug");

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
    private static void installPendingUpdates(string updatePath)
    {
        if (!Directory.Exists(updatePath))
        {
            return;
        }

        var di = new DirectoryInfo(updatePath);
        var files = di.GetFiles("TogglDesktopInstaller*.exe",
                                SearchOption.TopDirectoryOnly);
        if (files.Length > 1)
        {
            // Somethings fubar. Delete the updates to start over
            foreach (var file in files)
            {
                file.Delete();
            }
            return;
        }

        if (files.Length < 1)
        {
            return;
        }

        var updaterPath = Path.Combine(
            AppDomain.CurrentDomain.BaseDirectory,
            "TogglDesktopUpdater.exe");
        if (!File.Exists(updaterPath))
        {
            Debug("TogglDesktopUpdater.exe not found");
            return;
        }

        var psi = new ProcessStartInfo
        {
            FileName = updaterPath,
            Arguments = Process.GetCurrentProcess().Id
            + " " + string.Format("\"{0}\"", files[0].FullName)
            + " " + string.Format("\"{0}\"", System.Reflection.Assembly.GetEntryAssembly().Location)
        };
        var process = Process.Start(psi);
        if (process != null && !process.HasExited && process.Id != 0)
        {
            // Update has started. Quit, installer will restart me.
            Environment.Exit(0);
        }

        Debug("Failed to start updater process");
    }

    public static List<TogglGenericView> ConvertToViewItemList(IntPtr first)
    {
        using (var token = Performance.Measure("marshalling view item list"))
        {
            List<TogglGenericView> list = new List<TogglGenericView>();
            if (IntPtr.Zero == first)
            {
                token.WithInfo("count: 0");
                return list;
            }
            TogglGenericView n = (TogglGenericView)Marshal.PtrToStructure(
                first, typeof(TogglGenericView));
            while (true)
            {
                list.Add(n);
                if (n.Next == IntPtr.Zero)
                {
                    break;
                }
                n = (TogglGenericView)Marshal.PtrToStructure(
                    n.Next, typeof(TogglGenericView));
            }
            token.WithInfo("count: " + list.Count);
            return list;
        }
    }

    private static List<TogglAutocompleteView> ConvertToAutocompleteList(IntPtr first)
    {
        using (var token = Performance.Measure("marshalling auto complete list"))
        {
            List<TogglAutocompleteView> list = new List<TogglAutocompleteView>();
            if (IntPtr.Zero == first)
            {
                token.WithInfo("count: 0");
                return list;
            }
            TogglAutocompleteView n = (TogglAutocompleteView)Marshal.PtrToStructure(
                first, typeof (TogglAutocompleteView));
            while (true)
            {
                list.Add(n);
                if (n.Next == IntPtr.Zero)
                {
                    break;
                }
                n = (TogglAutocompleteView)Marshal.PtrToStructure(
                    n.Next, typeof (TogglAutocompleteView));
            }
            token.WithInfo("count: " + list.Count);
            return list;
        }
    }

    private static List<TogglTimeEntryView> ConvertToTimeEntryList(IntPtr first)
    {
        using (var token = Performance.Measure("marshalling time entry list"))
        {
            List<TogglTimeEntryView> list = new List<TogglTimeEntryView>();
            if (IntPtr.Zero == first)
            {
                token.WithInfo("count: 0");
                return list;
            }
            TogglTimeEntryView n = (TogglTimeEntryView)Marshal.PtrToStructure(
                first, typeof (TogglTimeEntryView));

            while (true)
            {
                list.Add(n);
                if (n.Next == IntPtr.Zero)
                {
                    break;
                }
                n = (TogglTimeEntryView)Marshal.PtrToStructure(
                    n.Next, typeof (TogglTimeEntryView));
            }
            token.WithInfo("count: " + list.Count);
            return list;
        }
    }

    private static readonly DateTime UnixEpoch =
        new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);


    public static DateTime DateTimeFromUnix(UInt64 unix_seconds)
    {
        return UnixEpoch.AddSeconds(unix_seconds).ToLocalTime();
    }

    public static Int64 UnixFromDateTime(DateTime value)
    {
        TimeSpan span = (value - UnixEpoch.ToLocalTime());
        return (Int64)span.TotalSeconds;
    }

    public static void NewError(string errmsg, bool user_error)
    {
        OnError(errmsg, user_error);
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

    public static void RegisterMainWindow(MainWindow window)
    {
        if (mainWindow != null)
            throw new Exception("Can only register main window once!");

        mainWindow = window;
    }

    public static bool AskToDeleteEntry(string guid)
    {
        var result = MessageBox.Show(mainWindow, "Delete time entry?", "Please confirm",
                                     MessageBoxButton.OKCancel, "DELETE ENTRY");

        if (result == MessageBoxResult.OK)
        {
            return DeleteTimeEntry(guid);
        }
        return false;
    }


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
}
}
