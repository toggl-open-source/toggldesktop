using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Reflection;
using System.Text;
using System.Collections.Generic;

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

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct Update
        {
            public string UpdateChannel;
            [MarshalAs(UnmanagedType.I1)]
            public bool IsChecking;
            [MarshalAs(UnmanagedType.I1)]
            public bool IsUpdateAvailable;
            public string URL;
            public string Version;
        }

        // Callbacks

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayApp(
            [MarshalAs(UnmanagedType.I1)]
            bool open);

        public delegate void DisplayApp(
            bool open);

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayError(
            string errmsg,
            [MarshalAs(UnmanagedType.I1)]
            bool user_error);

        public delegate void DisplayError(
            string errmsg,
            bool user_error);

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayUpdate(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            ref Update view);

        public delegate void DisplayUpdate(
            bool open,
            Update view);

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayOnlineState(
            [MarshalAs(UnmanagedType.I1)]
            bool is_online,
            string reason);

        public delegate void DisplayOnlineState(
            bool is_online,
            string reason);

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayURL(
            string url);

        public delegate void DisplayURL(
            string url);

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayLogin(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            UInt64 user_id);

        public delegate void DisplayLogin(
            bool open,
            UInt64 user_id);

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayReminder(
            string title,
            string informative_text);

        public delegate void DisplayReminder(
            string title,
            string informative_text);

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayTimeEntryList(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            IntPtr first);

        public delegate void DisplayTimeEntryList(
            bool open,
            List<TimeEntry> list);

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayAutocomplete(
            IntPtr first);

        public delegate void DisplayAutocomplete(
            List<AutocompleteItem> list);

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayViewItems(
            IntPtr first);

        public delegate void DisplayViewItems(
            List<Model> list);
        
        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayTimeEntryEditor(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            ref TimeEntry te,
            string focused_field_name);

        public delegate void DisplayTimeEntryEditor(
            bool open,
            TimeEntry te,
            string focused_field_name);

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplaySettings(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            ref Settings settings);

        public delegate void DisplaySettings(
            bool open,
            Settings settings);

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayTimerState(
            IntPtr te);

        public delegate void DisplayRunningTimerState(
            TimeEntry te);

        public delegate void DisplayStoppedTimerState();

        [UnmanagedFunctionPointer(convention)]
        private delegate void KopsikDisplayIdleNotification(
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
        private static extern System.IntPtr kopsik_context_init(
            string app_name,
            string app_version);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_context_clear(
            IntPtr context);

        public static void Clear()
        {
            kopsik_context_clear(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_set_environment(
            IntPtr context,
            string environment);

        // CA cert bundle path must be configured from UI

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_set_cacert_path(
            IntPtr context,
            string path);

        // DB path must be configured from UI

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_set_db_path(
            IntPtr context,
            string path);

        // Log path must be configured from UI

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_set_log_path(
            string path);

        // Log level is optional

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_set_log_level(
            string level);

        // API URL can be overriden from UI. Optional

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern System.IntPtr kopsik_set_api_url(
            IntPtr context,
            string path);

        // WebSocket URL can be overriden from UI. Optional

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern System.IntPtr kopsik_set_websocket_url(
            string path);

        // Configure the UI callbacks. Required.

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_app(
            IntPtr context,
            KopsikDisplayApp cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_error(
            IntPtr context,
            KopsikDisplayError cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_update(
            IntPtr context,
            KopsikDisplayUpdate cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_online_state(
            IntPtr context,
            KopsikDisplayOnlineState cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_url(
            IntPtr context,
            KopsikDisplayURL cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_login(
            IntPtr context,
            KopsikDisplayLogin cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_reminder(
            IntPtr context,
            KopsikDisplayReminder cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_time_entry_list(
            IntPtr context,
            KopsikDisplayTimeEntryList cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_time_entry_autocomplete(
            IntPtr context,
            KopsikDisplayAutocomplete cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_project_autocomplete(
            IntPtr context,
            KopsikDisplayAutocomplete cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_workspace_select(
            IntPtr context,
            KopsikDisplayViewItems cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_client_select(
            IntPtr context,
            KopsikDisplayViewItems cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_tags(
            IntPtr context,
            KopsikDisplayViewItems cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_time_entry_editor(
            IntPtr context,
            KopsikDisplayTimeEntryEditor cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_settings(
            IntPtr context,
            KopsikDisplaySettings cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_timer_state(
            IntPtr context,
            KopsikDisplayTimerState cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_idle_notification(
            IntPtr context,
            KopsikDisplayIdleNotification cb);

        // After UI callbacks are configured, start pumping UI events

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_context_start_events(
            IntPtr context);

        // User interaction with the app

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_login(
            IntPtr context,
            string email,
            string password);

        public static bool Login(string email, string password)
        {
            return kopsik_login(ctx, EncodeString(email), EncodeString(password));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_google_login(
            IntPtr context,
            string access_token);

        public static bool GoogleLogin(string access_token)
        {
            return kopsik_google_login(ctx, access_token);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_password_forgot(
            IntPtr context);

        public static void PasswordForgot()
        {
            kopsik_password_forgot(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_open_in_browser(
            IntPtr context);

        public static void OpenInBrowser()
        {
            kopsik_open_in_browser(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_get_support(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_feedback_send(
            IntPtr context,
            string topic,
            string details,
            string filename);

        public static bool SendFeedback(
            string topic,
            string details,
            string filename)
        {
            return kopsik_feedback_send(ctx,
                EncodeString(topic), EncodeString(details), EncodeString(filename));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern void kopsik_about(
            IntPtr context);

        public static void About()
        {
            kopsik_about(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_view_time_entry_list(
            IntPtr context);

        public static bool ViewTimeEntryList()
        {
            return kopsik_view_time_entry_list(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_edit(
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
            kopsik_edit(ctx, guid, edit_running_time_entry, focused_field_name);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_edit_preferences(
            IntPtr context);

        public static void EditPreferences()
        {
            kopsik_edit_preferences(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_continue(
            IntPtr context,
            string guid);

        public static bool Continue(string guid)
        {
            return kopsik_continue(ctx, guid);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_continue_latest(
            IntPtr context);

        public static bool ContinueLatest()
        {
            return kopsik_continue_latest(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_delete_time_entry(
            IntPtr context,
            string guid);

        public static bool DeleteTimeEntry(string guid)
        {
            return kopsik_delete_time_entry(ctx, guid);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_set_time_entry_duration(
            IntPtr context,
            string guid,
            string value);

        public static bool SetTimeEntryDuration(string guid, string value)
        {
            return kopsik_set_time_entry_duration(ctx, guid, EncodeString(value));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_set_time_entry_project(
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
            return kopsik_set_time_entry_project(ctx,
                guid, task_id, project_id, project_guid);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_set_time_entry_start_iso_8601(
            IntPtr context,
            string guid,
            string value);

        public static bool SetTimeEntryStart(string guid, string value)
        {
            return kopsik_set_time_entry_start_iso_8601(ctx, guid, EncodeString(value));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_set_time_entry_end_iso_8601(
            IntPtr context,
            string guid,
            string value);

        public static bool SetTimeEntryEnd(string guid, string value)
        {
            return kopsik_set_time_entry_end_iso_8601(ctx, guid, EncodeString(value));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_set_time_entry_tags(
            IntPtr context,
            string guid,
            string value);

        public static bool SetTimeEntryTags(string guid, string value)
        {
            return kopsik_set_time_entry_tags(ctx, guid, EncodeString(value));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_set_time_entry_billable(
            IntPtr context,
            string guid,
            [MarshalAs(UnmanagedType.I1)]
            bool billable);

        public static bool SetTimeEntryBillable(string guid, bool billable)
        {
            return kopsik_set_time_entry_billable(ctx, guid, billable);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_set_time_entry_description(
            IntPtr context,
            string guid,
            string value);

        public static bool SetTimeEntryDescription(string guid, string value)
        {
            return kopsik_set_time_entry_description(ctx, guid, EncodeString(value));
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_stop(
            IntPtr context);

        public static bool Stop()
        {
            return kopsik_stop(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_discard_time_at(
            IntPtr context,
            string guid,
            UInt64 at);

        public static bool DiscardTimeAt(string guid, UInt64 at)
        {
            return kopsik_discard_time_at(ctx, guid, at);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_set_settings(
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

        public static bool SetSettings(
            bool use_idle_detection,
            bool menubar_timer,
            bool dock_icon,
            bool on_top,
            bool reminder)
        {
            return kopsik_set_settings(
                ctx,
                use_idle_detection,
                menubar_timer,
                dock_icon,
                on_top,
                reminder);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_set_proxy_settings(
            IntPtr context,
            [MarshalAs(UnmanagedType.I1)]
            bool use_proxy,
            string proxy_host,
            UInt64 proxy_port,
            string proxy_username,
            string proxy_password);

        public static bool SetProxySettings(
            bool use_proxy,
            string proxy_host,
            UInt64 proxy_port,
            string proxy_username,
            string proxy_password)
        {
            return kopsik_set_proxy_settings(ctx,
                use_proxy,
                EncodeString(proxy_host),
                proxy_port,
                EncodeString(proxy_username),
                EncodeString(proxy_password));
        }
 
        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_logout(
            IntPtr context);

        public static bool Logout()
        {
            return kopsik_logout(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_clear_cache(
            IntPtr context);

        public static bool ClearCache()
        {
            return kopsik_clear_cache(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_start(
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
            return kopsik_start(ctx,
                EncodeString(description),
                EncodeString(duration),
                task_id,
                project_id);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_add_project(
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
            return kopsik_add_project(ctx,
                time_entry_guid,
                workspace_id,
                client_id,
                EncodeString(project_name),
                is_private);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_set_update_channel(
            IntPtr context,
            string update_channel);

        public static bool SetUpdateChannel(string channel)
        {
            return kopsik_set_update_channel(ctx, channel);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_sync(
            IntPtr context);

        public static void Sync()
        {
            kopsik_sync(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_timeline_toggle_recording(
            IntPtr context);

        public static void ToggleTimelineRecording()
        {
            kopsik_timeline_toggle_recording(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_set_sleep(
            IntPtr context);

        public static void SetSleep()
        {
            kopsik_set_sleep(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_set_wake(
            IntPtr context);

        public static void SetWake()
        {
            kopsik_set_wake(ctx);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_set_idle_seconds(
            IntPtr context,
            UInt64 idle_seconds);

        public static void SetIdleSeconds(UInt64 idle_seconds)
        {
            kopsik_set_idle_seconds(ctx, idle_seconds);
        }

        // Shared helpers

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_parse_time(
            string input,
            ref int hours,
            ref int minutes);

        public static bool ParseTime(
            string input,
            ref int hours,
            ref int minutes)
        {
            return kopsik_parse_time(input, ref hours, ref minutes);
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_format_duration_in_seconds_hhmmss(
            Int64 duration_in_seconds,
            StringBuilder sb,
            int max_strlen);

        public static string FormatDurationInSecondsHHMMSS(Int64 duration_in_seconds)
        {
            const int duration_len = 20;
            StringBuilder sb = new StringBuilder(duration_len);
            kopsik_format_duration_in_seconds_hhmmss(
                duration_in_seconds, sb, duration_len);
            return sb.ToString();
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_format_duration_in_seconds_hhmm(
            Int64 duration_in_seconds,
            StringBuilder sb,
            int max_strlen);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool kopsik_format_duration_in_seconds_pretty_hhmm(
            Int64 duration_in_seconds,
            StringBuilder sb,
            int max_strlen);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern Int64 kopsik_parse_duration_string_into_seconds(
            string duration_string);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_debug(
            string text);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_check_view_item_size(
    		int time_entry_view_item_size,
		    int autocomplete_view_item_size,
		    int view_item_size,
		    int settings_size,
            int update_view_item_size);

        // Events for C#

        public static event DisplayApp OnApp = delegate { };
        public static event DisplayError OnError = delegate { };
        public static event DisplayUpdate OnUpdate = delegate { };
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
            ctx = kopsik_context_init("windows_native_app", version);

            string cacert_path = Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory,
                "cacert.pem");
            kopsik_set_cacert_path(ctx, cacert_path);

            kopsik_check_view_item_size(
                Marshal.SizeOf(new TimeEntry()),
                Marshal.SizeOf(new AutocompleteItem()),
                Marshal.SizeOf(new Model()),
                Marshal.SizeOf(new Settings()),
                Marshal.SizeOf(new Update()));

            // Wire up events
            kopsik_on_app(ctx, delegate(bool open)
            {
                OnApp(open);
            });

            kopsik_on_error(ctx, delegate(string errmsg, bool user_error)
            {
                OnError(DecodeString(errmsg), user_error);
            });

            kopsik_on_update(ctx, delegate(bool open, ref Update view)
            {
                OnUpdate(open, view);
            });

            kopsik_on_online_state(ctx, delegate(bool is_online, string reason)
            {
                OnOnlineState(is_online, DecodeString(reason));
            });

            kopsik_on_login(ctx, delegate(bool open, UInt64 user_id)
            {
                OnLogin(open, user_id);
            });

            kopsik_on_reminder(ctx, delegate(string title, string informative_text)
            {
                OnReminder(DecodeString(title), DecodeString(informative_text));
            });

            kopsik_on_time_entry_list(ctx, delegate(bool open, IntPtr first)
            {
                OnTimeEntryList(open, ConvertToTimeEntryList(first));
            });

            kopsik_on_time_entry_autocomplete(ctx, delegate(IntPtr first)
            {
                OnTimeEntryAutocomplete(ConvertToAutocompleteList(first));
            });

            kopsik_on_project_autocomplete(ctx, delegate(IntPtr first)
            {
                OnProjectAutocomplete(ConvertToAutocompleteList(first));
            });

            kopsik_on_time_entry_editor(ctx, delegate(
                bool open,
                ref TimeEntry te,
                string focused_field_name)
            {
                OnTimeEntryEditor(open, te.ToUTF8(), focused_field_name);
            });

            kopsik_on_workspace_select(ctx, delegate(IntPtr first)
            {
                OnWorkspaceSelect(ConvertToViewItemList(first));
            });

            kopsik_on_client_select(ctx, delegate(IntPtr first)
            {
                OnClientSelect(ConvertToViewItemList(first));
            });

            kopsik_on_tags(ctx, delegate(IntPtr first)
            {
                OnTags(ConvertToViewItemList(first));
            });

            kopsik_on_settings(ctx, delegate(bool open, ref Settings settings)
            {
                OnSettings(open, settings.ToUTF8());
            });

            kopsik_on_timer_state(ctx, delegate(IntPtr te)
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

            kopsik_on_url(ctx, delegate(string url)
            {
                OnURL(url);
            });

            kopsik_on_idle_notification(ctx, delegate(
                string guid, string since, string duration, UInt64 started)
            {
                OnIdleNotification(
                    DecodeString(guid),
                    DecodeString(since),
                    DecodeString(duration),
                    started);
            });

            kopsik_set_environment(ctx, Properties.Settings.Default.Environment);

            // Configure log, db path
            string path = Path.Combine(Environment.GetFolderPath(
                Environment.SpecialFolder.ApplicationData), "Kopsik");
            System.IO.Directory.CreateDirectory(path);
            string log_path = Path.Combine(path, "kopsik.log");
            kopsik_set_log_path(log_path);
            kopsik_set_log_level("debug");
            string databasePath = Path.Combine(path, "kopsik.db");
            if (!kopsik_set_db_path(ctx, databasePath))
            {
                throw new System.Exception("Failed to initialize database at " + databasePath);
            }

            // Start pumping UI events
            return kopsik_context_start_events(ctx);
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
            new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Local);

        public static DateTime DateTimeFromUnix(UInt64 unix_seconds)
        {
            return UnixEpoch.AddSeconds(unix_seconds);
        }

        public static void NewError(string errmsg, bool user_error)
        {
            OnError(errmsg, user_error);
        }

    }
}
