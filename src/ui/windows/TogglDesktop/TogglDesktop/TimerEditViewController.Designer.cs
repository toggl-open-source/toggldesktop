using TogglDesktop;
namespace TogglDesktop
{
    partial class TimerEditViewController
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
            this.buttonStart = new System.Windows.Forms.Button();
            this.timerRunningDuration = new System.Windows.Forms.Timer(this.components);
            this.linkLabelProject = new System.Windows.Forms.Label();
            this.linkLabelDescription = new System.Windows.Forms.Label();
            this.linkLabelDuration = new System.Windows.Forms.Label();
            this.labelClearProject = new System.Windows.Forms.Label();
            this.panelLeft = new System.Windows.Forms.Panel();
            this.panelDescription = new System.Windows.Forms.Panel();
            this.descriptionTextBox = new TogglDesktop.AutoCompleteTextBox();
            this.durationPanel = new System.Windows.Forms.Panel();
            this.textBoxDuration = new TogglDesktop.AlphaBlendTextBox();
            this.panelLeft.SuspendLayout();
            this.panelDescription.SuspendLayout();
            this.durationPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonStart
            // 
            this.buttonStart.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(77)))), ((int)(((byte)(217)))), ((int)(((byte)(101)))));
            this.buttonStart.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.buttonStart.Dock = System.Windows.Forms.DockStyle.Right;
            this.buttonStart.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(75)))), ((int)(((byte)(200)))), ((int)(((byte)(0)))));
            this.buttonStart.FlatAppearance.BorderSize = 0;
            this.buttonStart.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.buttonStart.Font = new System.Drawing.Font("Lucida Sans Unicode", 16F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.buttonStart.ForeColor = System.Drawing.Color.White;
            this.buttonStart.Location = new System.Drawing.Point(234, 0);
            this.buttonStart.Margin = new System.Windows.Forms.Padding(0);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(80, 56);
            this.buttonStart.TabIndex = 3;
            this.buttonStart.Text = "Start";
            this.buttonStart.UseVisualStyleBackColor = false;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // timerRunningDuration
            // 
            this.timerRunningDuration.Interval = 1000;
            this.timerRunningDuration.Tick += new System.EventHandler(this.timerRunningDuration_Tick);
            // 
            // linkLabelProject
            // 
            this.linkLabelProject.AccessibleDescription = "A link label with project name, clicking on it will open the time entry editor";
            this.linkLabelProject.AccessibleName = "Time entry project";
            this.linkLabelProject.AccessibleRole = System.Windows.Forms.AccessibleRole.Link;
            this.linkLabelProject.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.linkLabelProject.Font = new System.Drawing.Font("Lucida Sans Unicode", 10F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.linkLabelProject.ForeColor = System.Drawing.Color.White;
            this.linkLabelProject.Location = new System.Drawing.Point(17, 35);
            this.linkLabelProject.Name = "linkLabelProject";
            this.linkLabelProject.Size = new System.Drawing.Size(132, 20);
            this.linkLabelProject.TabIndex = 7;
            this.linkLabelProject.Text = "NEW - TOGGL";
            this.linkLabelProject.UseMnemonic = false;
            this.linkLabelProject.Visible = false;
            this.linkLabelProject.Click += new System.EventHandler(this.linkLabelProject_Click);
            this.linkLabelProject.MouseEnter += new System.EventHandler(this.linkLabelProject_Enter);
            // 
            // linkLabelDescription
            // 
            this.linkLabelDescription.AccessibleDescription = "A link label with time entry description, clicking on it opens the time entry edi" +
    "tor";
            this.linkLabelDescription.AccessibleName = "Time entry description";
            this.linkLabelDescription.AccessibleRole = System.Windows.Forms.AccessibleRole.Link;
            this.linkLabelDescription.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.linkLabelDescription.AutoEllipsis = true;
            this.linkLabelDescription.Font = new System.Drawing.Font("Lucida Sans Unicode", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.linkLabelDescription.ForeColor = System.Drawing.Color.White;
            this.linkLabelDescription.Location = new System.Drawing.Point(17, 0);
            this.linkLabelDescription.Name = "linkLabelDescription";
            this.linkLabelDescription.Size = new System.Drawing.Size(132, 23);
            this.linkLabelDescription.TabIndex = 8;
            this.linkLabelDescription.Text = "(no description)";
            this.linkLabelDescription.UseMnemonic = false;
            this.linkLabelDescription.Visible = false;
            this.linkLabelDescription.Click += new System.EventHandler(this.linkLabelDescription_Click);
            // 
            // linkLabelDuration
            // 
            this.linkLabelDuration.AccessibleDescription = "A link label with time entry duration, clicking on it will open the time entry ed" +
    "itor";
            this.linkLabelDuration.AccessibleName = "Time entry duration";
            this.linkLabelDuration.AccessibleRole = System.Windows.Forms.AccessibleRole.Link;
            this.linkLabelDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 15F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.linkLabelDuration.ForeColor = System.Drawing.Color.White;
            this.linkLabelDuration.Location = new System.Drawing.Point(2, 32);
            this.linkLabelDuration.Margin = new System.Windows.Forms.Padding(0, 0, 3, 0);
            this.linkLabelDuration.Name = "linkLabelDuration";
            this.linkLabelDuration.Size = new System.Drawing.Size(108, 23);
            this.linkLabelDuration.TabIndex = 9;
            this.linkLabelDuration.Text = "00:00:00";
            this.linkLabelDuration.TextAlign = System.Drawing.ContentAlignment.TopRight;
            this.linkLabelDuration.UseMnemonic = false;
            this.linkLabelDuration.Visible = false;
            this.linkLabelDuration.Click += new System.EventHandler(this.linkLabelDuration_Click);
            // 
            // labelClearProject
            // 
            this.labelClearProject.AccessibleDescription = "A button that allows to remove the assigned project from the time entry";
            this.labelClearProject.AccessibleName = "Clear project";
            this.labelClearProject.AccessibleRole = System.Windows.Forms.AccessibleRole.PushButton;
            this.labelClearProject.BackColor = System.Drawing.Color.Transparent;
            this.labelClearProject.Cursor = System.Windows.Forms.Cursors.Hand;
            this.labelClearProject.Enabled = false;
            this.labelClearProject.Font = new System.Drawing.Font("Lucida Sans Unicode", 13F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Pixel, ((byte)(0)));
            this.labelClearProject.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(185)))), ((int)(((byte)(185)))), ((int)(((byte)(185)))));
            this.labelClearProject.Location = new System.Drawing.Point(4, 35);
            this.labelClearProject.Name = "labelClearProject";
            this.labelClearProject.Size = new System.Drawing.Size(15, 20);
            this.labelClearProject.TabIndex = 10;
            this.labelClearProject.Text = "x";
            this.labelClearProject.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            this.labelClearProject.Visible = false;
            this.labelClearProject.Click += new System.EventHandler(this.labelClearProject_Click);
            // 
            // panelLeft
            // 
            this.panelLeft.BackColor = System.Drawing.Color.Transparent;
            this.panelLeft.BackgroundImage = global::TogglDesktop.Properties.Resources.timerDarkBg;
            this.panelLeft.Controls.Add(this.panelDescription);
            this.panelLeft.Controls.Add(this.durationPanel);
            this.panelLeft.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panelLeft.Location = new System.Drawing.Point(0, 0);
            this.panelLeft.Margin = new System.Windows.Forms.Padding(2);
            this.panelLeft.Name = "panelLeft";
            this.panelLeft.Size = new System.Drawing.Size(234, 56);
            this.panelLeft.TabIndex = 11;
            // 
            // panelDescription
            // 
            this.panelDescription.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panelDescription.BackColor = System.Drawing.Color.Transparent;
            this.panelDescription.Controls.Add(this.linkLabelProject);
            this.panelDescription.Controls.Add(this.linkLabelDescription);
            this.panelDescription.Controls.Add(this.descriptionTextBox);
            this.panelDescription.Controls.Add(this.labelClearProject);
            this.panelDescription.Location = new System.Drawing.Point(0, 0);
            this.panelDescription.Name = "panelDescription";
            this.panelDescription.Size = new System.Drawing.Size(138, 53);
            this.panelDescription.TabIndex = 1;
            // 
            // descriptionTextBox
            // 
            this.descriptionTextBox.AcceptsTab = true;
            this.descriptionTextBox.AccessibleName = "Time entry description";
            this.descriptionTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.descriptionTextBox.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.CustomSource;
            this.descriptionTextBox.BackAlpha = 0;
            this.descriptionTextBox.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(59)))), ((int)(((byte)(59)))), ((int)(((byte)(59)))));
            this.descriptionTextBox.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.descriptionTextBox.Font = new System.Drawing.Font("Lucida Sans Unicode", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.descriptionTextBox.ForeColor = System.Drawing.Color.White;
            this.descriptionTextBox.Location = new System.Drawing.Point(20, 20);
            this.descriptionTextBox.Name = "descriptionTextBox";
            this.descriptionTextBox.Size = new System.Drawing.Size(115, 16);
            this.descriptionTextBox.TabIndex = 1;
            this.descriptionTextBox.Text = "What are you doing?";
            this.descriptionTextBox.Enter += new System.EventHandler(this.descriptionTextBox_Enter);
            this.descriptionTextBox.KeyUp += new System.Windows.Forms.KeyEventHandler(this.descriptionTextBox_KeyUp);
            this.descriptionTextBox.Leave += new System.EventHandler(this.descriptionTextBox_Leave);
            this.descriptionTextBox.PreviewKeyDown += new System.Windows.Forms.PreviewKeyDownEventHandler(this.descriptionTextBox_PreviewKeyDown);
            // 
            // durationPanel
            // 
            this.durationPanel.AutoSize = true;
            this.durationPanel.BackColor = System.Drawing.Color.Transparent;
            this.durationPanel.Controls.Add(this.textBoxDuration);
            this.durationPanel.Controls.Add(this.linkLabelDuration);
            this.durationPanel.Dock = System.Windows.Forms.DockStyle.Right;
            this.durationPanel.Location = new System.Drawing.Point(121, 0);
            this.durationPanel.Name = "durationPanel";
            this.durationPanel.Size = new System.Drawing.Size(113, 56);
            this.durationPanel.TabIndex = 2;
            // 
            // textBoxDuration
            // 
            this.textBoxDuration.AccessibleName = "Time entry duration";
            this.textBoxDuration.BackAlpha = 10;
            this.textBoxDuration.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(59)))), ((int)(((byte)(59)))), ((int)(((byte)(59)))));
            this.textBoxDuration.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.textBoxDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 15F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(186)));
            this.textBoxDuration.ForeColor = System.Drawing.Color.White;
            this.textBoxDuration.Location = new System.Drawing.Point(21, 17);
            this.textBoxDuration.Margin = new System.Windows.Forms.Padding(5, 3, 9, 3);
            this.textBoxDuration.Name = "textBoxDuration";
            this.textBoxDuration.Size = new System.Drawing.Size(78, 24);
            this.textBoxDuration.TabIndex = 2;
            this.textBoxDuration.Text = "00:00:00";
            this.textBoxDuration.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.textBoxDuration.Click += new System.EventHandler(this.textBoxDuration_Click);
            this.textBoxDuration.Enter += new System.EventHandler(this.textBoxDuration_Enter);
            this.textBoxDuration.Leave += new System.EventHandler(this.textBoxDuration_Leave);
            // 
            // TimerEditViewController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(59)))), ((int)(((byte)(59)))), ((int)(((byte)(59)))));
            this.Controls.Add(this.panelLeft);
            this.Controls.Add(this.buttonStart);
            this.Margin = new System.Windows.Forms.Padding(0);
            this.Name = "TimerEditViewController";
            this.Size = new System.Drawing.Size(314, 56);
            this.panelLeft.ResumeLayout(false);
            this.panelLeft.PerformLayout();
            this.panelDescription.ResumeLayout(false);
            this.panelDescription.PerformLayout();
            this.durationPanel.ResumeLayout(false);
            this.durationPanel.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private TogglDesktop.AlphaBlendTextBox textBoxDuration;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.Timer timerRunningDuration;
        private System.Windows.Forms.Label linkLabelProject;
        private System.Windows.Forms.Label linkLabelDescription;
        private System.Windows.Forms.Label linkLabelDuration;
        private AutoCompleteTextBox descriptionTextBox;
        private System.Windows.Forms.Label labelClearProject;
        private System.Windows.Forms.Panel panelLeft;
        private System.Windows.Forms.Panel durationPanel;
        private System.Windows.Forms.Panel panelDescription;
    }
}
