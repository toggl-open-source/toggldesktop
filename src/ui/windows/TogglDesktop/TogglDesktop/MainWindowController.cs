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
        private AboutWindowController aboutWindowController;
        private PreferencesWindowController preferencesWindowController;
        private FeedbackWindowController feedbackWindowController;
        private bool shuttingDown = false;
        private UInt64 userID = 0;

        public MainWindowController()
        {
            InitializeComponent();

            loginViewController = new LoginViewController();
            timeEntryListViewController = new TimeEntryListViewController();
            aboutWindowController = new AboutWindowController();
            preferencesWindowController = new PreferencesWindowController();
            feedbackWindowController = new FeedbackWindowController();
        }

        private void MainWindowController_Load(object sender, EventArgs e)
        {
            troubleBox.BackColor = Color.FromArgb(239, 226, 121);

            loadWindowLocation();

            KopsikApi.OnError += OnError;
            KopsikApi.OnUpdate += OnUpdate;
            KopsikApi.OnLogin += OnLogin;
            KopsikApi.OnTimeEntryList += OnTimeEntryList;
            KopsikApi.OnTimeEntryEditor += OnTimeEntryEditor;
            KopsikApi.OnOnlineState += OnOnlineState;
            KopsikApi.OnReminder += OnReminder;
            KopsikApi.OnAutocomplete += OnAutocomplete;
            KopsikApi.OnWorkspaceSelect += OnWorkspaceSelect;
            KopsikApi.OnClientSelect += OnClientSelect;
            KopsikApi.OnTags += OnTags;
            KopsikApi.OnURL += OnURL;

            if (!KopsikApi.Start())
            {
                MessageBox.Show("Missing callback. See the log file for details");
                shutdown();
            }
        }

        private void shutdown()
        {
            shuttingDown = true;
            Application.Exit();
        }

        private void loadWindowLocation()
        {
            if (Properties.Settings.Default.Maximized)
            {
                WindowState = FormWindowState.Maximized;
                Location = Properties.Settings.Default.Location;
                Size = Properties.Settings.Default.Size;
            }
            else if (Properties.Settings.Default.Minimized)
            {
                WindowState = FormWindowState.Minimized;
                Location = Properties.Settings.Default.Location;
                Size = Properties.Settings.Default.Size;
            }
            else
            {
                Location = Properties.Settings.Default.Location;
                Size = Properties.Settings.Default.Size;
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

        void OnUpdate(bool is_update_available, string url, string version)
        {
            DisplayUpdate(is_update_available, url, version);
        }

        void DisplayUpdate(bool is_update_available, string url, string version)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayUpdate(is_update_available, url, version); });
                return;
            }
            // FIXME:
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
                // FIXME: notify bugsnag
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
            userID = user_id;
            if (open) {
                Controls.Remove(timeEntryListViewController);
                Controls.Add(loginViewController);
                loginViewController.SetAcceptButton(this);
            }
            // FIXME: configure bugsnag
        }

        void OnTimeEntryList(bool open, ref KopsikApi.KopsikTimeEntryViewItem te)
        {
            List<KopsikApi.KopsikTimeEntryViewItem> list = KopsikApi.ConvertToTimeEntryList(ref te);
            DisplayTimeEntryList(open, list);
        }

        void DisplayTimeEntryList(bool open, List<KopsikApi.KopsikTimeEntryViewItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayTimeEntryList(open, list); });
                return;
            }
            Controls.Remove(loginViewController);
            Controls.Add(timeEntryListViewController);
            timeEntryListViewController.SetAcceptButton(this);
        }

        void OnTimeEntryEditor(
            bool open,
            ref KopsikApi.KopsikTimeEntryViewItem te,
            string focused_field_name)
        {
            KopsikApi.KopsikTimeEntryViewItem n = te;
            DisplayTimeEntryEditor(open, n, focused_field_name);
        }

        void DisplayTimeEntryEditor(
            bool open,
            KopsikApi.KopsikTimeEntryViewItem te,
            string focused_field_name) {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayTimeEntryEditor(open, te, focused_field_name); });
                return;
            }
            // FIXME:
        }

        private void MainWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            saveWindowLocation();

            if (!shuttingDown) {
                this.Hide();
                e.Cancel = true;
            }   
        }

        private void saveWindowLocation()
        {
            if (WindowState == FormWindowState.Maximized)
            {
                Properties.Settings.Default.Location = RestoreBounds.Location;
                Properties.Settings.Default.Size = RestoreBounds.Size;
                Properties.Settings.Default.Maximized = true;
                Properties.Settings.Default.Minimized = false;
            }
            else if (WindowState == FormWindowState.Normal)
            {
                Properties.Settings.Default.Location = Location;
                Properties.Settings.Default.Size = Size;
                Properties.Settings.Default.Maximized = false;
                Properties.Settings.Default.Minimized = false;
            }
            else
            {
                Properties.Settings.Default.Location = RestoreBounds.Location;
                Properties.Settings.Default.Size = RestoreBounds.Size;
                Properties.Settings.Default.Maximized = false;
                Properties.Settings.Default.Minimized = true;
            }
            Properties.Settings.Default.Save();
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
            shutdown();
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
            aboutWindowController.Show();
            aboutWindowController.BringToFront();
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

        void OnAutocomplete(ref KopsikApi.KopsikAutocompleteItem first)
        {
            List<KopsikApi.KopsikAutocompleteItem> list =
                KopsikApi.ConvertToAutocompleteList(ref first);
            DisplayAutocomplete(list);
        }

        void DisplayAutocomplete(List<KopsikApi.KopsikAutocompleteItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayAutocomplete(list); });
                return;
            }
            // FIXME:
        }

        void OnWorkspaceSelect(ref KopsikApi.KopsikViewItem first)
        {
            List<KopsikApi.KopsikViewItem> list = KopsikApi.ConvertToViewItemList(ref first);
            DisplayWorkspaceSelect(list);
        }

        void DisplayWorkspaceSelect(List<KopsikApi.KopsikViewItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayWorkspaceSelect(list); });
                return;
            }
            // FIXME:
        }

        void OnClientSelect(ref KopsikApi.KopsikViewItem first)
        {
            List<KopsikApi.KopsikViewItem> list = KopsikApi.ConvertToViewItemList(ref first);
            DisplayClientSelect(list);
        }

        void DisplayClientSelect(List<KopsikApi.KopsikViewItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayClientSelect(list); });
                return;
            }
            // FIXME: 
        }

        void OnTags(ref KopsikApi.KopsikViewItem first)
        {
            List<KopsikApi.KopsikViewItem> list = KopsikApi.ConvertToViewItemList(ref first);
            DisplayTags(list);
        }

        void DisplayTags(List<KopsikApi.KopsikViewItem> list) {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayTags(list); });
                return;
            }
            // FIXME: 
        }

    }
}
