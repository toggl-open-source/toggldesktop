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
            ctx_ = KopsikApi.kopsik_context_init(app_name, app_version);

            KopsikApi.kopsik_context_set_error_callback(ctx_, OnError);
            KopsikApi.kopsik_context_set_check_update_callback(ctx_, OnCheckUpdate);
            KopsikApi.kopsik_context_set_online_callback(ctx_, OnOnline);
            KopsikApi.kopsik_context_set_user_login_callback(ctx_, OnUserLogin);
            KopsikApi.kopsik_set_open_url_callback(ctx_, OnOpenURL);
            KopsikApi.kopsik_set_remind_callback(ctx_, OnRemind);

            string path = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                "Kopsik");
            System.IO.Directory.CreateDirectory(path);
            KopsikApi.kopsik_set_log_path(Path.Combine(path, "kopsik.log"));
            KopsikApi.kopsik_set_log_level("debug");
            KopsikApi.kopsik_set_db_path(ctx_, Path.Combine(path, "kopsik.db"));

            KopsikApi.kopsik_context_start_events(ctx_);
        }

        public static bool Login(string email, string password)
        {
            return KopsikApi.kopsik_login(ctx_, email, password);
        }

        public static bool Logout()
        {
            return KopsikApi.kopsik_logout(ctx_);
        }

        public static void PasswordForgot()
        {
            KopsikApi.kopsik_password_forgot(ctx_);
        }

        public static void Clear()
        {
            if (IntPtr.Zero == ctx_)
            {
                return;
            }
            KopsikApi.kopsik_context_clear(ctx_);
            ctx_ = IntPtr.Zero;
        }

        public static bool Start(string description, string duration, int task_id, int project_id)
        {
           return KopsikApi.kopsik_start(ctx_, description, duration, task_id, project_id);
        }

        public static bool Continue(string guid)
        {
            return KopsikApi.kopsik_continue(ctx_, guid);
        }

        public static bool ContinueLatest()
        {
            return KopsikApi.kopsik_continue_latest(ctx_);
        }

        public static bool Stop()
        {
            return KopsikApi.kopsik_stop(ctx_);
        }

        public static void Sync()
        {
            KopsikApi.kopsik_sync(ctx_);
        }

        public static void OpenInBrowser()
        {
            KopsikApi.kopsik_open_in_browser(ctx_);
        }

        public static event KopsikApi.KopsikErrorCallback OnError = delegate { };
        public static event KopsikApi.KopsikUserLoginCallback OnUserLogin = delegate { };
        public static event KopsikApi.KopsikCheckUpdateCallback OnCheckUpdate = delegate {};
        public static event KopsikApi.KopsikOnOnlineCallback OnOnline = delegate { };
        public static event KopsikApi.KopsikOpenURLCallback OnOpenURL = delegate { };
        public static event KopsikApi.KopsikRemindCallback OnRemind = delegate { };
    }
}

