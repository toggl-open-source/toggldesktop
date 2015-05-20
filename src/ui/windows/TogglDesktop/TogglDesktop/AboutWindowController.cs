using System;
using System.Diagnostics;
using System.Windows.Forms;

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
        if (CloseReason.WindowsShutDown == e.CloseReason)
        {
            return;
        }

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
