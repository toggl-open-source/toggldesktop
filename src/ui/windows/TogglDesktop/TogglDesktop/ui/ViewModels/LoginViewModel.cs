using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
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

namespace TogglDesktop.ViewModels
{
    public class LoginViewModel : ReactiveObject
    {
        private IList<CountryViewModel> _countries;
        private CountryViewModel _selectedCountry;
        private ConfirmAction _confirmAction;
        private readonly ObservableAsPropertyHelper<string> _confirmButtonText;
        private readonly ObservableAsPropertyHelper<string> _googleLoginButtonText;
        private readonly ObservableAsPropertyHelper<string> _signuplLoginToggleText;
        private HttpClientFactory _httpClientFactory;

        public LoginViewModel()
        {
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
                .ToProperty(this, x => x.SignupLoginToggleText, out _signuplLoginToggleText);
        }

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

        public string ConfirmButtonText => _confirmButtonText.Value;
        public string GoogleLoginButtonText => _googleLoginButtonText.Value;
        public string SignupLoginToggleText => _signuplLoginToggleText.Value;

        private void OnDisplayCountries(List<Toggl.TogglCountryView> list)
        {
            var countriesVm = list.Select(c => new CountryViewModel(c)).ToArray();
            Dispatcher.CurrentDispatcher.Invoke(() => { Countries = countriesVm; });
        }

        private void OnSettings(bool open, Toggl.TogglSettingsView settings)
        {
            _httpClientFactory = HttpClientFactoryFromProxySettings(settings);
        }

        public async void GoogleSignup()
        {
            await GoogleAuth(accessToken => Toggl.GoogleSignup(accessToken, SelectedCountry?.ID ?? default), "Signup");
        }

        public async void GoogleLogin()
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
        Unknown = 0,
        LogIn,
        SignUp
    }
}