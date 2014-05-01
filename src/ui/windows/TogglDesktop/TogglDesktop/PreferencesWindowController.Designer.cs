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
            this.textBox4 = new System.Windows.Forms.TextBox();
            this.textBox3 = new System.Windows.Forms.TextBox();
            this.textBox2 = new System.Windows.Forms.TextBox();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.checkBoxIdleDetection = new System.Windows.Forms.CheckBox();
            this.checkBoxRecordTimeline = new System.Windows.Forms.CheckBox();
            this.checkBoxRemindToTrackTime = new System.Windows.Forms.CheckBox();
            this.groupBoxProxySettings.SuspendLayout();
            this.SuspendLayout();
            // 
            // checkBoxUseProxy
            // 
            this.checkBoxUseProxy.AutoSize = true;
            this.checkBoxUseProxy.Location = new System.Drawing.Point(13, 24);
            this.checkBoxUseProxy.Name = "checkBoxUseProxy";
            this.checkBoxUseProxy.Size = new System.Drawing.Size(169, 17);
            this.checkBoxUseProxy.TabIndex = 0;
            this.checkBoxUseProxy.Text = "Use proxy to connect to Toggl";
            this.checkBoxUseProxy.UseVisualStyleBackColor = true;
            // 
            // groupBoxProxySettings
            // 
            this.groupBoxProxySettings.Controls.Add(this.textBox4);
            this.groupBoxProxySettings.Controls.Add(this.textBox3);
            this.groupBoxProxySettings.Controls.Add(this.textBox2);
            this.groupBoxProxySettings.Controls.Add(this.textBox1);
            this.groupBoxProxySettings.Controls.Add(this.label4);
            this.groupBoxProxySettings.Controls.Add(this.label3);
            this.groupBoxProxySettings.Controls.Add(this.label2);
            this.groupBoxProxySettings.Controls.Add(this.label1);
            this.groupBoxProxySettings.Location = new System.Drawing.Point(13, 47);
            this.groupBoxProxySettings.Name = "groupBoxProxySettings";
            this.groupBoxProxySettings.Size = new System.Drawing.Size(252, 143);
            this.groupBoxProxySettings.TabIndex = 1;
            this.groupBoxProxySettings.TabStop = false;
            this.groupBoxProxySettings.Text = "Proxy Settings";
            // 
            // textBox4
            // 
            this.textBox4.Location = new System.Drawing.Point(81, 108);
            this.textBox4.Name = "textBox4";
            this.textBox4.Size = new System.Drawing.Size(156, 20);
            this.textBox4.TabIndex = 7;
            // 
            // textBox3
            // 
            this.textBox3.Location = new System.Drawing.Point(81, 80);
            this.textBox3.Name = "textBox3";
            this.textBox3.Size = new System.Drawing.Size(156, 20);
            this.textBox3.TabIndex = 6;
            // 
            // textBox2
            // 
            this.textBox2.Location = new System.Drawing.Point(81, 54);
            this.textBox2.Name = "textBox2";
            this.textBox2.Size = new System.Drawing.Size(156, 20);
            this.textBox2.TabIndex = 5;
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(81, 28);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(156, 20);
            this.textBox1.TabIndex = 4;
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
            this.label3.Size = new System.Drawing.Size(53, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "Usernane";
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
            this.checkBoxIdleDetection.Location = new System.Drawing.Point(13, 196);
            this.checkBoxIdleDetection.Name = "checkBoxIdleDetection";
            this.checkBoxIdleDetection.Size = new System.Drawing.Size(90, 17);
            this.checkBoxIdleDetection.TabIndex = 2;
            this.checkBoxIdleDetection.Text = "Idle detection";
            this.checkBoxIdleDetection.UseVisualStyleBackColor = true;
            // 
            // checkBoxRecordTimeline
            // 
            this.checkBoxRecordTimeline.AutoSize = true;
            this.checkBoxRecordTimeline.Location = new System.Drawing.Point(13, 219);
            this.checkBoxRecordTimeline.Name = "checkBoxRecordTimeline";
            this.checkBoxRecordTimeline.Size = new System.Drawing.Size(99, 17);
            this.checkBoxRecordTimeline.TabIndex = 3;
            this.checkBoxRecordTimeline.Text = "Record timeline";
            this.checkBoxRecordTimeline.UseVisualStyleBackColor = true;
            // 
            // checkBoxRemindToTrackTime
            // 
            this.checkBoxRemindToTrackTime.AutoSize = true;
            this.checkBoxRemindToTrackTime.Location = new System.Drawing.Point(13, 242);
            this.checkBoxRemindToTrackTime.Name = "checkBoxRemindToTrackTime";
            this.checkBoxRemindToTrackTime.Size = new System.Drawing.Size(123, 17);
            this.checkBoxRemindToTrackTime.TabIndex = 4;
            this.checkBoxRemindToTrackTime.Text = "Remind to track time";
            this.checkBoxRemindToTrackTime.UseVisualStyleBackColor = true;
            // 
            // PreferencesWindowController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 266);
            this.Controls.Add(this.checkBoxRemindToTrackTime);
            this.Controls.Add(this.checkBoxRecordTimeline);
            this.Controls.Add(this.checkBoxIdleDetection);
            this.Controls.Add(this.groupBoxProxySettings);
            this.Controls.Add(this.checkBoxUseProxy);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "PreferencesWindowController";
            this.Text = "Preferences";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.PreferencesWindowController_FormClosing);
            this.groupBoxProxySettings.ResumeLayout(false);
            this.groupBoxProxySettings.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox checkBoxUseProxy;
        private System.Windows.Forms.GroupBox groupBoxProxySettings;
        private System.Windows.Forms.TextBox textBox4;
        private System.Windows.Forms.TextBox textBox3;
        private System.Windows.Forms.TextBox textBox2;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.CheckBox checkBoxIdleDetection;
        private System.Windows.Forms.CheckBox checkBoxRecordTimeline;
        private System.Windows.Forms.CheckBox checkBoxRemindToTrackTime;
    }
}