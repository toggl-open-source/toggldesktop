
using System;
using System.Windows;
using System.Windows.Media.Effects;

namespace TogglDesktop.WPF
{
    sealed class TogglWindowSample
    {
        public string Title { get { return "Toggl Desktop"; } }
    }

    partial class TogglWindow
    {
        private bool isToolWindow;

        public TogglWindow()
        {
            this.InitializeComponent();
        }

        public bool IsToolWindow
        {
            get { return this.isToolWindow; }
            set
            {
                if (this.IsInitialized)
                {
                    throw new InvalidOperationException("Can not change IsToolWindow after initialisation.");
                }

                if (value == false)
                    return;

                this.turnIntoToolWindow();
            }
        }

        private void turnIntoToolWindow()
        {
            this.Effect = new DropShadowEffect
            {
                BlurRadius = 16,
                ShadowDepth = 4,
                Opacity = 0.3,
                Direction = 270
            };
            this.mainGrid.Margin = new Thickness(10, 4, 10, 16);
            this.ResizeMode = ResizeMode.NoResize;
            this.AllowsTransparency = true;

            this.cogButton.Visibility = Visibility.Collapsed;
            this.minimizeButton.Visibility = Visibility.Collapsed;

            this.isToolWindow = true;
        }

        protected virtual void onCogButtonClick(object sender, RoutedEventArgs e)
        {
        }

        protected virtual void onMinimizeButtonClick(object sender, RoutedEventArgs e)
        {
            this.WindowState = WindowState.Minimized;
        }

        protected virtual void onCloseButtonClick(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

    }
}
