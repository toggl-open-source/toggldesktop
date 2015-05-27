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
        this.loginSignupToggleLabel = new System.Windows.Forms.Label();
        this.googleLoginTextField = new System.Windows.Forms.LinkLabel();
        this.password = new System.Windows.Forms.MaskedTextBox();
        this.email = new System.Windows.Forms.TextBox();
        this.passwordForgotTextField = new System.Windows.Forms.LinkLabel();
        this.togglLogoBox = new System.Windows.Forms.PictureBox();
        this.passwordPlaceholder = new System.Windows.Forms.Label();
        this.usernamePlaceholder = new System.Windows.Forms.Label();
        this.loginSignupToggle = new System.Windows.Forms.LinkLabel();
        this.confirmButton = new System.Windows.Forms.Button();
        ((System.ComponentModel.ISupportInitialize)(this.togglLogoBox)).BeginInit();
        this.SuspendLayout();
        // 
        // loginSignupToggleLabel
        // 
        this.loginSignupToggleLabel.Anchor = System.Windows.Forms.AnchorStyles.Top;
        this.loginSignupToggleLabel.AutoSize = true;
        this.loginSignupToggleLabel.BackColor = System.Drawing.Color.Transparent;
        this.loginSignupToggleLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
        this.loginSignupToggleLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(227)))), ((int)(((byte)(227)))), ((int)(((byte)(227)))));
        this.loginSignupToggleLabel.Location = new System.Drawing.Point(113, 396);
        this.loginSignupToggleLabel.Name = "loginSignupToggleLabel";
        this.loginSignupToggleLabel.Size = new System.Drawing.Size(75, 17);
        this.loginSignupToggleLabel.TabIndex = 15;
        this.loginSignupToggleLabel.Text = "or you can";
        // 
        // googleLoginTextField
        // 
        this.googleLoginTextField.AccessibleRole = System.Windows.Forms.AccessibleRole.PushButton;
        this.googleLoginTextField.ActiveLinkColor = System.Drawing.Color.White;
        this.googleLoginTextField.Anchor = System.Windows.Forms.AnchorStyles.Top;
        this.googleLoginTextField.AutoSize = true;
        this.googleLoginTextField.BackColor = System.Drawing.Color.Transparent;
        this.googleLoginTextField.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
        this.googleLoginTextField.Image = ((System.Drawing.Image)(resources.GetObject("googleLoginTextField.Image")));
        this.googleLoginTextField.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
        this.googleLoginTextField.LinkBehavior = System.Windows.Forms.LinkBehavior.NeverUnderline;
        this.googleLoginTextField.LinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(227)))), ((int)(((byte)(227)))), ((int)(((byte)(227)))));
        this.googleLoginTextField.Location = new System.Drawing.Point(60, 318);
        this.googleLoginTextField.Name = "googleLoginTextField";
        this.googleLoginTextField.Padding = new System.Windows.Forms.Padding(21, 5, 0, 5);
        this.googleLoginTextField.Size = new System.Drawing.Size(180, 27);
        this.googleLoginTextField.TabIndex = 3;
        this.googleLoginTextField.TabStop = true;
        this.googleLoginTextField.Text = "LOG IN WITH GOOGLE";
        this.googleLoginTextField.VisitedLinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(99)))), ((int)(((byte)(99)))), ((int)(((byte)(99)))));
        this.googleLoginTextField.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.googleLoginTextField_LinkClicked);
        // 
        // password
        // 
        this.password.AccessibleName = "Your password";
        this.password.Anchor = System.Windows.Forms.AnchorStyles.Top;
        this.password.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
        this.password.ForeColor = System.Drawing.Color.Black;
        this.password.Location = new System.Drawing.Point(26, 191);
        this.password.Name = "password";
        this.password.Size = new System.Drawing.Size(249, 26);
        this.password.TabIndex = 1;
        this.password.UseSystemPasswordChar = true;
        this.password.TextChanged += new System.EventHandler(this.password_TextChanged);
        this.password.Leave += new System.EventHandler(this.password_Leave);
        // 
        // email
        // 
        this.email.AccessibleName = "Your e-mail address";
        this.email.Anchor = System.Windows.Forms.AnchorStyles.Top;
        this.email.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
        this.email.ForeColor = System.Drawing.Color.Black;
        this.email.Location = new System.Drawing.Point(26, 134);
        this.email.Name = "email";
        this.email.Size = new System.Drawing.Size(249, 26);
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
        this.passwordForgotTextField.BackColor = System.Drawing.Color.Transparent;
        this.passwordForgotTextField.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
        this.passwordForgotTextField.LinkBehavior = System.Windows.Forms.LinkBehavior.NeverUnderline;
        this.passwordForgotTextField.LinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(227)))), ((int)(((byte)(227)))), ((int)(((byte)(227)))));
        this.passwordForgotTextField.Location = new System.Drawing.Point(170, 220);
        this.passwordForgotTextField.Name = "passwordForgotTextField";
        this.passwordForgotTextField.Size = new System.Drawing.Size(105, 15);
        this.passwordForgotTextField.TabIndex = 5;
        this.passwordForgotTextField.TabStop = true;
        this.passwordForgotTextField.Text = "Forgot password?";
        this.passwordForgotTextField.VisitedLinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(99)))), ((int)(((byte)(99)))), ((int)(((byte)(99)))));
        this.passwordForgotTextField.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.passwordForgotTextField_LinkClicked_1);
        // 
        // togglLogoBox
        // 
        this.togglLogoBox.Anchor = System.Windows.Forms.AnchorStyles.Top;
        this.togglLogoBox.BackColor = System.Drawing.Color.Transparent;
        this.togglLogoBox.Image = ((System.Drawing.Image)(resources.GetObject("togglLogoBox.Image")));
        this.togglLogoBox.Location = new System.Drawing.Point(90, 72);
        this.togglLogoBox.Name = "togglLogoBox";
        this.togglLogoBox.Size = new System.Drawing.Size(120, 36);
        this.togglLogoBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
        this.togglLogoBox.TabIndex = 8;
        this.togglLogoBox.TabStop = false;
        // 
        // passwordPlaceholder
        // 
        this.passwordPlaceholder.Anchor = System.Windows.Forms.AnchorStyles.Top;
        this.passwordPlaceholder.BackColor = System.Drawing.Color.White;
        this.passwordPlaceholder.Cursor = System.Windows.Forms.Cursors.IBeam;
        this.passwordPlaceholder.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
        this.passwordPlaceholder.ForeColor = System.Drawing.SystemColors.ControlDark;
        this.passwordPlaceholder.Location = new System.Drawing.Point(30, 194);
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
        this.usernamePlaceholder.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
        this.usernamePlaceholder.ForeColor = System.Drawing.SystemColors.ControlDark;
        this.usernamePlaceholder.Location = new System.Drawing.Point(30, 137);
        this.usernamePlaceholder.Name = "usernamePlaceholder";
        this.usernamePlaceholder.Size = new System.Drawing.Size(202, 18);
        this.usernamePlaceholder.TabIndex = 12;
        this.usernamePlaceholder.Text = "Your email address";
        this.usernamePlaceholder.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
        this.usernamePlaceholder.Click += new System.EventHandler(this.usernamePlaceholder_Click);
        // 
        // loginSignupToggle
        // 
        this.loginSignupToggle.AccessibleRole = System.Windows.Forms.AccessibleRole.PushButton;
        this.loginSignupToggle.ActiveLinkColor = System.Drawing.Color.White;
        this.loginSignupToggle.Anchor = System.Windows.Forms.AnchorStyles.Top;
        this.loginSignupToggle.AutoSize = true;
        this.loginSignupToggle.BackColor = System.Drawing.Color.Transparent;
        this.loginSignupToggle.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
        this.loginSignupToggle.LinkBehavior = System.Windows.Forms.LinkBehavior.NeverUnderline;
        this.loginSignupToggle.LinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(227)))), ((int)(((byte)(227)))), ((int)(((byte)(227)))));
        this.loginSignupToggle.Location = new System.Drawing.Point(89, 416);
        this.loginSignupToggle.Name = "loginSignupToggle";
        this.loginSignupToggle.Size = new System.Drawing.Size(122, 17);
        this.loginSignupToggle.TabIndex = 4;
        this.loginSignupToggle.TabStop = true;
        this.loginSignupToggle.Text = "Sign up for free";
        this.loginSignupToggle.VisitedLinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(99)))), ((int)(((byte)(99)))), ((int)(((byte)(99)))));
        this.loginSignupToggle.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.loginSignupToggle_LinkClicked);
        // 
        // confirmButton
        // 
        this.confirmButton.Anchor = System.Windows.Forms.AnchorStyles.Top;
        this.confirmButton.FlatAppearance.BorderSize = 0;
        this.confirmButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
        this.confirmButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
        this.confirmButton.Location = new System.Drawing.Point(62, 265);
        this.confirmButton.Name = "confirmButton";
        this.confirmButton.Size = new System.Drawing.Size(177, 38);
        this.confirmButton.TabIndex = 2;
        this.confirmButton.Text = "Log in";
        this.confirmButton.UseVisualStyleBackColor = true;
        this.confirmButton.Click += new System.EventHandler(this.confirmButton_Click);
        // 
        // LoginViewController
        // 
        this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
        this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
        this.AutoSize = true;
        this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(69)))), ((int)(((byte)(69)))), ((int)(((byte)(69)))));
        this.BackgroundImage = global::TogglDesktop.Properties.Resources.timerDarkBg;
        this.Controls.Add(this.loginSignupToggleLabel);
        this.Controls.Add(this.confirmButton);
        this.Controls.Add(this.loginSignupToggle);
        this.Controls.Add(this.usernamePlaceholder);
        this.Controls.Add(this.passwordPlaceholder);
        this.Controls.Add(this.togglLogoBox);
        this.Controls.Add(this.passwordForgotTextField);
        this.Controls.Add(this.email);
        this.Controls.Add(this.password);
        this.Controls.Add(this.googleLoginTextField);
        this.MinimumSize = new System.Drawing.Size(279, 367);
        this.Name = "LoginViewController";
        this.Size = new System.Drawing.Size(300, 484);
        this.Load += new System.EventHandler(this.LoginViewController_Load);
        ((System.ComponentModel.ISupportInitialize)(this.togglLogoBox)).EndInit();
        this.ResumeLayout(false);
        this.PerformLayout();

    }

    #endregion

    private System.Windows.Forms.LinkLabel googleLoginTextField;
    private System.Windows.Forms.MaskedTextBox password;
    private System.Windows.Forms.TextBox email;
    private System.Windows.Forms.LinkLabel passwordForgotTextField;
    private System.Windows.Forms.PictureBox togglLogoBox;
    private System.Windows.Forms.Label passwordPlaceholder;
    private System.Windows.Forms.Label usernamePlaceholder;
    private System.Windows.Forms.LinkLabel loginSignupToggle;
    private System.Windows.Forms.Button confirmButton;
    private System.Windows.Forms.Label loginSignupToggleLabel;
}
}
