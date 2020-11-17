using System;
using System.Windows;
using System.Windows.Controls;
using TogglDesktop.ViewModels;

namespace TogglDesktop.Behaviors
{
    public static class TimelineTimeEntryPopupHelper
    {
        public static void OpenPopup(this TimelineTimeEntryBlockPopup popup, FrameworkElement placementTarget, ScrollViewer scroll)
        {
            if (placementTarget.DataContext is TimeEntryBlock curBlock)
            {
                popup.DataContext = curBlock;
                popup.Popup.PlacementTarget = placementTarget;
                popup.Popup.IsOpen = true;
                var visibleTopOffset = scroll.VerticalOffset + 10;
                var visibleBottomOffset = scroll.VerticalOffset + scroll.ActualHeight - 10;
                var offset = curBlock.VerticalOffset + placementTarget.ActualHeight / 2;
                popup.Popup.VerticalOffset = Math.Min(Math.Max(visibleTopOffset, offset), visibleBottomOffset) -
                                              curBlock.VerticalOffset;
            }
        }

        public static void ClosePopup(this TimelineTimeEntryBlockPopup popup)
        {
            popup.Popup.IsOpen = false;
        }
    }
}
