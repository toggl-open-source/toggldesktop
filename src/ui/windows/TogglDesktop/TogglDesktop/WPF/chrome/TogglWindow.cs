using System;
using System.Windows;
using System.Windows.Media.Imaging;
using System.Windows.Shell;
using Brushes = System.Windows.Media.Brushes;
using Screen = System.Windows.Forms.Screen;
using Rectangle = System.Drawing.Rectangle;

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
            this.WindowStartupLocation = WindowStartupLocation.CenterOwner;
            this.updateWindowChrome();
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

        #region public methods

        public void SetIconState(bool tracking)
        {
            this.Icon = (BitmapImage)this.chrome.FindResource(tracking ? "IconRed" : "IconGray");
            this.chrome.SetIconState(tracking);
        }

        public void Hide(bool activateOwner = true)
        {
            if (activateOwner)
            {
                var owner = this.Owner;
                if (owner != null)
                {
                    owner.Show();
                    if (owner.WindowState == WindowState.Minimized)
                        owner.WindowState = WindowState.Normal;
                    owner.Topmost = true;
                    owner.Activate();
                }
            }

            base.Hide();
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

            this.SetIconState(true);

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

        protected override void OnLocationChanged(EventArgs e)
        {
            this.updateMaximumSize();

            if (this.WindowState != WindowState.Maximized && this.ResizeMode != ResizeMode.CanResize)
            {
                this.ResizeMode = ResizeMode.CanResize;
            }

            base.OnLocationChanged(e);
        }

        protected override void OnStateChanged(EventArgs e)
        {
            if (this.WindowState == WindowState.Maximized && this.ResizeMode != ResizeMode.NoResize)
            {
                this.WindowState = WindowState.Normal;
                this.ResizeMode = ResizeMode.NoResize;
                this.WindowState = WindowState.Maximized;
            }

            this.updateWindowChrome();

            base.OnStateChanged(e);
        }

        #endregion

        #region ui controlling

        private void updateWindowChrome()
        {
            var chrome = new WindowChrome
            {
                CaptionHeight = 40,
                CornerRadius = new CornerRadius(0),
                GlassFrameThickness = new Thickness(1),
                UseAeroCaptionButtons = false
            };

            if (this.WindowState == WindowState.Maximized)
            {
                chrome.ResizeBorderThickness = new Thickness(0);
            }

            WindowChrome.SetWindowChrome(this, chrome);
        }
        
        private void updateMaximumSize()
        {
            var screen = this.getCurrentScreen();

            this.MaxWidth = screen.WorkingArea.Width;
            this.MaxHeight = screen.WorkingArea.Height;
        }

        protected Screen getCurrentScreen()
        {
            return Screen.FromRectangle(new Rectangle(
                (int)this.Left, (int)this.Top,
                (int)this.Width, (int)this.Height
                ));
        }

        #endregion
    }
}
