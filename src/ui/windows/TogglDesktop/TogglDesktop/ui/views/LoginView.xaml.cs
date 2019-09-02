using System;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media.Animation;
using Google.Apis.Auth.OAuth2;
using Google.Apis.Oauth2.v2;
using TogglDesktop.Diagnostics;
using System.Collections.Generic;
using System.Windows.Navigation;
using Google.Apis.Auth.OAuth2.Responses;
using Google.Apis.Util;

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
        private bool countriesLoaded = false;
        private long selectedCountryID = -1;
        private List<TogglDesktop.Toggl.TogglCountryView> countriesList;

        public LoginView()
        {
            this.InitializeComponent();
            this.confirmSpinnerAnimation = (Storyboard)this.Resources["RotateConfirmSpinner"];

            this.IsVisibleChanged += this.onIsVisibleChanged;
            Toggl.OnDisplayCountries += this.onDisplayCountries;
        }

        private void onDisplayCountries(List<TogglDesktop.Toggl.TogglCountryView> list)
        {
            if (this.TryBeginInvoke(this.onDisplayCountries, list))
                return;

            this.countriesList = list;

            List<ComboItem> items = new List<ComboItem>();
            foreach (TogglDesktop.Toggl.TogglCountryView c in list)
            {
                items.Add(new ComboItem()
                {
                    Name = c.Name,
                    ID = (int)c.ID
                });
            }
            this.countrySelect.ItemsSource = items;
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
            if (!validateGoogleLoginSignup())
            {
                return;
            }

            switch (this.confirmAction)
            {
                case ConfirmAction.LogIn:
                    this.googleLogin();
                    break;
                case ConfirmAction.SignUp:
                    this.googleSignup();
                    break;
                default:
                    throw new ArgumentException(string.Format("Invalid action '{0}' in login form.", this.confirmAction));
            }
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
                    this.googleLoginButtonTextBlock.Text = "LOG IN WITH GOOGLE";
                    this.countryLabel.Visibility = Visibility.Collapsed;
                    this.countrySelect.Visibility = Visibility.Collapsed;
                    this.tosCheckbox.Visibility = Visibility.Collapsed;
                    this.signupLoginToggle.Content = "Sign up for free";
                    break;
                case ConfirmAction.SignUp:
                    this.confirmButtonText.Text = "SIGN UP";
                    this.forgotPasswordButton.Visibility = Visibility.Collapsed;
                    this.googleLoginButtonTextBlock.Text = "SIGN UP WITH GOOGLE";
                    this.countryLabel.Visibility = Visibility.Visible;
                    this.countrySelect.Visibility = Visibility.Visible;
                    this.tosCheckbox.Visibility = Visibility.Visible;
                    this.signupLoginToggle.Content = "Log in";
                    Task.Factory.StartNew(() =>
                    {
                        getCountries();
                    });
                    break;
                default:
                    throw new ArgumentException(string.Format("Invalid action '{0}' in login form.", action));
            }
            this.confirmAction = action;
        }

        private void getCountries()
        {
            Dispatcher.Invoke(() =>
            {
                if (!this.countriesLoaded)
                {
                    Toggl.GetCountries();
                    this.countriesLoaded = true;
                }
            });            
        }

        private void tryConfirm()
        {
            if (!this.validateLoginSignup())
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

        private async void confirm(Func<string, string, long, bool> confirmAction, string actionName)
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

                    success = await Task.Run(() => confirmAction(email, password, selectedCountryID));
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

        private bool validateLoginSignup()
        {
            if (this.emailTextBox.Text == "")
            {
                this.emailTextBox.Focus();
                Toggl.NewError("Please enter valid email address", true);
                return false;
            }
            if (this.passwordBox.Text == "")
            {
                this.passwordBox.Focus();
                Toggl.NewError("A password is required", true);
                return false;
            }

            if (this.confirmAction == ConfirmAction.SignUp
                && !validateMandatorySignupFields())
            {
                return false;
            }

            return true;
        }

        private bool validateMandatorySignupFields()
        {
            if (this.selectedCountryID == -1)
            {
                this.countrySelect.Focus();
                Toggl.NewError("Please select Country before signing up", true);
                return false;
            }
            if (!this.tosCheckbox.IsChecked.Value)
            {
                this.tosCheckbox.Focus();
                Toggl.NewError("You must agree to the terms of service and privacy policy to use Toggl", true);

                return false;
            }

            return true;
        }

        private bool validateGoogleLoginSignup()
        {
            if (this.confirmAction == ConfirmAction.SignUp
                && !validateMandatorySignupFields())
            {
                return false;
            }

            return true;
        }

        private async void googleLogin()
        {
            try
            {
                var credential = await obtainGoogleUserCredentialAsync();
                Toggl.GoogleLogin(credential.Token.AccessToken);
                await credential.RevokeTokenAsync(CancellationToken.None);
            }
            catch (Exception ex)
            {
                if (ex.Message.Contains("access_denied") ||
                    (ex.InnerException != null &&
                     ex.InnerException.Message.Contains("access_denied")))
                {
                    Toggl.NewError("Login process was canceled", true);
                }
                else
                {
                    Toggl.NewError(ex.Message, false);
                }
            }
        }

        private async void googleSignup()
        {
            try
            {
                var credential = await obtainGoogleUserCredentialAsync();
                Toggl.GoogleSignup(credential.Token.AccessToken, selectedCountryID);
                await credential.RevokeTokenAsync(CancellationToken.None);
            }
            catch (Exception ex)
            {
                if (ex.Message.Contains("access_denied") ||
                    (ex.InnerException != null &&
                    ex.InnerException.Message.Contains("access_denied")))
                {
                    Toggl.NewError("Signup process was canceled", true);
                }
                else
                {
                    Toggl.NewError(ex.Message, false);
                }
            }
        }

        private static async Task<UserCredential> obtainGoogleUserCredentialAsync()
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
            var isTokenExpired = credential.Token.IsExpired(SystemClock.Default);
            if (isTokenExpired)
            {
                await credential.RefreshTokenAsync(CancellationToken.None);
            }

            return credential;
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

        private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            System.Diagnostics.Process.Start(e.Uri.ToString());
        }

        private void countrySelect_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
        {
            TogglDesktop.Toggl.TogglCountryView item = this.countriesList[this.countrySelect.SelectedIndex];
            this.selectedCountryID = item.ID;
        }
    }

    class ComboItem
    {
        public string Name { get; set; }
        public int ID { get; set; }
    }
}
