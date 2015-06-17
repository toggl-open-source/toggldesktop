using System.Windows;

namespace TogglDesktop.WPF
{
    class PropertyHelpers
    {
        public static readonly DependencyProperty MarginRightProperty = DependencyProperty.RegisterAttached(
            "MarginRight",
            typeof(string),
            typeof(PropertyHelpers),
            new UIPropertyMetadata(onMarginRightPropertyChanged));

        public static string GetMarginRight(FrameworkElement element)
        {
            return (string)element.GetValue(MarginRightProperty);
        }

        public static void SetMarginRight(FrameworkElement element, string value)
        {
            element.SetValue(MarginRightProperty, value);
        }

        private static void onMarginRightPropertyChanged(DependencyObject obj, DependencyPropertyChangedEventArgs args)
        {
            var element = obj as FrameworkElement;

            if (element == null)
                return;

            int value;
            if (!int.TryParse((string)args.NewValue, out value))
                return;

            var margin = element.Margin;
            margin.Right = value;
            element.Margin = margin;
        }
    }
}
