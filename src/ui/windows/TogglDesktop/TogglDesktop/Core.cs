using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace TogglDesktop
{
    static class Core
    {
        public static void Startup(string app_name, string app_version)
        {
            ctx_ = kopsik_context_init(app_name, app_version);
            kopsik_context_init(app_name, app_version);
        }

        public static void Shutdown()
        {
            if (IntPtr.Zero == ctx_)
            {
                return;
            }
            kopsik_context_shutdown(ctx_);
            kopsik_context_clear(ctx_);
            ctx_ = IntPtr.Zero;
        }

        private static IntPtr ctx_ = IntPtr.Zero;

        private const string dll = "TogglDesktopDLL.dll";

        [DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr kopsik_context_init(string app_name, string app_version);

        [DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern void kopsik_context_startup();

        [DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern void kopsik_context_shutdown(IntPtr context);

        [DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern void kopsik_context_clear(IntPtr context);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void KopsikErrorCallback(string errmsg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void KopsikUserLoginCallback();
    }
}

