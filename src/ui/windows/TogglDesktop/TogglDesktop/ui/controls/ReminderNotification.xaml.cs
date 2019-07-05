using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public partial class ReminderNotification
    {
        private readonly TaskbarIcon icon;
        private readonly MainWindow mainWindow;

        public ReminderNotification(TaskbarIcon icon, MainWindow mainWindow)
        {
            this.icon = icon;
            this.mainWindow = mainWindow;
            InitializeComponent();
            Toggl.OnReminder += onReminder;
        }

        private void onReminder(string title, string informative_text)
        {
            if (this.TryBeginInvoke(onReminder, title, informative_text))
                return;

            Title = title;
            Message = informative_text;

            this.RemoveFromParent();

            icon.ShowCustomBalloon(this, PopupAnimation.Slide, 6000);
        }

        private void onNotificationMouseDown(object sender, MouseButtonEventArgs e)
        {
            icon.CloseBalloon();
            mainWindow.ShowOnTop();
        }

        private void onStartButtonClick(object sender, RoutedEventArgs e)
        {
            this.icon.CloseBalloon();
            var guid = Toggl.Start("", "", 0, 0, "", "", true);
            this.mainWindow.ShowOnTop();
            if (guid != null)
                Toggl.Edit(guid, true, Toggl.Description);
        }

    }
}