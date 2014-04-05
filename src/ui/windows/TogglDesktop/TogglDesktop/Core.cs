using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace TogglDesktop
{
    static class Core
    {
        private static IntPtr ctx_ = IntPtr.Zero;

        public static void Init(string app_name, string app_version)
        {
            ctx_ = DLL.kopsik_context_init(app_name, app_version);

            DLL.kopsik_context_set_error_callback(ctx_, OnError);
            DLL.kopsik_context_set_check_update_callback(ctx_, OnCheckUpdate);
            DLL.kopsik_context_set_online_callback(ctx_, OnOnline);
            DLL.kopsik_context_set_user_login_callback(ctx_, OnUserLogin);

            string path = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                "Kopsik");
            System.IO.Directory.CreateDirectory(path);
            DLL.kopsik_set_log_path(Path.Combine(path, "kopsik.log"));
            DLL.kopsik_set_log_level("debug");
            DLL.kopsik_set_db_path(ctx_, Path.Combine(path, "kopsik.db"));

            DLL.kopsik_context_start_events(ctx_);
        }

        public static bool Login(string email, string password)
        {
            return DLL.kopsik_login(ctx_, email, password);
        }

        public static void Clear()
        {
            if (IntPtr.Zero == ctx_)
            {
                return;
            }
            DLL.kopsik_context_clear(ctx_);
            ctx_ = IntPtr.Zero;
        }

        public static event DLL.KopsikErrorCallback OnError = delegate { };
        public static event DLL.KopsikUserLoginCallback OnUserLogin = delegate { };
        public static event DLL.KopsikCheckUpdateCallback OnCheckUpdate = delegate {};
        public static event DLL.KopsikOnOnlineCallback OnOnline = delegate { };
    }
}

