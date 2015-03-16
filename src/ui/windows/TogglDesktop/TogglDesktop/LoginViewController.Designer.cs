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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(LoginViewController));
            this.loginButton = new System.Windows.Forms.Button();
            this.googleLoginTextField = new System.Windows.Forms.LinkLabel();
            this.password = new System.Windows.Forms.MaskedTextBox();
            this.email = new System.Windows.Forms.TextBox();
            this.passwordForgotTextField = new System.Windows.Forms.LinkLabel();
            this.togglLogoBox = new System.Windows.Forms.PictureBox();
            this.linkLabelPreferences = new System.Windows.Forms.LinkLabel();
            this.signupButton = new System.Windows.Forms.Button();
            this.passwordPlaceholder = new System.Windows.Forms.Label();
            this.usernamePlaceholder = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.togglLogoBox)).BeginInit();
            this.SuspendLayout();
            // 
            // loginButton
            // 
            this.loginButton.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.loginButton.Location = new System.Drawing.Point(15, 208);
            this.loginButton.Name = "loginButton";
            this.loginButton.Size = new System.Drawing.Size(110, 35);
            this.loginButton.TabIndex = 2;
            this.loginButton.Text = "Log in";
            this.loginButton.UseVisualStyleBackColor = true;
            this.loginButton.Click += new System.EventHandler(this.loginButton_Click);
            // 
            // googleLoginTextField
            // 
            this.googleLoginTextField.AccessibleRole = System.Windows.Forms.AccessibleRole.PushButton;
            this.googleLoginTextField.ActiveLinkColor = System.Drawing.Color.White;
            this.googleLoginTextField.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.googleLoginTextField.AutoSize = true;
            this.googleLoginTextField.LinkColor = System.Drawing.Color.Gray;
            this.googleLoginTextField.Location = new System.Drawing.Point(68, 268);
            this.googleLoginTextField.Name = "googleLoginTextField";
            this.googleLoginTextField.Size = new System.Drawing.Size(134, 13);
            this.googleLoginTextField.TabIndex = 3;
            this.googleLoginTextField.TabStop = true;
            this.googleLoginTextField.Text = "Login with Google account";
            this.googleLoginTextField.VisitedLinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(99)))), ((int)(((byte)(99)))), ((int)(((byte)(99)))));
            this.googleLoginTextField.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.googleLoginTextField_LinkClicked);
            // 
            // password
            // 
            this.password.AccessibleName = "Your password";
            this.password.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.password.ForeColor = System.Drawing.Color.Black;
            this.password.Location = new System.Drawing.Point(15, 171);
            this.password.Name = "password";
            this.password.Size = new System.Drawing.Size(249, 20);
            this.password.TabIndex = 1;
            this.password.UseSystemPasswordChar = true;
            this.password.TextChanged += new System.EventHandler(this.password_TextChanged);
            this.password.Leave += new System.EventHandler(this.password_Leave);
            // 
            // email
            // 
            this.email.AccessibleName = "Your e-mail address";
            this.email.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.email.ForeColor = System.Drawing.Color.Black;
            this.email.Location = new System.Drawing.Point(15, 134);
            this.email.Name = "email";
            this.email.Size = new System.Drawing.Size(249, 20);
            this.email.TabIndex = 0;
            this.email.TextChanged += new System.EventHandler(this.email_TextChanged);
            this.email.Leave += new System.EventHandler(this.email_Leave);
            // 
            // passwordForgotTextField
            // 
            this.passwordForgotTextField.AccessibleRole = System.Windows.Forms.AccessibleRole.PushButton;
            this.passwordForgotTextField.ActiveLinkColor = System.Drawing.Color.White;
            this.passwordForgotTextField.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.passwordForgotTextField.AutoSize = true;
            this.passwordForgotTextField.LinkColor = System.Drawing.Color.Gray;
            this.passwordForgotTextField.Location = new System.Drawing.Point(88, 290);
            this.passwordForgotTextField.Name = "passwordForgotTextField";
            this.passwordForgotTextField.Size = new System.Drawing.Size(91, 13);
            this.passwordForgotTextField.TabIndex = 7;
            this.passwordForgotTextField.TabStop = true;
            this.passwordForgotTextField.Text = "Forgot password?";
            this.passwordForgotTextField.VisitedLinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(99)))), ((int)(((byte)(99)))), ((int)(((byte)(99)))));
            this.passwordForgotTextField.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.passwordForgotTextField_LinkClicked_1);
            // 
            // togglLogoBox
            // 
            this.togglLogoBox.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.togglLogoBox.Image = ((System.Drawing.Image)(resources.GetObject("togglLogoBox.Image")));
            this.togglLogoBox.Location = new System.Drawing.Point(71, 72);
            this.togglLogoBox.Name = "togglLogoBox";
            this.togglLogoBox.Size = new System.Drawing.Size(120, 36);
            this.togglLogoBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.togglLogoBox.TabIndex = 8;
            this.togglLogoBox.TabStop = false;
            // 
            // linkLabelPreferences
            // 
            this.linkLabelPreferences.AccessibleRole = System.Windows.Forms.AccessibleRole.PushButton;
            this.linkLabelPreferences.ActiveLinkColor = System.Drawing.Color.White;
            this.linkLabelPreferences.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.linkLabelPreferences.AutoSize = true;
            this.linkLabelPreferences.LinkColor = System.Drawing.Color.Gray;
            this.linkLabelPreferences.Location = new System.Drawing.Point(97, 313);
            this.linkLabelPreferences.Name = "linkLabelPreferences";
            this.linkLabelPreferences.Size = new System.Drawing.Size(64, 13);
            this.linkLabelPreferences.TabIndex = 9;
            this.linkLabelPreferences.TabStop = true;
            this.linkLabelPreferences.Text = "Preferences";
            this.linkLabelPreferences.VisitedLinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(99)))), ((int)(((byte)(99)))), ((int)(((byte)(99)))));
            this.linkLabelPreferences.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.linkLabelPreferences_LinkClicked);
            // 
            // signupButton
            // 
            this.signupButton.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.signupButton.Location = new System.Drawing.Point(154, 208);
            this.signupButton.Name = "signupButton";
            this.signupButton.Size = new System.Drawing.Size(110, 35);
            this.signupButton.TabIndex = 10;
            this.signupButton.Text = "Sign up";
            this.signupButton.UseVisualStyleBackColor = true;
            this.signupButton.Click += new System.EventHandler(this.signupButton_Click);
            // 
            // passwordPlaceholder
            // 
            this.passwordPlaceholder.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.passwordPlaceholder.BackColor = System.Drawing.Color.White;
            this.passwordPlaceholder.Cursor = System.Windows.Forms.Cursors.IBeam;
            this.passwordPlaceholder.ForeColor = System.Drawing.SystemColors.ControlDark;
            this.passwordPlaceholder.Location = new System.Drawing.Point(19, 172);
            this.passwordPlaceholder.Name = "passwordPlaceholder";
            this.passwordPlaceholder.Size = new System.Drawing.Size(132, 18);
            this.passwordPlaceholder.TabIndex = 11;
            this.passwordPlaceholder.Text = "Password";
            this.passwordPlaceholder.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.passwordPlaceholder.Click += new System.EventHandler(this.passwordPlaceholder_Click);
            // 
            // usernamePlaceholder
            // 
            this.usernamePlaceholder.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.usernamePlaceholder.BackColor = System.Drawing.Color.White;
            this.usernamePlaceholder.Cursor = System.Windows.Forms.Cursors.IBeam;
            this.usernamePlaceholder.ForeColor = System.Drawing.SystemColors.ControlDark;
            this.usernamePlaceholder.Location = new System.Drawing.Point(19, 135);
            this.usernamePlaceholder.Name = "usernamePlaceholder";
            this.usernamePlaceholder.Size = new System.Drawing.Size(130, 18);
            this.usernamePlaceholder.TabIndex = 12;
            this.usernamePlaceholder.Text = "Your email address";
            this.usernamePlaceholder.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.usernamePlaceholder.Click += new System.EventHandler(this.usernamePlaceholder_Click);
            // 
            // LoginViewController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(69)))), ((int)(((byte)(69)))), ((int)(((byte)(69)))));
            this.Controls.Add(this.usernamePlaceholder);
            this.Controls.Add(this.passwordPlaceholder);
            this.Controls.Add(this.signupButton);
            this.Controls.Add(this.linkLabelPreferences);
            this.Controls.Add(this.togglLogoBox);
            this.Controls.Add(this.passwordForgotTextField);
            this.Controls.Add(this.email);
            this.Controls.Add(this.password);
            this.Controls.Add(this.googleLoginTextField);
            this.Controls.Add(this.loginButton);
            this.MinimumSize = new System.Drawing.Size(279, 367);
            this.Name = "LoginViewController";
            this.Size = new System.Drawing.Size(279, 367);
            this.Load += new System.EventHandler(this.LoginViewController_Load);
            ((System.ComponentModel.ISupportInitialize)(this.togglLogoBox)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button loginButton;
        private System.Windows.Forms.LinkLabel googleLoginTextField;
        private System.Windows.Forms.MaskedTextBox password;
        private System.Windows.Forms.TextBox email;
        private System.Windows.Forms.LinkLabel passwordForgotTextField;
        private System.Windows.Forms.PictureBox togglLogoBox;
        private System.Windows.Forms.LinkLabel linkLabelPreferences;
        private System.Windows.Forms.Button signupButton;
        private System.Windows.Forms.Label passwordPlaceholder;
        private System.Windows.Forms.Label usernamePlaceholder;
    }
}
