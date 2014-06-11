namespace TogglDesktop
{
    partial class TimeEntryListViewController
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
            this.panelHeader = new System.Windows.Forms.Panel();
            this.EntriesList = new System.Windows.Forms.Panel();
            this.timerEditViewController = new TogglDesktop.TimerEditViewController();
            this.panelHeader.SuspendLayout();
            this.SuspendLayout();
            // 
            // panelHeader
            // 
            this.panelHeader.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panelHeader.Controls.Add(this.timerEditViewController);
            this.panelHeader.Location = new System.Drawing.Point(0, 0);
            this.panelHeader.Name = "panelHeader";
            this.panelHeader.Size = new System.Drawing.Size(314, 56);
            this.panelHeader.TabIndex = 0;
            // 
            // EntriesList
            // 
            this.EntriesList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.EntriesList.AutoScroll = true;
            this.EntriesList.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(204)))), ((int)(((byte)(204)))), ((int)(((byte)(204)))));
            this.EntriesList.Location = new System.Drawing.Point(0, 56);
            this.EntriesList.Name = "EntriesList";
            this.EntriesList.Size = new System.Drawing.Size(314, 295);
            this.EntriesList.TabIndex = 1;
            // 
            // timerEditViewController
            // 
            this.timerEditViewController.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.timerEditViewController.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(45)))), ((int)(((byte)(45)))), ((int)(((byte)(45)))));
            this.timerEditViewController.Location = new System.Drawing.Point(0, 0);
            this.timerEditViewController.Margin = new System.Windows.Forms.Padding(0);
            this.timerEditViewController.Name = "timerEditViewController";
            this.timerEditViewController.Size = new System.Drawing.Size(314, 56);
            this.timerEditViewController.TabIndex = 0;
            // 
            // TimeEntryListViewController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.EntriesList);
            this.Controls.Add(this.panelHeader);
            this.Name = "TimeEntryListViewController";
            this.Size = new System.Drawing.Size(317, 359);
            this.Load += new System.EventHandler(this.TimeEntryListViewController_Load);
            this.panelHeader.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panelHeader;
        private System.Windows.Forms.Panel EntriesList;
        private TimerEditViewController timerEditViewController;
    }
}
