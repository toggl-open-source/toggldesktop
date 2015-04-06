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
            this.panelShadow = new System.Windows.Forms.Panel();
            this.panel = new System.Windows.Forms.Panel();
            this.labelBillable = new System.Windows.Forms.Label();
            this.labelTag = new System.Windows.Forms.Label();
            this.labelDuration = new System.Windows.Forms.Label();
            this.tagBillableContinuePanel = new System.Windows.Forms.FlowLayoutPanel();
            this.labelContinue = new System.Windows.Forms.Label();
            this.ProjectColorPanel = new System.Windows.Forms.Panel();
            this.labelDescription = new System.Windows.Forms.Label();
            this.taskProjectPanel = new System.Windows.Forms.FlowLayoutPanel();
            this.labelTask = new System.Windows.Forms.Label();
            this.labelClient = new System.Windows.Forms.Label();
            this.labelProject = new System.Windows.Forms.Label();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.headerPanel.SuspendLayout();
            this.panel.SuspendLayout();
            this.tagBillableContinuePanel.SuspendLayout();
            this.taskProjectPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelFormattedDate
            // 
            this.labelFormattedDate.AccessibleName = "Date";
            this.labelFormattedDate.AutoSize = true;
            this.labelFormattedDate.Dock = System.Windows.Forms.DockStyle.Left;
            this.labelFormattedDate.Font = new System.Drawing.Font("Lucida Sans Unicode", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelFormattedDate.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(35)))), ((int)(((byte)(37)))), ((int)(((byte)(37)))));
            this.labelFormattedDate.Location = new System.Drawing.Point(0, 0);
            this.labelFormattedDate.MaximumSize = new System.Drawing.Size(0, 45);
            this.labelFormattedDate.MinimumSize = new System.Drawing.Size(0, 45);
            this.labelFormattedDate.Name = "labelFormattedDate";
            this.labelFormattedDate.Padding = new System.Windows.Forms.Padding(15, 0, 0, 0);
            this.labelFormattedDate.Size = new System.Drawing.Size(76, 45);
            this.labelFormattedDate.TabIndex = 10;
            this.labelFormattedDate.Text = "Thu 22. Jan";
            this.labelFormattedDate.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.labelFormattedDate.UseMnemonic = false;
            // 
            // labelDateDuration
            // 
            this.labelDateDuration.AccessibleName = "Total time tracked per date";
            this.labelDateDuration.AutoSize = true;
            this.labelDateDuration.Dock = System.Windows.Forms.DockStyle.Left;
            this.labelDateDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelDateDuration.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(111)))), ((int)(((byte)(111)))), ((int)(((byte)(111)))));
            this.labelDateDuration.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.labelDateDuration.Location = new System.Drawing.Point(76, 0);
            this.labelDateDuration.MaximumSize = new System.Drawing.Size(0, 45);
            this.labelDateDuration.MinimumSize = new System.Drawing.Size(0, 45);
            this.labelDateDuration.Name = "labelDateDuration";
            this.labelDateDuration.Padding = new System.Windows.Forms.Padding(0, 0, 11, 0);
            this.labelDateDuration.Size = new System.Drawing.Size(68, 45);
            this.labelDateDuration.TabIndex = 11;
            this.labelDateDuration.Text = "2 h 12 min";
            this.labelDateDuration.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.labelDateDuration.UseMnemonic = false;
            // 
            // headerPanel
            // 
            this.headerPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.headerPanel.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(247)))), ((int)(((byte)(247)))), ((int)(((byte)(247)))));
            this.headerPanel.Controls.Add(this.panelShadow);
            this.headerPanel.Controls.Add(this.labelDateDuration);
            this.headerPanel.Controls.Add(this.labelFormattedDate);
            this.headerPanel.Location = new System.Drawing.Point(0, 0);
            this.headerPanel.Name = "headerPanel";
            this.headerPanel.Size = new System.Drawing.Size(314, 45);
            this.headerPanel.TabIndex = 12;
            this.headerPanel.Visible = false;
            // 
            // panelShadow
            // 
            this.panelShadow.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panelShadow.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(247)))), ((int)(((byte)(247)))), ((int)(((byte)(247)))));
            this.panelShadow.BackgroundImage = global::TogglDesktop.Properties.Resources.shadow;
            this.panelShadow.Location = new System.Drawing.Point(0, -1);
            this.panelShadow.Margin = new System.Windows.Forms.Padding(0);
            this.panelShadow.Name = "panelShadow";
            this.panelShadow.Size = new System.Drawing.Size(314, 4);
            this.panelShadow.TabIndex = 12;
            // 
            // panel
            // 
            this.panel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panel.BackColor = System.Drawing.Color.White;
            this.panel.Controls.Add(this.labelBillable);
            this.panel.Controls.Add(this.labelTag);
            this.panel.Controls.Add(this.labelDuration);
            this.panel.Controls.Add(this.tagBillableContinuePanel);
            this.panel.Controls.Add(this.ProjectColorPanel);
            this.panel.Controls.Add(this.labelDescription);
            this.panel.Controls.Add(this.taskProjectPanel);
            this.panel.Location = new System.Drawing.Point(0, 45);
            this.panel.Name = "panel";
            this.panel.Size = new System.Drawing.Size(314, 55);
            this.panel.TabIndex = 15;
            this.panel.Click += new System.EventHandler(this.edit_Click);
            this.panel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // labelBillable
            // 
            this.labelBillable.AccessibleDescription = "A label with a dollar sign image, indicating if the time entry is billable";
            this.labelBillable.AccessibleName = "Billable label";
            this.labelBillable.AccessibleRole = System.Windows.Forms.AccessibleRole.Graphic;
            this.labelBillable.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelBillable.BackColor = System.Drawing.Color.Transparent;
            this.labelBillable.Image = global::TogglDesktop.Properties.Resources.icon_billable;
            this.labelBillable.Location = new System.Drawing.Point(168, 0);
            this.labelBillable.Margin = new System.Windows.Forms.Padding(3, 20, 0, 0);
            this.labelBillable.Name = "labelBillable";
            this.labelBillable.Padding = new System.Windows.Forms.Padding(1, 0, 0, 0);
            this.labelBillable.Size = new System.Drawing.Size(13, 55);
            this.labelBillable.TabIndex = 5;
            this.labelBillable.UseMnemonic = false;
            this.labelBillable.Click += new System.EventHandler(this.edit_Click);
            this.labelBillable.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // labelTag
            // 
            this.labelTag.AccessibleDescription = "A label with an image of tags, indicating if the time entry has any tags";
            this.labelTag.AccessibleName = "Tags label";
            this.labelTag.AccessibleRole = System.Windows.Forms.AccessibleRole.Graphic;
            this.labelTag.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelTag.BackColor = System.Drawing.Color.Transparent;
            this.labelTag.Image = global::TogglDesktop.Properties.Resources.icon_tags;
            this.labelTag.Location = new System.Drawing.Point(147, 0);
            this.labelTag.Name = "labelTag";
            this.labelTag.Padding = new System.Windows.Forms.Padding(3, 0, 4, 0);
            this.labelTag.Size = new System.Drawing.Size(23, 55);
            this.labelTag.TabIndex = 13;
            this.labelTag.UseMnemonic = false;
            this.labelTag.Click += new System.EventHandler(this.edit_Click);
            this.labelTag.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // labelDuration
            // 
            this.labelDuration.BackColor = System.Drawing.Color.Transparent;
            this.labelDuration.Dock = System.Windows.Forms.DockStyle.Right;
            this.labelDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelDuration.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(35)))), ((int)(((byte)(37)))), ((int)(((byte)(37)))));
            this.labelDuration.Location = new System.Drawing.Point(178, 0);
            this.labelDuration.MaximumSize = new System.Drawing.Size(75, 55);
            this.labelDuration.MinimumSize = new System.Drawing.Size(75, 55);
            this.labelDuration.Name = "labelDuration";
            this.labelDuration.Size = new System.Drawing.Size(75, 55);
            this.labelDuration.TabIndex = 8;
            this.labelDuration.Text = "03:21:30";
            this.labelDuration.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.labelDuration.UseMnemonic = false;
            this.labelDuration.Click += new System.EventHandler(this.labelDuration_Click);
            this.labelDuration.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // tagBillableContinuePanel
            // 
            this.tagBillableContinuePanel.BackColor = System.Drawing.Color.Transparent;
            this.tagBillableContinuePanel.Controls.Add(this.labelContinue);
            this.tagBillableContinuePanel.Dock = System.Windows.Forms.DockStyle.Right;
            this.tagBillableContinuePanel.FlowDirection = System.Windows.Forms.FlowDirection.RightToLeft;
            this.tagBillableContinuePanel.Location = new System.Drawing.Point(253, 0);
            this.tagBillableContinuePanel.Margin = new System.Windows.Forms.Padding(0);
            this.tagBillableContinuePanel.Name = "tagBillableContinuePanel";
            this.tagBillableContinuePanel.Size = new System.Drawing.Size(61, 55);
            this.tagBillableContinuePanel.TabIndex = 16;
            this.tagBillableContinuePanel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            this.tagBillableContinuePanel.MouseUp += new System.Windows.Forms.MouseEventHandler(this.tagBillableContinuePanel_MouseUp);
            // 
            // labelContinue
            // 
            this.labelContinue.AccessibleDescription = "A button with a play image, to continue the time entry";
            this.labelContinue.AccessibleName = "Continue time entry";
            this.labelContinue.AccessibleRole = System.Windows.Forms.AccessibleRole.PushButton;
            this.labelContinue.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.labelContinue.Cursor = System.Windows.Forms.Cursors.Hand;
            this.labelContinue.Image = global::TogglDesktop.Properties.Resources._continue;
            this.labelContinue.Location = new System.Drawing.Point(20, 9);
            this.labelContinue.Margin = new System.Windows.Forms.Padding(18, 9, 5, 0);
            this.labelContinue.Name = "labelContinue";
            this.labelContinue.Size = new System.Drawing.Size(36, 36);
            this.labelContinue.TabIndex = 14;
            this.labelContinue.UseMnemonic = false;
            this.labelContinue.Click += new System.EventHandler(this.labelContinue_Click);
            this.labelContinue.MouseEnter += new System.EventHandler(this.labelContinue_MouseEnter);
            this.labelContinue.MouseLeave += new System.EventHandler(this.labelContinue_MouseLeave);
            this.labelContinue.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // ProjectColorPanel
            // 
            this.ProjectColorPanel.Location = new System.Drawing.Point(0, 0);
            this.ProjectColorPanel.Name = "ProjectColorPanel";
            this.ProjectColorPanel.Size = new System.Drawing.Size(4, 55);
            this.ProjectColorPanel.TabIndex = 17;
            // 
            // labelDescription
            // 
            this.labelDescription.AccessibleName = "Time entry description";
            this.labelDescription.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelDescription.AutoEllipsis = true;
            this.labelDescription.Font = new System.Drawing.Font("Lucida Sans Unicode", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelDescription.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(35)))), ((int)(((byte)(37)))), ((int)(((byte)(37)))));
            this.labelDescription.Location = new System.Drawing.Point(15, 29);
            this.labelDescription.Name = "labelDescription";
            this.labelDescription.Size = new System.Drawing.Size(154, 17);
            this.labelDescription.TabIndex = 6;
            this.labelDescription.Text = "Blogpost about ne...";
            this.labelDescription.UseMnemonic = false;
            this.labelDescription.Click += new System.EventHandler(this.labelDescription_Click);
            this.labelDescription.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // taskProjectPanel
            // 
            this.taskProjectPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.taskProjectPanel.Controls.Add(this.labelTask);
            this.taskProjectPanel.Controls.Add(this.labelClient);
            this.taskProjectPanel.Controls.Add(this.labelProject);
            this.taskProjectPanel.Location = new System.Drawing.Point(12, 9);
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
            this.labelTask.AccessibleName = "Task name";
            this.labelTask.AutoSize = true;
            this.labelTask.Dock = System.Windows.Forms.DockStyle.Left;
            this.labelTask.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.labelTask.Font = new System.Drawing.Font("Lucida Sans Unicode", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(0)));
            this.labelTask.ForeColor = System.Drawing.Color.Gray;
            this.labelTask.Location = new System.Drawing.Point(3, 0);
            this.labelTask.Margin = new System.Windows.Forms.Padding(3, 0, 0, 0);
            this.labelTask.Name = "labelTask";
            this.labelTask.Size = new System.Drawing.Size(56, 14);
            this.labelTask.TabIndex = 10;
            this.labelTask.Text = "TaskName";
            this.labelTask.UseMnemonic = false;
            this.labelTask.Click += new System.EventHandler(this.project_Click);
            this.labelTask.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // labelClient
            // 
            this.labelClient.AccessibleName = "Client name";
            this.labelClient.AutoSize = true;
            this.labelClient.Dock = System.Windows.Forms.DockStyle.Left;
            this.labelClient.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.labelClient.Font = new System.Drawing.Font("Lucida Sans Unicode", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(0)));
            this.labelClient.ForeColor = System.Drawing.Color.DimGray;
            this.labelClient.Location = new System.Drawing.Point(59, 0);
            this.labelClient.Margin = new System.Windows.Forms.Padding(0);
            this.labelClient.Name = "labelClient";
            this.labelClient.Size = new System.Drawing.Size(61, 14);
            this.labelClient.TabIndex = 11;
            this.labelClient.Text = "ClientName";
            this.labelClient.UseMnemonic = false;
            this.labelClient.Click += new System.EventHandler(this.project_Click);
            // 
            // labelProject
            // 
            this.labelProject.AccessibleName = "Project name";
            this.labelProject.AutoSize = true;
            this.labelProject.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.labelProject.Font = new System.Drawing.Font("Lucida Sans Unicode", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.labelProject.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(35)))), ((int)(((byte)(37)))), ((int)(((byte)(37)))));
            this.labelProject.Location = new System.Drawing.Point(120, 0);
            this.labelProject.Margin = new System.Windows.Forms.Padding(0);
            this.labelProject.Name = "labelProject";
            this.labelProject.Size = new System.Drawing.Size(73, 14);
            this.labelProject.TabIndex = 9;
            this.labelProject.Text = "NEW - TOGGL";
            this.labelProject.UseMnemonic = false;
            this.labelProject.Click += new System.EventHandler(this.project_Click);
            this.labelProject.MouseMove += new System.Windows.Forms.MouseEventHandler(this.item_MouseMove);
            // 
            // TimeEntryCell
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(235)))), ((int)(((byte)(235)))), ((int)(((byte)(235)))));
            this.Controls.Add(this.panel);
            this.Controls.Add(this.headerPanel);
            this.Margin = new System.Windows.Forms.Padding(0, 1, 0, 1);
            this.Name = "TimeEntryCell";
            this.Size = new System.Drawing.Size(314, 101);
            this.MouseClick += new System.Windows.Forms.MouseEventHandler(this.TimeEntryCellWithHeader_MouseClick);
            this.headerPanel.ResumeLayout(false);
            this.headerPanel.PerformLayout();
            this.panel.ResumeLayout(false);
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
        private System.Windows.Forms.Panel ProjectColorPanel;
        private System.Windows.Forms.Panel panelShadow;
    }
}
