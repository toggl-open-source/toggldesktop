using System;
using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Interactivity;
using RoutedEventArgs = System.Windows.RoutedEventArgs;

namespace TogglDesktop.Behaviors
{
    public class RepositionPopupWithParentWindowBehavior : Behavior<Popup>
    {
        private Window _parentWindow;
        protected override void OnAttached()
        {
            base.OnAttached();
            AssociatedObject.Loaded += PopupOnLoaded;
        }

        private void PopupOnLoaded(object sender, RoutedEventArgs e)
        {
            AssociatedObject.Loaded -= PopupOnLoaded;
            _parentWindow = Window.GetWindow(AssociatedObject);
            if (_parentWindow != null)
            {
                _parentWindow.LocationChanged += WindowOnLocationChanged;
            }
        }

        private void WindowOnLocationChanged(object sender, EventArgs e)
        {
            // hack to force popup to reposition itself
            var offset = AssociatedObject.HorizontalOffset;
            AssociatedObject.HorizontalOffset = offset + 1;
            AssociatedObject.HorizontalOffset = offset;
        }

        protected override void OnDetaching()
        {
            if (_parentWindow != null)
            {
                _parentWindow.LocationChanged -= WindowOnLocationChanged;
            }
            base.OnDetaching();
        }
    }
}