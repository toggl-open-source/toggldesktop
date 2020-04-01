using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using Hardcodet.Wpf.TaskbarNotification;

namespace TogglDesktop
{
    public class TogglNotification : ContentControl
    {
        protected readonly Action Close;
        protected readonly Action ShowParentWindow;

        public TogglNotification(Action close, Action showParentWindow)
        {
            Close = close;
            ShowParentWindow = showParentWindow;
            this.MouseDown += OnMouseButtonEventHandler;
            void OnMouseButtonEventHandler(object sender, MouseButtonEventArgs args)
            {
                this.MouseDown -= OnMouseButtonEventHandler;
                close();
                showParentWindow();
            }
        }

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();
            var closeButton = this.Template.FindName("PART_CloseButton", this) as Button;
            closeButton.Click += OnCloseButton;
            void OnCloseButton(object sender, RoutedEventArgs args)
            {
                closeButton.Click -= OnCloseButton;
                Close();
            }
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
    }
}