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
        private static IntPtr ctx_ = IntPtr.Zero;

        private const string dll = "TogglDesktopDLL.dll";

        [DllImport(dll,
            CharSet = CharSet.Unicode,
            CallingConvention = CallingConvention.Cdecl)]
        private static extern System.IntPtr kopsik_context_init(
            string app_name, string app_version);

        public static void Init(string app_name, string app_version)
        {
            ctx_ = kopsik_context_init(app_name, app_version);
        }

        [DllImport(dll,
            CharSet = CharSet.Unicode,
            CallingConvention = CallingConvention.Cdecl)]
        private static extern void kopsik_context_start_events(IntPtr context);

        public static void StartEvents()
        {
            kopsik_context_start_events(ctx_);
        }

        [DllImport(dll,
            CharSet = CharSet.Unicode,
            CallingConvention = CallingConvention.Cdecl)]
        private static extern bool kopsik_login(
            IntPtr context, string email, string password);

        public static bool Login(string email, string password)
        {
            return kopsik_login(ctx_, email, password);
        }

        [DllImport(dll,
            CharSet = CharSet.Unicode,
            CallingConvention = CallingConvention.Cdecl)]
        private static extern void kopsik_context_clear(IntPtr context);

        public static void Clear()
        {
            if (IntPtr.Zero == ctx_)
            {
                return;
            }
            kopsik_context_clear(ctx_);
            ctx_ = IntPtr.Zero;
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void KopsikErrorCallback(string errmsg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void KopsikUserLoginCallback(
            UInt64 id, string fullname, string timeofdayformat);
    }
}

