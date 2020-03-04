using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Reactive;
using System.Reactive.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Threading;
using Google.Apis.Auth.OAuth2;
using Google.Apis.Auth.OAuth2.Flows;
using Google.Apis.Http;
using Google.Apis.Oauth2.v2;
using Google.Apis.Util;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using ReactiveUI.Validation.Extensions;
using ReactiveUI.Validation.Helpers;

namespace TogglDesktop.ViewModels
{
    public class LoginViewModel : ReactiveValidationObject<LoginViewModel>
    {
        private readonly Action _refreshLoginBindings;
        private readonly Action _refreshSignupBindings;
        private ValidationHelper _emailValidation;
        private ValidationHelper _passwordValidation;
        private ValidationHelper _selectedCountryValidation;
        private ValidationHelper _isTosCheckedValidation;
        private HttpClientFactory _httpClientFactory;

        public LoginViewModel(Action refreshLoginBindings, Action refreshSignupBindings)
            : base(RxApp.TaskpoolScheduler)
        {
            _refreshLoginBindings = refreshLoginBindings;
            _refreshSignupBindings = refreshSignupBindings;
            Toggl.OnDisplayCountries
                .ObserveOnDispatcher()
                .ToPropertyEx(this, x => x.Countries);
            Toggl.OnSettings.Subscribe(x => _httpClientFactory = HttpClientFactoryFromProxySettings(x.settings));
            this.WhenAnyValue(x => x.SelectedConfirmAction,
                    x => x == ConfirmAction.LogIn ? "Log in" : "Sign up")
                .ToPropertyEx(this, x => x.ConfirmButtonText);
            this.WhenAnyValue(x => x.SelectedConfirmAction,
                    x => x == ConfirmAction.LogIn ? "Log in with Google" : "Sign up with Google")
                .ToPropertyEx(this, x => x.GoogleLoginButtonText);
            this.WhenAnyValue(x => x.SelectedConfirmAction,
                    x => x == ConfirmAction.LogIn ? "Sign up for free" : "Back to Log in")
                .ToPropertyEx(this, x => x.SignupLoginToggleText);
            this.ObservableForProperty(x => x.SelectedConfirmAction)
                .Where(x => x.Value == ConfirmAction.SignUp)
                .Take(1)
                .ObserveOn(RxApp.TaskpoolScheduler)
                .Subscribe(_ => Toggl.GetCountries());
            ConfirmLoginSignupCommand = ReactiveCommand.CreateFromTask(ConfirmLoginSignupAsync);
            ConfirmGoogleLoginSignupCommand = ReactiveCommand.Create(ConfirmGoogleLoginSignup);
            IsLoginSignupExecuting = ConfirmLoginSignupCommand.IsExecuting
                .CombineLatest(ConfirmGoogleLoginSignupCommand.IsExecuting,
                    (isExecuting1, isExecuting2) => isExecuting1 || isExecuting2);
            IsLoginSignupExecuting
                .ToPropertyEx(this, x => x.IsLoading);
            IsLoginSignupExecuting
                .Select(x => !x)
                .ToPropertyEx(this, x => x.IsViewEnabled);
        }
        public ReactiveCommand<Unit, bool> ConfirmLoginSignupCommand { get; }
        public ReactiveCommand<Unit, Unit> ConfirmGoogleLoginSignupCommand { get; }
        public IObservable<bool> IsLoginSignupExecuting { get; }

        public IList<Toggl.TogglCountryView> Countries { [ObservableAsProperty] get; }

        [Reactive]
        public Toggl.TogglCountryView SelectedCountry { get; set; }

        [Reactive]
        public ConfirmAction SelectedConfirmAction { get; set; }

        [Reactive]
        public string Email { get; set; }

        [Reactive]
        public string Password { get; set; }

        [Reactive]
        public bool IsEmailFocused { get; set; }

        [Reactive]
        public bool IsPasswordFocused { get; set; }

        [Reactive]
        public bool IsCountrySelectionFocused { get; set; }

        [Reactive]
        public bool IsTosCheckboxFocused { get; set; }

        [Reactive]
        public bool IsTosChecked { get; set; }

        [Reactive]
        public bool ShowLoginError { get; private set; }

        public string ConfirmButtonText { [ObservableAsProperty] get; }
        public string GoogleLoginButtonText { [ObservableAsProperty] get; }
        public string SignupLoginToggleText { [ObservableAsProperty] get; }
        public bool IsLoading { [ObservableAsProperty] get; }
        public bool IsViewEnabled { [ObservableAsProperty] get; }

        private void EnsureValidationApplied()
        {
            if (_emailValidation == null)
            {
                _emailValidation = this.ValidationRule(
                    x => x.Email,
                    email => email.IsValidEmailAddress(),
                    "Please enter a valid email");
                _passwordValidation = this.ValidationRule(
                    x => x.Password,
                    password => !string.IsNullOrEmpty(password),
                    "A password is required");
                _selectedCountryValidation = this.ValidationRule(
                    x => x.SelectedCountry,
                    selectedCountry => selectedCountry.ID > 0,
                    "Please select country");
                _isTosCheckedValidation = this.ValidationRule(
                    x => x.IsTosChecked,
                    isTosChecked => isTosChecked,
                    "Please accept the terms");
            }
        }

