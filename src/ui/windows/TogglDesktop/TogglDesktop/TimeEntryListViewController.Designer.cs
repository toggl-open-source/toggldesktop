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
            this.timerEditViewController = new TogglDesktop.TimerEditViewController();
            this.entries = new System.Windows.Forms.FlowLayoutPanel();
            this.panelHeader.SuspendLayout();
            this.SuspendLayout();
            // 
            // panelHeader
            // 
            this.panelHeader.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panelHeader.Controls.Add(this.timerEditViewController);
            this.panelHeader.Location = new System.Drawing.Point(0, 0);
            this.panelHeader.Margin = new System.Windows.Forms.Padding(0);
            this.panelHeader.Name = "panelHeader";
            this.panelHeader.Size = new System.Drawing.Size(314, 56);
            this.panelHeader.TabIndex = 0;
            // 
            // timerEditViewController
            // 
            this.timerEditViewController.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.timerEditViewController.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(59)))), ((int)(((byte)(59)))), ((int)(((byte)(59)))));
            this.timerEditViewController.Location = new System.Drawing.Point(0, 0);
            this.timerEditViewController.Margin = new System.Windows.Forms.Padding(0);
            this.timerEditViewController.Name = "timerEditViewController";
            this.timerEditViewController.Size = new System.Drawing.Size(314, 56);
            this.timerEditViewController.TabIndex = 0;
            // 
            // entries
            // 
            this.entries.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.entries.AutoScroll = true;
            this.entries.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
            this.entries.Location = new System.Drawing.Point(0, 59);
            this.entries.Name = "entries";
            this.entries.Size = new System.Drawing.Size(311, 292);
            this.entries.TabIndex = 1;
            this.entries.WrapContents = false;
            // 
            // TimeEntryListViewController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.BackColor = System.Drawing.Color.WhiteSmoke;
            this.Controls.Add(this.entries);
            this.Controls.Add(this.panelHeader);
            this.Margin = new System.Windows.Forms.Padding(0);
            this.Name = "TimeEntryListViewController";
            this.Size = new System.Drawing.Size(314, 351);
            this.Load += new System.EventHandler(this.TimeEntryListViewController_Load);
            this.panelHeader.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panelHeader;
        private TimerEditViewController timerEditViewController;
        private System.Windows.Forms.FlowLayoutPanel entries;
    }
}
