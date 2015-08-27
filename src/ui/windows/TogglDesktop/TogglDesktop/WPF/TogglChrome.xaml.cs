using System.Windows;
using System.Windows.Media.Effects;

namespace TogglDesktop.WPF
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

        private void updateToolWindowChrome()
        {
            if (this.isToolWindow)
            {
                this.MainGrid.Margin = new Thickness(10, 4, 10, 16);

                this.CogButton.Visibility = Visibility.Collapsed;
                this.MinimizeButton.Visibility = Visibility.Collapsed;

                this.Effect = new DropShadowEffect
                {
                    BlurRadius = 16,
                    ShadowDepth = 4,
                    Opacity = 0.3,
                    Direction = 270
                };
            }
            else
            {
                this.MainGrid.Margin = new Thickness(0);

                this.CogButton.Visibility = Visibility.Visible;
                this.MinimizeButton.Visibility = Visibility.Visible;

                this.Effect = null;
            }
        }

        public void SetContent(UIElement content)
        {
            this.windowContentGrid.Children.Add(content);
        }
    }
}
