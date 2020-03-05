using System;
using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public partial class ReminderNotification
    {
        public ReminderNotification(TaskbarIcon icon, MainWindow mainWindow)
            : base(icon, mainWindow)
        {
            InitializeComponent();
            Toggl.OnReminder += onReminder;
        }

        private void onReminder(string title, string informative_text)
        {
            if (this.TryBeginInvoke(onReminder, title, informative_text))
                return;

            Title = title;
            Message = informative_text;

            if (!_icon.ShowNotification(this, PopupAnimation.Slide, TimeSpan.FromSeconds(10)))
            {
                _icon.ShowBalloonTip(title, informative_text, Properties.Resources.toggl, largeIcon: true);
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