using System;

namespace TogglDesktop
{
    using System;
    using System.Runtime.InteropServices;

    class WinSparkle
    {
        public delegate void Callback();
        public delegate int intCallback();

        // Note that some of these functions are not implemented by WinSparkle YET.
        [DllImport("WinSparkle.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void win_sparkle_init();
        [DllImport("WinSparkle.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void win_sparkle_cleanup();
        [DllImport("WinSparkle.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void win_sparkle_set_appcast_url(String url);
        [DllImport("WinSparkle.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void win_sparkle_set_app_details(String company_name,
                                             String app_name,
                                             String app_version);
        [DllImport("WinSparkle.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void win_sparkle_set_registry_path(String path);
        [DllImport("WinSparkle.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void win_sparkle_check_update_with_ui();
        [DllImport("WinSparkle.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void win_sparkle_check_update_without_ui();
        [DllImport("WinSparkle.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern void win_sparkle_set_can_shutdown_callback(intCallback callback);
        [DllImport("WinSparkle.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void win_sparkle_set_shutdown_request_callback(Callback callback);

        private static intCallback shouldShutdownCallBack;
        private static Callback shutdownCallBack;

        public static void setupWinSparkle()
        {
            shouldShutdownCallBack = new intCallback(shouldShutdown);
            shutdownCallBack = new Callback(shutDown);
            WinSparkle.win_sparkle_set_can_shutdown_callback(shouldShutdownCallBack);
            WinSparkle.win_sparkle_set_shutdown_request_callback(shutDown);
        }

        private static int shouldShutdown()
        {
            return 1;
        }

        private static void shutDown()
        {
            TogglDesktop.Program.Shutdown(0);
        }
    }
}