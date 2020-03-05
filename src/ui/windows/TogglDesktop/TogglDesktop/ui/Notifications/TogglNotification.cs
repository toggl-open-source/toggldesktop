using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public class TogglNotification : ContentControl
    {
        protected readonly TaskbarIcon _icon;
        protected readonly Window _parentWindow;

        public TogglNotification(TaskbarIcon icon, Window parentWindow)
        {
            _icon = icon;
            _parentWindow = parentWindow;
            this.MouseDown += OnNotificationMouseDown;
        }

        private void OnNotificationMouseDown(object sender, MouseButtonEventArgs args)
        {
            Close();
            _parentWindow.ShowOnTop();
        }

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();
            var closeButton = this.Template.FindName("PART_CloseButton", this) as Button;
            closeButton.Click += OnCloseButtonOnClick;
        }

        public static readonly DependencyProperty TitleProperty =
            DependencyProperty.Register(nameof(Title), typeof(string), typeof(TogglNotification),
                new UIPropertyMetadata(string.Empty));

        public static readonly DependencyProperty MessageProperty =
            DependencyProperty.Register(nameof(Message), typeof(string), typeof(TogglNotification),
                new UIPropertyMetadata(string.Empty));

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

        protected void Close() => _icon.CloseBalloon();

        private void OnCloseButtonOnClick(object sender, RoutedEventArgs args)
        {
            this.Close();
        }
    }
}