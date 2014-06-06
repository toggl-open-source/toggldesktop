using System;
using System.Windows.Forms;
using System.Threading;
using System.Threading.Tasks;
using Google.Apis.Auth.OAuth2;
using Google.Apis.Auth.OAuth2.Responses;
using Google.Apis.Oauth2.v2;
using Google.Apis.Services;

namespace TogglDesktop
{
    public partial class LoginViewController : UserControl
    {
        public LoginViewController()
        {
            InitializeComponent();
        }

        public void SetAcceptButton(Form frm)
        {
            frm.AcceptButton = loginButton;
        }

        private void loginButton_Click(object sender, EventArgs e)
        {
            if (0 == email.Text.Length)
            {
                email.Focus();
                return;
            }
            if (0 == password.Text.Length)
            {
                password.Focus();
                return;
            }
            KopsikApi.kopsik_login(KopsikApi.ctx, email.Text, password.Text);
            password.Clear();
        }

        private void passwordForgotTextField_LinkClicked_1(object sender, LinkLabelLinkClickedEventArgs e)
        {
            KopsikApi.kopsik_password_forgot(KopsikApi.ctx);
        }

        private void googleLoginTextField_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            googleLogin().Wait();
        }

        private async Task googleLogin() {
            UserCredential credential;
            credential = GoogleWebAuthorizationBroker.AuthorizeAsync(
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
                CancellationToken.None,
                null).Result;

            Oauth2Service userInfoService = new Oauth2Service(
                new BaseClientService.Initializer()
                {
                    HttpClientInitializer = credential,
                    ApplicationName = "Toggl Desktop"
                });

            var userInfo = userInfoService.Userinfo.Get().Execute();

            Console.WriteLine(userInfo.Email);
        }
    }
}