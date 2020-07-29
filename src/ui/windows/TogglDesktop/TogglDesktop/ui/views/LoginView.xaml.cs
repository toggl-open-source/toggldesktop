using System;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Navigation;
using System.Windows.Threading;
using MahApps.Metro.Behaviors;
using ReactiveUI;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public partial class LoginView : IMainView, IViewFor<LoginViewModel>
    {
        public LoginViewModel ViewModel
        {
            get => DataContext as LoginViewModel;
            set => DataContext = value;
        }

        object IViewFor.ViewModel
        {
            get => ViewModel;
            set => ViewModel = (LoginViewModel) value;
        }

        private readonly Storyboard confirmSpinnerAnimation;
        private Action onLogin;
        private object opacityAnimationToken;
        private bool isLoggingIn = false;

        private CompositeDisposable _disposable = new CompositeDisposable();

        public LoginView()
        {
            this.InitializeComponent();
            this.confirmSpinnerAnimation = (Storyboard)this.Resources["RotateConfirmSpinner"];
            DataContextChanged += (s, args) => this.Reset();
        }

        private void onSignupLoginToggleClick(object sender, RoutedEventArgs e)
        {
            if (ViewModel.SelectedConfirmAction == ConfirmAction.LogIn)
            {
                ViewModel.SelectedConfirmAction = ConfirmAction.SignUp;
            }
            else if (ViewModel.SelectedConfirmAction == ConfirmAction.SignUp)
            {
                ViewModel.SelectedConfirmAction = ConfirmAction.LogIn;
            }
            else if (ViewModel.SelectedConfirmAction == ConfirmAction.LogInAndLinkSSO)
            {
                ViewModel.LoginWithSSO.Execute().Subscribe();
            }
        }

        private void onForgotPasswordLinkClick(object sender, RoutedEventArgs e)
        {
            Toggl.PasswordForgot();
        }

        private void Reset()
        {
            if (ViewModel == null) return;

            _disposable?.Dispose();
            _disposable = new CompositeDisposable();
            ViewModel.IsLoginSignupExecuting.Subscribe(isExecuting =>
            {
                if (isExecuting)
                {
                    confirmSpinnerAnimation.Begin();
                }
                else
                {
                    confirmSpinnerAnimation.Stop();
                }
            });
            ViewModel.ConfirmLoginSignupCommand.IsExecuting.Subscribe(isExecuting => { isLoggingIn = isExecuting; });
            ViewModel.ConfirmLoginSignupCommand.Subscribe(isLoggedIn =>
            {
                if (isLoggedIn && this.onLogin != null)
                {
                    var action = this.onLogin;
                    this.onLogin = null;
                    action();
                }
            }).DisposeWith(_disposable);
            this.WhenAnyValue(x => x.passwordBox.IsKeyboardFocused)
                .BindTo(this, x => x.ViewModel.IsPasswordFocused)
                .DisposeWith(_disposable);
            ViewModel.FocusEmail.Subscribe(_ => this.emailTextBox.Focus()).DisposeWith(_disposable);
            ViewModel.FocusPassword.Subscribe(_ => this.passwordBox.Focus()).DisposeWith(_disposable);
            ViewModel.FocusCountrySelection.Subscribe(_ => this.countryComboBox.Focus()).DisposeWith(_disposable);
            ViewModel.FocusTosCheckbox.Subscribe(_ => this.tosCheckBox.Focus()).DisposeWith(_disposable);
        }

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

            // this isn't guaranteed to work when running without dispatcher:
            var focusEmailFieldAction = new Action(() => { this.emailTextBox.Focus(); });
            this.Dispatcher.BeginInvoke(DispatcherPriority.SystemIdle, focusEmailFieldAction);
        }

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
                        this.Reset();
                    }
                };
                if (this.isLoggingIn)
                {
                    this.onLogin = () => this.BeginAnimation(OpacityProperty, anim);
                }
                else
                {
                    this.BeginAnimation(OpacityProperty, anim);
                }
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
            if (errorMessage == "Invalid e-mail or password!" && ViewModel.SelectedConfirmAction == ConfirmAction.LogIn)
            {
                ViewModel.ShowLoginError = true;
                return true;
            }

            return false;
        }

        public Brush TitleBarBrush => this.Background;

        #endregion

        private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            Toggl.OpenInBrowser(e.Uri.ToString());
        }

        private void RefreshLoginBindings()
        {
            RefreshEmailTextBoxBinding();
            RefreshPasswordBoxBinding();
        }

        private void RefreshSignupBindings()
        {
            RefreshCountryComboBoxBinding();
            RefreshTosCheckBoxBinding();
        }

        private void RefreshEmailTextBoxBinding() => emailTextBox.GetBindingExpression(TextBox.TextProperty).UpdateSource();
        private void RefreshPasswordBoxBinding() => passwordBox.GetBindingExpression(PasswordBoxBindingBehavior.PasswordProperty).UpdateSource();

        private void RefreshCountryComboBoxBinding() => countryComboBox.GetBindingExpression(ComboBox.SelectedItemProperty).UpdateSource();
        private void RefreshTosCheckBoxBinding() => tosCheckBox.GetBindingExpression(CheckBox.IsCheckedProperty).UpdateSource();
    }
}
