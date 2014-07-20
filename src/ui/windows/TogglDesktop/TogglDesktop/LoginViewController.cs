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
            Toggl.Login(email.Text, password.Text);
            password.Clear();
        }

        private void passwordForgotTextField_LinkClicked_1(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Toggl.PasswordForgot();
        }

        private void googleLoginTextField_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            googleLogin();
        }

        async Task googleLogin()
        {
            try
            {
                UserCredential credential;
                credential = await GoogleWebAuthorizationBroker.AuthorizeAsync(
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
                    null);
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

        private void LoginViewController_Load(object sender, EventArgs e)
        {
            this.Dock = DockStyle.Fill;
        }

        private void email_Enter(object sender, EventArgs e)
        {
            if (email.Text == "Your email address")
            {
                email.Text = "";
                email.ForeColor = System.Drawing.SystemColors.WindowText;
            }
        }

        private void email_Leave(object sender, EventArgs e)
        {
            if (email.Text == "")
            {
                email.Text = "Your email address";
                email.ForeColor = System.Drawing.Color.Gray;
            }
        }

        private void password_Enter(object sender, EventArgs e)
        {
            if (password.Text == "Password")
            {
                password.Text = "";
                password.ForeColor = System.Drawing.SystemColors.WindowText;
            }
        }

        private void password_Leave(object sender, EventArgs e)
        {
            if (password.Text == "")
            {
                password.Text = "Password";
                password.ForeColor = System.Drawing.Color.Gray;
            }
        }

        private void linkLabelPreferences_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Toggl.EditPreferences();
        }
    }
}