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
            this.emptyLabel = new System.Windows.Forms.Label();
            this.entriesHost = new System.Windows.Forms.Integration.ElementHost();
            this.entries = new TogglDesktop.WPF.TimeEntryList();
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
            // emptyLabel
            // 
            this.emptyLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.emptyLabel.BackColor = System.Drawing.Color.Transparent;
            this.emptyLabel.Cursor = System.Windows.Forms.Cursors.Hand;
            this.emptyLabel.Font = new System.Drawing.Font("Lucida Sans Unicode", 14F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(0)));
            this.emptyLabel.ForeColor = System.Drawing.Color.Gray;
            this.emptyLabel.Location = new System.Drawing.Point(0, 56);
            this.emptyLabel.Margin = new System.Windows.Forms.Padding(0);
            this.emptyLabel.Name = "emptyLabel";
            this.emptyLabel.Padding = new System.Windows.Forms.Padding(10, 0, 10, 0);
            this.emptyLabel.Size = new System.Drawing.Size(314, 150);
            this.emptyLabel.TabIndex = 2;
            this.emptyLabel.Text = "Welcome, let\'s start tracking! If you have previous entries, click here to see th" +
    "em in reports";
            this.emptyLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.emptyLabel.Visible = false;
            this.emptyLabel.Click += new System.EventHandler(this.emptyLabel_Click);
            // 
            // entriesHost
            // 
            this.entriesHost.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.entriesHost.Location = new System.Drawing.Point(0, 56);
            this.entriesHost.Name = "entriesHost";
            this.entriesHost.Size = new System.Drawing.Size(314, 295);
            this.entriesHost.TabIndex = 3;
            this.entriesHost.Text = "elementHost1";
            this.entriesHost.Child = this.entries;
            // 
            // TimeEntryListViewController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.WhiteSmoke;
            this.Controls.Add(this.emptyLabel);
            this.Controls.Add(this.entriesHost);
            this.Controls.Add(this.panelHeader);
            this.Margin = new System.Windows.Forms.Padding(0);
            this.Name = "TimeEntryListViewController";
            this.Size = new System.Drawing.Size(314, 351);
            this.panelHeader.ResumeLayout(false);
            this.ResumeLayout(false);

    }

    #endregion

    private System.Windows.Forms.Panel panelHeader;
    private TimerEditViewController timerEditViewController;
    private System.Windows.Forms.Label emptyLabel;
    private System.Windows.Forms.Integration.ElementHost entriesHost;
    private WPF.TimeEntryList entries;
}
}
