using System;
using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Threading;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public class NotificationManager
    {
        private readonly TaskbarIcon _taskbarIcon;
        private readonly Window _parentWindow;
        private readonly DispatcherTimer _timer;
        private static readonly Random _random = new Random();

        public NotificationManager(TaskbarIcon taskbarIcon, Window parentWindow)
        {
            _taskbarIcon = taskbarIcon;
            _parentWindow = parentWindow;

            Toggl.OnReminder += OnReminder;
            Toggl.OnAutotrackerNotification += OnAutotrackerNotification;
            Toggl.OnDisplayPomodoro += OnDisplayPomodoro;
            Toggl.OnDisplayPomodoroBreak += OnDisplayPomodoro;

            _timer = new DispatcherTimer(TimeSpan.FromSeconds(0.5), DispatcherPriority.Background, OnTimerTick, _parentWindow.Dispatcher);

        }

        private void OnTimerTick(object sender, EventArgs args)
        {
            var randomNumber = _random.Next(4);
            switch (randomNumber)
            {
                case 0:
                    OnReminder("reminder title", "reminder text");
                    break;
                case 1:
                    OnAutotrackerNotification("(no project)", 0, 0);
                    break;
                case 2:
                    OnDisplayPomodoro("pomodoro title", "pomodoro text");
                    break;
                case 3:
                    OnDisplayPomodoro("pomodoro break title", "pomodoro break text");
                    break;
            }
        }

        private void OnReminder(string title, string informativeText)
        {
            if (_parentWindow.TryBeginInvoke(OnReminder, title, informativeText))
                return;

            void StartButtonClick()
            {
                _taskbarIcon.CloseBalloon();
                var guid = Toggl.Start("", "", 0, 0, "", "", true);
                _parentWindow.ShowOnTop();
                if (guid != null)
                    Toggl.Edit(guid, true, Toggl.Description);
            }

            var reminder =
                new ReminderNotification(_taskbarIcon.CloseBalloon, _parentWindow.ShowOnTop, StartButtonClick)
                {
                    Title = title, Message = informativeText
                };

            if (!_taskbarIcon.ShowNotification(reminder, PopupAnimation.Slide, TimeSpan.FromSeconds(10)))
            {
                _taskbarIcon.ShowBalloonTip(title, informativeText, Properties.Resources.toggl, largeIcon: true);
            }
        }

        private void OnDisplayPomodoro(string title, string informativeText)
        {
            if (_parentWindow.TryBeginInvoke(OnDisplayPomodoro, title, informativeText))
                return;

            void StartNewButtonClick()
            {
                _taskbarIcon.CloseBalloon();
                var guid = Toggl.Start("", "", 0, 0, "", "", true);
                _parentWindow.ShowOnTop();
                if (guid != null)
                    Toggl.Edit(guid, true, Toggl.Description);
            }

            void ContinueLatestButtonClick()
            {
                _taskbarIcon.CloseBalloon();
                Toggl.ContinueLatest(true);
            }

            var pomodoroNotification = new PomodoroNotification(
                    _taskbarIcon.CloseBalloon, _parentWindow.ShowOnTop, StartNewButtonClick, ContinueLatestButtonClick)
                {
                    Message = informativeText,
                    Title = title
                };

            if (!_taskbarIcon.ShowNotification(pomodoroNotification, PopupAnimation.Slide, null))
            {
                _taskbarIcon.ShowBalloonTip(title, informativeText, Properties.Resources.toggl, largeIcon: true);
            }
            else
            {
                System.Media.SystemSounds.Asterisk.Play();
            }
        }

        private void OnAutotrackerNotification(string projectName, ulong projectId, ulong taskId)
        {
            if (_parentWindow.TryBeginInvoke(OnAutotrackerNotification, projectName, projectId, taskId))
                return;

            void StartButtonClick()
            {
                _taskbarIcon.CloseBalloon();
                Toggl.Start("", "", taskId, projectId, null, null);
                _parentWindow.ShowOnTop();
            }

            var autotrackerNotification =
                new AutotrackerNotification(_taskbarIcon.CloseBalloon, _parentWindow.ShowOnTop, StartButtonClick)
                {
                    Message = @$"Start tracking ""{projectName}""?"
                };

            _taskbarIcon.ShowNotification(autotrackerNotification, PopupAnimation.Slide, TimeSpan.FromSeconds(6));
        }
    }
}