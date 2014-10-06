// this is WinSparkle.cs, wrapper code for calling the native WinSparkle DLL
using System;

namespace TogglDesktop // YOUR NAMESPACE CAN GO HERE
{
    using System;
    using System.Runtime.InteropServices;

    class WinSparkle
    {
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
    }
}