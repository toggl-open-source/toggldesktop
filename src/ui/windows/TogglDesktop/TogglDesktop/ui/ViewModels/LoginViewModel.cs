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
using ReactiveUI.Validation.Extensions;
using ReactiveUI.Validation.Helpers;

namespace TogglDesktop.ViewModels
{
    public class LoginViewModel : ReactiveValidationObject<LoginViewModel>
    {
        private readonly Action _refreshLoginBindings;
        private readonly Action _refreshSignupBindings;
        private IList<CountryViewModel> _countries;
        private CountryViewModel _selectedCountry;
        private ConfirmAction _confirmAction;
        private string _email;
        private string _password;
        private bool _isEmailFocused;
        private bool _isPasswordFocused;
        private bool _isCountrySelectionFocused;
        private bool _isTosCheckboxFocused;
        private bool _isTosChecked;
        private readonly ValidationHelper _emailValidation;
        private readonly ValidationHelper _passwordValidation;
        private readonly ValidationHelper _selectedCountryValidation;
        private readonly ValidationHelper _isTosCheckedValidation;
        private readonly ObservableAsPropertyHelper<string> _confirmButtonText;
        private readonly ObservableAsPropertyHelper<string> _googleLoginButtonText;
        private readonly ObservableAsPropertyHelper<string> _signupLoginToggleText;
        private HttpClientFactory _httpClientFactory;
        private readonly ObservableAsPropertyHelper<bool> _isLoading;
        private readonly ObservableAsPropertyHelper<bool> _isViewDisabled;

        public LoginViewModel(Action refreshLoginBindings, Action refreshSignupBindings)
        {
            _refreshLoginBindings = refreshLoginBindings;
            _refreshSignupBindings = refreshSignupBindings;
            Toggl.OnDisplayCountries += OnDisplayCountries;
            Toggl.OnSettings += OnSettings;
            this.WhenAnyValue(x => x.SelectedConfirmAction,
                    x => x == ConfirmAction.LogIn ? "Log in" : "Sign up")
                .ToProperty(this, x => x.ConfirmButtonText, out _confirmButtonText);
            this.WhenAnyValue(x => x.SelectedConfirmAction,
                    x => x == ConfirmAction.LogIn ? "Log in with Google" : "Sign up with Google")
                .ToProperty(this, x => x.GoogleLoginButtonText, out _googleLoginButtonText);
            this.WhenAnyValue(x => x.SelectedConfirmAction,
                    x => x == ConfirmAction.LogIn ? "Sign up for free" : "Back to Log in")
                .ToProperty(this, x => x.SignupLoginToggleText, out _signupLoginToggleText);
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
                .ToProperty(this, x => x.IsLoading, out _isLoading);
            IsLoginSignupExecuting
                .Select(x => !x)
                .ToProperty(this, x => x.IsViewEnabled, out _isViewDisabled);

            _emailValidation = this.ValidationRule(
                x => x.Email,
                email => !string.IsNullOrEmpty(email),
                "Please enter a valid email");
            _passwordValidation = this.ValidationRule(
                x => x.Password,
                password => !string.IsNullOrEmpty(password),
                "A password is required");
            _selectedCountryValidation = this.ValidationRule(
                x => x.SelectedCountry,
                selectedCountry => selectedCountry != null,
                "Please select country");
            _isTosCheckedValidation = this.ValidationRule(
                x => x.IsTosChecked,
                isTosChecked => isTosChecked,
                "Please accept the terms");
        }
        public ReactiveCommand<Unit, bool> ConfirmLoginSignupCommand { get; }
        public ReactiveCommand<Unit, Unit> ConfirmGoogleLoginSignupCommand { get; }
        public IObservable<bool> IsLoginSignupExecuting { get; }

        public IList<CountryViewModel> Countries
        {
            get => _countries;
            set => this.RaiseAndSetIfChanged(ref _countries, value);
        }

        public CountryViewModel SelectedCountry
        {
            get => _selectedCountry;
            set => this.RaiseAndSetIfChanged(ref _selectedCountry, value);
        }

        public ConfirmAction SelectedConfirmAction
        {
            get => _confirmAction;
            set => this.RaiseAndSetIfChanged(ref _confirmAction, value);
        }

        public string Email
        {
            get => _email;
            set => this.RaiseAndSetIfChanged(ref _email, value);
        }

        public string Password
        {
            get => _password;
            set => this.RaiseAndSetIfChanged(ref _password, value);
        }
        public bool IsEmailFocused
        {
            get => _isEmailFocused;
            set => this.RaiseAndSetIfChanged(ref _isEmailFocused, value);
        }

        public bool IsPasswordFocused
        {
            get => _isPasswordFocused;
            set => this.RaiseAndSetIfChanged(ref _isPasswordFocused, value);
        }

        public bool IsCountrySelectionFocused
        {
            get => _isCountrySelectionFocused;
            set => this.RaiseAndSetIfChanged(ref _isCountrySelectionFocused, value);
        }

        public bool IsTosCheckboxFocused
        {
            get => _isTosCheckboxFocused;
            set => this.RaiseAndSetIfChanged(ref _isTosCheckboxFocused, value);
        }

        public bool IsTosChecked
        {
            get => _isTosChecked;
            set => this.RaiseAndSetIfChanged(ref _isTosChecked, value);
        }

        public string ConfirmButtonText => _confirmButtonText.Value;
        public string GoogleLoginButtonText => _googleLoginButtonText.Value;
        public string SignupLoginToggleText => _signupLoginToggleText.Value;
        public bool IsLoading => _isLoading.Value;
        public bool IsViewEnabled => _isViewDisabled.Value;

        private bool PerformValidation(bool isGoogleLogin = false)
        {
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

            if (SelectedConfirmAction == ConfirmAction.LogIn)
            {
                _refreshLoginBindings();
            }
            else
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

        private void OnDisplayCountries(List<Toggl.TogglCountryView> list)
        {
            var countriesVm = list.Select(c => new CountryViewModel(c)).ToArray();
            Dispatcher.CurrentDispatcher.Invoke(() => { Countries = countriesVm; });
        }

        private void OnSettings(bool open, Toggl.TogglSettingsView settings)
        {
            _httpClientFactory = HttpClientFactoryFromProxySettings(settings);
        }

        public async Task GoogleSignupAsync()
        {
            await GoogleAuth(accessToken => Toggl.GoogleSignup(accessToken, SelectedCountry?.ID ?? default), "Signup");
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
            var selectedCountryId = SelectedCountry?.ID ?? -1;
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

        public class CountryViewModel
        {
            private readonly Toggl.TogglCountryView _countryView;
            public CountryViewModel(Toggl.TogglCountryView countryView)
            {
                _countryView = countryView;
            }

            public string Name => _countryView.Name;
            public long ID => _countryView.ID;
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