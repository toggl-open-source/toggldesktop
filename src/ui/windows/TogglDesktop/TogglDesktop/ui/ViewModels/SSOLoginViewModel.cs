using System;
using System.ComponentModel;
using System.Reactive;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Web;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using ReactiveUI.Validation.Extensions;
using ReactiveUI.Validation.Helpers;

namespace TogglDesktop.ViewModels
{
    public class SSOLoginViewModel : ReactiveValidationViewModel<SSOLoginViewModel>
    {
        private ValidationHelper _emailValidation;

        public SSOLoginViewModel(Action<string, string> openLoginForm) : base(false)
        {
            DisplaySignOnMode = true;
            LoginWithDifferentMethod = ReactiveCommand.Create(() => openLoginForm(null, null));
            LoginAndLinkSSO = ReactiveCommand.Create(() => openLoginForm(ConfirmationCode, Email));
            LoginCommand = ReactiveCommand.Create(Login);
            BackToSSOLogin = ReactiveCommand.Create(() => { DisplaySignOnMode = true; });
            var uriObservable = AuthUri.Where(uri => uri != null);
            uriObservable.Select(uri => HttpUtility.ParseQueryString(uri.Query).Get("apiToken"))
                .Where(token => !string.IsNullOrEmpty(token))
                .Subscribe(Toggl.LoginSSO);
            uriObservable.Select(uri => HttpUtility.ParseQueryString(uri.Query).Get("confirmation_code"))
                .ToPropertyEx(this, x => x.ConfirmationCode);
            uriObservable.Where(uri => HttpUtility.ParseQueryString(uri.Query).Get("apiToken") == null &&
                                       HttpUtility.ParseQueryString(uri.Query).Get("confirmation_code") == null)
                .Subscribe(uri => Toggl.NewError("Single Sign On is not configured for your email address." +
                                                 " Please try a different login method or contact your administrator.",
                    true));
            this.WhenAnyValue(x => x.ConfirmationCode)
                .Where(code => !string.IsNullOrEmpty(code))
                .Subscribe(next => DisplaySignOnMode = false);
            Toggl.OnLoginSSO += HandleDisplayLoginSSO;
            _emailValidation = this.ValidationRule(x => x.Email,
                x => x == null || x.IsValidEmailAddress(), "Please enter a valid email");
        }

        private bool Login()
        {
            ActivateValidation();
            if (!_emailValidation.IsValid) return false;
            return Toggl.GetIdentityProviderSSO(Email);
        }

        private void HandleDisplayLoginSSO(string ssoUrl)
        {
            Toggl.OpenInBrowser(ssoUrl);
        }

        public ReactiveCommand<Unit, Unit> LoginWithDifferentMethod { get; }

        public ReactiveCommand<Unit,bool> LoginCommand { get; }

        public ReactiveCommand<Unit, Unit> LoginAndLinkSSO { get; }

        public ReactiveCommand<Unit, Unit> BackToSSOLogin { get; }

        [Reactive]
        public string Email { get; set; }

        [Reactive]
        public bool DisplaySignOnMode { get; set; }

        public Subject<Uri> AuthUri { get; } = new Subject<Uri>();

        private string ConfirmationCode { [ObservableAsProperty] get; }

        public void Reset()
        {
            SnoozeValidation();
            Email = null;
        }
    }
}
