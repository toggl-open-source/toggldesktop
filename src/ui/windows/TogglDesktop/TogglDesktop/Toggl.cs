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

        private const string dll = "TogglDesktopDLL.dll";
        private const CharSet charset = CharSet.Ansi;
        private const CallingConvention convention = CallingConvention.Cdecl;

        private static string DecodeString(string default_encoded_text)
        {
            if (default_encoded_text == null)
            {
                return null;
            }
            byte[] b = Encoding.Default.GetBytes(default_encoded_text);
            return Encoding.UTF8.GetString(b);
        }

        private static string EncodeString(string utf8_encoded_text)
        {
            if (utf8_encoded_text == null)
            {
                return null;
            }
            byte[] b = Encoding.UTF8.GetBytes(utf8_encoded_text);
            return Encoding.Default.GetString(b);
        }

        // Models

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct TimeEntry
        {
            public Int64 DurationInSeconds;
            public string Description;
            public string ProjectAndTaskLabel;
            public string TaskLabel;
            public string ProjectLabel;
            public string ClientLabel;
            public UInt64 WID;
            public UInt64 PID;
            public UInt64 TID;
            public string Duration;
            public string Color;
            public string GUID;
            [MarshalAs(UnmanagedType.I1)]
            public bool Billable;
            public string Tags;
            public UInt64 Started;
            public UInt64 Ended;
            public string StartTimeString;
            public string EndTimeString;
            public UInt64 UpdatedAt;
            [MarshalAs(UnmanagedType.I1)]
            public bool DurOnly;
            public string DateHeader;
            public string DateDuration;
            [MarshalAs(UnmanagedType.I1)]
            public bool IsHeader;
            [MarshalAs(UnmanagedType.I1)]
            public bool CanAddProjects;
            [MarshalAs(UnmanagedType.I1)]
            public bool CanSeeBillable;
            public UInt64 DefaultWID;
            public IntPtr Next;

            public TimeEntry ToUTF8()
            {
                Description = DecodeString(Description);
                ProjectAndTaskLabel = DecodeString(ProjectAndTaskLabel);
                ProjectLabel = DecodeString(ProjectLabel);
                TaskLabel = DecodeString(TaskLabel);
                ClientLabel = DecodeString(ClientLabel);
                Duration = DecodeString(Duration);
                Tags = DecodeString(Tags);
                DateHeader = DecodeString(DateHeader);
                DateDuration = DecodeString(DateDuration);
                return this;
            }
        }

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct AutocompleteItem
        {
            public string Text;
            public string Description;
            public string ProjectAndTaskLabel;
            public string TaskLabel;
            public string ProjectLabel;
            public string ClientLabel;
            public string ProjectColor;
            public UInt64 TaskID;
            public UInt64 ProjectID;
            public UInt64 Type;
            public IntPtr Next;

            public override string ToString()
            {
                return this.Text;
            }

            public AutocompleteItem ToUTF8()
            {
                Text = DecodeString(Text);
                Description = DecodeString(Description);
                ProjectAndTaskLabel = DecodeString(ProjectAndTaskLabel);
                TaskLabel = DecodeString(TaskLabel);
                ProjectLabel = DecodeString(ProjectLabel);
                ClientLabel = DecodeString(ClientLabel);
                ProjectColor = DecodeString(ProjectColor);
                return this;
            }
        }

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct Model
        {
            public UInt64 ID;
            public UInt64 WID;
            public string GUID;
            public string Name;
            public IntPtr Next;

            public Model ToUTF8()
            {
                Name = DecodeString(Name);
                return this;
            }

            public override string ToString()
            {
                return Name;
            }
        }

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct Settings
        {
            [MarshalAs(UnmanagedType.I1)]
            public bool UseProxy;
            public string ProxyHost;
            public UInt64 ProxyPort;
            public string ProxyUsername;
            public string ProxyPassword;
            [MarshalAs(UnmanagedType.I1)]
            public bool UseIdleDetection;
            [MarshalAs(UnmanagedType.I1)]
            public bool MenubarTimer;
            [MarshalAs(UnmanagedType.I1)]
            public bool DockIcon;
            [MarshalAs(UnmanagedType.I1)]
            public bool OnTop;
            [MarshalAs(UnmanagedType.I1)]
            public bool Reminder;
            [MarshalAs(UnmanagedType.I1)]
            public bool RecordTimeline;

            public Settings ToUTF8()
            {
                ProxyHost = DecodeString(ProxyHost);
                ProxyUsername = DecodeString(ProxyUsername);
                ProxyPassword = DecodeString(ProxyPassword);
                return this;
            }
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
            string errmsg,
            [MarshalAs(UnmanagedType.I1)]
            bool user_error);

        public delegate void DisplayError(
            string errmsg,
            bool user_error);

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayOnlineState(
            [MarshalAs(UnmanagedType.I1)]
            bool is_online,
            string reason);

        public delegate void DisplayOnlineState(
            bool is_online,
            string reason);

        [UnmanagedFunctionPointer(convention)]
        private delegate void TogglDisplayURL(
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
            string title,
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
            string guid,
            string since,
            string duration,
            UInt64 started);

        public delegate void DisplayIdleNotification(
            string guid,
            string since,
            string duration,
            UInt64 started);

        // Initialize/destroy an instance of the app

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern System.IntPtr toggl_context_init(
            string app_name,
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
            string environment);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_disable_update_check(
            IntPtr context);

        // CA cert bundle path must be configured from UI

        [DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = convention)]
        private static extern void toggl_set_cacert_path_utf16(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string path);

        // DB path must be configured from UI

        [DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_db_path_utf16(
            IntPtr context,
            [MarshalAs(UnmanagedType.LPWStr)]
            string path);

        // Log path must be configured from UI

        [DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = convention)]
        private static extern void toggl_set_log_path_utf16(
            [MarshalAs(UnmanagedType.LPWStr)]
            string path);

        // Log level is optional

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_set_log_level(
            string level);

        // API URL can be overriden from UI. Optional

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern System.IntPtr toggl_set_api_url(
            IntPtr context,
            string path);

        // WebSocket URL can be overriden from UI. Optional

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern System.IntPtr toggl_set_websocket_url(
            string path);

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
        private static extern bool toggl_login(
            IntPtr context,
            string email,
            string password);

        public static bool Login(string email, string password)
        {
            return toggl_login(ctx, EncodeString(email), EncodeString(password));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_google_login(
            IntPtr context,
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
            string topic,
            string details,
            string filename);

        public static bool SendFeedback(
            string topic,
            string details,
            string filename)
        {
            return toggl_feedback_send(ctx,
                EncodeString(topic), EncodeString(details), EncodeString(filename));
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
            string guid,
            [MarshalAs(UnmanagedType.I1)]
            bool edit_running_time_entry,
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
            string guid);

        public static bool DeleteTimeEntry(string guid)
        {
            return toggl_delete_time_entry(ctx, guid);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_duration(
            IntPtr context,
            string guid,
            string value);

        public static bool SetTimeEntryDuration(string guid, string value)
        {
            return toggl_set_time_entry_duration(ctx, guid, EncodeString(value));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_project(
            IntPtr context,
            string guid,
            UInt64 task_id,
            UInt64 project_id,
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
        private static extern bool toggl_set_time_entry_start_iso_8601(
            IntPtr context,
            string guid,
            string value);

        public static bool SetTimeEntryStart(string guid, string value)
        {
            return toggl_set_time_entry_start_iso_8601(ctx, guid, EncodeString(value));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_end_iso_8601(
            IntPtr context,
            string guid,
            string value);

        public static bool SetTimeEntryEnd(string guid, string value)
        {
            return toggl_set_time_entry_end_iso_8601(ctx, guid, EncodeString(value));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_tags(
            IntPtr context,
            string guid,
            string value);

        public static bool SetTimeEntryTags(string guid, string value)
        {
            return toggl_set_time_entry_tags(ctx, guid, EncodeString(value));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_time_entry_billable(
            IntPtr context,
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
            string guid,
            string value);

        public static bool SetTimeEntryDescription(string guid, string value)
        {
            return toggl_set_time_entry_description(ctx, guid, EncodeString(value));
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
            string guid,
            UInt64 at);

        public static bool DiscardTimeAt(string guid, UInt64 at)
        {
            return toggl_discard_time_at(ctx, guid, at);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_settings(
            IntPtr context,
            [MarshalAs(UnmanagedType.I1)]
            bool use_idle_detection,
            [MarshalAs(UnmanagedType.I1)]
            bool menubar_timer,
            [MarshalAs(UnmanagedType.I1)]
            bool dock_icon,
            [MarshalAs(UnmanagedType.I1)]
            bool on_top,
            [MarshalAs(UnmanagedType.I1)]
            bool reminder);

        public static bool SetSettings(Settings settings)
        {
            if (!toggl_set_settings(
                ctx,
                settings.UseIdleDetection,
                true,
                true,
                settings.OnTop,
                settings.Reminder))
            {
                return false;
            }

            if (!toggl_set_proxy_settings(
                ctx,
                settings.UseProxy,
                EncodeString(settings.ProxyHost),
                settings.ProxyPort,
                EncodeString(settings.ProxyUsername),
                EncodeString(settings.ProxyPassword)))
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
            string proxy_host,
            UInt64 proxy_port,
            string proxy_username,
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
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_start(
            IntPtr context,
            string description,
            string duration,
            UInt64 task_id,
            UInt64 project_id);

        public static bool Start(
            string description,
            string duration,
            UInt64 task_id,
            UInt64 project_id)
        {
            return toggl_start(ctx,
                EncodeString(description),
                EncodeString(duration),
                task_id,
                project_id);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_add_project(
            IntPtr context,
            string time_entry_guid,
            UInt64 workspace_id,
            UInt64 client_id,
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
                EncodeString(project_name),
                is_private);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_set_update_channel(
            IntPtr context,
            string update_channel);

        public static bool SetUpdateChannel(string channel)
        {
            return toggl_set_update_channel(ctx, channel);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.LPStr)]
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
        private static extern void toggl_set_idle_seconds(
            IntPtr context,
            UInt64 idle_seconds);

        public static void SetIdleSeconds(UInt64 idle_seconds)
        {
            toggl_set_idle_seconds(ctx, idle_seconds);
        }

        // Shared helpers

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool toggl_parse_time(
            string input,
            ref int hours,
            ref int minutes);

        public static bool ParseTime(
            string input,
            ref int hours,
            ref int minutes)
        {
            return toggl_parse_time(input, ref hours, ref minutes);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        private static extern string toggl_format_duration_in_seconds_hhmmss(
            Int64 duration_in_seconds);

        public static string FormatDurationInSecondsHHMMSS(Int64 duration_in_seconds)
        {
            return toggl_format_duration_in_seconds_hhmmss(duration_in_seconds);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void toggl_check_view_struct_size(
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

            string cacert_path = Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory,
                "cacert.pem");
            toggl_set_cacert_path_utf16(ctx, cacert_path);

            toggl_check_view_struct_size(
                Marshal.SizeOf(new TimeEntry()),
                Marshal.SizeOf(new AutocompleteItem()),
                Marshal.SizeOf(new Model()),
                Marshal.SizeOf(new Settings()));

            // Wire up events
            toggl_on_show_app(ctx, delegate(bool open)
            {
                OnApp(open);
            });

            toggl_on_error(ctx, delegate(string errmsg, bool user_error)
            {
                OnError(DecodeString(errmsg), user_error);
            });

            toggl_on_online_state(ctx, delegate(bool is_online, string reason)
            {
                OnOnlineState(is_online, DecodeString(reason));
            });

            toggl_on_login(ctx, delegate(bool open, UInt64 user_id)
            {
                OnLogin(open, user_id);
            });

            toggl_on_reminder(ctx, delegate(string title, string informative_text)
            {
                OnReminder(DecodeString(title), DecodeString(informative_text));
            });

            toggl_on_time_entry_list(ctx, delegate(bool open, IntPtr first)
            {
                OnTimeEntryList(open, ConvertToTimeEntryList(first));
            });

            toggl_on_time_entry_autocomplete(ctx, delegate(IntPtr first)
            {
                OnTimeEntryAutocomplete(ConvertToAutocompleteList(first));
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
                OnTimeEntryEditor(open, te.ToUTF8(), focused_field_name);
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
                OnSettings(open, settings.ToUTF8());
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
                OnRunningTimerState(view.ToUTF8());
            });

            toggl_on_url(ctx, delegate(string url)
            {
                OnURL(url);
            });

            toggl_on_idle_notification(ctx, delegate(
                string guid, string since, string duration, UInt64 started)
            {
                OnIdleNotification(
                    DecodeString(guid),
                    DecodeString(since),
                    DecodeString(duration),
                    started);
            });

            toggl_set_environment(ctx, Properties.Settings.Default.Environment);

            if (IsUpdateCheckDisabled())
            {
                toggl_disable_update_check(ctx);
            }

            // Configure log, db path
            string path = Path.Combine(Environment.GetFolderPath(
                Environment.SpecialFolder.ApplicationData), "Kopsik");
            System.IO.Directory.CreateDirectory(path);
            string logPath = Path.Combine(path, "kopsik.log");
            toggl_set_log_path_utf16(logPath);
            toggl_set_log_level("debug");
            string databasePath = Path.Combine(path, "kopsik.db");
            if (!toggl_set_db_path_utf16(ctx, databasePath))
            {
                throw new System.Exception("Failed to initialize database at " + databasePath);
            }

            // Start pumping UI events
            return toggl_ui_start(ctx);
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
                list.Add(n.ToUTF8());
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
                list.Add(n.ToUTF8());
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
                list.Add(n.ToUTF8());
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
