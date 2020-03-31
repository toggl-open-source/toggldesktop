using System;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Controls.Primitives;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public partial class ReminderNotification
    {
        public ReminderNotification(TaskbarIcon icon, MainWindow mainWindow)
            : base(icon, mainWindow)
        {
            InitializeComponent();
            Toggl.OnReminder.ObserveOnDispatcher().Subscribe(onReminder);
        }

        private void onReminder((string title, string informativeText) x)
        {
            var (title, informativeText) = x;
            Title = title;
            Message = informativeText;

            this.RemoveFromParent();

            if (!_icon.ShowNotification(this, PopupAnimation.Slide, TimeSpan.FromSeconds(10)))
            {
                _icon.ShowBalloonTip(title, informativeText, Properties.Resources.toggl, largeIcon: true);
            }
        }

        private void onStartButtonClick(object sender, RoutedEventArgs e)
        {
            Close();
            var guid = Toggl.Start("", "", 0, 0, "", "", true);
            _parentWindow.ShowOnTop();
            if (guid != null)
                Toggl.Edit(guid, true, Toggl.Description);
        }

    }
}