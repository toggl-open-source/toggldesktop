using System;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Threading;
using ReactiveUI;
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
            get => DataContext as SSOLoginViewModel;
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
            // this isn't guaranteed to work when running without dispatcher:
            var focusEmailFieldAction = new Action(() =>  _emailTextBox?.Focus() );
            this.Dispatcher.BeginInvoke(DispatcherPriority.SystemIdle, focusEmailFieldAction);
        }

        private const double opacityFadeTime = 0.25;
        private object opacityAnimationToken;
        public void Deactivate(bool allowAnimation)
        {
            ViewModel?.Reset();
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
            return false;
        }

        public Brush TitleBarBrush => this.Background;

        private TextBox _emailTextBox;
        private void HandleEmailTextBoxLoaded(object sender, RoutedEventArgs e)
        {
            _emailTextBox = sender as TextBox;
            if (_emailTextBox != null)
            {
                ViewModel.WhenAnyValue(x => x.HasErrors)
                    .Where(hasError => hasError)
                    .Subscribe(_ => ShowErrorAndFocus(_emailTextBox));
            }
        }

        private void ShowErrorAndFocus(TextBox textBox)
        {
            textBox.GetBindingExpression(TextBox.TextProperty)?.UpdateSource();
            textBox.Focus();
        }
    }
}
