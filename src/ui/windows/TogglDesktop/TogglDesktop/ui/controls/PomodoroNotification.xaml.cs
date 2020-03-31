using System;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Controls.Primitives;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public partial class PomodoroNotification
    {
        public PomodoroNotification(TaskbarIcon icon, MainWindow mainWindow)
            : base(icon, mainWindow)
        {
            this.InitializeComponent();

            Toggl.OnDisplayPomodoro.ObserveOnDispatcher().Subscribe(this.onDisplayPomodoro);
            Toggl.OnDisplayPomodoroBreak.ObserveOnDispatcher().Subscribe(this.onDisplayPomodoro);
        }

        private void onDisplayPomodoro((string title, string informativeText) x)
        {
            var (title, informativeText) = x;

            this.Message = informativeText;
            this.Title = title;

            this.RemoveFromParent();

            if (!_icon.ShowNotification(this, PopupAnimation.Slide, null))
            {
                _icon.ShowBalloonTip(title, informativeText, Properties.Resources.toggl, largeIcon: true);
            }
            else
            {
                System.Media.SystemSounds.Asterisk.Play();
            }
        }

        private void onContinueButtonClick(object sender, RoutedEventArgs e)
        {
            Close();
            Toggl.ContinueLatest(true);
        }

        private void onStartNewButtonClick(object sender, RoutedEventArgs e)
        {
            Close();
            var guid = Toggl.Start("", "", 0, 0, "", "", true);
            _parentWindow.ShowOnTop();
            if (guid != null)
                Toggl.Edit(guid, true, Toggl.Description);
        }
    }
}
