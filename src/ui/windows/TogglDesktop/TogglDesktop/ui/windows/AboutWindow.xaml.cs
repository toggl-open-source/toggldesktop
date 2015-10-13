using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace TogglDesktop
{
    public partial class AboutWindow
    {
        public AboutWindow()
        {
            this.InitializeComponent();

            this.updateText.Text = "";

            this.versionText.Text = Program.Version();

            var isUpdatCheckDisabled = Toggl.IsUpdateCheckDisabled();
            this.releaseChannelComboBox.ShowOnlyIf(!isUpdatCheckDisabled, true);
            this.releaseChannelLabel.ShowOnlyIf(!isUpdatCheckDisabled, true);

            Toggl.OnDisplayUpdate += this.onDisplayUpdate;
        }

        private void onDisplayUpdate(string url)
        {
            if (this.TryBeginInvoke(this.onDisplayUpdate, url))
                return;

            // TODO: Display update status
        }

        private void windowKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
            {
                this.Hide();
                e.Handled = true;
            }
        }

        protected override void onCloseButtonClick(object sender, RoutedEventArgs e)
        {
            this.Hide();
        }

        private void onReleaseChannelSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var value = this.releaseChannelComboBox.SelectedValue;
            if (value == null)
                return;

            Toggl.SetUpdateChannel(value.ToString());
        }

        public void UpdateReleaseChannel()
        {
            var channel = Toggl.UpdateChannel();
            this.releaseChannelComboBox.SelectedValue = channel;
        }

        private void onGithubLinkClick(object sender, RoutedEventArgs e)
        {
            Process.Start("https://github.com/toggl/toggldesktop");
        }

        private void onChangelogLinkClick(object sender, RoutedEventArgs e)
        {
            Process.Start("http://toggl.github.io/toggldesktop");
        }
    }
}
