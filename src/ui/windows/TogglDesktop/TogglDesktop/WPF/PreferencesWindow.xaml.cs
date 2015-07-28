
using System.Windows;
using System.Windows.Input;

namespace TogglDesktop.WPF
{
    public partial class PreferencesWindow
    {
        public PreferencesWindow()
        {
            this.InitializeComponent();

            Toggl.OnSettings += this.onSettings;
            Toggl.OnLogin += this.onLogin;
        }

        private void onLogin(bool open, ulong userID)
        {
            if (this.TryInvoke(() => this.onLogin(open, userID)))
                return;

            //TODO
        }

        private void onSettings(bool open, Toggl.Settings settings)
        {
            if (this.TryInvoke(() => this.onSettings(open, settings)))
                return;

            //TODO

            if (open)
            {
                this.Show();
                this.Topmost = true;
            }
        }

        private void saveButtonClicked(object sender, RoutedEventArgs e)
        {
            //TODO

            this.Hide();
        }

        private void cancelButtonClicked(object sender, RoutedEventArgs e)
        {
            this.Hide();
        }

        private void closeButtonClick(object sender, RoutedEventArgs e)
        {
            this.Hide();
        }

        private void headerLeftMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.DragMove();
        }
    }
}
