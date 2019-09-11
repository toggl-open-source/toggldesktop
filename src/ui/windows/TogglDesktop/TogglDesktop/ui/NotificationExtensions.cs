using System;
using System.Windows;
using System.Windows.Controls.Primitives;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public static class NotificationExtensions
    {
        public static bool ShowNotification(this TaskbarIcon icon, UIElement element, PopupAnimation animation,
            TimeSpan? timeout = null)
        {
            try
            {
                icon.ShowCustomBalloon(element, animation, (int?)timeout?.TotalMilliseconds);
            }
            catch (Exception e)
            {
                BugsnagService.NotifyBugsnag(new Exception("Failed to show a custom notification", e));
                return false;
            }

            return true;
        }
    }
}