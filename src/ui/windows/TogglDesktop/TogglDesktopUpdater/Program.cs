using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace TogglDesktopUpdater
{
    static class Program
    {
        private static Bugsnag.Client bugsnag;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.ThreadException += Application_ThreadException;
            AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            var configuration = new Bugsnag.Configuration("aa13053a88d5133b688db0f25ec103b7");
            configuration.AppVersion = Application.ProductVersion;
            configuration.ReleaseStage = "production";
            bugsnag = new Bugsnag.Client(configuration);

            Application.Run(new MainForm());
        }

        public static void NotifyBugsnag(Exception e)
        {
            try
            {
                bugsnag.Notify(e);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Could not notify bugsnag: ", ex);
            }
        }

        public static void UpdateBugsnagMetadata(string tabName, Dictionary<string, string> keyValuePairs)
        {
            bugsnag.BeforeNotify(report =>
            {
                report.Event.Metadata.Add(tabName, keyValuePairs);
            });
        }

        static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            bugsnag.Notify(e.ExceptionObject as Exception);
        }

        static void Application_ThreadException(object sender, System.Threading.ThreadExceptionEventArgs e)
        {
            bugsnag.Notify(e.Exception);
        }

    }
}
