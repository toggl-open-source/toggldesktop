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

            bugsnag = new Bugsnag.Clients.BaseClient("aa13053a88d5133b688db0f25ec103b7");
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
