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

        public MainWindowController()
        {
            InitializeComponent();

            loginViewController = new LoginViewController();
        }

        private void MainWindowController_Load(object sender, EventArgs e)
        {
            troubleBox.BackColor = Color.FromArgb(239, 226, 121);

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

            Core.OnUserLogin += Core_OnUserLogin;
            Core.OnError += Core_OnError;
            Core.OnCheckUpdate += Core_OnCheckUpdate;
            Core.OnOnline += Core_OnOnline;

            Assembly assembly = Assembly.GetExecutingAssembly();
            FileVersionInfo versionInfo = FileVersionInfo.GetVersionInfo(assembly.Location);
            Core.Init("windows_native_app", versionInfo.ProductVersion);
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
                Controls.Add(loginViewController);
                loginViewController.SetAcceptButton(this);
                return;
            }
        }

        private void MainWindowController_FormClosing(object sender, FormClosingEventArgs e)
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
    }
}
