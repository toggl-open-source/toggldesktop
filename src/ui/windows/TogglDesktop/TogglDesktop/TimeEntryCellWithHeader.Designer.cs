namespace TogglDesktop
{
    partial class TimeEntryCellWithHeader
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
            this.labelProject = new System.Windows.Forms.Label();
            this.labelDuration = new System.Windows.Forms.Label();
            this.labelDescription = new System.Windows.Forms.Label();
            this.labelBillable = new System.Windows.Forms.Label();
            this.labelFormattedDate = new System.Windows.Forms.Label();
            this.labelDateDuration = new System.Windows.Forms.Label();
            this.headerPanel = new System.Windows.Forms.Panel();
            this.labelTag = new System.Windows.Forms.Label();
            this.labelContinue = new System.Windows.Forms.Label();
            this.headerPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelProject
            // 
            this.labelProject.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelProject.Font = new System.Drawing.Font("Lucida Sans Unicode", 13F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelProject.Location = new System.Drawing.Point(15, 77);
            this.labelProject.Name = "labelProject";
            this.labelProject.Size = new System.Drawing.Size(170, 17);
            this.labelProject.TabIndex = 9;
            this.labelProject.Text = "NEW - TOGGL";
            this.labelProject.Click += new System.EventHandler(this.labelProject_Click);
            // 
            // labelDuration
            // 
            this.labelDuration.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelDuration.AutoSize = true;
            this.labelDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelDuration.Location = new System.Drawing.Point(250, 68);
            this.labelDuration.Name = "labelDuration";
            this.labelDuration.Size = new System.Drawing.Size(64, 16);
            this.labelDuration.TabIndex = 8;
            this.labelDuration.Text = "03:21:30";
            this.labelDuration.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.labelDuration.Click += new System.EventHandler(this.labelDuration_Click);
            // 
            // labelDescription
            // 
            this.labelDescription.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelDescription.Font = new System.Drawing.Font("Lucida Sans Unicode", 13F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelDescription.Location = new System.Drawing.Point(15, 60);
            this.labelDescription.Name = "labelDescription";
            this.labelDescription.Size = new System.Drawing.Size(170, 17);
            this.labelDescription.TabIndex = 6;
            this.labelDescription.Text = "Blogpost about ne...";
            this.labelDescription.Click += new System.EventHandler(this.labelDescription_Click);
            // 
            // labelBillable
            // 
            this.labelBillable.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelBillable.Image = global::TogglDesktop.Properties.Resources.icon_billable;
            this.labelBillable.Location = new System.Drawing.Point(200, 68);
            this.labelBillable.Name = "labelBillable";
            this.labelBillable.Size = new System.Drawing.Size(13, 13);
            this.labelBillable.TabIndex = 5;
            // 
            // labelFormattedDate
            // 
            this.labelFormattedDate.AutoSize = true;
            this.labelFormattedDate.Font = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelFormattedDate.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(108)))), ((int)(((byte)(108)))), ((int)(((byte)(108)))));
            this.labelFormattedDate.Location = new System.Drawing.Point(15, 13);
            this.labelFormattedDate.Name = "labelFormattedDate";
            this.labelFormattedDate.Size = new System.Drawing.Size(67, 15);
            this.labelFormattedDate.TabIndex = 10;
            this.labelFormattedDate.Text = "Thu 22. Jan";
            // 
            // labelDateDuration
            // 
            this.labelDateDuration.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelDateDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelDateDuration.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(108)))), ((int)(((byte)(108)))), ((int)(((byte)(108)))));
            this.labelDateDuration.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.labelDateDuration.Location = new System.Drawing.Point(220, 13);
            this.labelDateDuration.Name = "labelDateDuration";
            this.labelDateDuration.Size = new System.Drawing.Size(74, 13);
            this.labelDateDuration.TabIndex = 11;
            this.labelDateDuration.Text = "2 h 12 min";
            this.labelDateDuration.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // headerPanel
            // 
            this.headerPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.headerPanel.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(235)))), ((int)(((byte)(235)))), ((int)(((byte)(235)))));
            this.headerPanel.Controls.Add(this.labelFormattedDate);
            this.headerPanel.Controls.Add(this.labelDateDuration);
            this.headerPanel.Location = new System.Drawing.Point(0, 0);
            this.headerPanel.Name = "headerPanel";
            this.headerPanel.Size = new System.Drawing.Size(314, 45);
            this.headerPanel.TabIndex = 12;
            // 
            // labelTag
            // 
            this.labelTag.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelTag.Image = global::TogglDesktop.Properties.Resources.icon_tags;
            this.labelTag.Location = new System.Drawing.Point(185, 68);
            this.labelTag.Name = "labelTag";
            this.labelTag.Size = new System.Drawing.Size(13, 13);
            this.labelTag.TabIndex = 13;
            // 
            // labelContinue
            // 
            this.labelContinue.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelContinue.Image = global::TogglDesktop.Properties.Resources._continue;
            this.labelContinue.Location = new System.Drawing.Point(220, 63);
            this.labelContinue.Name = "labelContinue";
            this.labelContinue.Size = new System.Drawing.Size(24, 24);
            this.labelContinue.TabIndex = 14;
            this.labelContinue.Click += new System.EventHandler(this.labelContinue_Click);
            // 
            // TimeEntryCellWithHeader
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(250)))), ((int)(((byte)(250)))), ((int)(((byte)(250)))));
            this.Controls.Add(this.labelContinue);
            this.Controls.Add(this.labelTag);
            this.Controls.Add(this.headerPanel);
            this.Controls.Add(this.labelProject);
            this.Controls.Add(this.labelDuration);
            this.Controls.Add(this.labelDescription);
            this.Controls.Add(this.labelBillable);
            this.Name = "TimeEntryCellWithHeader";
            this.Size = new System.Drawing.Size(314, 100);
            this.MouseClick += new System.Windows.Forms.MouseEventHandler(this.TimeEntryCellWithHeader_MouseClick);
            this.headerPanel.ResumeLayout(false);
            this.headerPanel.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelProject;
        private System.Windows.Forms.Label labelDuration;
        private System.Windows.Forms.Label labelDescription;
        private System.Windows.Forms.Label labelBillable;
        private System.Windows.Forms.Label labelFormattedDate;
        private System.Windows.Forms.Label labelDateDuration;
        private System.Windows.Forms.Panel headerPanel;
        private System.Windows.Forms.Label labelTag;
        private System.Windows.Forms.Label labelContinue;
    }
}
