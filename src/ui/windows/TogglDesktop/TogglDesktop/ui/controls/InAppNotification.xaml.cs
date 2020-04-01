using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Animation;

namespace TogglDesktop
{
    public partial class InAppNotification : UserControl
    {
        public InAppNotification()
        {
            InitializeComponent();
        }

        public static readonly DependencyProperty TitleProperty = DependencyProperty.Register(
            "Title", typeof(string), typeof(InAppNotification), new PropertyMetadata(default(string)));

        public string Title
        {
            get { return (string) GetValue(TitleProperty); }
            set { SetValue(TitleProperty, value); }
        }

        public static readonly DependencyProperty TextProperty = DependencyProperty.Register(
            "Text", typeof(string), typeof(InAppNotification), new PropertyMetadata(default(string)));

        public string Text
        {
            get { return (string) GetValue(TextProperty); }
            set { SetValue(TextProperty, value); }
        }

        public static readonly DependencyProperty ButtonProperty = DependencyProperty.Register(
            "Button", typeof(string), typeof(InAppNotification), new PropertyMetadata(default(string)));

        public string Button
        {
            get { return (string) GetValue(ButtonProperty); }
            set { SetValue(ButtonProperty, value); }
        }

        public static readonly DependencyProperty UrlProperty = DependencyProperty.Register(
            "Url", typeof(string), typeof(InAppNotification), new PropertyMetadata(default(string)));

        public string Url
        {
            get { return (string) GetValue(UrlProperty); }
            set { SetValue(UrlProperty, value); }
        }

        public void RunAppearAnimation()
        {
            var sb = new Storyboard();
            var slideAnimation = new DoubleAnimation
            {
                Duration = TimeSpan.FromSeconds(1),
                From = 0,
                To = 300,
                DecelerationRatio = 0.3f
            };
            Storyboard.SetTargetProperty(slideAnimation, new PropertyPath("MaxHeight"));
            sb.Children.Add(slideAnimation);
            this.MaxHeight = 0;
            this.Visibility = Visibility.Visible;
            this.BeginStoryboard(sb);
        }

        private void CloseButton_OnClick(object sender, RoutedEventArgs e)
        {
            CloseWithAnimation();
            Toggl.TrackClickCloseButtonInAppMessage();
        }

        private void ButtonBase_OnClick(object sender, RoutedEventArgs e)
        {
            Toggl.OpenInBrowser(Url);
            CloseWithAnimation();
            Toggl.TrackClickActionButtonInAppMessage();
        }

        private void CloseWithAnimation()
        {
            var sb = new Storyboard();
            var slideAnimation = new DoubleAnimation
            {
                Duration = TimeSpan.FromSeconds(0.3),
                From = this.ActualHeight,
                To = 0,
                DecelerationRatio = 0.3f
            };
            Storyboard.SetTargetProperty(slideAnimation, new PropertyPath("MaxHeight"));
            sb.Children.Add(slideAnimation);
            this.MaxHeight = this.ActualHeight;
            this.BeginStoryboard(sb);
            slideAnimation.Completed += (sender, args) => this.Visibility = Visibility.Collapsed;
        }
    }
}