using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace TogglDesktop.Behaviors
{
    public static class TextBoxHelper
    {
        public static readonly DependencyProperty IsPaddingClickableProperty =
            DependencyProperty.RegisterAttached(
                "IsPaddingClickable", typeof (bool), typeof (TextBoxHelper),
                new UIPropertyMetadata(default(bool), OnIsPaddingClickablePropertyChanged));

        public static bool GetIsPaddingClickable(DependencyObject obj)
        {
            return (bool) obj.GetValue(IsPaddingClickableProperty);
        }

        public static void SetIsPaddingClickable(DependencyObject obj, bool value)
        {
            obj.SetValue(IsPaddingClickableProperty, value);
        }

        private static void OnIsPaddingClickablePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs args)
        {
            if (d is TextBox textBox)
            {
                if ((bool)args.NewValue)
                {
                    textBox.PreviewMouseLeftButtonDown += OnPreviewMouseLeftButtonDown;
                }
                else
                {
                    textBox.PreviewMouseLeftButtonDown -= OnPreviewMouseLeftButtonDown;
                }
            }
        }

        private static void OnPreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (!(sender is TextBox textBox))
            {
                return;
            }

            const int TextBoxInternalMargin = 6; // Margin property of PART_Message of MetroTextBox from MahApps.Metro

            var point = e.GetPosition(textBox);
            if (textBox.ActualWidth - point.X < textBox.Padding.Right + TextBoxInternalMargin)
            {
                textBox.CaretIndex = textBox.Text.Length;
            }
            else if (point.X < textBox.Padding.Left + TextBoxInternalMargin)
            {
                textBox.CaretIndex = 0;
            }
        }
    }
}