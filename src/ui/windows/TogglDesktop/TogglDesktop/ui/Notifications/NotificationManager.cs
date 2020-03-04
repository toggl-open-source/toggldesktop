using System;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Controls.Primitives;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public class NotificationManager
    {
        private readonly TaskbarIcon _taskbarIcon;
        private readonly Window _parentWindow;

        public NotificationManager(TaskbarIcon taskbarIcon, Window parentWindow)
        {
            _taskbarIcon = taskbarIcon;
            _parentWindow = parentWindow;

            Toggl.OnReminder.ObserveOnDispatcher().Subscribe(OnReminder);
            Toggl.OnAutotrackerNotification.ObserveOnDispatcher().Subscribe(OnAutotrackerNotification);
            Toggl.OnDisplayPomodoro.ObserveOnDispatcher().Subscribe(OnDisplayPomodoro);
            Toggl.OnDisplayPomodoroBreak.ObserveOnDispatcher().Subscribe(OnDisplayPomodoro);
        }

        private void OnReminder((string title, string informativeText) x)
        {
            var (title, informativeText) = x;

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

        private void OnDisplayPomodoro((string title, string informativeText) x)
        {
            var (title, informativeText) = x;

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

        private void OnAutotrackerNotification((string projectName, ulong projectId, ulong taskId) x)
        {
            var (projectName, projectId, taskId) = x;

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