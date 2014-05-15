using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace TogglDesktop
{
    static class DLL
    {
        private const string dll = "TogglDesktopDLL.dll";
        private const CharSet charset = CharSet.Ansi;
        private const CallingConvention convention = CallingConvention.Cdecl;

        // Models

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct KopsikTimeEntryViewItem
        {
            public int DurationInSeconds;
            public string Description;
            public string ProjectAndTaskLabel;
            public int WID;
            public int PID;
            public int TID;
            public string Duration;
            public string Color;
            public string GUID;
            public bool Billabel;
            public string Tags;
            public int Started;
            public int Ended;
            public string StartTimeString;
            public string EndTimeString;
            public int UpdatedAt;
            public string DateHeader;
            public string DateDuration;
            public bool DurOnly;
            public bool IsHeader;
            public IntPtr Next;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct KopsikAutocompleteItem
        {
            public string Text;
            public string Description;
            public string ProjectAndTaskLabel;
            public int TaskID;
            public int ProjectID;
            public IntPtr Next;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct KopsikViewItem
        {
            public int ID;
            public string GUID;
            public string Name;
            public IntPtr Next;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = charset)]
        public struct KopsikSettingsViewItem
        {
            public bool UseProxy;
            public string ProxyHost;
            public int ProxyPort;
            public string ProxyUsername;
            public string ProxyPassword;
            public bool UseIdleDetection;
            public bool MenubarTimer;
            public bool DockIcon;
            public bool OnTop;
            public bool Reminder;
            public bool RecordTimeline;
        }

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern System.IntPtr kopsik_context_init(
            string app_name, string app_version);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_context_start_events(IntPtr context);

        [DllImport(dll, CharSet = CharSet.Auto, CallingConvention = convention)]
        public static extern void kopsik_context_set_error_callback(
            IntPtr context, KopsikErrorCallback cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_context_set_check_update_callback(
            IntPtr context, KopsikCheckUpdateCallback cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_context_set_online_callback(
            IntPtr context, KopsikOnOnlineCallback cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_context_set_user_login_callback(
            IntPtr context, KopsikUserLoginCallback cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_set_open_url_callback(
            IntPtr context, KopsikOpenURLCallback cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_set_remind_callback(
            IntPtr context, KopsikRemindCallback cb);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_set_log_path(string path);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_set_log_level(string level);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_set_db_path(IntPtr context, string path);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern bool kopsik_login(
            IntPtr context, string email, string password);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern bool kopsik_logout(
            IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_password_forgot(IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_context_clear(IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern bool kopsik_start(IntPtr context,
            string description, string duration, int task_id, int project_id);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern bool kopsik_continue(IntPtr context, string guid);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern bool kopsik_continue_latest(IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern bool kopsik_stop(IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_sync(IntPtr context);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_open_in_browser(IntPtr context);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikErrorCallback(string errmsg);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikUserLoginCallback(
            UInt64 id, string fullname, string timeofdayformat);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikCheckUpdateCallback(
            bool is_update_available, string url, string version);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikOnOnlineCallback();

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikOpenURLCallback(string url);

        [UnmanagedFunctionPointer(convention)]
        public delegate void KopsikRemindCallback();
    }
}

