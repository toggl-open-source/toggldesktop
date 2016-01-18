
using System.Windows;

namespace TogglDesktop
{
    public partial class PomodoroNotification
    {
        public PomodoroNotification()
        {
            this.InitializeComponent();

            Toggl.OnDisplayPomodoro += this.onDisplayPomodoro;
        }

        private void onDisplayPomodoro(string title, string informativetext)
        {
            if (this.TryBeginInvoke(onDisplayPomodoro, title, informativetext))
                return;

            this.timeText.Text = informativetext;
            this.Title = title;

            this.Show();
        }

        protected override void onCloseButtonClick(object sender, RoutedEventArgs e)
        {
            this.Hide();
        }

        private void onStopClicked(object sender, RoutedEventArgs e)
        {
            this.Hide();
            Toggl.Stop();
            Toggl.ShowApp();
        }

        private void onResetClick(object sender, RoutedEventArgs e)
        {
            //TODO: call api
        }
    }
}
