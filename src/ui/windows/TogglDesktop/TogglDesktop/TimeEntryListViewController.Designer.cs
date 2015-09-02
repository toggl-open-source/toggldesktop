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
        this.emptyLabel = new System.Windows.Forms.Label();
        this.miniTimerHost = new System.Windows.Forms.Integration.ElementHost();
        this.timerEditViewController = new TogglDesktop.WPF.Timer();
        this.entriesHost = new System.Windows.Forms.Integration.ElementHost();
        this.entries = new TogglDesktop.WPF.TimeEntryList();
        this.SuspendLayout();
        //
        // emptyLabel
        //
        this.emptyLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                                  | System.Windows.Forms.AnchorStyles.Right)));
        this.emptyLabel.BackColor = System.Drawing.Color.Transparent;
        this.emptyLabel.Cursor = System.Windows.Forms.Cursors.Hand;
        this.emptyLabel.Font = new System.Drawing.Font("Lucida Sans Unicode", 14F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(0)));
        this.emptyLabel.ForeColor = System.Drawing.Color.Gray;
        this.emptyLabel.Location = new System.Drawing.Point(0, 60);
        this.emptyLabel.Margin = new System.Windows.Forms.Padding(0);
        this.emptyLabel.Name = "emptyLabel";
        this.emptyLabel.Padding = new System.Windows.Forms.Padding(10, 0, 10, 0);
        this.emptyLabel.Size = new System.Drawing.Size(464, 150);
        this.emptyLabel.TabIndex = 2;
        this.emptyLabel.Text = "Welcome, let\'s start tracking! If you have previous entries, click here to see th" +
                               "em in reports";
        this.emptyLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
        this.emptyLabel.Visible = false;
        this.emptyLabel.Click += new System.EventHandler(this.emptyLabel_Click);
        //
        // miniTimerHost
        //
        this.miniTimerHost.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                                     | System.Windows.Forms.AnchorStyles.Right)));
        this.miniTimerHost.Location = new System.Drawing.Point(0, 0);
        this.miniTimerHost.Margin = new System.Windows.Forms.Padding(0);
        this.miniTimerHost.Name = "miniTimerHost";
        this.miniTimerHost.Size = new System.Drawing.Size(464, 60);
        this.miniTimerHost.TabIndex = 4;
        this.miniTimerHost.Text = "elementHost1";
        this.miniTimerHost.Child = this.timerEditViewController;
        //
        // entriesHost
        //
        this.entriesHost.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                                   | System.Windows.Forms.AnchorStyles.Left)
                                   | System.Windows.Forms.AnchorStyles.Right)));
        this.entriesHost.Location = new System.Drawing.Point(0, 60);
        this.entriesHost.Margin = new System.Windows.Forms.Padding(0);
        this.entriesHost.Name = "entriesHost";
        this.entriesHost.Size = new System.Drawing.Size(464, 410);
        this.entriesHost.TabIndex = 3;
        this.entriesHost.Text = "elementHost1";
        this.entriesHost.Child = this.entries;
        //
        // TimeEntryListViewController
        //
        this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
        this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
        this.BackColor = System.Drawing.Color.WhiteSmoke;
        this.Controls.Add(this.miniTimerHost);
        this.Controls.Add(this.emptyLabel);
        this.Controls.Add(this.entriesHost);
        this.Margin = new System.Windows.Forms.Padding(0);
        this.Name = "TimeEntryListViewController";
        this.Size = new System.Drawing.Size(464, 470);
        this.ResumeLayout(false);

    }

    #endregion

    private System.Windows.Forms.Label emptyLabel;
    private System.Windows.Forms.Integration.ElementHost entriesHost;
    private WPF.TimeEntryList entries;
    private System.Windows.Forms.Integration.ElementHost miniTimerHost;
    private WPF.Timer timerEditViewController;
}
}
