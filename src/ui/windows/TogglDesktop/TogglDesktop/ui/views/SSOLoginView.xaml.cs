using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Forms.VisualStyles;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    /// <summary>
    /// Interaction logic for SSOLoginView.xaml
    /// </summary>
    public partial class SSOLoginView : IMainView
    {
        public SSOLoginViewModel ViewModel
        {
            get => (SSOLoginViewModel) DataContext;
            set => DataContext = value;
        }

        public SSOLoginView()
        {
            InitializeComponent();
            
        }

        public void Activate(bool allowAnimation)
        {
            if (allowAnimation)
            {
                var anim = new DoubleAnimation(0, 1, TimeSpan.FromSeconds(opacityFadeTime));
                this.BeginAnimation(OpacityProperty, anim);
            }
            else
            {
                this.BeginAnimation(OpacityProperty, null);
            }

            IsEnabled = true;
            Visibility = Visibility.Visible;
        }

        private const double opacityFadeTime = 0.25;
        private object opacityAnimationToken;
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

        public bool TryShowErrorInsideView(string errorMessage)
        {
            //throw new NotImplementedException();
            return false;
        }

        public Brush TitleBarBrush => this.Background;
    }
}
