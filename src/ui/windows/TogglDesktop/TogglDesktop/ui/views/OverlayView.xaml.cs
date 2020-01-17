using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media.Animation;
using Google.Apis.Auth.OAuth2;
using Google.Apis.Oauth2.v2;
using TogglDesktop.Diagnostics;
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
            TopText.Text = "You no longer have access to your last Workspace. Create a new workspace on Toggl.com to continue tracking";
            confirmButtonText.Text = "Log in to Toggl.com";
            BottomText.Inlines.Clear();
            BottomText.Inlines.Add(new Run("Created your new workspace?"));
            BottomText.Inlines.Add(new Underline(new Run("Force sync")));
        }

        private void setupTos()
        {
            Run run1 = new Run("Our Terms of Service and Privacy Policy have changed\n\n");
            run1.FontWeight = FontWeights.Bold;
            run1.FontSize = 20;

            Run run2 = new Run("Please read and accept our updated ");

            Underline u1 = new Underline(new Run("Terms of Service"));

            Hyperlink hyperlink = new Hyperlink(u1)
            {
                NavigateUri = new Uri("https://toggl.com/legal/terms")
            };
            hyperlink.RequestNavigate += new System.Windows.Navigation.RequestNavigateEventHandler(openTos);

            Underline u2 = new Underline(new Run("Privacy Policy"));

            Hyperlink hyperlink2 = new Hyperlink(u2)
            {
                NavigateUri = new Uri("https://toggl.com/legal/privacy")
            };
            hyperlink2.RequestNavigate += new System.Windows.Navigation.RequestNavigateEventHandler(openPrivacy);


            TopText.Inlines.Clear();
            TopText.Inlines.Add(run1);
            TopText.Inlines.Add(run2);
            TopText.Inlines.Add(hyperlink);
            TopText.Inlines.Add(new Run(" and "));
            TopText.Inlines.Add(hyperlink2);
            TopText.Inlines.Add(new Run(" to continue using Toggl."));

            confirmButtonText.Text = "I accept";

            Run run = new Run("support@toggl.com");
            run.FontWeight = FontWeights.Bold;
            BottomText.Inlines.Clear();
            BottomText.Inlines.Add(new Run("If you have any questions, contact us at "));
            BottomText.Inlines.Add(run);
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

        private void TextBlock_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (this.currentType == 0)
            {
                Toggl.FullSync();
            }
        }

        private void onActionButtonClick(object sender, RoutedEventArgs e)
        {
            if (this.currentType == 0)
            {
                Toggl.OpenInBrowser();
            }
            else if (this.currentType == 1)
            {
                Toggl.AcceptToS();
            }
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

    }
}
