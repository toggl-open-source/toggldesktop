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
    static class KopsikApi
    {
        public static IntPtr ctx = IntPtr.Zero;

        public const string Project = "project";
        public const string Duration = "duration";
        public const string Description = "description";

        private const string dll = "TogglDesktopDLL.dll";
        private const CharSet charset = CharSet.Ansi;
        private const CallingConvention convention = CallingConvention.Cdecl;

        // Models

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct KopsikTimeEntryViewItem
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
            public string DateHeader;
            public string DateDuration;
            [MarshalAs(UnmanagedType.I1)]
            public bool DurOnly;
            [MarshalAs(UnmanagedType.I1)]
            public bool IsHeader;
            public IntPtr Next;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct KopsikAutocompleteItem
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
        }

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct KopsikViewItem
        {
            public UInt64 ID;
            public UInt64 WID;
            public string GUID;
            public string Name;
            public IntPtr Next;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct KopsikSettingsViewItem
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
            [MarshalAs(UnmanagedType.I1)]
            public bool IgnoreCert;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct KopsikUpdateViewItem
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
        public delegate void KopsikDisplayError(
            string errmsg,
            [MarshalAs(UnmanagedType.I1)]
            bool user_error);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikDisplayUpdate(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            ref KopsikUpdateViewItem view);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikDisplayOnlineState(
            [MarshalAs(UnmanagedType.I1)]
            bool is_online);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikDisplayURL(
            string url);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikDisplayLogin(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            UInt64 user_id);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikDisplayReminder(
            string title,
            string informative_text);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikDisplayTimeEntryList(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            ref KopsikTimeEntryViewItem first);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikDisplayAutocomplete(
            ref KopsikAutocompleteItem first);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikDisplayViewItems(
            ref KopsikViewItem first);
        
        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikDisplayTimeEntryEditor(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            ref KopsikTimeEntryViewItem te,
            string focused_field_name);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikDisplaySettings(
            [MarshalAs(UnmanagedType.I1)]
            bool open,
            ref KopsikSettingsViewItem settings);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikDisplayTimerState(
            IntPtr te);

        // Initialize/destroy an instance of the app

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern System.IntPtr kopsik_context_init(
            string app_name,
            string app_version);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_context_clear(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_set_environment(
            IntPtr context,
            string environment);

        // DB path must be configured from UI

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_set_db_path(
            IntPtr context,
            string path);

        // Log path must be configured from UI

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_set_log_path(
            string path);

        // Log level is optional

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_set_log_level(
            string level);

        // API URL can be overriden from UI. Optional

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern System.IntPtr kopsik_set_api_url(
            IntPtr context,
            string path);

        // WebSocket URL can be overriden from UI. Optional

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern System.IntPtr kopsik_set_websocket_url(
            string path);

        // Configure the UI callbacks. Required.

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        private static extern void kopsik_on_error(
            IntPtr context,
            KopsikDisplayError cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_update(
            IntPtr context,
            KopsikDisplayUpdate cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_online_state(
            IntPtr context,
            KopsikDisplayOnlineState cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_url(
            IntPtr context,
            KopsikDisplayURL cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_login(
            IntPtr context,
            KopsikDisplayLogin cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_reminder(
            IntPtr context,
            KopsikDisplayReminder cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_time_entry_list(
            IntPtr context,
            KopsikDisplayTimeEntryList cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_time_entry_autocomplete(
            IntPtr context,
            KopsikDisplayAutocomplete cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_project_autocomplete(
            IntPtr context,
            KopsikDisplayAutocomplete cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_workspace_select(
            IntPtr context,
            KopsikDisplayViewItems cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_client_select(
            IntPtr context,
            KopsikDisplayViewItems cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_tags(
            IntPtr context,
            KopsikDisplayViewItems cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_time_entry_editor(
            IntPtr context,
            KopsikDisplayTimeEntryEditor cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_settings(
            IntPtr context,
            KopsikDisplaySettings cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_on_timer_state(
            IntPtr context,
            KopsikDisplayTimerState cb);

        // After UI callbacks are configured, start pumping UI events

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_context_start_events(
            IntPtr context);

        // User interaction with the app

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_login(
            IntPtr context,
            string email,
            string password);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_password_forgot(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_open_in_browser(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_get_support(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_feedback_send(
            IntPtr context,
            string topic,
            string details,
            string filename);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern void kopsik_about(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_view_time_entry_list(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_edit(
            IntPtr context,
            string guid,
            [MarshalAs(UnmanagedType.I1)]
            bool edit_running_time_entry,
            string focused_field_name);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_edit_preferences(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_continue(
            IntPtr context,
            string guid);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_continue_latest(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_delete_time_entry(
            IntPtr context,
            string guid);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_set_time_entry_duration(
            IntPtr context,
            string guid,
            string value);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_set_time_entry_project(
            IntPtr context,
            string guid,
            UInt64 task_id,
            UInt64 project_id,
            string project_guid);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_set_time_entry_start_iso_8601(
            IntPtr context,
            string guid,
            string value);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_set_time_entry_end_iso_8601(
            IntPtr context,
            string guid,
            string value);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_set_time_entry_tags(
            IntPtr context,
            string guid,
            string value);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_set_time_entry_billable(
            IntPtr context,
            string guid,
            [MarshalAs(UnmanagedType.I1)]
            bool billable);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_set_time_entry_description(
            IntPtr context,
            string guid,
            string value);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_stop(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_stop_running_time_entry_at(
            IntPtr context,
            UInt64 at);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_set_settings(
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
            bool reminder,
            [MarshalAs(UnmanagedType.I1)]
            bool ignore_cert);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_set_proxy_settings(
            IntPtr context,
            [MarshalAs(UnmanagedType.I1)]
            bool use_proxy,
            string proxy_host,
            UInt64 proxy_port,
            string proxy_username,
            string proxy_password);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_logout(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_clear_cache(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_start(
            IntPtr context,
            string description,
            string duration,
            UInt64 task_id,
            UInt64 project_id);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_add_project(
            IntPtr context,
            string time_entry_guid,
            UInt64 workspace_id,
            UInt64 client_id,
            string project_name,
            [MarshalAs(UnmanagedType.I1)]
            bool is_private);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_check_for_updates(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_set_update_channel(
            IntPtr context,
            string update_channel);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern bool kopsik_user_can_see_billable_flag(
            IntPtr context,
            string time_entry_guid,
            ref bool can_see);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_sync(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_timeline_toggle_recording(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_set_sleep(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_set_wake(
            IntPtr context);

        // Shared helpers

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_parse_time(
            string input,
            ref int hours,
            ref int minutes);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_format_duration_in_seconds_hhmmss(
            Int64 duration_in_seconds,
            StringBuilder sb,
            int max_strlen);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_format_duration_in_seconds_hhmm(
            Int64 duration_in_seconds,
            StringBuilder sb,
            int max_strlen);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        [return: MarshalAs(UnmanagedType.I1)]
        public static extern bool kopsik_format_duration_in_seconds_pretty_hhmm(
            Int64 duration_in_seconds,
            StringBuilder sb,
            int max_strlen);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern Int64 kopsik_parse_duration_string_into_seconds(
            string duration_string);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_debug(
            IntPtr context,
            string text);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_check_view_item_size(
    		int time_entry_view_item_size,
		    int autocomplete_view_item_size,
		    int view_item_size,
		    int settings_size,
            int update_view_item_size);

        // Events for C#

        public static event KopsikApi.KopsikDisplayError OnError = delegate { };
        public static event KopsikApi.KopsikDisplayUpdate OnUpdate = delegate { };
        public static event KopsikApi.KopsikDisplayOnlineState OnOnlineState = delegate { };
        public static event KopsikApi.KopsikDisplayLogin OnLogin = delegate { };
        public static event KopsikApi.KopsikDisplayReminder OnReminder = delegate { };
        public static event KopsikApi.KopsikDisplayTimeEntryList OnTimeEntryList = delegate { };
        public static event KopsikApi.KopsikDisplayAutocomplete OnTimeEntryAutocomplete = delegate { };
        public static event KopsikApi.KopsikDisplayAutocomplete OnProjectAutocomplete = delegate { };
        public static event KopsikApi.KopsikDisplayTimeEntryEditor OnTimeEntryEditor = delegate { };
        public static event KopsikApi.KopsikDisplayViewItems OnWorkspaceSelect = delegate { };
        public static event KopsikApi.KopsikDisplayViewItems OnClientSelect = delegate { };
        public static event KopsikApi.KopsikDisplayViewItems OnTags = delegate { };
        public static event KopsikApi.KopsikDisplaySettings OnSettings = delegate { };
        public static event KopsikApi.KopsikDisplayTimerState OnTimerState = delegate { };
        public static event KopsikApi.KopsikDisplayURL OnURL = delegate { };

        // Start

        public static bool Start(string version)
        {
            ctx = kopsik_context_init("windows_native_app", version);

            StringBuilder sb = new StringBuilder();
            
            sb.Append("C# sizeof(Int64)=");
            sb.Append(Marshal.SizeOf((Int64)0));
            sb.Append(", sizeof(UInt64)=");
            sb.Append(Marshal.SizeOf((UInt64)0));
            sb.Append(", sizeof(bool)=");
            sb.Append(Marshal.SizeOf(true));
            sb.Append(", sizeof(IntPtr)=");
            sb.Append(Marshal.SizeOf((IntPtr)0));
            sb.Append(", sizeof(KopsikTimeEntryViewItem)=");
            sb.Append(Marshal.SizeOf(new KopsikTimeEntryViewItem()));
            sb.Append(", sizeof(KopsikAutocompleteItem)=");
            sb.Append(Marshal.SizeOf(new KopsikAutocompleteItem()));
            sb.Append(", sizeof(KopsikViewItem)=");
            sb.Append(Marshal.SizeOf(new KopsikViewItem()));
            sb.Append(", sizeof(KopsikSettingsViewItem)=");
            sb.Append(Marshal.SizeOf(new KopsikSettingsViewItem()));
            kopsik_debug(ctx, sb.ToString());

            kopsik_check_view_item_size(
                Marshal.SizeOf(new KopsikTimeEntryViewItem()),
                Marshal.SizeOf(new KopsikAutocompleteItem()),
                Marshal.SizeOf(new KopsikViewItem()),
                Marshal.SizeOf(new KopsikSettingsViewItem()),
                Marshal.SizeOf(new KopsikUpdateViewItem()));

            // Wire up events
            kopsik_on_error(ctx, OnError);
            kopsik_on_update(ctx, OnUpdate);                
            kopsik_on_online_state(ctx, OnOnlineState);
            kopsik_on_login(ctx, OnLogin);
            kopsik_on_reminder(ctx, OnReminder);
            kopsik_on_time_entry_list(ctx, OnTimeEntryList);
            kopsik_on_time_entry_autocomplete(ctx, OnTimeEntryAutocomplete);
            kopsik_on_project_autocomplete(ctx, OnProjectAutocomplete);
            kopsik_on_time_entry_editor(ctx, OnTimeEntryEditor);
            kopsik_on_workspace_select(ctx, OnWorkspaceSelect);
            kopsik_on_client_select(ctx, OnClientSelect);
            kopsik_on_tags(ctx, OnTags);
            kopsik_on_settings(ctx, OnSettings);
            kopsik_on_timer_state(ctx, OnTimerState);
            kopsik_on_url(ctx, OnURL);

            // FIXME: Get environment from app settings
            kopsik_set_environment(ctx, "development");

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

        public static List<KopsikApi.KopsikViewItem> ConvertToViewItemList(
            ref KopsikApi.KopsikViewItem first)
        {
            List<KopsikApi.KopsikViewItem> list = new List<KopsikApi.KopsikViewItem>();
            if (Object.ReferenceEquals(null, first))
            {
                return list;
            }
            KopsikApi.KopsikViewItem n = first;
            while (true)
            {
                list.Add(n);
                if (n.Next == IntPtr.Zero)
                {
                    break;
                }
                n = (KopsikApi.KopsikViewItem)Marshal.PtrToStructure(
                    n.Next, typeof(KopsikApi.KopsikViewItem));
            };
            return list;
        }

        public static List<KopsikApi.KopsikAutocompleteItem> ConvertToAutocompleteList(
            ref KopsikApi.KopsikAutocompleteItem first)
        {
            List<KopsikApi.KopsikAutocompleteItem> list =
                new List<KopsikApi.KopsikAutocompleteItem>();
            if (Object.ReferenceEquals(null, first))
            {
                return list;
            }
            KopsikApi.KopsikAutocompleteItem n = first;
            while (true)
            {
                list.Add(n);
                if (n.Next == IntPtr.Zero)
                {
                    break;
                }
                n = (KopsikApi.KopsikAutocompleteItem)Marshal.PtrToStructure(
                    n.Next, typeof(KopsikApi.KopsikAutocompleteItem));
            };
            return list;
        }

        public static List<KopsikApi.KopsikTimeEntryViewItem> ConvertToTimeEntryList(
            ref KopsikApi.KopsikTimeEntryViewItem first)
        {
            List<KopsikApi.KopsikTimeEntryViewItem> list =
                new List<KopsikApi.KopsikTimeEntryViewItem>();
            if (Object.ReferenceEquals(null, first))
            {
                return list;
            }
            KopsikApi.KopsikTimeEntryViewItem n = first;
            while (true)
            {
                list.Add(n);
                if (n.Next == IntPtr.Zero)
                {
                    break;
                }
                n = (KopsikApi.KopsikTimeEntryViewItem)Marshal.PtrToStructure(
                    n.Next, typeof(KopsikApi.KopsikTimeEntryViewItem));
            };
            return list;
        }

        private static readonly DateTime UnixEpoch =
            new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Local);

        public static DateTime DateTimeFromUnix(UInt64 unix_seconds)
        {
            return UnixEpoch.AddSeconds(unix_seconds);
        }

    }
}
