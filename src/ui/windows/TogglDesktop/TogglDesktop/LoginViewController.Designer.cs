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
            this.passwordForgotTextField = new System.Windows.Forms.LinkLabel();
            this.togglLogoBox = new System.Windows.Forms.PictureBox();
            this.loginSignupToggle = new System.Windows.Forms.LinkLabel();
            this.confirmButton = new System.Windows.Forms.Button();
            this.passwordBoxHost = new System.Windows.Forms.Integration.ElementHost();
            this.loginPasswordBox1 = new TogglDesktop.LoginPasswordBox();
            this.emailBoxHost = new System.Windows.Forms.Integration.ElementHost();
            this.loginTextBox1 = new TogglDesktop.LoginTextBox();
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
            this.togglLogoBox.Location = new System.Drawing.Point(90, 57);
            this.togglLogoBox.Name = "togglLogoBox";
            this.togglLogoBox.Size = new System.Drawing.Size(120, 36);
            this.togglLogoBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.togglLogoBox.TabIndex = 8;
            this.togglLogoBox.TabStop = false;
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
            // passwordBoxHost
            // 
            this.passwordBoxHost.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.passwordBoxHost.Location = new System.Drawing.Point(26, 174);
            this.passwordBoxHost.Name = "passwordBoxHost";
            this.passwordBoxHost.Size = new System.Drawing.Size(249, 43);
            this.passwordBoxHost.TabIndex = 1;
            this.passwordBoxHost.Child = this.loginPasswordBox1;
            // 
            // emailBoxHost
            // 
            this.emailBoxHost.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.emailBoxHost.Location = new System.Drawing.Point(26, 114);
            this.emailBoxHost.Name = "emailBoxHost";
            this.emailBoxHost.Size = new System.Drawing.Size(249, 43);
            this.emailBoxHost.TabIndex = 0;
            this.emailBoxHost.Child = this.loginTextBox1;
            // 
            // LoginViewController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(69)))), ((int)(((byte)(69)))), ((int)(((byte)(69)))));
            this.BackgroundImage = global::TogglDesktop.Properties.Resources.timerDarkBg;
            this.Controls.Add(this.passwordBoxHost);
            this.Controls.Add(this.emailBoxHost);
            this.Controls.Add(this.loginSignupToggleLabel);
            this.Controls.Add(this.confirmButton);
            this.Controls.Add(this.loginSignupToggle);
            this.Controls.Add(this.togglLogoBox);
            this.Controls.Add(this.passwordForgotTextField);
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
    private System.Windows.Forms.LinkLabel passwordForgotTextField;
    private System.Windows.Forms.PictureBox togglLogoBox;
    private System.Windows.Forms.LinkLabel loginSignupToggle;
    private System.Windows.Forms.Button confirmButton;
    private System.Windows.Forms.Label loginSignupToggleLabel;
    private System.Windows.Forms.Integration.ElementHost emailBoxHost;
    private LoginTextBox loginTextBox1;
    private System.Windows.Forms.Integration.ElementHost passwordBoxHost;
    private LoginPasswordBox loginPasswordBox1;
}
}
