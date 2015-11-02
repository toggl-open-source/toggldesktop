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
    public partial class LoginView : IMainView
    {
        private enum ConfirmAction
        {
            Unknown = 0,
            LogIn,
            SignUp
        }

        private readonly Storyboard confirmSpinnerAnimation;

        private Action onLogin;
        private object opacityAnimationToken;

        private ConfirmAction confirmAction = ConfirmAction.Unknown;
        private bool loggingIn;

        public LoginView()
        {
            this.InitializeComponent();
            this.confirmSpinnerAnimation = (Storyboard)this.Resources["RotateConfirmSpinner"];

            this.IsVisibleChanged += this.onIsVisibleChanged;
        }

        private void onIsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if (this.IsVisible)
                this.setConfirmAction(ConfirmAction.LogIn);
        }

        #region events

        protected override void OnKeyUp(KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                this.tryConfirm();
                e.Handled = true;
            }
        }

        private void onConfirmButtonClick(object sender, RoutedEventArgs e)
        {
            this.tryConfirm();
        }


        private void onSignupLoginToggleClick(object sender, RoutedEventArgs e)
        {
            switch (this.confirmAction)
            {
                case ConfirmAction.LogIn:
                    this.setConfirmAction(ConfirmAction.SignUp);
                    break;
                case ConfirmAction.SignUp:
                    this.setConfirmAction(ConfirmAction.LogIn);
                    break;
                default:
                    throw new ArgumentException(string.Format("Invalid action '{0}' in login form.", this.confirmAction));
            }
        }

        private void onGoogleLoginClick(object sender, RoutedEventArgs e)
        {
            this.googleLogin();
        }

        private void onForgotPasswordButtonClick(object sender, RoutedEventArgs e)
        {
            Toggl.PasswordForgot();
        }

        #endregion

        #region controlling

        private void setConfirmAction(ConfirmAction action)
        {
            switch (action)
            {
                case ConfirmAction.LogIn:
                    this.confirmButtonText.Text = "LOG IN";
                    this.forgotPasswordButton.Visibility = Visibility.Visible;
                    this.googleLoginButton.Visibility = Visibility.Visible;
                    this.signupLoginToggle.Content = "Sign up for free";
                    break;
                case ConfirmAction.SignUp:
                    this.confirmButtonText.Text = "SIGN UP";
                    this.forgotPasswordButton.Visibility = Visibility.Hidden;
                    this.googleLoginButton.Visibility = Visibility.Hidden;
                    this.signupLoginToggle.Content = "Log in";
                    break;
                default:
                    throw new ArgumentException(string.Format("Invalid action '{0}' in login form.", action));
            }
            this.confirmAction = action;
        }

        private void tryConfirm()
        {
            if (!this.validateFields())
            {
                return;
            }
            switch (this.confirmAction)
            {
                case ConfirmAction.LogIn:
                    this.confirm(Toggl.Login, "log in");
                    break;
                case ConfirmAction.SignUp:
                    this.confirm(Toggl.Signup, "sign up");
                    break;
                default:
                    throw new ArgumentException(string.Format("Invalid action '{0}' in login form.", this.confirmAction));
            }
        }

        private async void confirm(Func<string, string, bool> confirmAction, string actionName)
        {
            using (Performance.Measure("attempting " + actionName))
            {
                var success = false;
                try
                {
                    this.loggingIn = true;
                    this.disableForm();

                    var email = this.emailTextBox.Text;
                    var password = this.passwordBox.Text;

                    success = await Task.Run(() => confirmAction(email, password));
                }
                finally
                {
                    this.loggingIn = false;
                    if (success && this.onLogin != null)
                    {
                        var action = this.onLogin;
                        this.onLogin = null;
                        action();
                    }
                    else
                    {
                        this.reset();
                    }
                }
            }
        }

        private void enableForm()
        {
            this.formPanel.IsEnabled = true;
            this.formPanel.Opacity = 1;
            this.confirmButtonText.Visibility = Visibility.Visible;
            this.confirmButtonSpinner.Visibility = Visibility.Collapsed;
            this.confirmSpinnerAnimation.Stop();
        }

        private void disableForm()
        {
            this.formPanel.IsEnabled = false;
            this.formPanel.Opacity = 0.5;
            this.confirmButtonText.Visibility = Visibility.Collapsed;
            this.confirmButtonSpinner.Visibility = Visibility.Visible;
            this.confirmSpinnerAnimation.Begin();
        }

        private bool validateFields()
        {
            if (this.emailTextBox.Text == "")
            {
                this.emailTextBox.Focus();
                return false;
            }
            if (this.passwordBox.Text == "")
            {
                this.passwordBox.Focus();
                return false;
            }
            return true;
        }

        private async void googleLogin()
        {
            try
            {
                var credential = await GoogleWebAuthorizationBroker.AuthorizeAsync(
                    new ClientSecrets
                    {
                        ClientId = "426090949585-uj7lka2mtanjgd7j9i6c4ik091rcv6n5.apps.googleusercontent.com",
                        ClientSecret = "6IHWKIfTAMF7cPJsBvoGxYui"
                    },
                    new[]
                    {
                        Oauth2Service.Scope.UserinfoEmail,
                        Oauth2Service.Scope.UserinfoProfile
                    },
                    "user",
                    CancellationToken.None);
                Toggl.GoogleLogin(credential.Token.AccessToken);
                await credential.RevokeTokenAsync(CancellationToken.None);
            }
            catch (AggregateException ex)
            {
                if (ex.InnerException != null &&
                    ex.InnerException.Message.Contains("access_denied"))
                {
                    Toggl.NewError("Login process was canceled", true);
                }
                else
                {
                    Toggl.NewError(ex.Message, false);
                }
            }
        }

        private void reset()
        {
            this.enableForm();
            this.passwordBox.Clear();
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

            this.reset();
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
                        this.reset();
                    }
                };
                if (this.loggingIn)
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


        #endregion
    }
}
