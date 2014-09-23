namespace TogglDesktop
{
    partial class TimeEntryCell
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
            this.components = new System.ComponentModel.Container();
            this.labelFormattedDate = new System.Windows.Forms.Label();
            this.labelDateDuration = new System.Windows.Forms.Label();
            this.headerPanel = new System.Windows.Forms.Panel();
            this.panel = new System.Windows.Forms.Panel();
            this.tagBillableContinuePanel = new System.Windows.Forms.FlowLayoutPanel();
            this.labelContinue = new System.Windows.Forms.Label();
            this.labelBillable = new System.Windows.Forms.Label();
            this.labelTag = new System.Windows.Forms.Label();
            this.taskProjectPanel = new System.Windows.Forms.FlowLayoutPanel();
            this.labelTask = new System.Windows.Forms.Label();
            this.labelProject = new System.Windows.Forms.Label();
            this.labelClient = new System.Windows.Forms.Label();
            this.labelDescription = new System.Windows.Forms.Label();
            this.labelDuration = new System.Windows.Forms.Label();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.headerPanel.SuspendLayout();
            this.panel.SuspendLayout();
            this.tagBillableContinuePanel.SuspendLayout();
            this.taskProjectPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelFormattedDate
            // 
            this.labelFormattedDate.AutoSize = true;
            this.labelFormattedDate.Font = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelFormattedDate.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(133)))), ((int)(((byte)(133)))), ((int)(((byte)(133)))));
            this.labelFormattedDate.Location = new System.Drawing.Point(15, 15);
            this.labelFormattedDate.Name = "labelFormattedDate";
            this.labelFormattedDate.Size = new System.Drawing.Size(67, 15);
            this.labelFormattedDate.TabIndex = 10;
            this.labelFormattedDate.Text = "Thu 22. Jan";
            this.labelFormattedDate.UseMnemonic = false;
            // 
            // labelDateDuration
            // 
            this.labelDateDuration.AutoSize = true;
            this.labelDateDuration.Dock = System.Windows.Forms.DockStyle.Right;
            this.labelDateDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelDateDuration.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(133)))), ((int)(((byte)(133)))), ((int)(((byte)(133)))));
            this.labelDateDuration.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.labelDateDuration.Location = new System.Drawing.Point(238, 0);
            this.labelDateDuration.MaximumSize = new System.Drawing.Size(0, 45);
            this.labelDateDuration.MinimumSize = new System.Drawing.Size(0, 45);
            this.labelDateDuration.Name = "labelDateDuration";
            this.labelDateDuration.Padding = new System.Windows.Forms.Padding(0, 0, 11, 0);
            this.labelDateDuration.Size = new System.Drawing.Size(76, 45);
            this.labelDateDuration.TabIndex = 11;
            this.labelDateDuration.Text = "2 h 12 min";
            this.labelDateDuration.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.labelDateDuration.UseMnemonic = false;
            // 
            // headerPanel
            // 
            this.headerPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.headerPanel.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(239)))), ((int)(((byte)(239)))), ((int)(((byte)(239)))));
            this.headerPanel.Controls.Add(this.labelFormattedDate);
            this.headerPanel.Controls.Add(this.labelDateDuration);
            this.headerPanel.Location = new System.Drawing.Point(0, 0);
            this.headerPanel.Name = "headerPanel";
            this.headerPanel.Size = new System.Drawing.Size(314, 45);
            this.headerPanel.TabIndex = 12;
            this.headerPanel.Visible = false;
            // 
            // panel
            // 
            this.panel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panel.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(250)))), ((int)(((byte)(250)))), ((int)(((byte)(250)))));
            this.panel.Controls.Add(this.tagBillableContinuePanel);
            this.panel.Controls.Add(this.taskProjectPanel);
            this.panel.Controls.Add(this.labelDescription);
            this.panel.Controls.Add(this.labelDuration);
            this.panel.Location = new System.Drawing.Point(0, 45);
            this.panel.Name = "panel";
            this.panel.Size = new System.Drawing.Size(314, 55);
            this.panel.TabIndex = 15;
            this.panel.Click += new System.EventHandler(this.edit_Click);
            this.panel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // tagBillableContinuePanel
            // 
            this.tagBillableContinuePanel.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(250)))), ((int)(((byte)(250)))), ((int)(((byte)(250)))));
            this.tagBillableContinuePanel.Controls.Add(this.labelContinue);
            this.tagBillableContinuePanel.Controls.Add(this.labelBillable);
            this.tagBillableContinuePanel.Controls.Add(this.labelTag);
            this.tagBillableContinuePanel.Dock = System.Windows.Forms.DockStyle.Right;
            this.tagBillableContinuePanel.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.tagBillableContinuePanel.Location = new System.Drawing.Point(169, 0);
            this.tagBillableContinuePanel.Name = "tagBillableContinuePanel";
            this.tagBillableContinuePanel.Size = new System.Drawing.Size(70, 55);
            this.tagBillableContinuePanel.TabIndex = 16;
            this.tagBillableContinuePanel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            this.tagBillableContinuePanel.MouseUp += new System.Windows.Forms.MouseEventHandler(this.tagBillableContinuePanel_MouseUp);
            // 
            // labelContinue
            // 
            this.labelContinue.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelContinue.Cursor = System.Windows.Forms.Cursors.Hand;
            this.labelContinue.Image = global::TogglDesktop.Properties.Resources._continue;
            this.labelContinue.Location = new System.Drawing.Point(43, 15);
            this.labelContinue.Margin = new System.Windows.Forms.Padding(3, 15, 3, 0);
            this.labelContinue.Name = "labelContinue";
            this.labelContinue.Size = new System.Drawing.Size(24, 24);
            this.labelContinue.TabIndex = 14;
            this.labelContinue.UseMnemonic = false;
            this.labelContinue.Click += new System.EventHandler(this.labelContinue_Click);
            this.labelContinue.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // labelBillable
            // 
            this.labelBillable.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelBillable.Image = global::TogglDesktop.Properties.Resources.icon_billable;
            this.labelBillable.Location = new System.Drawing.Point(24, 20);
            this.labelBillable.Margin = new System.Windows.Forms.Padding(3, 20, 3, 0);
            this.labelBillable.Name = "labelBillable";
            this.labelBillable.Size = new System.Drawing.Size(13, 13);
            this.labelBillable.TabIndex = 5;
            this.labelBillable.UseMnemonic = false;
            this.labelBillable.Click += new System.EventHandler(this.edit_Click);
            this.labelBillable.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // labelTag
            // 
            this.labelTag.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelTag.Image = global::TogglDesktop.Properties.Resources.icon_tags;
            this.labelTag.Location = new System.Drawing.Point(5, 20);
            this.labelTag.Margin = new System.Windows.Forms.Padding(3, 20, 3, 0);
            this.labelTag.Name = "labelTag";
            this.labelTag.Size = new System.Drawing.Size(13, 13);
            this.labelTag.TabIndex = 13;
            this.labelTag.UseMnemonic = false;
            this.labelTag.Click += new System.EventHandler(this.edit_Click);
            this.labelTag.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // taskProjectPanel
            // 
            this.taskProjectPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.taskProjectPanel.Controls.Add(this.labelTask);
            this.taskProjectPanel.Controls.Add(this.labelProject);
            this.taskProjectPanel.Controls.Add(this.labelClient);
            this.taskProjectPanel.Location = new System.Drawing.Point(12, 29);
            this.taskProjectPanel.Margin = new System.Windows.Forms.Padding(0);
            this.taskProjectPanel.Name = "taskProjectPanel";
            this.taskProjectPanel.Size = new System.Drawing.Size(154, 17);
            this.taskProjectPanel.TabIndex = 15;
            this.taskProjectPanel.WrapContents = false;
            this.taskProjectPanel.Click += new System.EventHandler(this.project_Click);
            this.taskProjectPanel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // labelTask
            // 
            this.labelTask.AutoSize = true;
            this.labelTask.Font = new System.Drawing.Font("Lucida Sans Unicode", 12.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(0)));
            this.labelTask.Location = new System.Drawing.Point(3, 0);
            this.labelTask.Margin = new System.Windows.Forms.Padding(3, 0, 0, 0);
            this.labelTask.Name = "labelTask";
            this.labelTask.Size = new System.Drawing.Size(74, 16);
            this.labelTask.TabIndex = 10;
            this.labelTask.Text = "TaskName";
            this.labelTask.UseMnemonic = false;
            this.labelTask.Click += new System.EventHandler(this.project_Click);
            this.labelTask.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // labelProject
            // 
            this.labelProject.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelProject.AutoSize = true;
            this.labelProject.Font = new System.Drawing.Font("Lucida Sans Unicode", 13F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelProject.Location = new System.Drawing.Point(77, 0);
            this.labelProject.Margin = new System.Windows.Forms.Padding(0);
            this.labelProject.Name = "labelProject";
            this.labelProject.Size = new System.Drawing.Size(106, 16);
            this.labelProject.TabIndex = 9;
            this.labelProject.Text = "NEW - TOGGL";
            this.labelProject.UseMnemonic = false;
            this.labelProject.Click += new System.EventHandler(this.project_Click);
            this.labelProject.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // labelClient
            // 
            this.labelClient.AutoSize = true;
            this.labelClient.Font = new System.Drawing.Font("Lucida Sans Unicode", 13F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(0)));
            this.labelClient.ForeColor = System.Drawing.Color.Gray;
            this.labelClient.Location = new System.Drawing.Point(183, 0);
            this.labelClient.Margin = new System.Windows.Forms.Padding(0);
            this.labelClient.Name = "labelClient";
            this.labelClient.Size = new System.Drawing.Size(81, 16);
            this.labelClient.TabIndex = 11;
            this.labelClient.Text = "ClientName";
            this.labelClient.UseMnemonic = false;
            this.labelClient.Click += new System.EventHandler(this.project_Click);
            // 
            // labelDescription
            // 
            this.labelDescription.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelDescription.AutoEllipsis = true;
            this.labelDescription.Font = new System.Drawing.Font("Lucida Sans Unicode", 13F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelDescription.Location = new System.Drawing.Point(15, 9);
            this.labelDescription.Name = "labelDescription";
            this.labelDescription.Size = new System.Drawing.Size(154, 17);
            this.labelDescription.TabIndex = 6;
            this.labelDescription.Text = "Blogpost about ne...";
            this.labelDescription.UseMnemonic = false;
            this.labelDescription.Click += new System.EventHandler(this.labelDescription_Click);
            this.labelDescription.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // labelDuration
            // 
            this.labelDuration.AutoSize = true;
            this.labelDuration.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(250)))), ((int)(((byte)(250)))), ((int)(((byte)(250)))));
            this.labelDuration.Dock = System.Windows.Forms.DockStyle.Right;
            this.labelDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelDuration.Location = new System.Drawing.Point(239, 0);
            this.labelDuration.MaximumSize = new System.Drawing.Size(0, 55);
            this.labelDuration.MinimumSize = new System.Drawing.Size(0, 55);
            this.labelDuration.Name = "labelDuration";
            this.labelDuration.Padding = new System.Windows.Forms.Padding(0, 0, 11, 0);
            this.labelDuration.Size = new System.Drawing.Size(75, 55);
            this.labelDuration.TabIndex = 8;
            this.labelDuration.Text = "03:21:30";
            this.labelDuration.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.labelDuration.UseMnemonic = false;
            this.labelDuration.Click += new System.EventHandler(this.labelDuration_Click);
            this.labelDuration.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // TimeEntryCell
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(204)))), ((int)(((byte)(204)))), ((int)(((byte)(204)))));
            this.Controls.Add(this.panel);
            this.Controls.Add(this.headerPanel);
            this.Margin = new System.Windows.Forms.Padding(0, 1, 0, 1);
            this.Name = "TimeEntryCell";
            this.Size = new System.Drawing.Size(314, 101);
            this.MouseClick += new System.Windows.Forms.MouseEventHandler(this.TimeEntryCellWithHeader_MouseClick);
            this.headerPanel.ResumeLayout(false);
            this.headerPanel.PerformLayout();
            this.panel.ResumeLayout(false);
            this.panel.PerformLayout();
            this.tagBillableContinuePanel.ResumeLayout(false);
            this.taskProjectPanel.ResumeLayout(false);
            this.taskProjectPanel.PerformLayout();
            this.ResumeLayout(false);

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
        private System.Windows.Forms.Panel panel;
        private System.Windows.Forms.ToolTip toolTip;
        private System.Windows.Forms.FlowLayoutPanel taskProjectPanel;
        private System.Windows.Forms.Label labelTask;
        private System.Windows.Forms.Label labelClient;
        private System.Windows.Forms.FlowLayoutPanel tagBillableContinuePanel;
    }
}
