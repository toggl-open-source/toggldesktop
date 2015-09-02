using System;
using System.Threading;
using System.Windows;
using System.Windows.Input;
using Google.Apis.Auth.OAuth2;
using Google.Apis.Oauth2.v2;

namespace TogglDesktop.WPF
{
    public partial class LoginView
    {
        private enum ConfirmAction
        {
            Unknown = 0,
            LogIn,
            SignUp
        }

        private ConfirmAction confirmAction = ConfirmAction.Unknown;

        public LoginView()
        {
            this.InitializeComponent();
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
                    this.confirmButton.Content = "Log in";
                    this.forgotPasswordButton.Visibility = Visibility.Visible;
                    this.googleLoginButton.Visibility = Visibility.Visible;
                    this.signupLoginToggle.Content = "Sign up for free";
                    break;
                case ConfirmAction.SignUp:
                    this.confirmButton.Content = "Sign up";
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
            switch (this.confirmAction)
            {
                case ConfirmAction.LogIn:
                    this.login();
                    break;
                case ConfirmAction.SignUp:
                    this.signup();
                    break;
                default:
                    throw new ArgumentException(string.Format("Invalid action '{0}' in login form.", this.confirmAction));
            }
        }

        private void login()
        {
            if (!this.validateFields())
            {
                return;
            }
            Toggl.Login(this.emailTextBox.Text, this.passwordBox.Text);
            this.passwordBox.Clear();
        }

        private void signup()
        {
            if (!this.validateFields())
            {
                return;
            }
            if (this.passwordBox.Text == "")
            {
                this.passwordBox.Focus();
                return;
            }
            Toggl.Signup(this.emailTextBox.Text, this.passwordBox.Text);
            this.passwordBox.Clear();
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
                    new[] {
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

        #endregion

    }
}
