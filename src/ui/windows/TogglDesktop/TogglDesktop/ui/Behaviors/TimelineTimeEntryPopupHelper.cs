using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using TogglDesktop.ViewModels;

namespace TogglDesktop.Behaviors
{
    public static class TimelineTimeEntryPopupHelper
    {
        public static void OpenPopupWithRightPlacement(this TimelineTimeEntryBlockPopup popup, FrameworkElement placementTarget, ScrollViewer scroll)
        {
            if (placementTarget.DataContext is TimeEntryBlock curBlock)
            {
                var visibleTopOffset = scroll.VerticalOffset + 10;
                var visibleBottomOffset = scroll.VerticalOffset + scroll.ActualHeight - 10;
                var offset = curBlock.VerticalOffset + placementTarget.ActualHeight / 2;
                popup.OpenPopup(PlacementMode.Right, placementTarget, curBlock, 0, Math.Min(Math.Max(visibleTopOffset, offset), visibleBottomOffset) -
                                                                                   curBlock.VerticalOffset);
            }
        }

        public static void OpenPopup(this TimelineTimeEntryBlockPopup popup, PlacementMode mode,
            FrameworkElement placementTarget, object dataContext, double horizontal, double vertical)
        {
            popup.DataContext = dataContext;
            popup.PlacePopup(mode, placementTarget, horizontal, vertical);
            popup.Popup.IsOpen = true;
        }

        public static void PlacePopup(this TimelineTimeEntryBlockPopup popup, PlacementMode mode,
            FrameworkElement placementTarget,
            double horizontal, double vertical)
        {
            popup.Popup.Placement = mode;
            popup.Popup.PlacementTarget = placementTarget;
            popup.Popup.VerticalOffset = vertical;
            popup.Popup.HorizontalOffset = horizontal;
        }

        public static void ClosePopup(this TimelineTimeEntryBlockPopup popup)
        {
            popup.Popup.IsOpen = false;
        }
    }
}
