using System;
using System.Windows.Forms;

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
    }
}