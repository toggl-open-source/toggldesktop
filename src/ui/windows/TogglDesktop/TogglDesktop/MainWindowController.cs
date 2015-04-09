using System;
using System.Drawing;
using System.Windows.Forms;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Drawing.Drawing2D;
using System.IO;

namespace TogglDesktop
{
    public partial class MainWindowController : TogglForm
    {
        [DllImport("user32.dll")]
        private static extern int ReleaseCapture();

        [DllImport("user32.dll")]
        private static extern int SendMessage(IntPtr hwnd, int msg, int wparam, int lparam);

        private const int wmNcLButtonDown = 0xA1;
        private const int wmNcLButtonUp = 0xA2;
        private const int HtBottomRight = 17;
        private bool isResizing = false;

        private List<Icon> statusIcons = new List<Icon>();

        private LoginViewController loginViewController;
        private TimeEntryListViewController timeEntryListViewController;
        private TimeEntryEditViewController timeEntryEditViewController;
        private AboutWindowController aboutWindowController;
        private PreferencesWindowController preferencesWindowController;
        private FeedbackWindowController feedbackWindowController;
        private IdleNotificationWindowController idleNotificationWindowController;
        private EditForm editForm;
        private Control editableEntry;

        private bool isTracking = false;
        private Point defaultContentPosition =  new System.Drawing.Point(0, 0);
        private Point errorContentPosition = new System.Drawing.Point(0, 28);
        private bool remainOnTop = false;
        private bool topDisabled = false;

        private static MainWindowController instance;

        KeyboardHook startHook = new KeyboardHook();
        KeyboardHook showHook = new KeyboardHook();

        private Timer runScriptTimer;

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

