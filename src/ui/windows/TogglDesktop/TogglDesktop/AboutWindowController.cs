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
    public partial class AboutWindowController : Form
    {
        private string updateURL;

        public AboutWindowController()
        {
            InitializeComponent();

            labelVersion.Text = TogglDesktop.Program.Version();

            TogglApi.OnUpdate += OnUpdate;
        }

        void OnUpdate(bool open, TogglApi.KopsikUpdateViewItem view)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnUpdate(open, view); });
                return;
            }

            updateURL = view.URL;

            comboBoxChannel.Tag = "ignore";
            try
            {
                comboBoxChannel.Text = view.UpdateChannel;
            }
            finally
            {
                comboBoxChannel.Tag = null;
            }

            if (view.IsChecking)
            {
                buttonCheckingForUpdate.Enabled = false;
                comboBoxChannel.Enabled = false;
                buttonCheckingForUpdate.Text = "Checking for update..";
                return;
            }

            comboBoxChannel.Enabled = true;

            if (view.IsUpdateAvailable)
            {
                buttonCheckingForUpdate.Text = string.Format(
                    "Click here to download update! {0}", view.Version);
                buttonCheckingForUpdate.Enabled = true;
            }
            else
            {
                buttonCheckingForUpdate.Text = "TogglDesktop is up to date.";
                buttonCheckingForUpdate.Enabled = false;
            }
        }

        private void AboutWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            Hide();
            e.Cancel = true;
        }

        private void buttonCheckingForUpdate_Click(object sender, EventArgs e)
        {
            Process.Start(updateURL);
            TogglDesktop.Program.Shutdown(0);
        }

        private void AboutWindowController_Load(object sender, EventArgs e)
        {

        }

        private void comboBoxChannel_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (null == comboBoxChannel.Tag)
            {
                TogglApi.SetUpdateChannel(comboBoxChannel.Text);
            }
        }

        private void linkLabelGithub_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Process.Start(linkLabelGithub.Text);
        }
    }
}
