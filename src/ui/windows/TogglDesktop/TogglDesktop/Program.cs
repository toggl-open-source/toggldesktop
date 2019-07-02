using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;
using System.Threading;
using System.Windows.Interop;
using System.Windows.Media;
using Application = System.Windows.Forms.Application;

namespace TogglDesktop
{
static class Program
{
    private const string appGUID = "29067F3B-F706-46CB-92D2-1EA1E72A4CE3";
    private static Bugsnag.Client bugsnag;
    private static UInt64 uid;

    public static bool IsLoggedIn
    {
        get
        {
            return uid > 0;
        }
    }

    /// <summary>
    /// The main entry point for the application.
    /// </summary>
    [STAThread]
    static void Main()
    {
        Win32.SetCurrentProcessExplicitAppUserModelID("Toggl.TogglDesktop." + Version);
        Win32.AttachConsole(Win32.ATTACH_PARENT_PROCESS);

        using (Mutex mutex = new Mutex(false, "Global\\" + Environment.UserName + "_" + appGUID))
        {
            if (!mutex.WaitOne(0, false))
            {
                // See if we get hold of the other process.
                // If we do, activate it's window and exit.
                Process current = Process.GetCurrentProcess();
                Process[] instances = Process.GetProcessesByName(current.ProcessName);
                foreach (Process p in instances)
                {
                    if (p.Id != current.Id)
                    {
                        // gotcha
                        IntPtr hWnd = p.MainWindowHandle;
                        if (hWnd == IntPtr.Zero)
                        {
                            hWnd = Win32.SearchForWindow(current.ProcessName, "Toggl Desktop");
                        }

                        Win32.ShowWindow(hWnd, Win32.SW_RESTORE);
                        Win32.SetForegroundWindow(hWnd);

                        return;
                    }
                }

                // If not, print an error message and exit.
                System.Windows.MessageBox.Show("Another copy of Toggl Desktop is already running." +
                                               Environment.NewLine + "This copy will now quit.");
                return;
            }

            Toggl.InitialiseLog();

            var configuration = new Bugsnag.Configuration("aa13053a88d5133b688db0f25ec103b7");
            configuration.AppVersion = Version;

#if TOGGL_PRODUCTION_BUILD
            configuration.ReleaseStage = "production";
#else
            configuration.ReleaseStage = "development";
#endif

            bugsnag = new Bugsnag.Client(configuration);
            bugsnag.BeforeNotify(report =>
            {
                report.Event.User = new Bugsnag.Payload.User { Id = uid.ToString() };
                report.Event.Metadata.Add("Details", new Dictionary<string, string>
                {
                    { "Channel", Toggl.UpdateChannel() }
                });
            });

            Toggl.OnLogin += delegate(bool open, UInt64 user_id)
            {
                uid = user_id;
            };

            Toggl.OnError += delegate(string errmsg, bool user_error)
            {
                Toggl.Debug(errmsg);
                try
                {
                    if (!user_error && bugsnag.Configuration.ReleaseStage != "development")
                    {
                        notifyBugsnag(new Exception(errmsg));
                    }
                }
                catch (Exception ex)
                {
                    Toggl.Debug("Could not check if can notify bugsnag: " + ex);
                }
            };

            Application.ThreadException += Application_ThreadException;
            AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            RenderOptions.ProcessRenderMode = RenderMode.SoftwareOnly;

            new App().Run();
        }
    }

    static void notifyBugsnag(Exception e)
    {
        Toggl.Debug("Notifying bugsnag: " + e);
        try
        {
            bugsnag.Notify(e);
        }
        catch (Exception ex)
        {
            Toggl.Debug("Could not notify bugsnag: " + ex);
        }
    }

    static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
    {
        notifyBugsnag(e.ExceptionObject as Exception);
    }

    static void Application_ThreadException(object sender, System.Threading.ThreadExceptionEventArgs e)
    {
        notifyBugsnag(e.Exception);
    }

    public static void Shutdown(int exitCode)
    {
        Toggl.Clear();

        Environment.Exit(exitCode);
    }

    private static string GetVersionFromAssembly()
    {
        Assembly assembly = Assembly.GetExecutingAssembly();
        FileVersionInfo versionInfo = FileVersionInfo.GetVersionInfo(assembly.Location);
        return string.Format("{0}.{1}.{2}",
                             versionInfo.ProductMajorPart,
                             versionInfo.ProductMinorPart,
                             versionInfo.ProductBuildPart);
    }

    public static string Version { get; } = GetVersionFromAssembly();
}
}
