using System.Windows;
using System.Windows.Controls;

namespace TogglDesktop
{
    public class TogglNotification : Control
    {
        public static readonly DependencyProperty ContentProperty =
            DependencyProperty.Register(nameof(Content), typeof(object), typeof(TogglNotification),
                new PropertyMetadata(null));

        public static readonly DependencyProperty TitleProperty =
            DependencyProperty.Register(nameof(Title), typeof(string), typeof(TogglNotification),
                new UIPropertyMetadata(string.Empty));

        public static readonly DependencyProperty MessageProperty =
            DependencyProperty.Register(nameof(Message), typeof(string), typeof(TogglNotification),
                new UIPropertyMetadata(string.Empty));

        public object Content
        {
            get => GetValue(ContentProperty);
            set => SetValue(ContentProperty, value);
        }

        public string Title
        {
            get => (string) GetValue(TitleProperty);
            set => SetValue(TitleProperty, value);
        }

        public string Message
        {
            get => (string) GetValue(MessageProperty);
            set => SetValue(MessageProperty, value);
        }
    }
}