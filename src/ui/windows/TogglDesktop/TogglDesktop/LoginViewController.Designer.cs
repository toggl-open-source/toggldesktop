namespace TogglDesktop
{
    partial class LoginViewController
    {
        /// <summary> 
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.loginButton = new System.Windows.Forms.Button();
            this.googleLoginTextField = new System.Windows.Forms.LinkLabel();
            this.password = new System.Windows.Forms.MaskedTextBox();
            this.email = new System.Windows.Forms.TextBox();
            this.passwordForgotTextField = new System.Windows.Forms.LinkLabel();
            this.SuspendLayout();
            // 
            // loginButton
            // 
            this.loginButton.Location = new System.Drawing.Point(98, 162);
            this.loginButton.Name = "loginButton";
            this.loginButton.Size = new System.Drawing.Size(75, 23);
            this.loginButton.TabIndex = 2;
            this.loginButton.Text = "Sign in";
            this.loginButton.UseVisualStyleBackColor = true;
            this.loginButton.Click += new System.EventHandler(this.loginButton_Click);
            // 
            // googleLoginTextField
            // 
            this.googleLoginTextField.AutoSize = true;
            this.googleLoginTextField.Location = new System.Drawing.Point(95, 188);
            this.googleLoginTextField.Name = "googleLoginTextField";
            this.googleLoginTextField.Size = new System.Drawing.Size(66, 13);
            this.googleLoginTextField.TabIndex = 3;
            this.googleLoginTextField.TabStop = true;
            this.googleLoginTextField.Text = "Google login";
            this.googleLoginTextField.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.googleLoginTextField_LinkClicked);
            // 
            // password
            // 
            this.password.Location = new System.Drawing.Point(3, 136);
            this.password.Name = "password";
            this.password.Size = new System.Drawing.Size(273, 20);
            this.password.TabIndex = 1;
            this.password.UseSystemPasswordChar = true;
            // 
            // email
            // 
            this.email.Location = new System.Drawing.Point(4, 110);
            this.email.Name = "email";
            this.email.Size = new System.Drawing.Size(272, 20);
            this.email.TabIndex = 0;
            // 
            // passwordForgotTextField
            // 
            this.passwordForgotTextField.AutoSize = true;
            this.passwordForgotTextField.Location = new System.Drawing.Point(95, 210);
            this.passwordForgotTextField.Name = "passwordForgotTextField";
            this.passwordForgotTextField.Size = new System.Drawing.Size(91, 13);
            this.passwordForgotTextField.TabIndex = 7;
            this.passwordForgotTextField.TabStop = true;
            this.passwordForgotTextField.Text = "Forgot password?";
            // 
            // LoginViewController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.passwordForgotTextField);
            this.Controls.Add(this.email);
            this.Controls.Add(this.password);
            this.Controls.Add(this.googleLoginTextField);
            this.Controls.Add(this.loginButton);
            this.Name = "LoginViewController";
            this.Size = new System.Drawing.Size(279, 345);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button loginButton;
        private System.Windows.Forms.LinkLabel googleLoginTextField;
        private System.Windows.Forms.MaskedTextBox password;
        private System.Windows.Forms.TextBox email;
        private System.Windows.Forms.LinkLabel passwordForgotTextField;
    }
}
