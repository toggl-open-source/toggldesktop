using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media.Animation;
using Google.Apis.Auth.OAuth2;
using Google.Apis.Oauth2.v2;
using TogglDesktop.Diagnostics;

namespace TogglDesktop
{
    /// <summary>
    /// Interaction logic for MissingWSView.xaml
    /// </summary>
    public partial class MissingWSView : IMainView
    {
        private object opacityAnimationToken;

        public MissingWSView()
        {
            InitializeComponent();
        }
        #region controlling
        private void TextBlock_MouseDown(object sender, MouseButtonEventArgs e)
        {
            Toggl.FullSync();
        }

        private void onLoginButtonClick(object sender, RoutedEventArgs e)
        {
            Toggl.OpenInBrowser();
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


        #endregion

    }
}
