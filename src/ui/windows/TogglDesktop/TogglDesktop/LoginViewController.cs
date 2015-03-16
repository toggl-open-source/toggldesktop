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

        private bool validateFields()
        {
            if (0 == email.Text.Length)
            {
                email.Focus();
                return false;
            }
            if (0 == password.Text.Length)
            {
                password.Focus();
                return false;
            }
            return true;
        }

        private void loginButton_Click(object sender, EventArgs e)
        {
            if (!validateFields())
            {
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
            Dock = DockStyle.Fill;
        }

        private void email_Leave(object sender, EventArgs e)
        {
            usernamePlaceholder.Visible = (email.Text == "");
        }

        private void password_Leave(object sender, EventArgs e)
        {
            passwordPlaceholder.Visible = (password.Text == "");
        }

        private void linkLabelPreferences_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Toggl.EditPreferences();
        }

        private void signupButton_Click(object sender, EventArgs e)
        {
            if (!validateFields())
            {
                return;
            }
            if (password.Text == "")
            {
                password.Clear();
                password.Focus();
                return;
            }
            Toggl.Signup(email.Text, password.Text);
            password.Clear();
        }

        private void usernamePlaceholder_Click(object sender, EventArgs e)
        {
            email.Focus();
        }

        private void passwordPlaceholder_Click(object sender, EventArgs e)
        {
            password.Focus();
        }

        private void email_TextChanged(object sender, EventArgs e)
        {
            if (email.Text.Length == 0)
            {
                usernamePlaceholder.Visible = true;
            }
            else if (email.Text.Length == 1)
            {
                usernamePlaceholder.Visible = false;
            }
        }

        private void password_TextChanged(object sender, EventArgs e)
        {
            if (password.Text.Length == 0)
            {
                passwordPlaceholder.Visible = true;
            }
            else if (password.Text.Length == 1)
            {
                passwordPlaceholder.Visible = false;
            }
        }
    }
}