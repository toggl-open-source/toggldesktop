using System;
using System.Drawing;
using System.Windows.Forms;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace TogglDesktop
{
    public partial class MainWindowController : Form
    {
        private LoginViewController loginViewController;
        private TimeEntryListViewController timeEntryListViewController;
        private TimeEntryEditViewController timeEntryEditViewController;
        private AboutWindowController aboutWindowController;
        private PreferencesWindowController preferencesWindowController;
        private FeedbackWindowController feedbackWindowController;
        private IdleNotificationWindowController idleNotificationWindowController;

        private bool isUpgradeDialogVisible = false;
        private bool isTracking = false;
        private bool isNetworkError = false;
        private Point defaultContentPosition =  new System.Drawing.Point(0, 0);
        private Point errorContentPosition = new System.Drawing.Point(0, 28);
        private bool remainOnTop = false;

        [StructLayout(LayoutKind.Sequential)]
        struct LASTINPUTINFO
        {
            public static readonly int SizeOf =
                Marshal.SizeOf(typeof(LASTINPUTINFO));

            [MarshalAs(UnmanagedType.U4)]
            public int cbSize;

            [MarshalAs(UnmanagedType.U4)]
            public int dwTime;
        }

        [DllImport("user32.dll")]
        static extern bool GetLastInputInfo(out LASTINPUTINFO plii);

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetWindowPos(IntPtr hWnd,
            int hWndInsertAfter, int x, int u, int cx, int cy, int uFlags);

        private const int HWND_TOPMOST = -1;
        private const int HWND_NOTOPMOST = -2;
        private const int SWP_NOMOVE = 0x0002;
        private const int SWP_NOSIZE = 0x0001;

        public MainWindowController()
        {
            InitializeComponent();

            loginViewController = new LoginViewController();
            timeEntryListViewController = new TimeEntryListViewController();
            timeEntryEditViewController = new TimeEntryEditViewController();
            aboutWindowController = new AboutWindowController();
            preferencesWindowController = new PreferencesWindowController();
            feedbackWindowController = new FeedbackWindowController();
            idleNotificationWindowController = new IdleNotificationWindowController();
        }

        private void MainWindowController_Load(object sender, EventArgs e)
        {
            troubleBox.BackColor = Color.FromArgb(239, 226, 121);
            contentPanel.Location = defaultContentPosition;

            Utils.LoadWindowLocation(this);

            Toggl.OnApp += OnApp;
            Toggl.OnError += OnError;
            Toggl.OnUpdate += OnUpdate;
            Toggl.OnLogin += OnLogin;
            Toggl.OnTimeEntryList += OnTimeEntryList;
            Toggl.OnTimeEntryEditor += OnTimeEntryEditor;
            Toggl.OnOnlineState += OnOnlineState;
            Toggl.OnReminder += OnReminder;
            Toggl.OnURL += OnURL;
            Toggl.OnRunningTimerState += OnRunningTimerState;
            Toggl.OnStoppedTimerState += OnStoppedTimerState;
            Toggl.OnSettings += OnSettings;
            Toggl.OnIdleNotification += OnIdleNotification;

            if (!Toggl.Start(TogglDesktop.Program.Version()))
            {
                MessageBox.Show("Missing callback. See the log file for details");
                TogglDesktop.Program.Shutdown(1);
            }
        }

        void OnRunningTimerState(Toggl.TimeEntry te)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnRunningTimerState(te); });
                return;
            }
            isTracking = true;
            enableMenuItems();
            displayTrayIcon(true);
        }

        void OnStoppedTimerState()
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnStoppedTimerState(); });
                return;
            }
            isTracking = false;
            enableMenuItems();
            displayTrayIcon(true);
        }

        void OnSettings(bool open, Toggl.Settings settings)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnSettings(open, settings); });
                return;
            }
            remainOnTop = settings.OnTop;
            setWindowPos();
            timerIdleDetection.Enabled = settings.UseIdleDetection;
        }

        private void displayTrayIcon(bool is_online)
        {
            if (is_online)
            {
                if (TogglDesktop.Program.IsLoggedIn && isTracking)
                {
                    trayIcon.Icon = Properties.Resources.toggltray;
                }
                else
                {
                    trayIcon.Icon = Properties.Resources.toggltray_inactive;
                }
            }
            else
            {
                if (TogglDesktop.Program.IsLoggedIn && isTracking)
                {
                    trayIcon.Icon = Properties.Resources.toggl_offline_active;
                }
                else
                {
                    trayIcon.Icon = Properties.Resources.toggl_offline_inactive;
                }
            }
          
        }

        void OnOnlineState(bool is_online, string reason)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnOnlineState(is_online, reason); });
                return;
            }
            if (!is_online)
            {
                isNetworkError = true;

                errorLabel.Text = reason;
                troubleBox.Visible = true;
                contentPanel.Location = errorContentPosition;
                displayTrayIcon(false);
            }
            else if (isNetworkError)
            {
                isNetworkError = false;

                troubleBox.Visible = false;
                contentPanel.Location = defaultContentPosition;
                displayTrayIcon(true);
            }
        }

        void OnUpdate(bool open, Toggl.Update view)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnUpdate(open, view); });
                return;
            }
            if (open)
            {
                aboutWindowController.Show();
                aboutWindowController.TopMost = true;
            }
            if (!view.IsUpdateAvailable)
            {
                return;
            }
            if (isUpgradeDialogVisible || aboutWindowController.Visible)
            {
                return;
            }
            isUpgradeDialogVisible = true;
            DialogResult dr = MessageBox.Show(
                "There's a new version of this app available (" + view.Version + ")." +
                Environment.NewLine + "Proceed with the download?",
                "New version available",
                MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            isUpgradeDialogVisible = false;
            if (DialogResult.Yes == dr)
            {
                Process.Start(view.URL);
            }
        }

        void OnURL(string url)
        {
            Process.Start(url);
        }

        void OnApp(bool open)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnApp(open); });
                return;
            }
            if (open) {
                show();
            }
        }

        void OnError(string errmsg, bool user_error)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnError(errmsg, user_error); });
                return;
            }

            isNetworkError = false;

            errorLabel.Text = errmsg;
            troubleBox.Visible = true;
            contentPanel.Location = errorContentPosition;

            if (!user_error)
            {
                Bugsnag.Library.BugSnag bs = new Bugsnag.Library.BugSnag();
                bs.Notify(new Exception(errmsg));
            }
        }

        void OnIdleNotification(
            string guid, string since, string duration, UInt64 started)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnIdleNotification(guid, since, duration, started); });
                return;
            }
            idleNotificationWindowController.Show();
            idleNotificationWindowController.BringToFront();
        }

        void OnLogin(bool open, UInt64 user_id)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnLogin(open, user_id); });
                return;
            }
            if (open) {
                contentPanel.Controls.Remove(timeEntryListViewController);
                contentPanel.Controls.Remove(timeEntryEditViewController);
                contentPanel.Controls.Add(loginViewController);
                loginViewController.SetAcceptButton(this);
            }
            enableMenuItems();
            displayTrayIcon(true);
        }

        private void enableMenuItems()
        {
            bool isLoggedIn = TogglDesktop.Program.IsLoggedIn;

            newToolStripMenuItem.Enabled = isLoggedIn;
            continueToolStripMenuItem.Enabled = isLoggedIn && !isTracking;
            stopToolStripMenuItem.Enabled = isLoggedIn && isTracking;
            syncToolStripMenuItem.Enabled = isLoggedIn;
            logoutToolStripMenuItem.Enabled = isLoggedIn;
            clearCacheToolStripMenuItem.Enabled = isLoggedIn;
            sendFeedbackToolStripMenuItem.Enabled = isLoggedIn;
            openInBrowserToolStripMenuItem.Enabled = isLoggedIn;
        }

        void OnTimeEntryList(bool open, List<Toggl.TimeEntry> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnTimeEntryList(open, list); });
                return;
            }
            if (open)
            {
                troubleBox.Visible = false;
                contentPanel.Location = defaultContentPosition;
                contentPanel.Controls.Remove(loginViewController);
                contentPanel.Controls.Remove(timeEntryEditViewController);
                contentPanel.Controls.Add(timeEntryListViewController);
                timeEntryListViewController.SetAcceptButton(this);
            }
        }

        void OnTimeEntryEditor(
            bool open,
            Toggl.TimeEntry te,
            string focused_field_name)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnTimeEntryEditor(open, te, focused_field_name); });
                return;
            }
            if (open)
            {
                contentPanel.Controls.Remove(loginViewController);
                contentPanel.Controls.Remove(timeEntryListViewController);
                contentPanel.Controls.Add(timeEntryEditViewController);
                timeEntryEditViewController.setupView(this, focused_field_name);
            }
        }

        private void MainWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            Utils.SaveWindowLocation(this);

            if (!TogglDesktop.Program.ShuttingDown) {
                this.Hide();
                e.Cancel = true;
            }
        }

        private void buttonDismissError_Click(object sender, EventArgs e)
        {
            troubleBox.Visible = false;
            contentPanel.Location = defaultContentPosition;
        }

        private void sendFeedbackToolStripMenuItem_Click(object sender, EventArgs e)
        {
            feedbackWindowController.Show();
            feedbackWindowController.BringToFront();
        }

        private void quitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (Visible)
            {
                Utils.SaveWindowLocation(this);
            }
            TogglDesktop.Program.Shutdown(0);
        }

        private void toggleVisibility()
        {
            if (WindowState == FormWindowState.Minimized)
            {
                WindowState = FormWindowState.Normal;
                return;
            }
            if (Visible)
            {
                Hide();
                return;
            }
            show();
        }

        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Toggl.Start("", "", 0, 0);
        }

        private void continueToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Toggl.ContinueLatest();
        }

        private void stopToolStripMenuItem_Click(object sender, EventArgs e)
        {
           Toggl.Stop();
        }

        private void showToolStripMenuItem_Click(object sender, EventArgs e)
        {
            show();
        }

        private void syncToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Toggl.Sync();
        }

        private void openInBrowserToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Toggl.OpenInBrowser();
        }

        private void preferencesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Toggl.EditPreferences();
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Toggl.About();
        }

        private void logoutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Toggl.Logout();
        }

        private void show()
        {
            Show();
            TopMost = true;
            setWindowPos();
        }

        private void setWindowPos()
        {
            if (remainOnTop)
            {
                SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }
            else
            {
                SetWindowPos(Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }
        }

        void OnReminder(string title, string informative_text)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnReminder(title, informative_text); });
                return;
            }
            trayIcon.ShowBalloonTip(6000 * 100, title, informative_text, ToolTipIcon.None);
        }

        private void clearCacheToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult dr = MessageBox.Show(
                "This will remove your Toggl user data from this PC and log you out of the Toggl Desktop app. " +
                "Any unsynced data will be lost." +
                Environment.NewLine + "Do you want to continue?",
                "Clear Cache",
                MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            if (DialogResult.Yes == dr) 
            {
                Toggl.ClearCache();
            }
        }

        private void trayIcon_BalloonTipClicked(object sender, EventArgs e)
        {
            show();
        }

        private void timerIdleDetection_Tick(object sender, EventArgs e)
        {
            int idleTime = 0;
            LASTINPUTINFO lastInputInfo = new LASTINPUTINFO();
            lastInputInfo.cbSize = Marshal.SizeOf(lastInputInfo);
            lastInputInfo.dwTime = 0;

            int envTicks = Environment.TickCount;

            if (GetLastInputInfo(out lastInputInfo))
            {
                int lastInputTick = lastInputInfo.dwTime;
                idleTime = envTicks - lastInputTick;
            }

            int idle_seconds = 0;
            if (idleTime > 0)
            {
                idle_seconds = idleTime / 1000;
            }
            else
            {
                idle_seconds = idleTime;
            }

            if (idle_seconds < 0)
            {
                idle_seconds = 0;
            }

            Toggl.SetIdleSeconds((ulong)idle_seconds);
        }

        private void trayIcon_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                toggleVisibility();
            }
        }
    }
}
