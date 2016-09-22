
using System;
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
            Toggl.OnDisplayPomodoroBreak += this.onDisplayPomodoroBreak;
        }

        private void onDisplayPomodoro(string title, string informativetext)
        {
            if (this.TryBeginInvoke(onDisplayPomodoro, title, informativetext))
                return;

            this.reminderText.Text = informativetext;
            this.titleText.Text = title;

            this.RemoveFromParent();

            this.icon.ShowCustomBalloon(this, PopupAnimation.Slide, null);

            System.Media.SystemSounds.Asterisk.Play();
        }

        private void onDisplayPomodoroBreak(string title, string informativetext)
        {
            if (this.TryBeginInvoke(onDisplayPomodoroBreak, title, informativetext))
                return;

            this.reminderText.Text = informativetext;
            this.titleText.Text = title;

            this.RemoveFromParent();

            this.icon.ShowCustomBalloon(this, PopupAnimation.Slide, null);

            System.Media.SystemSounds.Asterisk.Play();
        }

        private void onNotificationMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.icon.CloseBalloon();
        }

        private void onContinueButtonClick(object sender, RoutedEventArgs e)
        {
            this.icon.CloseBalloon();
            Toggl.ContinueLatest(true);
        }

        private void onStartNewButtonClick(object sender, RoutedEventArgs e)
        {
            this.icon.CloseBalloon();
            var guid = Toggl.Start("", "", 0, 0, "", "", true);
            this.mainWindow.ShowOnTop();
            if (guid != null)
                Toggl.Edit(guid, true, Toggl.Description);
        }
    }
}
