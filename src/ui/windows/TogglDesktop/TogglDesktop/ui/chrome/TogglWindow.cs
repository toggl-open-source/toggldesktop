using System;
using System.Windows;

namespace TogglDesktop
{
    public class TogglWindow : ChromelessWindow
    {
        private bool canClickIcon;

        private TogglChrome chrome;

        public override int WindowHeaderHeight { get { return 40; } }

        #region public properties

        public TogglChrome Chrome
        {
            get { return this.chrome; }
        }

        public bool CanClickIcon
        {
            get { return this.canClickIcon; }
            set
            {
                if (this.IsInitialized)
                {
                    throw new InvalidOperationException("Can not change IsToolWindow after initialisation.");
                }

                this.canClickIcon = value;
            }
        }

        #endregion

        #region public methods

        public override void SetIconState(bool tracking)
        {
            base.SetIconState(tracking);

            this.chrome.SetIconState(tracking);
        }

        #endregion

        #region chrome initialisation

        protected override void OnInitialized(EventArgs e)
        {
            base.OnInitialized(e);

            this.replaceChrome();
        }

        private void replaceChrome()
        {
            this.chrome = new TogglChrome
            {
                DataContext = this,
                IsToolWindow = this.IsToolWindow,
                CanClickIcon = this.CanClickIcon
            };

            this.SetIconState(true);

            this.setupChromeEvents();

            var oldContent = this.Content;
            this.Content = this.chrome;

            this.chrome.SetContent(oldContent as UIElement);
        }

        private void setupChromeEvents()
        {
            this.chrome.IconButton.Click += this.onIconButtonClick;
            this.chrome.CloseButton.Click += this.onCloseButtonClick;
            this.chrome.MinimizeButton.Click += this.onMinimizeButtonClick;
            this.chrome.CogButton.Click += this.onCogButtonClick;
        }

        #endregion

        #region ui events

        protected virtual void onIconButtonClick(object sender, RoutedEventArgs e)
        {
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

        #endregion


    }
}
