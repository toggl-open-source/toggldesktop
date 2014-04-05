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

        private void LoginViewController_Load(object sender, EventArgs e)
        {
            troubleBox.BackColor = Color.FromArgb(239, 226, 121);
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
            Core.Login(email.Text, password.Text);
        }

        private void passwordForgotTextField_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Process.Start("FIXME: forgot password is a const");
        }

        private void googleLoginTextField_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            MessageBox.Show("FIXME: google login");
        }

    }
}
