using System;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace TogglDesktop
{
    public partial class AboutWindow
    {
        public string[] Channels { get; } = new[] {"Stable", "Beta", "Dev"};

        public AboutWindow()
        {
            this.InitializeComponent();
            this.Closing += this.HideWindowOnClosing;

            this.updateText.Text = "";
            this.restartButton.Visibility = Visibility.Collapsed;

            this.versionText.Text = $"Version {Program.Version()} {Utils.Bitness()}";

            var isUpdatCheckDisabled = Toggl.IsUpdateCheckDisabled();
            this.releaseChannelComboBox.ShowOnlyIf(!isUpdatCheckDisabled, true);
            this.releaseChannelLabel.ShowOnlyIf(!isUpdatCheckDisabled, true);

            Toggl.OnDisplayUpdateDownloadState += this.onDisplayUpdateDownloadState;
        }

        private void onDisplayUpdateDownloadState(string version, Toggl.DownloadStatus status)
        {
            if (this.TryBeginInvoke(this.onDisplayUpdateDownloadState, version, status))
                return;

            string format;
            switch (status)
            {
                case Toggl.DownloadStatus.Started:
                {
                    format = "Downloading version {0} ...";
                    this.restartButton.Visibility = Visibility.Collapsed;
                    break;
                }
                case Toggl.DownloadStatus.Done:
                {
                    format = "New version {0} available!";
                    this.restartButton.IsEnabled = true;
                    this.restartButton.Visibility = Visibility.Visible;
                    break;
                }
                default:
                    throw new ArgumentOutOfRangeException("status", status, null);
            }

            this.updateText.Text = string.Format(format, version);
            this.updateText.Visibility = Visibility.Visible;
        }

        private void windowKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
            {
                this.Hide();
                e.Handled = true;
            }
        }

        private void onReleaseChannelSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var value = this.releaseChannelComboBox.SelectedValue;
            if (value == null)
                return;
            var channel = value.ToString().ToLower();
            Toggl.SetUpdateChannel(channel);
        }

        public void UpdateReleaseChannel()
        {
            var channel = Toggl.UpdateChannel();
            this.releaseChannelComboBox.SelectedItem = Channels.First(x => x.Equals(channel, StringComparison.OrdinalIgnoreCase));
        }

        private void onGithubLinkClick(object sender, RoutedEventArgs e)
        {
            Toggl.OpenInBrowser("https://github.com/toggl/toggldesktop");
        }

        private void onChangelogLinkClick(object sender, RoutedEventArgs e)
        {
            Toggl.OpenInBrowser("http://toggl.github.io/toggldesktop");
        }

        private void onRestartButtonClick(object sender, RoutedEventArgs e)
        {
            this.restartButton.IsEnabled = false;

            this.Hide();

            Toggl.RestartAndUpdate();

            MessageBox.Show(this, "Something went wrong.\nPlease restart Toggl Desktop manually.");
        }
    }
}
