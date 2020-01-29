using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public partial class PomodoroNotification
    {
        public PomodoroNotification(TaskbarIcon icon, MainWindow mainWindow)
            : base(icon, mainWindow)
        {
            this.InitializeComponent();

            Toggl.OnDisplayPomodoro += this.onDisplayPomodoro;
            Toggl.OnDisplayPomodoroBreak += this.onDisplayPomodoroBreak;
        }

        private void onDisplayPomodoro(string title, string informativetext)
        {
            if (this.TryBeginInvoke(onDisplayPomodoro, title, informativetext))
                return;

            this.Message = informativetext;
            this.Title = title;

            this.RemoveFromParent();

            if (!_icon.ShowNotification(this, PopupAnimation.Slide, null))
            {
                _icon.ShowBalloonTip(title, informativetext, Properties.Resources.toggl, largeIcon: true);
            }
            else
            {
                System.Media.SystemSounds.Asterisk.Play();
            }
        }

        private void onDisplayPomodoroBreak(string title, string informativetext)
        {
            if (this.TryBeginInvoke(onDisplayPomodoroBreak, title, informativetext))
                return;

            this.Message = informativetext;
            this.Title = title;

            this.RemoveFromParent();

            if (!_icon.ShowNotification(this, PopupAnimation.Slide, null))
            {
                _icon.ShowBalloonTip(title, informativetext, Properties.Resources.toggl, largeIcon: true);
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
