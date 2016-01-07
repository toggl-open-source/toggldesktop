using System;
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
    private static Bugsnag.Clients.BaseClient bugsnag;
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

            bugsnag = new Bugsnag.Clients.BaseClient("2a46aa1157256f759053289f2d687c2f");

#if INVS
            bugsnag.Config.ReleaseStage = "development";
#else
            bugsnag.Config.ReleaseStage = "production";
#endif

            Toggl.OnLogin += delegate(bool open, UInt64 user_id)
            {
                uid = user_id;
            };

            Toggl.OnError += delegate(string errmsg, bool user_error)
            {
                Toggl.Debug(errmsg);
                try
                {
                    if (!user_error && bugsnag.Config.ReleaseStage != "development")
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

            RenderOptions.ProcessRenderMode = RenderMode.Default;

            new App().Run();
        }
    }

    static void notifyBugsnag(Exception e)
    {
        Toggl.Debug("Notifying bugsnag: " + e);
        try
        {
            var metadata = new Bugsnag.Metadata();
            metadata.AddToTab("Details", "UserID", uid.ToString());
            metadata.AddToTab("Details", "OSVersion", Environment.OSVersion.ToString());
            metadata.AddToTab("Details", "Version", Version());
            metadata.AddToTab("Details", "Channel", Toggl.UpdateChannel());
            bugsnag.Notify(e, metadata);
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

    public static string Version()
    {
        Assembly assembly = Assembly.GetExecutingAssembly();
        FileVersionInfo versionInfo = FileVersionInfo.GetVersionInfo(assembly.Location);
        return string.Format("{0}.{1}.{2}",
                             versionInfo.ProductMajorPart,
                             versionInfo.ProductMinorPart,
                             versionInfo.ProductBuildPart);
    }
}
}
