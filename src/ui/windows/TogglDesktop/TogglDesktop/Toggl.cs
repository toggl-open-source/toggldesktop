using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Reflection;
using System.Text;
using System.Collections.Generic;
using Microsoft.Win32;

namespace TogglDesktop
{
    public static class Toggl
    {
        private static IntPtr ctx = IntPtr.Zero;

        public const string Project = "project";
        public const string Duration = "duration";
        public const string Description = "description";

        public const string TagSeparator = "\t";

        private const string dll = "TogglDesktopDLL.dll";
        private const CharSet charset = CharSet.Unicode;
        private const CallingConvention convention = CallingConvention.Cdecl;

        // Models

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct TimeEntry
        {
            public Int64 DurationInSeconds;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string Description;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string ProjectAndTaskLabel;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string TaskLabel;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string ProjectLabel;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string ClientLabel;
            public UInt64 WID;
            public UInt64 PID;
            public UInt64 TID;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string Duration;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string Color;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string GUID;
            [MarshalAs(UnmanagedType.I1)]
            public bool Billable;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string Tags;
            public UInt64 Started;
            public UInt64 Ended;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string StartTimeString;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string EndTimeString;
            public UInt64 UpdatedAt;
            [MarshalAs(UnmanagedType.I1)]
            public bool DurOnly;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string DateHeader;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string DateDuration;
            [MarshalAs(UnmanagedType.I1)]
            public bool IsHeader;
            [MarshalAs(UnmanagedType.I1)]
            public bool CanAddProjects;
            [MarshalAs(UnmanagedType.I1)]
            public bool CanSeeBillable;
            public UInt64 DefaultWID;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string WorkspaceName;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string Error;
            public IntPtr Next;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct AutocompleteItem
        {
            [MarshalAs(UnmanagedType.LPWStr)]
            public string Text;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string Description;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string ProjectAndTaskLabel;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string TaskLabel;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string ProjectLabel;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string ClientLabel;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string Project;
            public UInt64 TaskID;
            public UInt64 ProjectID;
            public UInt64 Type;
            public IntPtr Next;

            public override string ToString()
            {
                return Text;
            }
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct Model
        {
            public UInt64 ID;
            public UInt64 WID;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string GUID;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string Name;
            public IntPtr Next;

            public override string ToString()
            {
                return Name;
            }
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct Settings
        {
            [MarshalAs(UnmanagedType.I1)]
            public bool UseProxy;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string ProxyHost;
            public UInt64 ProxyPort;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string ProxyUsername;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string ProxyPassword;
            [MarshalAs(UnmanagedType.I1)]
            public bool UseIdleDetection;
            [MarshalAs(UnmanagedType.I1)]
            public bool MenubarTimer;
            [MarshalAs(UnmanagedType.I1)]
            public bool MenubarProject;
            [MarshalAs(UnmanagedType.I1)]
            public bool DockIcon;
            [MarshalAs(UnmanagedType.I1)]
            public bool OnTop;
            [MarshalAs(UnmanagedType.I1)]
            public bool Reminder;
            [MarshalAs(UnmanagedType.I1)]
            public bool RecordTimeline;
            public UInt64 IdleMinutes;
            [MarshalAs(UnmanagedType.I1)]
            public bool FocusOnShortcut;
            public UInt64 ReminderMinutes;
            [MarshalAs(UnmanagedType.I1)]
            public bool ManualMode;
            [MarshalAs(UnmanagedType.I1)]
            public bool AutodetectProxy;
        }

        // Callbacks

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayApp(
            [MarshalAs(UnmanagedType.I1)]
            bool open);

        public delegate void DisplayApp(
            bool open);

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayError(
            [MarshalAs(UnmanagedType.LPWStr)]
            string errmsg,
            [MarshalAs(UnmanagedType.I1)]
            bool user_error);

        public delegate void DisplayError(
            string errmsg,
            bool user_error);

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayOnlineState(
            Int64 state);

        public delegate void DisplayOnlineState(
            Int64 state);

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayURL(
            [MarshalAs(UnmanagedType.LPWStr)]
            string url);

        public delegate void DisplayURL(
            string url);

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayLogin(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            UInt64 user_id);

        public delegate void DisplayLogin(
            bool open,
            UInt64 user_id);

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayReminder(
            [MarshalAs(UnmanagedType.LPWStr)]
            string title,
            [MarshalAs(UnmanagedType.LPWStr)]
            string informative_text);

        public delegate void DisplayReminder(
            string title,
            string informative_text);

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayTimeEntryList(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            IntPtr first);

        public delegate void DisplayTimeEntryList(
            bool open,
            List<TimeEntry> list);

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayAutocomplete(
            IntPtr first);

        public delegate void DisplayAutocomplete(
            List<AutocompleteItem> list);

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayViewItems(
            IntPtr first);

        public delegate void DisplayViewItems(
            List<Model> list);
        
        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayTimeEntryEditor(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            ref TimeEntry te,
            [MarshalAs(UnmanagedType.LPWStr)]
            string focused_field_name);

        public delegate void DisplayTimeEntryEditor(
            bool open,
            TimeEntry te,
            string focused_field_name);

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplaySettings(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            ref Settings settings);

        public delegate void DisplaySettings(
            bool open,
            Settings settings);

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayTimerState(
            IntPtr te);

        public delegate void DisplayRunningTimerState(
            TimeEntry te);

        public delegate void DisplayStoppedTimerState();

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayIdleNotification(
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid,
            [MarshalAs(UnmanagedType.LPWStr)]
            string since,
            [MarshalAs(UnmanagedType.LPWStr)]
            string duration,
            UInt64 started,
            [MarshalAs(UnmanagedType.LPWStr)]
            string description);

        public delegate void DisplayIdleNotification(
            string guid,
            string since,
            string duration,
            UInt64 started,
            string description);

        // Initialize/destroy an instance of the app

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern System.IntPtr toggl_context_init(
            [MarshalAs(UnmanagedType.LPWStr)]
            string app_name,
            [MarshalAs(UnmanagedType.LPWStr)]
            string app_version);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_context_clear(
            IntPtr context);

        public static void Clear()
        {
            toggl_context_clear(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_environment(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string environment);

        // CA cert bundle path must be configured from UI

        [DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = convention)]
        private static extern void toggl_set_cacert_path(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string path);

        // DB path must be configured from UI

        [DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_db_path(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string path);

        // Configure updates download path to use the silent updater

        [DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = convention)]
        private static extern void toggl_set_update_path(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string path);

        // Log path must be configured from UI

        [DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = convention)]
        private static extern void toggl_set_log_path(
            [MarshalAs(UnmanagedType.LPWStr)]
            string path);

        // Log level is optional

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_set_log_level(
            [MarshalAs(UnmanagedType.LPWStr)]
            string level);

        // User can tell the lib to display the app.

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_show_app(
            IntPtr context);

        public static void ShowApp()
        {
            toggl_show_app(ctx);
        }

        // Configure the UI callbacks. Required.

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_show_app(
            IntPtr context,
            TogglDisplayApp cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_error(
            IntPtr context,
            TogglDisplayError cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_online_state(
            IntPtr context,
            TogglDisplayOnlineState cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_url(
            IntPtr context,
            TogglDisplayURL cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_login(
            IntPtr context,
            TogglDisplayLogin cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_reminder(
            IntPtr context,
            TogglDisplayReminder cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_time_entry_list(
            IntPtr context,
            TogglDisplayTimeEntryList cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_time_entry_autocomplete(
            IntPtr context,
            TogglDisplayAutocomplete cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_mini_timer_autocomplete(
            IntPtr context,
            TogglDisplayAutocomplete cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_project_autocomplete(
            IntPtr context,
            TogglDisplayAutocomplete cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_workspace_select(
            IntPtr context,
            TogglDisplayViewItems cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_client_select(
            IntPtr context,
            TogglDisplayViewItems cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_tags(
            IntPtr context,
            TogglDisplayViewItems cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_time_entry_editor(
            IntPtr context,
            TogglDisplayTimeEntryEditor cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_settings(
            IntPtr context,
            TogglDisplaySettings cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_timer_state(
            IntPtr context,
            TogglDisplayTimerState cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_on_idle_notification(
            IntPtr context,
            TogglDisplayIdleNotification cb);

        // After UI callbacks are configured, start pumping UI events

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_ui_start(
            IntPtr context);

        // User interaction with the app

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_signup(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string email,
            [MarshalAs(UnmanagedType.LPWStr)]
            string password);

        public static bool Signup(string email, string password)
        {
            return toggl_signup(ctx, email, password);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_login(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string email,
            [MarshalAs(UnmanagedType.LPWStr)]
            string password);

        public static bool Login(string email, string password)
        {
            return toggl_login(ctx, email, password);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_google_login(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string access_token);

        public static bool GoogleLogin(string access_token)
        {
            return toggl_google_login(ctx, access_token);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_password_forgot(
            IntPtr context);

        public static void PasswordForgot()
        {
            toggl_password_forgot(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_open_in_browser(
            IntPtr context);

        public static void OpenInBrowser()
        {
            toggl_open_in_browser(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_get_support(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_feedback_send(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string topic,
            [MarshalAs(UnmanagedType.LPWStr)]
            string details,
            [MarshalAs(UnmanagedType.LPWStr)]
            string filename);

        public static bool SendFeedback(
            string topic,
            string details,
            string filename)
        {
            return toggl_feedback_send(ctx, topic, details, filename);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_view_time_entry_list(
            IntPtr context);

        public static bool ViewTimeEntryList()
        {
            return toggl_view_time_entry_list(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_edit(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid,
            [MarshalAs(UnmanagedType.I1)]
            bool edit_running_time_entry,
            [MarshalAs(UnmanagedType.LPWStr)]
            string focused_field_name);

        public static void Edit(
            string guid,
            bool edit_running_time_entry,
            string focused_field_name)
        {
            toggl_edit(ctx, guid, edit_running_time_entry, focused_field_name);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_edit_preferences(
            IntPtr context);

        public static void EditPreferences()
        {
            toggl_edit_preferences(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_continue(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid);

        public static bool Continue(string guid)
        {
            return toggl_continue(ctx, guid);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_continue_latest(
            IntPtr context);

        public static bool ContinueLatest()
        {
            return toggl_continue_latest(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_delete_time_entry(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid);

        public static bool DeleteTimeEntry(string guid)
        {
            return toggl_delete_time_entry(ctx, guid);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_duration(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid,
            [MarshalAs(UnmanagedType.LPWStr)]
            string value);

        public static bool SetTimeEntryDuration(string guid, string value)
        {
            return toggl_set_time_entry_duration(ctx, guid, value);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_project(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid,
            UInt64 task_id,
            UInt64 project_id,
            [MarshalAs(UnmanagedType.LPWStr)]
            string project_guid);

        public static bool SetTimeEntryProject(
            string guid,
            UInt64 task_id,
            UInt64 project_id,
            string project_guid)
        {
            return toggl_set_time_entry_project(ctx,
                guid, task_id, project_id, project_guid);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_start(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid,
            [MarshalAs(UnmanagedType.LPWStr)]
            string value);

        public static bool SetTimeEntryStart(string guid, string value)
        {
            return toggl_set_time_entry_start(ctx, guid, value);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_date(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid,
            Int64 value);

        public static bool SetTimeEntryDate(string guid, DateTime value)
        {
            return toggl_set_time_entry_date(ctx, guid, UnixFromDateTime(value));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_end(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid,
            [MarshalAs(UnmanagedType.LPWStr)]
            string value);

        public static bool SetTimeEntryEnd(string guid, string value)
        {
            return toggl_set_time_entry_end(ctx, guid, value);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_tags(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid,
            [MarshalAs(UnmanagedType.LPWStr)]
            string value);

        public static bool SetTimeEntryTags(string guid, string value)
        {
            return toggl_set_time_entry_tags(ctx, guid, value);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_billable(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid,
            [MarshalAs(UnmanagedType.I1)]
            bool billable);

        public static bool SetTimeEntryBillable(string guid, bool billable)
        {
            return toggl_set_time_entry_billable(ctx, guid, billable);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_description(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid,
            [MarshalAs(UnmanagedType.LPWStr)]
            string value);

        public static bool SetTimeEntryDescription(string guid, string value)
        {
            return toggl_set_time_entry_description(ctx, guid, value);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_stop(
            IntPtr context);

        public static bool Stop()
        {
            return toggl_stop(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_discard_time_at(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string guid,
            UInt64 at,
            [MarshalAs(UnmanagedType.I1)]
            bool split);

        public static bool DiscardTimeAt(string guid, UInt64 at, bool split)
        {
            return toggl_discard_time_at(ctx, guid, at, split);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_window_settings(
            IntPtr context,
            Int64 window_x,
            Int64 window_y,
            Int64 window_h,
            Int64 window_w);

        public static bool SetWindowSettings(
            Int64 x,
            Int64 y,
            Int64 h,
            Int64 w)
        {
            return toggl_set_window_settings(ctx, x, y, h, w);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_window_settings(
            IntPtr context,
            ref Int64 window_x,
            ref Int64 window_y,
            ref Int64 window_h,
            ref Int64 window_w);

        public static bool WindowSettings(
            ref Int64 x,
            ref Int64 y,
            ref Int64 h,
            ref Int64 w)
        {
            return toggl_window_settings(ctx, ref x, ref y, ref h, ref w);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_settings_use_idle_detection(
            IntPtr context,
            [MarshalAs(UnmanagedType.I1)]
            bool use_idle_detection);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_settings_autodetect_proxy(
            IntPtr context,
            [MarshalAs(UnmanagedType.I1)]
            bool autodetect_proxy);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_settings_focus_on_shortcut(
            IntPtr context,
            [MarshalAs(UnmanagedType.I1)]
            bool focus_on_shortcut);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_settings_idle_minutes(
            IntPtr context,
            UInt64 idle_minutes);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_settings_on_top(
            IntPtr context,
            [MarshalAs(UnmanagedType.I1)]
            bool on_top);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_settings_reminder(
            IntPtr context,
            [MarshalAs(UnmanagedType.I1)]
            bool reminder);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_settings_reminder_minutes(
            IntPtr context,
            UInt64 reminder_minutes);

        public static bool SetSettings(Settings settings)
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

            return toggl_timeline_toggle_recording(ctx, settings.RecordTimeline);
        }

        public static bool IsTimelineRecordingEnabled()
        {
            return toggl_timeline_is_recording_enabled(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_proxy_settings(
            IntPtr context,
            [MarshalAs(UnmanagedType.I1)]
            bool use_proxy,
            [MarshalAs(UnmanagedType.LPWStr)]
            string proxy_host,
            UInt64 proxy_port,
            [MarshalAs(UnmanagedType.LPWStr)]
            string proxy_username,
            [MarshalAs(UnmanagedType.LPWStr)]
            string proxy_password);
 
        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_logout(
            IntPtr context);

        public static bool Logout()
        {
            return toggl_logout(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_clear_cache(
            IntPtr context);

        public static bool ClearCache()
        {
            return toggl_clear_cache(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.LPWStr)]
        private static extern string toggl_start(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string description,
            [MarshalAs(UnmanagedType.LPWStr)]
            string duration,
            UInt64 task_id,
            UInt64 project_id);

        public static string Start(
            string description,
            string duration,
            UInt64 task_id,
            UInt64 project_id)
        {
            return toggl_start(ctx,
                description,
                duration,
                task_id,
                project_id);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_add_project(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string time_entry_guid,
            UInt64 workspace_id,
            UInt64 client_id,
            [MarshalAs(UnmanagedType.LPWStr)]
            string project_name,
            [MarshalAs(UnmanagedType.I1)]
            bool is_private);

        public static bool AddProject(
            string time_entry_guid,
            UInt64 workspace_id,
            UInt64 client_id,
            string project_name,
            bool is_private)
        {
            return toggl_add_project(ctx,
                time_entry_guid,
                workspace_id,
                client_id,
                project_name,
                is_private);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_create_client(
            IntPtr context,
            UInt64 workspace_id,
            [MarshalAs(UnmanagedType.LPWStr)]
            string client_name);

        public static bool AddClient(
            UInt64 workspace_id,
            string client_name)
        {
            return toggl_create_client(ctx,
                workspace_id,
                client_name);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_update_channel(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string update_channel);

        public static bool SetUpdateChannel(string channel)
        {
            return toggl_set_update_channel(ctx, channel);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.LPWStr)]
        private static extern string toggl_get_update_channel(
            IntPtr context);

        public static string UpdateChannel()
        {
            return toggl_get_update_channel(ctx);
        }


        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_sync(
            IntPtr context);

        public static void Sync()
        {
            toggl_sync(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_timeline_toggle_recording(
            IntPtr context,
            [MarshalAs(UnmanagedType.I1)]
            bool record_timeline);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_timeline_is_recording_enabled(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_set_sleep(
            IntPtr context);

        public static void SetSleep()
        {
            toggl_set_sleep(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_set_wake(
            IntPtr context);

        public static void SetWake()
        {
            toggl_set_wake(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_disable_update_check(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_set_idle_seconds(
            IntPtr context,
            UInt64 idle_seconds);

        public static void SetIdleSeconds(UInt64 idle_seconds)
        {
            toggl_set_idle_seconds(ctx, idle_seconds);
        }

        // Shared helpers

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.LPWStr)]
        private static extern string toggl_format_tracking_time_duration(
            Int64 duration_in_seconds);

        public static string FormatDurationInSecondsHHMMSS(Int64 duration_in_seconds)
        {
            return toggl_format_tracking_time_duration(duration_in_seconds);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.LPWStr)]
        private static extern string toggl_run_script(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPStr)]
            string script,
            ref Int64 err);

        public static string RunScript(string script, ref Int64 err)
        {
            return toggl_run_script(ctx, script, ref err);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern bool toggl_check_view_struct_size(
    		int time_entry_view_item_size,
		    int autocomplete_view_item_size,
		    int view_item_size,
		    int settings_size);

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

        // Start

        public static bool Start(string version)
        {
            ctx = toggl_context_init("windows_native_app", version);

			toggl_set_environment(ctx, "production");

            string cacert_path = Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory,
                "cacert.pem");
            toggl_set_cacert_path(ctx, cacert_path);

            bool valid = toggl_check_view_struct_size(
                Marshal.SizeOf(new TimeEntry()),
                Marshal.SizeOf(new AutocompleteItem()),
                Marshal.SizeOf(new Model()),
                Marshal.SizeOf(new Settings()));
            if (!valid) {
                throw new System.InvalidOperationException("Invalid struct size, please check log file(s)");
            }

            // Wire up events
            toggl_on_show_app(ctx, delegate(bool open)
            {
                OnApp(open);
            });

            toggl_on_error(ctx, delegate(string errmsg, bool user_error)
            {
                OnError(errmsg, user_error);
            });

            toggl_on_online_state(ctx, delegate(Int64 state)
            {
                OnOnlineState(state);
            });

            toggl_on_login(ctx, delegate(bool open, UInt64 user_id)
            {
                OnLogin(open, user_id);
            });

            toggl_on_reminder(ctx, delegate(string title, string informative_text)
            {
                OnReminder(title, informative_text);
            });

            toggl_on_time_entry_list(ctx, delegate(bool open, IntPtr first)
            {
                OnTimeEntryList(open, ConvertToTimeEntryList(first));
            });

            toggl_on_time_entry_autocomplete(ctx, delegate(IntPtr first)
            {
                OnTimeEntryAutocomplete(ConvertToAutocompleteList(first));
            });

            toggl_on_mini_timer_autocomplete(ctx, delegate(IntPtr first)
            {
                OnMinitimerAutocomplete(ConvertToAutocompleteList(first));
            });

            toggl_on_project_autocomplete(ctx, delegate(IntPtr first)
            {
                OnProjectAutocomplete(ConvertToAutocompleteList(first));
            });

            toggl_on_time_entry_editor(ctx, delegate(
                bool open,
                ref TimeEntry te,
                string focused_field_name)
            {
                OnTimeEntryEditor(open, te, focused_field_name);
            });

            toggl_on_workspace_select(ctx, delegate(IntPtr first)
            {
                OnWorkspaceSelect(ConvertToViewItemList(first));
            });

            toggl_on_client_select(ctx, delegate(IntPtr first)
            {
                OnClientSelect(ConvertToViewItemList(first));
            });

            toggl_on_tags(ctx, delegate(IntPtr first)
            {
                OnTags(ConvertToViewItemList(first));
            });

            toggl_on_settings(ctx, delegate(bool open, ref Settings settings)
            {
                OnSettings(open, settings);
            });

            toggl_on_timer_state(ctx, delegate(IntPtr te)
            {
                if (te == IntPtr.Zero)
                {
                    OnStoppedTimerState();
                    return;
                }
                TimeEntry view =
                    (TimeEntry)Marshal.PtrToStructure(
                    te, typeof(TimeEntry));
                OnRunningTimerState(view);
            });

            toggl_on_url(ctx, delegate(string url)
            {
                OnURL(url);
            });

            toggl_on_idle_notification(ctx, delegate(
                string guid,
                string since,
                string duration,
                UInt64 started,
                string description)
            {
                OnIdleNotification(guid, since, duration, started, description);
            });

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
            installPendingUpdates(updatePath);

            // Configure log, db path
            Directory.CreateDirectory(path);

            string logPath = Path.Combine(path, "toggldesktop.log");
            toggl_set_log_path(logPath);
            toggl_set_log_level("debug");

            // Rename database file, if not done yet
            string olddatabasepath = Path.Combine(path, "kopsik.db");
            string databasePath = Path.Combine(path, "toggldesktop.db");
            if (File.Exists(olddatabasepath) && !File.Exists(databasePath))
            {
                File.Move(olddatabasepath, databasePath);
            }

            if (!toggl_set_db_path(ctx, databasePath))
            {
                throw new System.Exception("Failed to initialize database at " + databasePath);
            }

            toggl_set_update_path(ctx, updatePath);

            // Start pumping UI events
            return toggl_ui_start(ctx);
        }

        private static void installPendingUpdates(string updatePath)
        {
            if (!Directory.Exists(updatePath))
            {
                return;
            }

            DirectoryInfo di = new DirectoryInfo(updatePath);
            FileInfo[] files = di.GetFiles("TogglDesktopInstaller*.exe",
                SearchOption.TopDirectoryOnly);
            if (files.Length > 1)
            {
                // Somethings fubar. Delete the updates to start over
                foreach (FileInfo fi in files)
                {
                    fi.Delete();
                }
                return;
            }

            if (files.Length < 1) 
            {
                return;
            }

            string updater = Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory,
                "TogglDesktopUpdater.exe");
            if (!File.Exists(updater))
            {
                Console.WriteLine("TogglDesktopUpdater.exe not found");
                return;
            }

            ProcessStartInfo psi = new ProcessStartInfo();
            psi.FileName = updater;
            psi.Arguments = Process.GetCurrentProcess().Id.ToString()
                + " " + string.Format("\"{0}\"", files[0].FullName)
                + " " + string.Format("\"{0}\"", System.Reflection.Assembly.GetEntryAssembly().Location);
            Process process = Process.Start(psi);
            if (!process.HasExited && process.Id != 0)
            {
                // Update has started. Quit, installer will restart me.
                Environment.Exit(0);
                return;
            }

            Console.WriteLine("Failed to start updater process");
        }

        public static List<Model> ConvertToViewItemList(IntPtr first)
        {
            List<Model> list = new List<Model>();
            if (IntPtr.Zero == first)
            {
                return list;
            }
            Model n = (Model)Marshal.PtrToStructure(
                first, typeof(Model));
            while (true)
            {
                list.Add(n);
                if (n.Next == IntPtr.Zero)
                {
                    break;
                }
                n = (Model)Marshal.PtrToStructure(
                    n.Next, typeof(Model));
            };
            return list;
        }

        private static List<AutocompleteItem> ConvertToAutocompleteList(IntPtr first)
        {
            List<AutocompleteItem> list = new List<AutocompleteItem>();
            if (IntPtr.Zero == first)
            {
                return list;
            }
            AutocompleteItem n = (AutocompleteItem)Marshal.PtrToStructure(
                first, typeof(AutocompleteItem));
            while (true)
            {
                list.Add(n);
                if (n.Next == IntPtr.Zero)
                {
                    break;
                }
                n = (AutocompleteItem)Marshal.PtrToStructure(
                    n.Next, typeof(AutocompleteItem));
            };
            return list;
        }

        private static List<TimeEntry> ConvertToTimeEntryList(IntPtr first)
        {
            List<TimeEntry> list = new List<TimeEntry>();
            if (IntPtr.Zero == first)
            {
                return list;
            }
            TimeEntry n = (TimeEntry)Marshal.PtrToStructure(
                first, typeof(TimeEntry));

            while (true)
            {
                list.Add(n);
                if (n.Next == IntPtr.Zero)
                {
                    break;
                }
                n = (TimeEntry)Marshal.PtrToStructure(
                    n.Next, typeof(TimeEntry));
            };
            return list;
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
            object value = Registry.GetValue(
                "HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Toggl\\TogglDesktop",
                "UpdateCheckDisabled",
                false);
            if (value == null)
            {
                return false;
            }
            return Convert.ToBoolean(value);
        }
    }
}
