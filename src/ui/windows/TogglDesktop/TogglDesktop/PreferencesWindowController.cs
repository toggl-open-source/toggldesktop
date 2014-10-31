using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TogglDesktop
{
    public partial class PreferencesWindowController : TogglForm
    {
        public PreferencesWindowController()
        {
            InitializeComponent();

            Toggl.OnSettings += OnSettings;
            Toggl.OnLogin += OnLogin;
        }

        private void PreferencesWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            e.Cancel = true;
            Hide();
        }

        void OnLogin(bool open, UInt64 user_id)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnLogin(open, user_id); });
                return;
            }
            checkBoxRecordTimeline.Enabled = !open && user_id != 0;
            checkBoxRecordTimeline.Checked = Toggl.IsTimelineRecordingEnabled();
        }

        void OnSettings(bool open, Toggl.Settings settings)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnSettings(open, settings); });
                return;
            }
            groupBoxProxySettings.Enabled = settings.UseProxy;
            checkBoxUseProxy.Checked = settings.UseProxy;
            textBoxProxyHost.Text = settings.ProxyHost;
            textBoxProxyPort.Text = settings.ProxyPort.ToString();
            textBoxProxyUsername.Text = settings.ProxyUsername;
            textBoxProxyPassword.Text = settings.ProxyPassword;
            checkBoxIdleDetection.Checked = settings.UseIdleDetection;
            checkBoxRecordTimeline.Checked = settings.RecordTimeline;
            checkBoxOnTop.Checked = settings.OnTop;
            checkBoxRemindToTrackTime.Checked = settings.Reminder;
            textBoxIdleMinutes.Text = settings.IdleMinutes.ToString();

            if (open)
            {
                Show();
                TopMost = true;
            }
        }

        private void checkBoxUseProxy_CheckedChanged(object sender, EventArgs e)
        {
            groupBoxProxySettings.Enabled = checkBoxUseProxy.Checked;
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void buttonSave_Click(object sender, EventArgs e)
        {
            ulong port = 0;
            if (!ulong.TryParse(textBoxProxyPort.Text, out port))
            {
                port = 0;
            }

            ulong idleMinutes = 5;
            if (!ulong.TryParse(textBoxIdleMinutes.Text, out idleMinutes))
            {
                idleMinutes = 5;
            }

            Toggl.Settings settings = new Toggl.Settings();

            settings.UseProxy = checkBoxUseProxy.Checked;
            settings.ProxyPort = port;
            settings.ProxyHost = textBoxProxyHost.Text;
            settings.ProxyUsername = textBoxProxyUsername.Text;
            settings.ProxyPassword = textBoxProxyPassword.Text;

            settings.UseIdleDetection = checkBoxIdleDetection.Checked;
            settings.OnTop = checkBoxOnTop.Checked;
            settings.Reminder = checkBoxRemindToTrackTime.Checked;
            settings.IdleMinutes = idleMinutes;

            settings.MenubarTimer = true;
            settings.DockIcon = true;

            settings.RecordTimeline = checkBoxRecordTimeline.Checked;

            if (!Toggl.SetSettings(settings))
            {
                return;
            }

            Close();
        }

        private void checkBoxIdleDetection_CheckedChanged(object sender, EventArgs e)
        {
            textBoxIdleMinutes.Enabled = checkBoxIdleDetection.Checked;
        }
    }
}
