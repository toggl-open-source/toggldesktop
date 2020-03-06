using System;
using System.Windows;
using System.Windows.Media.Animation;
using Brush = System.Windows.Media.Brush;

namespace TogglDesktop
{
    /// <summary>
    /// Interaction logic for OverlayView.xaml
    /// </summary>
    public partial class OverlayView : IMainView
    {
        private object opacityAnimationToken;
        private int currentType;

        public OverlayView()
        {
            InitializeComponent();
        }

        public Brush TitleBarBrush => this.Background;
        #region setup
        public void setType(int type)
        {
            this.currentType = type;
            if (this.currentType == 0)
            {
                setupMissingWS();
            }
            else if (this.currentType == 1)
            {
                setupTos();
            }
        }

        private void setupMissingWS()
        {
            tosChangesPanel.Visibility = Visibility.Collapsed;
            missingWorkspacePanel.Visibility = Visibility.Visible;
        }

        private void setupTos()
        {
            tosChangesPanel.Visibility = Visibility.Visible;
            missingWorkspacePanel.Visibility = Visibility.Collapsed;
        }

        #endregion
        #region controlling
        private void openTos(object sender, System.Windows.Navigation.RequestNavigateEventArgs e)
        {
            Toggl.OpenToS();
        }

        private void openPrivacy(object sender, System.Windows.Navigation.RequestNavigateEventArgs e)
        {
            Toggl.OpenPrivacy();
        }

        private void onAcceptTosClick(object sender, RoutedEventArgs e)
        {
            Toggl.AcceptToS();
        }
        #endregion

        #region fade in/out

        private const double opacityFadeTime = 0.25;

        public void Activate(bool allowAnimation)
        {
            this.opacityAnimationToken = null;

            if (allowAnimation)
            {
                var anim = new DoubleAnimation(0, 1, TimeSpan.FromSeconds(opacityFadeTime));
                this.BeginAnimation(OpacityProperty, anim);
            }
            else
            {
                this.BeginAnimation(OpacityProperty, null);
            }

            this.IsEnabled = true;
            this.Visibility = Visibility.Visible;
        }

        public void Deactivate(bool allowAnimation)
        {
            this.opacityAnimationToken = null;

            if (allowAnimation)
            {
                var anim = new DoubleAnimation(0, TimeSpan.FromSeconds(opacityFadeTime));
                this.opacityAnimationToken = anim;
                anim.Completed += (sender, args) =>
                {
                    if (this.opacityAnimationToken == anim)
                    {
                        this.Visibility = Visibility.Collapsed;
                    }
                };

                this.BeginAnimation(OpacityProperty, anim);

            }
            else
            {
                this.BeginAnimation(OpacityProperty, null);
                this.Visibility = Visibility.Collapsed;
            }

            this.IsEnabled = false;
        }

        public bool HandlesError(string errorMessage)
        {
            return false;
        }

        #endregion

        private void CreateWorkspaceOnClick(object sender, System.Windows.Navigation.RequestNavigateEventArgs e)
        {
            Toggl.OpenInBrowser();
        }

        private void ReLoginOnClick(object sender, RoutedEventArgs e)
        {
            Toggl.Logout();
        }

        private void RefreshNowOnClick(object sender, RoutedEventArgs e)
        {
            Toggl.Sync();
        }
    }
}
