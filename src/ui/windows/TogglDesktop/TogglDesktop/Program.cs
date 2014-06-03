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
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Bugsnag.Library.BugSnag bs = new Bugsnag.Library.BugSnag()
            {
                apiKey = "2a46aa1157256f759053289f2d687c2f"
            };

            Application.ThreadException += new System.Threading.ThreadExceptionEventHandler(Application_ThreadException);
            AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(CurrentDomain_UnhandledException);
            Application.ApplicationExit += new EventHandler(onApplicationExit);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainWindowController());
        }

        static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            Bugsnag.Library.BugSnag bs = new Bugsnag.Library.BugSnag();
            bs.Notify(e.ExceptionObject as Exception);
        }

        static void Application_ThreadException(object sender, System.Threading.ThreadExceptionEventArgs e)
        {
            Bugsnag.Library.BugSnag bs = new Bugsnag.Library.BugSnag();
            bs.Notify(e.Exception);
        }

        private static void onApplicationExit(object sender, EventArgs e)
        {
            KopsikApi.kopsik_context_clear(KopsikApi.ctx);
        }
    }
}
