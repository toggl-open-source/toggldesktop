using System.Windows;
using System.Windows.Controls.Primitives;

namespace TogglDesktop.Behaviors
{
    public static class ButtonBehavior
    {
        public static readonly DependencyProperty IsCloseWindowButtonProperty =
            DependencyProperty.RegisterAttached(
            "IsCloseWindowButton",
            typeof(bool),
            typeof(ButtonBehavior),
            new UIPropertyMetadata(default(bool), OnDefaultButtonPropertyChanged));

        public static void SetIsCloseWindowButton(DependencyObject element, bool value)
        {
            element.SetValue(IsCloseWindowButtonProperty, value);
        }

        public static bool GetIsCloseWindowButton(DependencyObject element)
        {
            return (bool) element.GetValue(IsCloseWindowButtonProperty);
        }

        private static void OnDefaultButtonPropertyChanged(DependencyObject dpo, DependencyPropertyChangedEventArgs args)
        {
            if (dpo is ButtonBase button)
            {
                if ((bool)args.NewValue)
                {
                    button.Click += OnCloseButtonClick;
                }
                else
                {
                    button.Click -= OnCloseButtonClick; ;
                }
            }
        }

        private static void OnCloseButtonClick(object sender, RoutedEventArgs e)
        {
            var button = (ButtonBase)sender;
            var parentWindow = Window.GetWindow(button);
            parentWindow?.Close();
        }
    }
}