namespace TogglDesktop
{
    partial class PreferencesWindowController
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

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(PreferencesWindowController));
            this.checkBoxUseProxy = new System.Windows.Forms.CheckBox();
            this.groupBoxProxySettings = new System.Windows.Forms.GroupBox();
            this.textBoxProxyPassword = new System.Windows.Forms.TextBox();
            this.textBoxProxyUsername = new System.Windows.Forms.TextBox();
            this.textBoxProxyPort = new System.Windows.Forms.TextBox();
            this.textBoxProxyHost = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.checkBoxIdleDetection = new System.Windows.Forms.CheckBox();
            this.checkBoxRecordTimeline = new System.Windows.Forms.CheckBox();
            this.checkBoxRemindToTrackTime = new System.Windows.Forms.CheckBox();
            this.checkBoxOnTop = new System.Windows.Forms.CheckBox();
            this.buttonSave = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.panelMain = new System.Windows.Forms.Panel();
            this.groupBoxProxySettings.SuspendLayout();
            this.panelMain.SuspendLayout();
            this.SuspendLayout();
            // 
            // checkBoxUseProxy
            // 
            this.checkBoxUseProxy.AutoSize = true;
            this.checkBoxUseProxy.Location = new System.Drawing.Point(15, 16);
            this.checkBoxUseProxy.Name = "checkBoxUseProxy";
            this.checkBoxUseProxy.Size = new System.Drawing.Size(169, 17);
            this.checkBoxUseProxy.TabIndex = 0;
            this.checkBoxUseProxy.Text = "Use proxy to connect to Toggl";
            this.checkBoxUseProxy.UseVisualStyleBackColor = true;
            this.checkBoxUseProxy.CheckedChanged += new System.EventHandler(this.checkBoxUseProxy_CheckedChanged);
            // 
            // groupBoxProxySettings
            // 
            this.groupBoxProxySettings.Controls.Add(this.textBoxProxyPassword);
            this.groupBoxProxySettings.Controls.Add(this.textBoxProxyUsername);
            this.groupBoxProxySettings.Controls.Add(this.textBoxProxyPort);
            this.groupBoxProxySettings.Controls.Add(this.textBoxProxyHost);
            this.groupBoxProxySettings.Controls.Add(this.label4);
            this.groupBoxProxySettings.Controls.Add(this.label3);
            this.groupBoxProxySettings.Controls.Add(this.label2);
            this.groupBoxProxySettings.Controls.Add(this.label1);
            this.groupBoxProxySettings.Location = new System.Drawing.Point(15, 39);
            this.groupBoxProxySettings.Name = "groupBoxProxySettings";
            this.groupBoxProxySettings.Size = new System.Drawing.Size(252, 143);
            this.groupBoxProxySettings.TabIndex = 1;
            this.groupBoxProxySettings.TabStop = false;
            this.groupBoxProxySettings.Text = "Proxy Settings";
            // 
            // textBoxProxyPassword
            // 
            this.textBoxProxyPassword.AccessibleName = "Proxy password";
            this.textBoxProxyPassword.Location = new System.Drawing.Point(81, 108);
            this.textBoxProxyPassword.Name = "textBoxProxyPassword";
            this.textBoxProxyPassword.PasswordChar = '*';
            this.textBoxProxyPassword.Size = new System.Drawing.Size(156, 20);
            this.textBoxProxyPassword.TabIndex = 7;
            // 
            // textBoxProxyUsername
            // 
            this.textBoxProxyUsername.AccessibleName = "Proxy username";
            this.textBoxProxyUsername.Location = new System.Drawing.Point(81, 80);
            this.textBoxProxyUsername.Name = "textBoxProxyUsername";
            this.textBoxProxyUsername.Size = new System.Drawing.Size(156, 20);
            this.textBoxProxyUsername.TabIndex = 6;
            // 
            // textBoxProxyPort
            // 
            this.textBoxProxyPort.AccessibleName = "Proxy port";
            this.textBoxProxyPort.Location = new System.Drawing.Point(81, 54);
            this.textBoxProxyPort.Name = "textBoxProxyPort";
            this.textBoxProxyPort.Size = new System.Drawing.Size(156, 20);
            this.textBoxProxyPort.TabIndex = 5;
            // 
            // textBoxProxyHost
            // 
            this.textBoxProxyHost.AccessibleName = "Proxy host";
            this.textBoxProxyHost.Location = new System.Drawing.Point(81, 28);
            this.textBoxProxyHost.Name = "textBoxProxyHost";
            this.textBoxProxyHost.Size = new System.Drawing.Size(156, 20);
            this.textBoxProxyHost.TabIndex = 4;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 108);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(53, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "Password";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(6, 80);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(55, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "Username";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 54);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(26, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Port";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 28);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(29, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Host";
            // 
            // checkBoxIdleDetection
            // 
            this.checkBoxIdleDetection.AutoSize = true;
            this.checkBoxIdleDetection.Location = new System.Drawing.Point(15, 188);
            this.checkBoxIdleDetection.Name = "checkBoxIdleDetection";
            this.checkBoxIdleDetection.Size = new System.Drawing.Size(90, 17);
            this.checkBoxIdleDetection.TabIndex = 2;
            this.checkBoxIdleDetection.Text = "Idle detection";
            this.checkBoxIdleDetection.UseVisualStyleBackColor = true;
            // 
            // checkBoxRecordTimeline
            // 
            this.checkBoxRecordTimeline.AutoSize = true;
            this.checkBoxRecordTimeline.Location = new System.Drawing.Point(15, 211);
            this.checkBoxRecordTimeline.Name = "checkBoxRecordTimeline";
            this.checkBoxRecordTimeline.Size = new System.Drawing.Size(99, 17);
            this.checkBoxRecordTimeline.TabIndex = 3;
            this.checkBoxRecordTimeline.Text = "Record timeline";
            this.checkBoxRecordTimeline.UseVisualStyleBackColor = true;
            // 
            // checkBoxRemindToTrackTime
            // 
            this.checkBoxRemindToTrackTime.AutoSize = true;
            this.checkBoxRemindToTrackTime.Checked = true;
            this.checkBoxRemindToTrackTime.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxRemindToTrackTime.Location = new System.Drawing.Point(15, 257);
            this.checkBoxRemindToTrackTime.Name = "checkBoxRemindToTrackTime";
            this.checkBoxRemindToTrackTime.Size = new System.Drawing.Size(123, 17);
            this.checkBoxRemindToTrackTime.TabIndex = 4;
            this.checkBoxRemindToTrackTime.Text = "Remind to track time";
            this.checkBoxRemindToTrackTime.UseVisualStyleBackColor = true;
            // 
            // checkBoxOnTop
            // 
            this.checkBoxOnTop.AutoSize = true;
            this.checkBoxOnTop.Location = new System.Drawing.Point(15, 234);
            this.checkBoxOnTop.Name = "checkBoxOnTop";
            this.checkBoxOnTop.Size = new System.Drawing.Size(141, 17);
            this.checkBoxOnTop.TabIndex = 6;
            this.checkBoxOnTop.Text = "On top of other windows";
            this.checkBoxOnTop.UseVisualStyleBackColor = true;
            // 
            // buttonSave
            // 
            this.buttonSave.Location = new System.Drawing.Point(15, 292);
            this.buttonSave.Name = "buttonSave";
            this.buttonSave.Size = new System.Drawing.Size(75, 23);
            this.buttonSave.TabIndex = 7;
            this.buttonSave.Text = "Save";
            this.buttonSave.UseVisualStyleBackColor = true;
            this.buttonSave.Click += new System.EventHandler(this.buttonSave_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(96, 292);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 8;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // panelMain
            // 
            this.panelMain.Controls.Add(this.groupBoxProxySettings);
            this.panelMain.Controls.Add(this.buttonCancel);
            this.panelMain.Controls.Add(this.checkBoxUseProxy);
            this.panelMain.Controls.Add(this.buttonSave);
            this.panelMain.Controls.Add(this.checkBoxIdleDetection);
            this.panelMain.Controls.Add(this.checkBoxOnTop);
            this.panelMain.Controls.Add(this.checkBoxRecordTimeline);
            this.panelMain.Controls.Add(this.checkBoxRemindToTrackTime);
            this.panelMain.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panelMain.Location = new System.Drawing.Point(0, 0);
            this.panelMain.Name = "panelMain";
            this.panelMain.Size = new System.Drawing.Size(300, 333);
            this.panelMain.TabIndex = 9;
            // 
            // PreferencesWindowController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(300, 333);
            this.Controls.Add(this.panelMain);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(300, 363);
            this.MinimumSize = new System.Drawing.Size(300, 363);
            this.Name = "PreferencesWindowController";
            this.Text = "Preferences";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.PreferencesWindowController_FormClosing);
            this.groupBoxProxySettings.ResumeLayout(false);
            this.groupBoxProxySettings.PerformLayout();
            this.panelMain.ResumeLayout(false);
            this.panelMain.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.CheckBox checkBoxUseProxy;
        private System.Windows.Forms.GroupBox groupBoxProxySettings;
        private System.Windows.Forms.TextBox textBoxProxyPassword;
        private System.Windows.Forms.TextBox textBoxProxyUsername;
        private System.Windows.Forms.TextBox textBoxProxyPort;
        private System.Windows.Forms.TextBox textBoxProxyHost;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.CheckBox checkBoxIdleDetection;
        private System.Windows.Forms.CheckBox checkBoxRecordTimeline;
        private System.Windows.Forms.CheckBox checkBoxRemindToTrackTime;
        private System.Windows.Forms.CheckBox checkBoxOnTop;
        private System.Windows.Forms.Button buttonSave;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Panel panelMain;
    }
}