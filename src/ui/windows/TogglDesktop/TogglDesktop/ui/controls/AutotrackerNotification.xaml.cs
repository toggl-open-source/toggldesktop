using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public partial class AutotrackerNotification
    {
        private readonly TaskbarIcon icon;
        private readonly MainWindow mainWindow;

        private ulong projectId;

        public AutotrackerNotification(TaskbarIcon icon, MainWindow mainWindow)
        {
            this.icon = icon;
            this.mainWindow = mainWindow;
            this.InitializeComponent();

            Toggl.OnAutotrackerNotification += this.onAutotrackerNotification;
        }

        private void onAutotrackerNotification(string projectName, ulong projectId)
        {
            if (this.TryBeginInvoke(this.onAutotrackerNotification, projectName, projectId))
                return;

            this.projectText.Text = string.Format("Track {0}?", projectName);
            this.projectId = projectId;

            this.RemoveFromParent();

            this.icon.ShowCustomBalloon(this, PopupAnimation.Slide, 6000);
        }

        private void onStartButtonClick(object sender, RoutedEventArgs e)
        {
            this.icon.CloseBalloon();
            Toggl.Start("", "", 0, this.projectId, null, null);
            this.mainWindow.ShowOnTop();
        }

        private void onNotificationMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.icon.CloseBalloon();
        }
    }
}