        private bool PerformValidation(bool isGoogleLogin = false)
        {
            ShowLoginError = false;
            EnsureValidationApplied();
            IsEmailFocused = false;
            IsPasswordFocused = false;
            IsCountrySelectionFocused = false;
            IsTosCheckboxFocused = false;

            if (!isGoogleLogin && !_emailValidation.IsValid)
            {
                IsEmailFocused = true;
            }
            else if (!isGoogleLogin && !_passwordValidation.IsValid)
            {
                IsPasswordFocused = true;
            }
            else if (SelectedConfirmAction == ConfirmAction.SignUp && !_selectedCountryValidation.IsValid)
            {
                IsCountrySelectionFocused = true;
            }
            else if (SelectedConfirmAction == ConfirmAction.SignUp && !_isTosCheckedValidation.IsValid)
            {
                IsTosCheckboxFocused = true;
            }
            else
            {
                return true;
            }

            if (!isGoogleLogin)
            {
                _refreshLoginBindings();
            }

            if (SelectedConfirmAction == ConfirmAction.SignUp)
            {
                _refreshSignupBindings();
            }

            return false;
        }

        private async Task<bool> ConfirmLoginSignupAsync()
        {
            if (!PerformValidation())
            {
                return false;
            }

            var success = false;
            switch (SelectedConfirmAction)
            {
                case ConfirmAction.LogIn:
                    success = await ConfirmAsync(Toggl.Login);
                    break;
                case ConfirmAction.SignUp:
                    success = await ConfirmAsync(Toggl.Signup);
                    break;
                default:
                    throw new ArgumentOutOfRangeException();
            }

            ShowLoginError = !success;
            return success;
        }

        private async void ConfirmGoogleLoginSignup()
        {
            if (!PerformValidation(isGoogleLogin: true))
            {
                return;
            }

            switch (SelectedConfirmAction)
            {
                case ConfirmAction.LogIn:
                    await GoogleLoginAsync();
                    break;
                case ConfirmAction.SignUp:
                    await GoogleSignupAsync();
                    break;
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }

        public async Task GoogleSignupAsync()
        {
            await GoogleAuth(accessToken => Toggl.GoogleSignup(accessToken, SelectedCountry.ID), "Signup");
        }

        public async Task GoogleLoginAsync()
        {
            await GoogleAuth(accessToken => Toggl.GoogleLogin(accessToken), "Login");
        }

        private async Task GoogleAuth(Action<string> authAction, string authProcessName)
        {
            try
            {
                var credential = await ObtainGoogleUserCredentialAsync();
                authAction(credential.Token.AccessToken);
                await credential.RevokeTokenAsync(CancellationToken.None);
            }
            catch (Exception ex)
            {
                if (ex.Message.Contains("access_denied") ||
                    (ex.InnerException != null &&
                     ex.InnerException.Message.Contains("access_denied")))
                {
                    Toggl.NewError($"{authProcessName} process was canceled", true);
                }
                else
                {
                    Toggl.NewError(ex.Message, false);
                }
            }
        }

        private async Task<UserCredential> ObtainGoogleUserCredentialAsync()
        {
            var initializer = new GoogleAuthorizationCodeFlow.Initializer
            {
                ClientSecrets = new ClientSecrets
                {
                    ClientId = "426090949585-uj7lka2mtanjgd7j9i6c4ik091rcv6n5.apps.googleusercontent.com",
                    ClientSecret = "6IHWKIfTAMF7cPJsBvoGxYui"
                },
                HttpClientFactory = _httpClientFactory
            };
            var credential = await GoogleWebAuthorizationBroker.AuthorizeAsync(
                initializer,
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

        private async Task<bool> ConfirmAsync(Func<string, string, long, bool> confirmAction)
        {
            var email = Email;
            var password = Password;
            var selectedCountryId = SelectedCountry.ID;
            return await Task.Run(() => confirmAction(email, password, selectedCountryId));
        }

        private static HttpClientFactory HttpClientFactoryFromProxySettings(Toggl.TogglSettingsView settings)
        {
            var proxyHttpClientFactory = new ProxySupportedHttpClientFactory
            {
                UseProxy = settings.UseProxy
            };
            if (settings.AutodetectProxy)
            {
                proxyHttpClientFactory.Proxy = WebRequest.DefaultWebProxy;
            }
            else if (settings.UseProxy)
            {
                var proxy = new WebProxy(settings.ProxyHost + ":" + settings.ProxyPort, true);
                if (!string.IsNullOrEmpty(settings.ProxyUsername))
                {
                    proxy.Credentials = new NetworkCredential(settings.ProxyUsername, settings.ProxyPassword);
                }
                proxyHttpClientFactory.Proxy = proxy;
            }

            return proxyHttpClientFactory;
        }

        private class ProxySupportedHttpClientFactory : HttpClientFactory
        {
            public bool UseProxy { get; set; }
            public IWebProxy Proxy { get; set; }
            protected override HttpMessageHandler CreateHandler(CreateHttpClientArgs args)
            {
                var webRequestHandler = new WebRequestHandler
                {
                    UseProxy = this.UseProxy,
                    UseCookies = false
                };
                if (webRequestHandler.UseProxy)
                {
                    webRequestHandler.Proxy = Proxy;
                }

                return webRequestHandler;
            }
        }
    }

    public enum ConfirmAction
    {
        LogIn,
        SignUp
    }
}