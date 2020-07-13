using System;
using System.Reactive;
using System.Reactive.Linq;
using System.Web;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using ReactiveUI.Validation.Helpers;

namespace TogglDesktop.ViewModels
{
    public class SSOLoginViewModel : ReactiveValidationObject<SSOLoginViewModel>
    {
        private ValidationHelper _uriValidationRule;

        public SSOLoginViewModel(Action<string, string> openLoginForm)
        {
            IsSignOnMode = true;
            LoginWithDifferentMethod = ReactiveCommand.Create(() => openLoginForm(null, null));
            LoginAndLinkSSO = ReactiveCommand.Create(() => openLoginForm(ConfirmationCode, Email));
            Login = ReactiveCommand.Create(() => Toggl.GetIdentityProviderSSO(Email));
            BackToSSOLogin = ReactiveCommand.Create(() => { IsSignOnMode = true; });
            this.WhenAnyValue(x => x.AuthUri)
                .Where(uri => uri != null)
                .Select(uri => HttpUtility.ParseQueryString(uri.Query).Get("apiToken"))
                .Where(token => !string.IsNullOrEmpty(token))
                .Subscribe(Toggl.LoginSSO);
            this.WhenAnyValue(x => x.AuthUri)
                .Where(uri => uri != null)
                .Select(uri => HttpUtility.ParseQueryString(uri.Query).Get("confirmation_code"))
                .ToPropertyEx(this, x => x.ConfirmationCode);
            this.WhenAnyValue(x => x.ConfirmationCode)
                .Where(code => !string.IsNullOrEmpty(code))
                .Subscribe(next => IsSignOnMode = false);
            Toggl.OnLoginSSO += HandleDisplayLoginSSO;
        }

        //private bool IsAuthUriValid()
        //{
        //    _uriValidationRule ??= this.ValidationRule(x => x.AuthUri,
        //        IsSuccessfulLogin, "Single Sign On is not configured for your email address." +
        //                           " Please try a different login method or contact your administrator.");
        //    return _uriValidationRule.IsValid;
        //}

        private void HandleDisplayLoginSSO(string ssoUrl)
        {
            System.Diagnostics.Process.Start(ssoUrl);
        }

        public ReactiveCommand<Unit, Unit> LoginWithDifferentMethod { get; }

        public ReactiveCommand<Unit,bool> Login { get; }

        public ReactiveCommand<Unit, Unit> LoginAndLinkSSO { get; }

        public ReactiveCommand<Unit, Unit> BackToSSOLogin { get; }

        [Reactive]
        public string Email { get; set; }

        [Reactive]
        public bool IsSignOnMode { get; set; }

        [Reactive]
        public Uri AuthUri { get; set; }

        private string ConfirmationCode { [ObservableAsProperty] get; }

        //private bool IsSuccessfulLogin(Uri uri)
        //{
        //    if (uri == null)
        //        return false;

        //    return HttpUtility.ParseQueryString(uri.Query).Get("confirmation_code") != null &&
        //           HttpUtility.ParseQueryString(uri.Query).Get("email") != null;
        //}
    }
}
