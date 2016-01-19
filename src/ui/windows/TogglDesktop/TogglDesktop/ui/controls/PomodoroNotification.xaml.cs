
using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public partial class PomodoroNotification
    {
        private readonly TaskbarIcon icon;
        private readonly MainWindow mainWindow;

        public PomodoroNotification(TaskbarIcon icon, MainWindow mainWindow)
        {
            this.icon = icon;
            this.mainWindow = mainWindow;
            this.InitializeComponent();

            Toggl.OnDisplayPomodoro += this.onDisplayPomodoro;
        }

        private void onDisplayPomodoro(string title, string informativetext)
        {
            if (this.TryBeginInvoke(onDisplayPomodoro, title, informativetext))
                return;

            this.reminderText.Text = informativetext;
            this.titleText.Text = title;

            this.RemoveFromParent();

            this.icon.ShowCustomBalloon(this, PopupAnimation.Slide, 6000);
        }

        private void onNotificationMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.icon.CloseBalloon();
        }

        private void onStopButtonClick(object sender, RoutedEventArgs e)
        {
            this.icon.CloseBalloon();
            Toggl.Stop();
            this.mainWindow.ShowOnTop();
        }
    }
}
