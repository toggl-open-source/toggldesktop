using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Shell;

namespace TogglDesktop.WPF
{
    public class TogglWindow : Window
    {
        private bool isToolWindow;

        private TogglChrome chrome;

        public int WindowHeaderHeight { get { return 40; } }

        public TogglWindow()
        {
            this.WindowStyle = WindowStyle.None;
            WindowChrome.SetWindowChrome(this, new WindowChrome
            {
                CaptionHeight = 40,
                CornerRadius = new CornerRadius(0),
                //ResizeBorderThickness = new Thickness(0),
                GlassFrameThickness = new Thickness(1),
                UseAeroCaptionButtons = false
            });
        }

        #region public properties

        public bool IsToolWindow
        {
            get { return this.isToolWindow; }
            set
            {
                if (this.IsInitialized)
                {
                    throw new InvalidOperationException("Can not change IsToolWindow after initialisation.");
                }

                this.isToolWindow = value;
            }
        }

        #endregion

        #region chrome initialisation

        protected override void OnInitialized(EventArgs e)
        {
            base.OnInitialized(e);

            if (this.IsToolWindow)
            {
                this.turnIntoToolWindow();
            }

            this.replaceChrome();
        }

        private void replaceChrome()
        {
            this.chrome = new TogglChrome
            {
                DataContext = this,
                IsToolWindow = this.IsToolWindow
            };

            this.setupChromeEvents();

            var oldContent = this.Content;
            this.Content = this.chrome;

            this.chrome.SetContent(oldContent as UIElement);
        }

        private void turnIntoToolWindow()
        {
            this.ResizeMode = ResizeMode.NoResize;
            this.AllowsTransparency = true;
            this.Background = Brushes.Transparent;

            this.isToolWindow = true;
        }

        private void setupChromeEvents()
        {
            this.chrome.CloseButton.Click += this.onCloseButtonClick;
            this.chrome.MinimizeButton.Click += this.onMinimizeButtonClick;
            this.chrome.CogButton.Click += this.onCogButtonClick;
        }

        #endregion

        #region ui events

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

        #endregion
    }
}
