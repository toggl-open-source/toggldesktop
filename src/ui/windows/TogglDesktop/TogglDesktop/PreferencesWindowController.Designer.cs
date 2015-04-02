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
            this.checkBoxUseSystemProxySettings = new System.Windows.Forms.CheckBox();
            this.textBoxReminderMinutes = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.btnClearStartStopTimer = new System.Windows.Forms.Button();
            this.btnRecordStartStopShortcut = new System.Windows.Forms.Button();
            this.btnClearShowHideShortcut = new System.Windows.Forms.Button();
            this.btnRecordShowHideShortcut = new System.Windows.Forms.Button();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.textBoxIdleMinutes = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.groupBoxProxySettings.SuspendLayout();
            this.panelMain.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // checkBoxUseProxy
            // 
            this.checkBoxUseProxy.AutoSize = true;
            this.checkBoxUseProxy.Location = new System.Drawing.Point(12, 37);
            this.checkBoxUseProxy.Name = "checkBoxUseProxy";
            this.checkBoxUseProxy.Size = new System.Drawing.Size(169, 17);
            this.checkBoxUseProxy.TabIndex = 1;
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
            this.groupBoxProxySettings.Location = new System.Drawing.Point(12, 60);
            this.groupBoxProxySettings.Name = "groupBoxProxySettings";
            this.groupBoxProxySettings.Size = new System.Drawing.Size(252, 143);
            this.groupBoxProxySettings.TabIndex = 2;
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
            this.checkBoxIdleDetection.Location = new System.Drawing.Point(12, 209);
            this.checkBoxIdleDetection.Name = "checkBoxIdleDetection";
            this.checkBoxIdleDetection.Size = new System.Drawing.Size(90, 17);
            this.checkBoxIdleDetection.TabIndex = 3;
            this.checkBoxIdleDetection.Text = "Idle detection";
            this.checkBoxIdleDetection.UseVisualStyleBackColor = true;
            this.checkBoxIdleDetection.CheckedChanged += new System.EventHandler(this.checkBoxIdleDetection_CheckedChanged);
            // 
            // checkBoxRecordTimeline
            // 
            this.checkBoxRecordTimeline.AutoSize = true;
            this.checkBoxRecordTimeline.Location = new System.Drawing.Point(12, 232);
            this.checkBoxRecordTimeline.Name = "checkBoxRecordTimeline";
            this.checkBoxRecordTimeline.Size = new System.Drawing.Size(99, 17);
            this.checkBoxRecordTimeline.TabIndex = 5;
            this.checkBoxRecordTimeline.Text = "Record timeline";
            this.checkBoxRecordTimeline.UseVisualStyleBackColor = true;
            // 
            // checkBoxRemindToTrackTime
            // 
            this.checkBoxRemindToTrackTime.AutoSize = true;
            this.checkBoxRemindToTrackTime.Location = new System.Drawing.Point(12, 278);
            this.checkBoxRemindToTrackTime.Name = "checkBoxRemindToTrackTime";
            this.checkBoxRemindToTrackTime.Size = new System.Drawing.Size(123, 17);
            this.checkBoxRemindToTrackTime.TabIndex = 7;
            this.checkBoxRemindToTrackTime.Text = "Remind to track time";
            this.checkBoxRemindToTrackTime.UseVisualStyleBackColor = true;
            this.checkBoxRemindToTrackTime.CheckedChanged += new System.EventHandler(this.checkBoxRemindToTrackTime_CheckedChanged);
            // 
            // checkBoxOnTop
            // 
            this.checkBoxOnTop.AutoSize = true;
            this.checkBoxOnTop.Location = new System.Drawing.Point(12, 255);
            this.checkBoxOnTop.Name = "checkBoxOnTop";
            this.checkBoxOnTop.Size = new System.Drawing.Size(141, 17);
            this.checkBoxOnTop.TabIndex = 6;
            this.checkBoxOnTop.Text = "On top of other windows";
            this.checkBoxOnTop.UseVisualStyleBackColor = true;
            // 
            // buttonSave
            // 
            this.buttonSave.Location = new System.Drawing.Point(12, 383);
            this.buttonSave.Name = "buttonSave";
            this.buttonSave.Size = new System.Drawing.Size(75, 23);
            this.buttonSave.TabIndex = 11;
            this.buttonSave.Text = "Save";
            this.buttonSave.UseVisualStyleBackColor = true;
            this.buttonSave.Click += new System.EventHandler(this.buttonSave_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(93, 383);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 12;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // panelMain
            // 
            this.panelMain.Controls.Add(this.checkBoxUseSystemProxySettings);
            this.panelMain.Controls.Add(this.textBoxReminderMinutes);
            this.panelMain.Controls.Add(this.label8);
            this.panelMain.Controls.Add(this.groupBox1);
            this.panelMain.Controls.Add(this.textBoxIdleMinutes);
            this.panelMain.Controls.Add(this.label5);
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
            this.panelMain.Size = new System.Drawing.Size(280, 420);
            this.panelMain.TabIndex = 6;
            // 
            // checkBoxUseSystemProxySettings
            // 
            this.checkBoxUseSystemProxySettings.AutoSize = true;
            this.checkBoxUseSystemProxySettings.Location = new System.Drawing.Point(12, 14);
            this.checkBoxUseSystemProxySettings.Name = "checkBoxUseSystemProxySettings";
            this.checkBoxUseSystemProxySettings.Size = new System.Drawing.Size(147, 17);
            this.checkBoxUseSystemProxySettings.TabIndex = 0;
            this.checkBoxUseSystemProxySettings.Text = "Use system proxy settings";
            this.checkBoxUseSystemProxySettings.UseVisualStyleBackColor = true;
            this.checkBoxUseSystemProxySettings.CheckedChanged += new System.EventHandler(this.checkBoxUseSystemProxySettings_CheckedChanged);
            // 
            // textBoxReminderMinutes
            // 
            this.textBoxReminderMinutes.Location = new System.Drawing.Point(164, 276);
            this.textBoxReminderMinutes.Name = "textBoxReminderMinutes";
            this.textBoxReminderMinutes.Size = new System.Drawing.Size(31, 20);
            this.textBoxReminderMinutes.TabIndex = 8;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(201, 278);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(43, 13);
            this.label8.TabIndex = 9;
            this.label8.Text = "minutes";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.btnClearStartStopTimer);
            this.groupBox1.Controls.Add(this.btnRecordStartStopShortcut);
            this.groupBox1.Controls.Add(this.btnClearShowHideShortcut);
            this.groupBox1.Controls.Add(this.btnRecordShowHideShortcut);
            this.groupBox1.Controls.Add(this.label7);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Location = new System.Drawing.Point(12, 301);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(252, 76);
            this.groupBox1.TabIndex = 10;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Global shortcuts";
            // 
            // btnClearStartStopTimer
            // 
            this.btnClearStartStopTimer.Location = new System.Drawing.Point(227, 47);
            this.btnClearStartStopTimer.Name = "btnClearStartStopTimer";
            this.btnClearStartStopTimer.Size = new System.Drawing.Size(19, 23);
            this.btnClearStartStopTimer.TabIndex = 5;
            this.btnClearStartStopTimer.Text = "x";
            this.btnClearStartStopTimer.UseVisualStyleBackColor = true;
            this.btnClearStartStopTimer.Click += new System.EventHandler(this.btnClearStartStopTimer_Click);
            // 
            // btnRecordStartStopShortcut
            // 
            this.btnRecordStartStopShortcut.Location = new System.Drawing.Point(128, 47);
            this.btnRecordStartStopShortcut.Name = "btnRecordStartStopShortcut";
            this.btnRecordStartStopShortcut.Size = new System.Drawing.Size(101, 23);
            this.btnRecordStartStopShortcut.TabIndex = 4;
            this.btnRecordStartStopShortcut.Text = "Record Shortcut";
            this.btnRecordStartStopShortcut.UseVisualStyleBackColor = true;
            this.btnRecordStartStopShortcut.Click += new System.EventHandler(this.btnRecordStartStopShortcut_Click);
            this.btnRecordStartStopShortcut.KeyUp += new System.Windows.Forms.KeyEventHandler(this.btnRecordStartStopShortcut_KeyUp);
            // 
            // btnClearShowHideShortcut
            // 
            this.btnClearShowHideShortcut.Location = new System.Drawing.Point(227, 19);
            this.btnClearShowHideShortcut.Name = "btnClearShowHideShortcut";
            this.btnClearShowHideShortcut.Size = new System.Drawing.Size(19, 23);
            this.btnClearShowHideShortcut.TabIndex = 3;
            this.btnClearShowHideShortcut.Text = "x";
            this.btnClearShowHideShortcut.UseVisualStyleBackColor = true;
            this.btnClearShowHideShortcut.Click += new System.EventHandler(this.btnClearShowHideShortcut_Click);
            // 
            // btnRecordShowHideShortcut
            // 
            this.btnRecordShowHideShortcut.Location = new System.Drawing.Point(128, 19);
            this.btnRecordShowHideShortcut.Name = "btnRecordShowHideShortcut";
            this.btnRecordShowHideShortcut.Size = new System.Drawing.Size(101, 23);
            this.btnRecordShowHideShortcut.TabIndex = 2;
            this.btnRecordShowHideShortcut.Text = "Record Shortcut";
            this.btnRecordShowHideShortcut.UseVisualStyleBackColor = true;
            this.btnRecordShowHideShortcut.Click += new System.EventHandler(this.btnRecordShowHideShortcut_Click);
            this.btnRecordShowHideShortcut.KeyUp += new System.Windows.Forms.KeyEventHandler(this.btnRecordShowHideShortcut_KeyUp);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(6, 52);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(101, 13);
            this.label7.TabIndex = 1;
            this.label7.Text = "Continue/Stop timer";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(6, 25);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(91, 13);
            this.label6.TabIndex = 0;
            this.label6.Text = "Show/Hide Toggl";
            // 
            // textBoxIdleMinutes
            // 
            this.textBoxIdleMinutes.Location = new System.Drawing.Point(164, 207);
            this.textBoxIdleMinutes.Name = "textBoxIdleMinutes";
            this.textBoxIdleMinutes.Size = new System.Drawing.Size(31, 20);
            this.textBoxIdleMinutes.TabIndex = 4;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(201, 209);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(43, 13);
            this.label5.TabIndex = 4;
            this.label5.Text = "minutes";
            // 
            // PreferencesWindowController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(280, 420);
            this.Controls.Add(this.panelMain);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(280, 460);
            this.MinimumSize = new System.Drawing.Size(280, 460);
            this.Name = "PreferencesWindowController";
            this.Text = "Preferences";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.PreferencesWindowController_FormClosing);
            this.groupBoxProxySettings.ResumeLayout(false);
            this.groupBoxProxySettings.PerformLayout();
            this.panelMain.ResumeLayout(false);
            this.panelMain.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
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
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox textBoxIdleMinutes;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button btnClearStartStopTimer;
        private System.Windows.Forms.Button btnRecordStartStopShortcut;
        private System.Windows.Forms.Button btnClearShowHideShortcut;
        private System.Windows.Forms.Button btnRecordShowHideShortcut;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox textBoxReminderMinutes;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.CheckBox checkBoxUseSystemProxySettings;
    }
}