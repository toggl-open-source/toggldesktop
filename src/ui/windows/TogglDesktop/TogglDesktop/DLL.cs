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
        public static extern void kopsik_set_log_path(string path);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_set_log_level(string level);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_set_db_path(IntPtr context, string path);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern bool kopsik_login(
            IntPtr context, string email, string password);

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_password_forgot();

        [DllImport(dll, CharSet = charset, CallingConvention = convention)]
        public static extern void kopsik_context_clear(IntPtr context);

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
    }
}

