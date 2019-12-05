using System.Windows;
using System.Windows.Controls;

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

        private void CloseButton_OnClick(object sender, RoutedEventArgs e)
        {
            this.Visibility = Visibility.Collapsed;
            Toggl.TrackClickCloseButtonInAppMessage();
        }

        private void ButtonBase_OnClick(object sender, RoutedEventArgs e)
        {
            Toggl.OpenInBrowser(Url);
            this.Visibility = Visibility.Collapsed;
            Toggl.TrackClickActionButtonInAppMessage();
        }
    }
}