        [DllImport("user32", CallingConvention = CallingConvention.Winapi)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool ShowScrollBar(IntPtr hwnd, int wBar, [MarshalAs(UnmanagedType.Bool)] bool bShow);

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetWindowPos(IntPtr hWnd,
            int hWndInsertAfter, int x, int u, int cx, int cy, int uFlags);

        private const int HWND_TOPMOST = -1;
        private const int HWND_NOTOPMOST = -2;
        private const int SWP_NOMOVE = 0x0002;
        private const int SWP_NOSIZE = 0x0001;
        private const int SB_HORZ = 0;

        public MainWindowController()
        {
            InitializeComponent();

            instance = this;

            startHook.KeyPressed +=
                new EventHandler<KeyPressedEventArgs>(hookStartKeyPressed);

            showHook.KeyPressed +=
                new EventHandler<KeyPressedEventArgs>(hookShowKeyPressed);
        }

        void setGlobalShortCutKeys()
        {
            try
            {
                startHook.Clear();
                string startKey = Properties.Settings.Default.StartKey;
                if (startKey != null && startKey != "")
                {
                    startHook.RegisterHotKey(
                        Properties.Settings.Default.StartModifiers,
                        (Keys)Enum.Parse(typeof(Keys), startKey));
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Could not register start shortcut: ", e);
            }

            try
            {
                showHook.Clear();
                string showKey = Properties.Settings.Default.ShowKey;
                if (showKey != null && showKey != "")
                {
                    showHook.RegisterHotKey(
                        Properties.Settings.Default.ShowModifiers,
                        (Keys)Enum.Parse(typeof(Keys), showKey));
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Could not register show hotkey: ", e);
            }
        }

        void hookStartKeyPressed(object sender, KeyPressedEventArgs e)
        {
            if (isTracking)
            {
                Toggl.Stop();
            }
            else
            {
                Toggl.ContinueLatest();
            }
        }

        void hookShowKeyPressed(object sender, KeyPressedEventArgs e)
        {
            if (Visible)
            {
                Hide();
                if (editForm.Visible)
                {
                    editForm.CloseButton_Click(null, null);
                }
                feedbackWindowController.Close();
                aboutWindowController.Close();
                preferencesWindowController.Close();
            }
            else
            {
                show();
            }
        }

        public void toggleMenu()
        {
            Point pt = new Point(Width - 80, 0);
            pt = PointToScreen(pt);
            trayIconMenu.Show(pt);
        }

        protected override void OnShown(EventArgs e)
        {
            hideHorizontalScrollBar();
            base.OnShown(e);
        }

        public static void DisableTop()
        {
            instance.topDisabled = true;
            instance.setWindowPos();
        }

        public static void EnableTop()
        {
            instance.topDisabled = false;
            instance.setWindowPos();
        }

        public void RemoveTrayIcon()
        {
            trayIcon.Visible = false;
        }

        private const int kTogglTray = 0;
        private const int kTogglTrayInactive = 1;
        private const int kToggl = 2;
        private const int kTogglInactive = 3;
        private const int kTogglOfflineActive = 4;
        private const int kTogglOfflineInactive = 5;

        private void loadStatusIcons()
        {
            if (statusIcons.Count > 0)
            {
                throw new InvalidOperationException("Status images already loaded");
            }
            statusIcons.Add(Properties.Resources.toggltray);
            statusIcons.Add(Properties.Resources.toggltray_inactive);
            statusIcons.Add(Properties.Resources.toggl);
            statusIcons.Add(Properties.Resources.toggl_inactive);
            statusIcons.Add(Properties.Resources.toggl_offline_active);
            statusIcons.Add(Properties.Resources.toggl_offline_inactive);
        }

        private void MainWindowController_Load(object sender, EventArgs e)
        {
            troubleBox.BackColor = Color.FromArgb(239, 226, 121);
            contentPanel.Location = defaultContentPosition;

            loadStatusIcons();

            Toggl.OnApp += OnApp;
            Toggl.OnError += OnError;
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

            loginViewController = new LoginViewController();
            timeEntryListViewController = new TimeEntryListViewController();
            timeEntryEditViewController = new TimeEntryEditViewController();
            aboutWindowController = new AboutWindowController();
            preferencesWindowController = new PreferencesWindowController();
            feedbackWindowController = new FeedbackWindowController();
            idleNotificationWindowController = new IdleNotificationWindowController();

            initEditForm();
            timeEntryListViewController.setEditPopup(editForm);

            FlowLayoutPanel listing = timeEntryListViewController.getListing();
            if (listing != null)
            {
                listing.Scroll += MainWindowControllerEntries_Scroll;
                listing.MouseWheel += MainWindowControllerEntries_Scroll;
            }

            if (!Toggl.Start(TogglDesktop.Program.Version()))
            {
                try
                {
                    DisableTop();
                    MessageBox.Show("Missing callback. See the log file for details");
                } finally {
                    EnableTop();
                }
                TogglDesktop.Program.Shutdown(1);
            }

            Utils.LoadWindowLocation(this, editForm);

            aboutWindowController.initAndCheck();

            runScriptTimer = new Timer();
            runScriptTimer.Interval = 1000;
            runScriptTimer.Tick += runScriptTimer_Tick;
            runScriptTimer.Start();
        }

        void runScriptTimer_Tick(object sender, EventArgs e)
        {
            runScriptTimer.Stop();

            string scriptPath = parseScriptPath();
            if (null == scriptPath)
            {
                return;
            }

            System.Threading.ThreadPool.QueueUserWorkItem(delegate
            {
                if (!File.Exists(scriptPath))
                {
                    Console.WriteLine("Script file does not exist: " + scriptPath);
                    TogglDesktop.Program.Shutdown(0);
                }

                string script = File.ReadAllText(scriptPath);

                Int64 err = 0;
                string result = Toggl.RunScript(script, ref err);
                if (0 != err)
                {
                    Console.WriteLine(string.Format("Failed to run script, err = {0}", err));
                }
                Console.WriteLine(result);

                if (0 == err)
                {
                    TogglDesktop.Program.Shutdown(0);
                }
            }, null);
        }

        private string parseScriptPath()
        {
            string[] args = Environment.GetCommandLineArgs();
            for (int i = 0; i < args.Length; i++)
            {
                if (args[i].Contains("script") && args[i].Contains("path"))
                {
                    return args[i + 1];
                }
            }

            return null;
        }

        private void MainWindowControllerEntries_Scroll(object sender, EventArgs e)
        {
            recalculatePopupPosition();
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
            updateStatusIcons(true);

            string newText = "Toggl Desktop";
            if (te.Description.Length > 0) {
                runningToolStripMenuItem.Text = te.Description.Replace("&", "&&");
                newText = te.Description + " - Toggl Desktop";
            }
            else
            {
                runningToolStripMenuItem.Text = "Timer is tracking";
            }
            if (newText.Length > 63)
            {
                newText = newText.Substring(0, 60) + "...";
            }
            Text = newText;
            if (trayIcon != null)
            {
                trayIcon.Text = Text;
            }
            updateResizeHandleBackground();
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
            updateStatusIcons(true);

            runningToolStripMenuItem.Text = "Timer is not tracking";
            Text = "Toggl Desktop";
            trayIcon.Text = Text;
            updateResizeHandleBackground();
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
            setGlobalShortCutKeys();
        }

        private void updateStatusIcons(bool is_online)
        {
            if (0 == statusIcons.Count)
            {
                return;
            }

            Icon tray = null;
            Icon form = null;

            if (is_online)
            {
                if (TogglDesktop.Program.IsLoggedIn && isTracking)
                {
                    tray = statusIcons[kTogglTray];
                    form = statusIcons[kToggl];
                }
                else
                {
                    tray = statusIcons[kTogglTrayInactive];
                    form = statusIcons[kTogglInactive];
                }
            }
            else
            {
                if (TogglDesktop.Program.IsLoggedIn && isTracking)
                {
                    tray = statusIcons[kTogglOfflineActive];
                    form = statusIcons[kToggl];
                }
                else
                {
                    tray = statusIcons[kTogglOfflineInactive];
                    form = statusIcons[kTogglInactive];
                }
            }

            if (Icon != form)
            {
                Icon = form;
            }

            if (null != trayIcon)
            {
                if (trayIcon.Icon != tray)
                {
                    trayIcon.Icon = tray;
                }
            }
        }

        void OnOnlineState(Int64 state)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnOnlineState(state); });
                return;
            }
            // FIXME: render online state on bottom of the window
            updateStatusIcons(0 == state);
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

