using System;
using System.Windows.Forms;

namespace TogglDesktopUpdater
{
    static class Program
    {
        private static Bugsnag.Clients.BaseClient bugsnag = null;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            bugsnag = new Bugsnag.Clients.BaseClient("2a46aa1157256f759053289f2d687c2f");
            bugsnag.Config.ReleaseStage = "production";

            Application.Run(new MainForm());
        }

        public static void NotifyBugsnag(Exception e, Bugsnag.Metadata metadata)
        {
            try
            {
                if (null == metadata)
                {
                    metadata = new Bugsnag.Metadata();
                }
                metadata.AddToTab("Details", "OSVersion", Environment.OSVersion.ToString());
                metadata.AddToTab("Details", "applicationVersion", Application.ProductVersion);
                bugsnag.Notify(e, metadata);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Could not notify bugsnag: ", ex);
            }
        }

        static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            NotifyBugsnag(e.ExceptionObject as Exception, null);
        }

        static void Application_ThreadException(object sender, System.Threading.ThreadExceptionEventArgs e)
        {
            NotifyBugsnag(e.Exception, null);
        }

    }
}
