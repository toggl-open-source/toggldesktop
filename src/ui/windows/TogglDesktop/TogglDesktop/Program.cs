using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Diagnostics;
using System.Threading;
using System.Text;

namespace TogglDesktop
{
    static class Program
    {
        public static bool ShuttingDown = false;
        private const string appGUID = "29067F3B-F706-46CB-92D2-1EA1E72A4CE3";
        public static Bugsnag.Library.BugSnag bugsnag = null;
        private static UInt64 uid = 0;
        private static MainWindowController mainWindowController;

        [DllImport("user32.dll", SetLastError = true)]
        private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, ref SearchData data);

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern int GetWindowText(IntPtr hWnd, StringBuilder lpString, int nMaxCount);

        [DllImport("User32")]
        private static extern int SetForegroundWindow(IntPtr hwnd);

        [DllImport("User32")]
        private static extern bool ShowWindow(IntPtr hwnd, int cmdshow);
        private const int SW_RESTORE = 9;

        public static bool IsLoggedIn
        {
            get
            {
                return uid > 0;
            }
        }

        [DllImport("kernel32.dll")]
        static extern bool AttachConsole(int dwProcessId);
        private const int ATTACH_PARENT_PROCESS = -1;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            AttachConsole(ATTACH_PARENT_PROCESS);

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
                                hWnd = SearchForWindow(current.ProcessName, "Toggl Desktop");
                            }
                            ShowWindow(hWnd, SW_RESTORE);
                            SetForegroundWindow(hWnd);
                            return;
                        }
                    }

                    // If not, print an error message and exit.
                    MessageBox.Show("Another copy of Toggl Desktop is already running." +
                        Environment.NewLine + "This copy will now quit.");
                    return;
                }

                bugsnag = new Bugsnag.Library.BugSnag()
                {
                    apiKey = "2a46aa1157256f759053289f2d687c2f",
                    OSVersion = Environment.OSVersion.ToString(),
                    applicationVersion = Version()
                };

                Toggl.OnLogin += delegate(bool open, UInt64 user_id)
                {
                    uid = user_id;
                };

                Toggl.OnError += delegate(string errmsg, bool user_error)
                {
                    Console.WriteLine(errmsg);
                    try
                    {
                        if (!user_error && Properties.Settings.Default.Environment != "development")
                        {
                            notifyBugsnag(new Exception(errmsg));
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Could not check if can notify bugsnag: ", ex);
                    }
                };

                Application.ThreadException += new System.Threading.ThreadExceptionEventHandler(Application_ThreadException);
                AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(CurrentDomain_UnhandledException);

                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);

                mainWindowController = new MainWindowController();
                Application.Run(mainWindowController);
            }
        }

        public static IntPtr SearchForWindow(string wndclass, string title)
        {
            SearchData sd = new SearchData { Wndclass = wndclass, Title = title };
            EnumWindows(new EnumWindowsProc(EnumProc), ref sd);
            return sd.hWnd;
        }

        public class SearchData
        {
            public string Wndclass;
            public string Title;
            public IntPtr hWnd;
        } 

        private delegate bool EnumWindowsProc(IntPtr hWnd, ref SearchData data);

        public static bool EnumProc(IntPtr hWnd, ref SearchData data)
        {
            StringBuilder sb = new StringBuilder(1024);
            GetWindowText(hWnd, sb, sb.Capacity);
            if (sb.ToString().Contains(data.Title))
            {
                data.hWnd = hWnd;
                return false;    // Found the window
            }
            return true;
        }

        static void notifyBugsnag(Exception e)
        {
            try
            {
                bugsnag.Notify(e, new
                {
                    UserID = uid.ToString(),
                    channel = Toggl.UpdateChannel()
                });
            }
            catch (Exception ex)
            {
                Console.WriteLine("Could not notify bugsnag: ", ex);
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
            ShuttingDown = true;

            if (mainWindowController != null)
            {
                mainWindowController.RemoveTrayIcon();
            }

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
