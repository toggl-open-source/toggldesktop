namespace TogglDesktop
{
    partial class AboutWindowController
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AboutWindowController));
            this.pictureBoxLogo = new System.Windows.Forms.PictureBox();
            this.labelAppName = new System.Windows.Forms.Label();
            this.labelVersion = new System.Windows.Forms.Label();
            this.richTextBoxCredits = new System.Windows.Forms.RichTextBox();
            this.buttonCheckingForUpdate = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.comboBoxChannel = new System.Windows.Forms.ComboBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxLogo)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBoxLogo
            // 
            this.pictureBoxLogo.Location = new System.Drawing.Point(94, 12);
            this.pictureBoxLogo.Name = "pictureBoxLogo";
            this.pictureBoxLogo.Size = new System.Drawing.Size(100, 68);
            this.pictureBoxLogo.TabIndex = 0;
            this.pictureBoxLogo.TabStop = false;
            // 
            // labelAppName
            // 
            this.labelAppName.AutoSize = true;
            this.labelAppName.Location = new System.Drawing.Point(108, 83);
            this.labelAppName.Name = "labelAppName";
            this.labelAppName.Size = new System.Drawing.Size(74, 13);
            this.labelAppName.TabIndex = 1;
            this.labelAppName.Text = "TogglDesktop";
            this.labelAppName.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // labelVersion
            // 
            this.labelVersion.AutoSize = true;
            this.labelVersion.Location = new System.Drawing.Point(108, 108);
            this.labelVersion.Name = "labelVersion";
            this.labelVersion.Size = new System.Drawing.Size(69, 13);
            this.labelVersion.TabIndex = 2;
            this.labelVersion.Text = "Version 1.0.0";
            // 
            // richTextBoxCredits
            // 
            this.richTextBoxCredits.Enabled = false;
            this.richTextBoxCredits.Location = new System.Drawing.Point(12, 139);
            this.richTextBoxCredits.Name = "richTextBoxCredits";
            this.richTextBoxCredits.Size = new System.Drawing.Size(260, 71);
            this.richTextBoxCredits.TabIndex = 3;
            this.richTextBoxCredits.Text = "";
            // 
            // buttonCheckingForUpdate
            // 
            this.buttonCheckingForUpdate.Location = new System.Drawing.Point(12, 216);
            this.buttonCheckingForUpdate.Name = "buttonCheckingForUpdate";
            this.buttonCheckingForUpdate.Size = new System.Drawing.Size(260, 23);
            this.buttonCheckingForUpdate.TabIndex = 4;
            this.buttonCheckingForUpdate.Text = "Checking for update...";
            this.buttonCheckingForUpdate.UseVisualStyleBackColor = true;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 253);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(87, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "Release channel";
            // 
            // comboBoxChannel
            // 
            this.comboBoxChannel.FormattingEnabled = true;
            this.comboBoxChannel.Items.AddRange(new object[] {
            "stable",
            "beta",
            "dev"});
            this.comboBoxChannel.Location = new System.Drawing.Point(151, 253);
            this.comboBoxChannel.Name = "comboBoxChannel";
            this.comboBoxChannel.Size = new System.Drawing.Size(121, 21);
            this.comboBoxChannel.TabIndex = 6;
            // 
            // AboutWindowController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 290);
            this.Controls.Add(this.comboBoxChannel);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.buttonCheckingForUpdate);
            this.Controls.Add(this.richTextBoxCredits);
            this.Controls.Add(this.labelVersion);
            this.Controls.Add(this.labelAppName);
            this.Controls.Add(this.pictureBoxLogo);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "AboutWindowController";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.AboutWindowController_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxLogo)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBoxLogo;
        private System.Windows.Forms.Label labelAppName;
        private System.Windows.Forms.Label labelVersion;
        private System.Windows.Forms.RichTextBox richTextBoxCredits;
        private System.Windows.Forms.Button buttonCheckingForUpdate;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox comboBoxChannel;
    }
}