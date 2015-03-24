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
        public AboutWindowController()
        {
            InitializeComponent();

            labelVersion.Text = TogglDesktop.Program.Version();

            bool updateCheckDisabled = Toggl.IsUpdateCheckDisabled();
            comboBoxChannel.Visible = !updateCheckDisabled;
            labelReleaseChannel.Visible = !updateCheckDisabled;
        }

        private void AboutWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            Hide();
            e.Cancel = true;
        }

        private void comboBoxChannel_SelectedIndexChanged(object sender, EventArgs e)
        {
            Toggl.SetUpdateChannel(comboBoxChannel.Text);
        }

        private void linkLabelGithub_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Process.Start(linkLabelGithub.Text);
        }

        public void ShowUpdates()
        {
            Show();
            if (comboBoxChannel.Visible)
            {
                string channel = Toggl.UpdateChannel();
                comboBoxChannel.SelectedIndex = comboBoxChannel.Items.IndexOf(channel);
            }
        }

        internal void initAndCheck()
        {
            string channel = Toggl.UpdateChannel();
            comboBoxChannel.SelectedIndex = comboBoxChannel.Items.IndexOf(channel);
        }
    }
}
