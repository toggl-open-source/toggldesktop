using System.Windows;
using System.Windows.Media.Effects;
using System.Windows.Media.Imaging;

namespace TogglDesktop
{
    sealed class TogglChromeSample
    {
        public string Title { get; private set; }

        public TogglChromeSample()
            : this("Toggl Desktop")
        {
            
        }

        public TogglChromeSample(string title)
        {
            this.Title = title;
        }
    }

    partial class TogglChrome
    {
        private bool isToolWindow;

        public TogglChrome()
        {
            this.InitializeComponent();
        }

        public void SetIconState(bool tracking)
        {
            this.icon.Source = (BitmapImage)this.FindResource(tracking ? "IconRed" : "IconGray");
        }

        public bool IsToolWindow
        {
            get { return this.isToolWindow; }
            set
            {
                if (this.isToolWindow == value)
                    return;
                this.isToolWindow = value;
                this.updateToolWindowChrome();
            }
        }

        public bool CanClickIcon
        {
            get { return this.IconButton.IsEnabled; }
            set { this.IconButton.IsEnabled = value; }
        }

        private void updateToolWindowChrome()
        {
            if (this.isToolWindow)
            {
                this.CogButton.Visibility = Visibility.Collapsed;
                this.MinimizeButton.Visibility = Visibility.Collapsed;
            }
            else
            {
                this.CogButton.Visibility = Visibility.Visible;
                this.MinimizeButton.Visibility = Visibility.Visible;
            }
        }

        public void SetContent(UIElement content)
        {
            this.windowContentGrid.Children.Add(content);
        }

        public void AddToHeaderButtons(UIElement element)
        {
            this.buttonPanel.Children.Insert(0, element);
        }
    }
}
