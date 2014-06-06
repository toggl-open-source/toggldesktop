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
        private bool isUpgradeDialogVisible = false;
        private bool isTracking = false;

        public MainWindowController()
        {
            InitializeComponent();

            loginViewController = new LoginViewController();
            timeEntryListViewController = new TimeEntryListViewController();
            timeEntryEditViewController = new TimeEntryEditViewController();
            aboutWindowController = new AboutWindowController();
            preferencesWindowController = new PreferencesWindowController();
            feedbackWindowController = new FeedbackWindowController();
        }

        private void MainWindowController_Load(object sender, EventArgs e)
        {
            troubleBox.BackColor = Color.FromArgb(239, 226, 121);

            Utils.LoadWindowLocation(this);

            KopsikApi.OnError += OnError;
            KopsikApi.OnUpdate += OnUpdate;
            KopsikApi.OnLogin += OnLogin;
            KopsikApi.OnTimeEntryList += OnTimeEntryList;
            KopsikApi.OnTimeEntryEditor += OnTimeEntryEditor;
            KopsikApi.OnOnlineState += OnOnlineState;
            KopsikApi.OnReminder += OnReminder;
            KopsikApi.OnURL += OnURL;
            KopsikApi.OnTimerState += OnTimerState;

            if (!KopsikApi.Start(TogglDesktop.Program.Version()))
            {
                MessageBox.Show("Missing callback. See the log file for details");
                TogglDesktop.Program.Shutdown(1);
            }
        }

        void OnTimerState(IntPtr te)
        {
            DisplayTimerState(te != IntPtr.Zero);
        }

        void DisplayTimerState(bool is_tracking) {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayTimerState(is_tracking); });
                return;
            }
            isTracking = is_tracking;
            enableMenuItems();
            displayTrayIcon();
        }

        private void displayTrayIcon()
        {
            if (TogglDesktop.Program.IsLoggedIn && isTracking)
            {
                trayIcon.Icon = Properties.Resources.toggl;
            }
            else
            {
                trayIcon.Icon = Properties.Resources.toggl_inactive;
            }
        }

        void OnOnlineState(bool is_online)
        {
            DisplayOnlineState(is_online);
        }

        void DisplayOnlineState(bool is_online)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayOnlineState(is_online); });
                return;
            }
            // FIXME:
        }

        void OnUpdate(bool open, ref KopsikApi.KopsikUpdateViewItem view)
        {
            DisplayUpdate(open, view);
        }

        void DisplayUpdate(bool open, KopsikApi.KopsikUpdateViewItem view)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayUpdate(open, view); });
                return;
            }
            if (open)
            {
                aboutWindowController.Show();
                aboutWindowController.BringToFront();
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

        void OnError(string errmsg, bool user_error)
        {
            DisplayError(errmsg, user_error);
        }

        void DisplayError(string errmsg, bool user_error)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayError(errmsg, user_error); });
                return;
            }
            errorLabel.Text = errmsg;
            troubleBox.Visible = true;

            if (!user_error)
            {
                Bugsnag.Library.BugSnag bs = new Bugsnag.Library.BugSnag();
                bs.Notify(new Exception(errmsg));
            }
        }

        void OnLogin(bool open, UInt64 user_id)
        {
            DisplayLogin(open, user_id);
        }

        void DisplayLogin(bool open, UInt64 user_id)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayLogin(open, user_id); });
                return;
            }
            TogglDesktop.Program.UserID = user_id;
            if (open) {
                Controls.Remove(timeEntryListViewController);
                Controls.Remove(timeEntryEditViewController);
                Controls.Add(loginViewController);
                loginViewController.SetAcceptButton(this);
            }
            enableMenuItems();
            displayTrayIcon();
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

        void OnTimeEntryList(bool open, IntPtr first)
        {
            DisplayTimeEntryList(open);
        }

        void DisplayTimeEntryList(bool open)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayTimeEntryList(open); });
                return;
            }
            if (open)
            {
                Controls.Remove(loginViewController);
                Controls.Remove(timeEntryEditViewController);
                Controls.Add(timeEntryListViewController);
                timeEntryListViewController.SetAcceptButton(this);
            }
        }

        void OnTimeEntryEditor(
            bool open,
            ref KopsikApi.KopsikTimeEntryViewItem te,
            string focused_field_name)
        {
            DisplayTimeEntryEditor(open, focused_field_name);
        }

        void DisplayTimeEntryEditor(
            bool open,
            string focused_field_name) {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayTimeEntryEditor(open, focused_field_name); });
                return;
            }
            if (open)
            {
                Controls.Remove(loginViewController);
                Controls.Remove(timeEntryListViewController);
                Controls.Add(timeEntryEditViewController);
                timeEntryEditViewController.SetAcceptButton(this);
                timeEntryEditViewController.SetFocus(focused_field_name);
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
        }

        private void sendFeedbackToolStripMenuItem_Click(object sender, EventArgs e)
        {
            feedbackWindowController.Show();
            feedbackWindowController.BringToFront();
        }

        private void quitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TogglDesktop.Program.Shutdown(0);
        }

        private void trayIcon_DoubleClick(object sender, EventArgs e)
        {
            if (this.Visible)
            {
                this.Hide();
                return;
            }
            show();
        }

        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_start(KopsikApi.ctx, "", "", 0, 0);
            show();
        }

        private void continueToolStripMenuItem_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_continue_latest(KopsikApi.ctx);
            show();
        }

        private void stopToolStripMenuItem_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_stop(KopsikApi.ctx);
            show();
        }

        private void showToolStripMenuItem_Click(object sender, EventArgs e)
        {
            show();
        }

        private void syncToolStripMenuItem_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_sync(KopsikApi.ctx);
        }

        private void openInBrowserToolStripMenuItem_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_open_in_browser(KopsikApi.ctx);
        }

        private void preferencesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_edit_preferences(KopsikApi.ctx);
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_about(KopsikApi.ctx);
        }

        private void logoutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_logout(KopsikApi.ctx);
        }

        private void show()
        {
            this.Show();
            this.BringToFront();
        }

        void OnReminder(string title, string informative_text)
        {
            DisplayReminder(title, informative_text);
        }

        void DisplayReminder(string title, string informative_text)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayReminder(title, informative_text); });
                return;
            }
            // FIXME:
        }

        private void clearCacheToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult dr = MessageBox.Show(
                "This will remove your Toggl user data from this PC and log you out of the Toggl Desktop app. " +
                "Any unsynced data will be lost." +
                Environment.NewLine + "Do you want to continue?",
                "Clear Cache",
                MessageBoxButtons.OKCancel, MessageBoxIcon.Question);
            if (DialogResult.Yes == dr) 
            {
                KopsikApi.kopsik_clear_cache(KopsikApi.ctx);
            }
        }
    }
}
