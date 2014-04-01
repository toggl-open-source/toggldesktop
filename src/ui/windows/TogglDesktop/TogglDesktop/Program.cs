using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace TogglDesktop
{
    static class Program
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void KopsikViewItemChangeCallback(
            int result,
            string errmsg,
            IntPtr change);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void KopsikResultCallback(
            int result,
            string errmsg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void KopsikErrorCallback(
            string errmsg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void KopsikCheckUpdateCallback(
            int result,
            string errmsg,
            int is_update_available,
            string url,
            string version);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void KopsikOnOnlineCallback();

        [DllImport("TogglDesktopDLL.dll",
            CharSet = CharSet.Unicode,
            CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr kopsik_context_init(
            string app_name,
            string app_version,
            KopsikViewItemChangeCallback change_callback,
            KopsikErrorCallback on_error_callback,
            KopsikCheckUpdateCallback check_updates_callback,
            KopsikOnOnlineCallback on_online_callback);

        [DllImport("TogglDesktopDLL.dll",
            CharSet = CharSet.Unicode,
            CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr kopsik_context_shutdown(
            IntPtr context);

        [DllImport("TogglDesktopDLL.dll",
            CharSet = CharSet.Unicode,
            CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr kopsik_context_clear(
            IntPtr context);

        private static IntPtr ctx;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            ctx = kopsik_context_init("windows_native_app", "1.0",
                null, null, null, null);
            Application.ApplicationExit += new EventHandler(onApplicationExit);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainWindowController());
        }

        private static void onApplicationExit(object sender, EventArgs e)
        {
            if (ctx != IntPtr.Zero)
            {
                kopsik_context_shutdown(ctx);
                kopsik_context_clear(ctx);
                ctx = IntPtr.Zero;
            }
        }
    }
}
