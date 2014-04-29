using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace TogglDesktop
{
    static class Kopsik
    {
        private static IntPtr ctx_ = IntPtr.Zero;

        public static void Init(string app_name, string app_version)
        {
            ctx_ = DLL.kopsik_context_init(app_name, app_version);

            DLL.kopsik_context_set_error_callback(ctx_, OnError);
            DLL.kopsik_context_set_check_update_callback(ctx_, OnCheckUpdate);
            DLL.kopsik_context_set_online_callback(ctx_, OnOnline);
            DLL.kopsik_context_set_user_login_callback(ctx_, OnUserLogin);
            DLL.kopsik_set_open_url_callback(ctx_, OnOpenURL);
            DLL.kopsik_set_remind_callback(ctx_, OnRemind);

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

        public static bool Logout()
        {
            return DLL.kopsik_logout(ctx_);
        }

        public static void PasswordForgot()
        {
            DLL.kopsik_password_forgot(ctx_);
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

        public static void Start(string description, string duration, int task_id, int project_id)
        {
            DLL.kopsik_start(ctx_, description, duration, task_id, project_id);
        }

        public static void Stop()
        {
            DLL.kopsik_stop(ctx_);
        }

        public static event DLL.KopsikErrorCallback OnError = delegate { };
        public static event DLL.KopsikUserLoginCallback OnUserLogin = delegate { };
        public static event DLL.KopsikCheckUpdateCallback OnCheckUpdate = delegate {};
        public static event DLL.KopsikOnOnlineCallback OnOnline = delegate { };
        public static event DLL.KopsikOpenURLCallback OnOpenURL = delegate { };
        public static event DLL.KopsikRemindCallback OnRemind = delegate { };
    }
}

