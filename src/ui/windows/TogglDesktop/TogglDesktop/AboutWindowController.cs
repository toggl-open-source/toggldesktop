using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;

namespace TogglDesktop
{
    public partial class AboutWindowController : TogglForm
    {
        private WinSparkleDotNet.WinSparkle WinSparkle = null;

        public AboutWindowController()
        {
            InitializeComponent();

            labelVersion.Text = TogglDesktop.Program.Version();

            Toggl.OnUpdate += OnUpdate;

            bool updateCheckDisabled = Toggl.IsUpdateCheckDisabled();
            comboBoxChannel.Visible = !updateCheckDisabled;
            labelReleaseChannel.Visible = !updateCheckDisabled;

            WinSparkle = new WinSparkleDotNet.WinSparkle();
        }

        void OnUpdate(bool open, Toggl.Update view)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnUpdate(open, view); });
                return;
            }

            comboBoxChannel.Tag = "ignore";
            try
            {
                comboBoxChannel.Text = view.UpdateChannel;
            }
            finally
            {
                comboBoxChannel.Tag = null;
            }
        }

        private void AboutWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            Hide();
            e.Cancel = true;
            WinSparkle.Cleanup();
        }

        private void buttonCheckingForUpdate_Click(object sender, EventArgs e)
        {
            checkChannelUpdate(true);
        }

        private void AboutWindowController_Load(object sender, EventArgs e)
        {
        }

        private void comboBoxChannel_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (null == comboBoxChannel.Tag)
            {
                Toggl.SetUpdateChannel(comboBoxChannel.Text);
                checkChannelUpdate(false);
            }
        }

        private void checkChannelUpdate(bool withUi)
        {
            if (comboBoxChannel.Text.Length == 0) return;
            String url = "https://assets.toggl.com/installers/windows_" + comboBoxChannel.Text + "_appcast.xml";
            WinSparkle.SetAppCastUrl(url);
            if (withUi)
            {
                WinSparkle.CheckUpdateWithUi();
            }
            else
            {
                WinSparkle.CheckUpdateWithoutUi();
            }
        }

        private void linkLabelGithub_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Process.Start(linkLabelGithub.Text);
        }
    }
}
