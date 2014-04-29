using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Reflection;
using System.Diagnostics;

namespace TogglDesktop
{
    public partial class MainWindowController : Form
    {
        private LoginViewController loginViewController;
        private TimeEntryListViewController timeEntryListViewController;

        public MainWindowController()
        {
            InitializeComponent();

            loginViewController = new LoginViewController();
            timeEntryListViewController = new TimeEntryListViewController();
        }

        private void MainWindowController_Load(object sender, EventArgs e)
        {
            troubleBox.BackColor = Color.FromArgb(239, 226, 121);

            loadWindowLocation();

            Core.OnUserLogin += Core_OnUserLogin;
            Core.OnError += Core_OnError;
            Core.OnCheckUpdate += Core_OnCheckUpdate;
            Core.OnOnline += Core_OnOnline;

            Assembly assembly = Assembly.GetExecutingAssembly();
            FileVersionInfo versionInfo = FileVersionInfo.GetVersionInfo(assembly.Location);
            Core.Init("windows_native_app", versionInfo.ProductVersion);
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

        void Core_OnOnline()
        {
            throw new NotImplementedException();
        }

        void Core_OnCheckUpdate(bool is_update_available, string url, string version)
        {
            throw new NotImplementedException();
        }

        void Core_OnError(string errmsg)
        {
            errorLabel.Text = errmsg;
            troubleBox.Visible = true;
        }

        void Core_OnUserLogin(ulong id, string fullname, string timeofdayformat)
        {
            if (0 == id) {
                Controls.Remove(timeEntryListViewController);
                Controls.Add(loginViewController);
                loginViewController.SetAcceptButton(this);
                return;
            }
            Controls.Remove(loginViewController);
            Controls.Add(timeEntryListViewController);
            timeEntryListViewController.SetAcceptButton(this);
        }

        private void MainWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            saveWindowLocation();
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
        }

        private void quitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }
    }
}