            errorLabel.Text = errmsg;
            errorToolTip.SetToolTip(errorLabel, errmsg);
            troubleBox.Visible = true;
            contentPanel.Location = errorContentPosition;
        }

        void OnIdleNotification(
            string guid,
            string since,
            string duration,
            UInt64 started,
            string description)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnIdleNotification(guid, since, duration, started, description); });
                return;
            }

            idleNotificationWindowController.ShowWindow();
        }

        void OnLogin(bool open, UInt64 user_id)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnLogin(open, user_id); });
                return;
            }
            if (open) {
                if (editForm.Visible)
                {
                    editForm.Hide();
                    editForm.GUID = null;
                }
                contentPanel.Controls.Remove(timeEntryListViewController);
                contentPanel.Controls.Remove(timeEntryEditViewController);
                contentPanel.Controls.Add(loginViewController);
                MinimumSize = new Size(loginViewController.MinimumSize.Width, loginViewController.MinimumSize.Height + 40);
                loginViewController.SetAcceptButton(this);
                resizeHandle.BackColor = Color.FromArgb(69, 69, 69);
            }
            enableMenuItems();
            updateStatusIcons(true);

            if (open || 0 == user_id)
            {
                runningToolStripMenuItem.Text = "Timer is not tracking";
            }
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
                MinimumSize = new Size(230, 86);
                contentPanel.Controls.Add(timeEntryListViewController);
                timeEntryListViewController.SetAcceptButton(this);
                if (editForm.Visible)
                {
                    editForm.Hide();
                    editForm.GUID = null;
                }
            }
        }

        public static Control FindControlAtPoint(Control container, Point pos)
        {
            if (null == container)
            {
                return null;
            }

            if (container.GetType() == typeof(TimeEntryCell) || container.GetType() == typeof(TimerEditViewController))
            {
                return container;
            }

            foreach (Control c in container.Controls)
            {
                if (c.Visible && c.Bounds.Contains(pos))
                {
                    Control child = FindControlAtPoint(c, new Point(pos.X - c.Left, pos.Y - c.Top));
                    if (child != null && (child.GetType() == typeof(TimeEntryCell) || child.GetType() == typeof(TimerEditViewController)))
                    {
                        return child;
                    }
                }
            }

            return null;
        }

        public static Control FindControlAtCursor(Form form)
        {
            Point pos = Cursor.Position;
            if (form.Bounds.Contains(pos))
                return FindControlAtPoint(form, form.PointToClient(Cursor.Position));
            return null;
        }

        private void initEditForm()
        {
            editForm = new EditForm
            {
                ControlBox = false,
                StartPosition = FormStartPosition.Manual
            };
            editForm.Controls.Add(timeEntryEditViewController);
            editForm.editView = timeEntryEditViewController;
        }

        public void PopupInput(Toggl.TimeEntry te)
        {
            if (te.GUID == editForm.GUID) {
                if (editableEntry.GetType() == typeof(TimeEntryCell))
                {
                    ((TimeEntryCell)editableEntry).opened = false;
                }
                editForm.CloseButton_Click(null, null);
                return;
            }
            if (editableEntry != null && editableEntry.GetType() == typeof(TimeEntryCell))
            {
                ((TimeEntryCell)editableEntry).opened = false;
            }
            editForm.reset();
            editableEntry = getSelectedEntryByGUID(te.GUID);
            if (null == editableEntry)
            {
                return;
            }
            setEditFormLocation(te.DurationInSeconds < 0);
            editForm.GUID = te.GUID;
            editForm.Show();
        }

        private Control getSelectedEntryByGUID(string GUID)
        {
            Control c = timeEntryListViewController.findControlByGUID(GUID);
            if ( c != null) {
                return c;
            }
            return FindControlAtCursor(this);
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
                MinimumSize = new Size(230, 86);
                timeEntryEditViewController.setupView(this, focused_field_name);
                PopupInput(te);                
            }
        }

        private void MainWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            Utils.SaveWindowLocation(this, editForm);

            if (!TogglDesktop.Program.ShuttingDown)
            {
                Hide();
                e.Cancel = true;
            }

            if (editForm.Visible)
            {
                if (editableEntry.GetType() == typeof(TimeEntryCell))
                {
                    ((TimeEntryCell)editableEntry).opened = false;
                }
                editForm.ClosePopup();
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
            feedbackWindowController.TopMost = true;
        }

        private void quitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (Visible)
            {
                Utils.SaveWindowLocation(this, editForm);
            }

            TogglDesktop.Program.Shutdown(0);
        }

        private void toggleVisibility()
        {
            if (WindowState == FormWindowState.Minimized)
            {
                WindowState = FormWindowState.Normal;
                show();
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
            aboutWindowController.ShowUpdates();
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
            if (remainOnTop && !topDisabled)
            {
                SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                if (editForm != null)
                {
                    editForm.setWindowPos(HWND_TOPMOST);
                }
                return;
            }
            SetWindowPos(Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            if (editForm != null)
            {
                editForm.setWindowPos(HWND_NOTOPMOST);
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
            DialogResult dr;
            try
            {
                DisableTop();
                dr = MessageBox.Show(
                    "This will remove your Toggl user data from this PC and log you out of the Toggl Desktop app. " +
                    "Any unsynced data will be lost." +
                    Environment.NewLine + "Do you want to continue?",
                    "Clear Cache",
                    MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            }
            finally
            {
                EnableTop();
            }
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
            LASTINPUTINFO lastInputInfo = new LASTINPUTINFO();
            lastInputInfo.cbSize = Marshal.SizeOf(lastInputInfo);
            lastInputInfo.dwTime = 0;
            if (!GetLastInputInfo(out lastInputInfo)) {
                return;
            }
            int idle_seconds = unchecked(Environment.TickCount - (int)lastInputInfo.dwTime) / 1000;
            if (idle_seconds < 1) {
                return;
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

        private void MainWindowController_Activated(object sender, EventArgs e)
        {
            Toggl.SetWake();
        }

        private void MainWindowController_LocationChanged(object sender, EventArgs e)
        {
            recalculatePopupPosition();
        }

        private void setEditFormLocation(bool running)
        {
            if (Screen.AllScreens.Length > 1)
            {
                foreach (Screen s in Screen.AllScreens)
                {
                    if (s.WorkingArea.IntersectsWith(DesktopBounds))
                    {
                        calculateEditFormPosition(running, s);
                        break;
                    }
                }
            }
            else
            {
                calculateEditFormPosition(running,Screen.PrimaryScreen);
            }
        }

        private void calculateEditFormPosition(bool running, Screen s)
        {
            Point ctrlpt = PointToScreen(editableEntry.Location);
            int arrowTop = 0;
            bool left = false;

            if (Location.X < editForm.Width && (s.Bounds.Width - (Location.X + Width)) < editForm.Width)
            {
                ctrlpt.X += (Width/2);
            }
            else
            {
                if ((editForm.Width + ctrlpt.X + Width) > (s.WorkingArea.Location.X + s.Bounds.Width))
                {
                    ctrlpt.X -= editForm.Width;
                    left = true;
                }
                else
                {
                    ctrlpt.X += Width;
                }
            }

            if (running)
            {
                arrowTop = timeEntryListViewController.EntriesTop / 2;
            }
            else
            {
                if (editableEntry.GetType() == typeof(TimeEntryCell))
                {
                    ctrlpt.Y += timeEntryListViewController.EntriesTop + (((TimeEntryCell)editableEntry).getTopLocation()) - (editForm.Height / 2);
                    ((TimeEntryCell)editableEntry).opened = true;
                }
            }
            editForm.setPlacement(left, arrowTop, ctrlpt, s, this);

        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            if (keyData == Keys.Escape)
            {
                if (editForm.Visible)
                {
                    if (editableEntry.GetType() == typeof(TimeEntryCell))
                    {
                        ((TimeEntryCell)editableEntry).opened = false;
                    }
                    editForm.ClosePopup();
                }
            }
            return base.ProcessCmdKey(ref msg, keyData);
        }

        private void MainWindowController_SizeChanged(object sender, EventArgs e)
        {
            recalculatePopupPosition();
            if (timeEntryListViewController != null)
            {
                hideHorizontalScrollBar();
            }
            resizeHandle.Location = new Point(Width-16, Height-56);
            updateResizeHandleBackground();
        }

        private void updateResizeHandleBackground() {
            if (contentPanel.Controls.Contains(loginViewController))
            {
                resizeHandle.BackColor = Color.FromArgb(69, 69, 69);
            }
            else if (Height <= MinimumSize.Height)
            {
                String c = "#4dd965";
                if(isTracking) {
                    c = "#ff3d32";
                } 
                resizeHandle.BackColor = ColorTranslator.FromHtml(c);
            }
            else
            {
                resizeHandle.BackColor = System.Drawing.Color.Transparent;
            }
        }
        private void recalculatePopupPosition()
        {
            if (editForm != null && editForm.Visible && editableEntry != null)
            {
                setEditFormLocation(editableEntry.GetType() == typeof(TimerEditViewController));
            }
        }

        private void hideHorizontalScrollBar()
        {
            ShowScrollBar(timeEntryListViewController.getListing().Handle, SB_HORZ, false);
        }

        private void resizeHandle_MouseDown(object sender, MouseEventArgs e)
        {
            isResizing = true;
        }

        private void resizeHandle_MouseMove(object sender, MouseEventArgs e)
        {
            if (isResizing)
            {
                isResizing = (e.Button == MouseButtons.Left);
                ReleaseCapture();
                int buttonEvent = (isResizing) ? wmNcLButtonDown : wmNcLButtonUp;
                SendMessage(Handle, buttonEvent, HtBottomRight, 0);
            }
        }
    }
}
