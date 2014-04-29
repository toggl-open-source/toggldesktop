using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;

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
            Kopsik.Login(email.Text, password.Text);
            password.Clear();
        }

        private void passwordForgotTextField_LinkClicked_1(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Kopsik.PasswordForgot();
        }

        private void LoginViewController_Load(object sender, EventArgs e)
        {
            Kopsik.OnOpenURL += Core_OnOpenURL;
        }

        void Core_OnOpenURL(string url)
        {
            Process.Start(url);
        }

    }
}