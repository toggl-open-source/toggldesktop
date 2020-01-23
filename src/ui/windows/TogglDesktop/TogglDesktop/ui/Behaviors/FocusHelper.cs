using System.Windows;
using System.Windows.Input;

namespace TogglDesktop.Behaviors
{
    public static class FocusHelper
    {
        public static bool GetIsFocused(DependencyObject obj)
        {
            return (bool) obj.GetValue(IsFocusedProperty);
        }

        public static void SetIsFocused(DependencyObject obj, bool value)
        {
            obj.SetValue(IsFocusedProperty, value);
        }

        public static readonly DependencyProperty IsFocusedProperty =
            DependencyProperty.RegisterAttached(
                "IsFocused", typeof (bool), typeof (FocusHelper),
                new UIPropertyMetadata(false, OnIsFocusedPropertyChanged));

        private static void OnIsFocusedPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var uie = (UIElement) d;
            if ((bool) e.NewValue)
            {
                uie.Focus();
            }
            // else
            // {
            //     Keyboard.ClearFocus(); // removes the keyboard focus
            //     // the logical focus remains intact
            // }
        }
    }}