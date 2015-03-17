namespace TogglDesktop
{
    partial class IdleNotificationWindowController
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(IdleNotificationWindowController));
            this.labelIdleSince = new System.Windows.Forms.Label();
            this.labelIdleDuration = new System.Windows.Forms.Label();
            this.buttonKeepTime = new System.Windows.Forms.Button();
            this.buttonDiscardTime = new System.Windows.Forms.Button();
            this.panelMain = new System.Windows.Forms.Panel();
            this.labelDescription = new System.Windows.Forms.Label();
            this.runningLabel = new System.Windows.Forms.Label();
            this.buttonSplitIdleTimeIntoNewEntry = new System.Windows.Forms.Button();
            this.panelMain.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelIdleSince
            // 
            this.labelIdleSince.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelIdleSince.Font = new System.Drawing.Font("Lucida Sans Unicode", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelIdleSince.Location = new System.Drawing.Point(13, 15);
            this.labelIdleSince.Name = "labelIdleSince";
            this.labelIdleSince.Size = new System.Drawing.Size(198, 17);
            this.labelIdleSince.TabIndex = 0;
            this.labelIdleSince.Text = "You have been idle since 12:34:56";
            this.labelIdleSince.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // labelIdleDuration
            // 
            this.labelIdleDuration.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelIdleDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelIdleDuration.Location = new System.Drawing.Point(13, 40);
            this.labelIdleDuration.Name = "labelIdleDuration";
            this.labelIdleDuration.Size = new System.Drawing.Size(198, 14);
            this.labelIdleDuration.TabIndex = 1;
            this.labelIdleDuration.Text = "(5 minutes)";
            this.labelIdleDuration.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // buttonKeepTime
            // 
            this.buttonKeepTime.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonKeepTime.Font = new System.Drawing.Font("Lucida Sans Unicode", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.buttonKeepTime.Location = new System.Drawing.Point(13, 129);
            this.buttonKeepTime.Name = "buttonKeepTime";
            this.buttonKeepTime.Size = new System.Drawing.Size(198, 23);
            this.buttonKeepTime.TabIndex = 2;
            this.buttonKeepTime.Text = "Keep time";
            this.buttonKeepTime.UseVisualStyleBackColor = true;
            this.buttonKeepTime.Click += new System.EventHandler(this.buttonKeepTime_Click);
            // 
            // buttonDiscardTime
            // 
            this.buttonDiscardTime.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonDiscardTime.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonDiscardTime.Font = new System.Drawing.Font("Lucida Sans Unicode", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.buttonDiscardTime.Location = new System.Drawing.Point(13, 158);
            this.buttonDiscardTime.Name = "buttonDiscardTime";
            this.buttonDiscardTime.Size = new System.Drawing.Size(198, 23);
            this.buttonDiscardTime.TabIndex = 3;
            this.buttonDiscardTime.Text = "Discard time";
            this.buttonDiscardTime.UseVisualStyleBackColor = true;
            this.buttonDiscardTime.Click += new System.EventHandler(this.buttonDiscardTime_Click);
            // 
            // panelMain
            // 
            this.panelMain.Controls.Add(this.labelDescription);
            this.panelMain.Controls.Add(this.runningLabel);
            this.panelMain.Controls.Add(this.buttonSplitIdleTimeIntoNewEntry);
            this.panelMain.Controls.Add(this.labelIdleSince);
            this.panelMain.Controls.Add(this.buttonDiscardTime);
            this.panelMain.Controls.Add(this.labelIdleDuration);
            this.panelMain.Controls.Add(this.buttonKeepTime);
            this.panelMain.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panelMain.Location = new System.Drawing.Point(0, 0);
            this.panelMain.Name = "panelMain";
            this.panelMain.Size = new System.Drawing.Size(225, 220);
            this.panelMain.TabIndex = 4;
            // 
            // labelDescription
            // 
            this.labelDescription.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelDescription.AutoEllipsis = true;
            this.labelDescription.Font = new System.Drawing.Font("Lucida Sans Unicode", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelDescription.Location = new System.Drawing.Point(13, 95);
            this.labelDescription.Name = "labelDescription";
            this.labelDescription.Size = new System.Drawing.Size(198, 17);
            this.labelDescription.TabIndex = 6;
            this.labelDescription.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // runningLabel
            // 
            this.runningLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.runningLabel.Font = new System.Drawing.Font("Lucida Sans Unicode", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.runningLabel.Location = new System.Drawing.Point(10, 70);
            this.runningLabel.Name = "runningLabel";
            this.runningLabel.Size = new System.Drawing.Size(198, 17);
            this.runningLabel.TabIndex = 5;
            this.runningLabel.Text = "Running Time entry:";
            this.runningLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // buttonSplitIdleTimeIntoNewEntry
            // 
            this.buttonSplitIdleTimeIntoNewEntry.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonSplitIdleTimeIntoNewEntry.Location = new System.Drawing.Point(13, 188);
            this.buttonSplitIdleTimeIntoNewEntry.Name = "buttonSplitIdleTimeIntoNewEntry";
            this.buttonSplitIdleTimeIntoNewEntry.Size = new System.Drawing.Size(198, 23);
            this.buttonSplitIdleTimeIntoNewEntry.TabIndex = 4;
            this.buttonSplitIdleTimeIntoNewEntry.Text = "Add idle time as a new time entry";
            this.buttonSplitIdleTimeIntoNewEntry.UseVisualStyleBackColor = true;
            this.buttonSplitIdleTimeIntoNewEntry.Click += new System.EventHandler(this.buttonSplitIdleTimeIntoNewEntry_Click);
            // 
            // IdleNotificationWindowController
            // 
            this.AcceptButton = this.buttonKeepTime;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonDiscardTime;
            this.ClientSize = new System.Drawing.Size(225, 220);
            this.Controls.Add(this.panelMain);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(225, 260);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(225, 260);
            this.Name = "IdleNotificationWindowController";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Idle Notification";
            this.TopMost = true;
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.IdleNotificationWindowController_FormClosing);
            this.Load += new System.EventHandler(this.IdleNotificationWindowController_Load);
            this.panelMain.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Label labelIdleSince;
        private System.Windows.Forms.Label labelIdleDuration;
        private System.Windows.Forms.Button buttonKeepTime;
        private System.Windows.Forms.Button buttonDiscardTime;
        private System.Windows.Forms.Panel panelMain;
        private System.Windows.Forms.Button buttonSplitIdleTimeIntoNewEntry;
        private System.Windows.Forms.Label labelDescription;
        private System.Windows.Forms.Label runningLabel;
    }
}