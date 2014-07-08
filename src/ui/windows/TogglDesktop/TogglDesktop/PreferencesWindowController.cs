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
    public partial class PreferencesWindowController : Form
    {
        private bool loading;
        private bool mustSaveProxySettings;
        private bool mustSaveSettings;

        public PreferencesWindowController()
        {
            InitializeComponent();

            Toggl.OnSettings += OnSettings;
        }

        private void PreferencesWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            e.Cancel = true;
            if (mustSaveSettings)
            {
                if (!saveSettings())
                {
                    return;
                }
            }
            if (mustSaveProxySettings)
            {
                if (!saveProxySettings())
                {
                    return;
                }
            }
            Hide();
        }

        void OnSettings(bool open, Toggl.Settings settings)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnSettings(open, settings); });
                return;
            }
            mustSaveProxySettings = false;
            mustSaveSettings = false;
            loading = true;
            try
            {
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
            }
            finally
            {
                loading = false;
            }
            if (open)
            {
                Show();
                TopMost = true;
            }
        }

        private void checkBoxUseProxy_CheckedChanged(object sender, EventArgs e)
        {
            mustSaveProxySettings = true;
            groupBoxProxySettings.Enabled = checkBoxUseProxy.Checked;
        }

        private bool saveProxySettings()
        {
            if (loading || !mustSaveProxySettings)
            {
                return true;
            }
            ulong port = 0;
            if (!ulong.TryParse(textBoxProxyPort.Text, out port))
            {
                port = 0;
            }
            return Toggl.SetProxySettings(
                checkBoxUseProxy.Checked,
                textBoxProxyHost.Text,
                port,
                textBoxProxyUsername.Text,
                textBoxProxyPassword.Text);
        }

        private bool saveSettings()
        {
            if (loading || !mustSaveSettings)
            {
                return true;
            }
            return Toggl.SetSettings(
                checkBoxIdleDetection.Checked,
                true,
                true,
                checkBoxOnTop.Checked,
                checkBoxRemindToTrackTime.Checked);
        }

        private void textBoxProxyHost_TextChanged(object sender, EventArgs e)
        {
            mustSaveProxySettings = true;
        }

        private void textBoxProxyPort_TextChanged(object sender, EventArgs e)
        {
            mustSaveProxySettings = true;
        }

        private void textBoxProxyUsername_TextChanged(object sender, EventArgs e)
        {
            mustSaveProxySettings = true;
        }

        private void textBoxProxyPassword_TextChanged(object sender, EventArgs e)
        {
            mustSaveProxySettings = true;
        }

        private void checkBoxIdleDetection_CheckedChanged(object sender, EventArgs e)
        {
            mustSaveSettings = true;
        }

        private void checkBoxRecordTimeline_CheckedChanged(object sender, EventArgs e)
        {
            Toggl.ToggleTimelineRecording();
        }

        private void checkBoxOnTop_CheckedChanged(object sender, EventArgs e)
        {
            mustSaveSettings = true;
        }

        private void checkBoxRemindToTrackTime_CheckedChanged(object sender, EventArgs e)
        {
            mustSaveSettings = true;
        }

        private void checkBoxIgnoreCert_CheckedChanged(object sender, EventArgs e)
        {
            mustSaveSettings = true;
        }
    }
}
