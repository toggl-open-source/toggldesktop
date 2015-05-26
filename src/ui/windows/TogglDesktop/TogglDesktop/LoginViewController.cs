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
    private enum ConfirmAction
    {
        Unknown = 0,
        LogIn,
        SignUp
    }

    private ConfirmAction confirmAction = ConfirmAction.Unknown;

    public LoginViewController()
    {
        InitializeComponent();
        setConfirmAction(ConfirmAction.LogIn);
    }

    public void SetAcceptButton(Form frm)
    {
        frm.AcceptButton = confirmButton;
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
        usernamePlaceholder.Visible = (email.Text.Length == 0);
    }

    private void password_TextChanged(object sender, EventArgs e)
    {
        passwordPlaceholder.Visible = (password.Text.Length == 0);
    }

    private void loginSignupToggle_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
    {
        switch (confirmAction)
        {
            case ConfirmAction.LogIn:
                setConfirmAction(ConfirmAction.SignUp);
                break;
            case ConfirmAction.SignUp:
                setConfirmAction(ConfirmAction.LogIn);
                break;
            default:
                throw new ArgumentException(string.Format("Invalid action '{0}' in login form.", confirmAction));
        }
    }

    private void confirmButton_Click(object sender, EventArgs e)
    {
        switch (confirmAction)
        {
            case ConfirmAction.LogIn:
                login();
                break;
            case ConfirmAction.SignUp:
                signup();
                break;
            default:
                throw new ArgumentException(string.Format("Invalid action '{0}' in login form.", confirmAction));
        }
    }

    private void setConfirmAction(ConfirmAction action)
    {
        switch (action)
        {
            case ConfirmAction.LogIn:
                confirmButton.Text = "Log in";
                passwordForgotTextField.Show();
                googleLoginTextField.Show();
                loginSignupToggle.Text = "Sign up for free";
                break;
            case ConfirmAction.SignUp:
                confirmButton.Text = "Sign up";
                passwordForgotTextField.Hide();
                googleLoginTextField.Hide();
                loginSignupToggle.Text = "Log in";
                break;
            default:
                throw new ArgumentException(string.Format("Invalid action '{0}' in login form.", action));
        }
        confirmAction = action;
    }

    private void login()
    {
        if (!validateFields())
        {
            return;
        }
        Toggl.Login(email.Text, password.Text);
        password.Clear();
    }

    private void signup()
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

}
}