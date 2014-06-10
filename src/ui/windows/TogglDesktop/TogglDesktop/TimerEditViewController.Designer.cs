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
            this.comboBoxDescription = new System.Windows.Forms.ComboBox();
            this.textBoxDuration = new System.Windows.Forms.TextBox();
            this.buttonStart = new System.Windows.Forms.Button();
            this.linkLabelDescription = new System.Windows.Forms.LinkLabel();
            this.linkLabelDuration = new System.Windows.Forms.LinkLabel();
            this.timerRunningDuration = new System.Windows.Forms.Timer(this.components);
            this.linkLabelProject = new System.Windows.Forms.LinkLabel();
            this.SuspendLayout();
            // 
            // comboBoxDescription
            // 
            this.comboBoxDescription.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxDescription.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxDescription.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxDescription.FormattingEnabled = true;
            this.comboBoxDescription.Location = new System.Drawing.Point(4, 4);
            this.comboBoxDescription.Name = "comboBoxDescription";
            this.comboBoxDescription.Size = new System.Drawing.Size(189, 21);
            this.comboBoxDescription.TabIndex = 0;
            this.comboBoxDescription.Text = "What are you doing?";
            this.comboBoxDescription.SelectedIndexChanged += new System.EventHandler(this.comboBoxDescription_SelectedIndexChanged);
            this.comboBoxDescription.DropDownClosed += new System.EventHandler(this.comboBoxDescription_DropDownClosed);
            this.comboBoxDescription.Enter += new System.EventHandler(this.comboBoxDescription_Enter);
            this.comboBoxDescription.Leave += new System.EventHandler(this.comboBoxDescription_Leave);
            // 
            // textBoxDuration
            // 
            this.textBoxDuration.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxDuration.Location = new System.Drawing.Point(200, 4);
            this.textBoxDuration.Name = "textBoxDuration";
            this.textBoxDuration.Size = new System.Drawing.Size(31, 20);
            this.textBoxDuration.TabIndex = 1;
            this.textBoxDuration.Text = "0 sec";
            this.textBoxDuration.Enter += new System.EventHandler(this.textBoxDuration_Enter);
            this.textBoxDuration.Leave += new System.EventHandler(this.textBoxDuration_Leave);
            // 
            // buttonStart
            // 
            this.buttonStart.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonStart.Location = new System.Drawing.Point(238, 4);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(64, 23);
            this.buttonStart.TabIndex = 2;
            this.buttonStart.Text = "Start";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // linkLabelDescription
            // 
            this.linkLabelDescription.AutoSize = true;
            this.linkLabelDescription.Location = new System.Drawing.Point(114, 32);
            this.linkLabelDescription.Name = "linkLabelDescription";
            this.linkLabelDescription.Size = new System.Drawing.Size(79, 13);
            this.linkLabelDescription.TabIndex = 5;
            this.linkLabelDescription.TabStop = true;
            this.linkLabelDescription.Text = "(no description)";
            this.linkLabelDescription.Visible = false;
            this.linkLabelDescription.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.linkLabelDescription_LinkClicked);
            // 
            // linkLabelDuration
            // 
            this.linkLabelDuration.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.linkLabelDuration.AutoSize = true;
            this.linkLabelDuration.Location = new System.Drawing.Point(200, 32);
            this.linkLabelDuration.Name = "linkLabelDuration";
            this.linkLabelDuration.Size = new System.Drawing.Size(33, 13);
            this.linkLabelDuration.TabIndex = 6;
            this.linkLabelDuration.TabStop = true;
            this.linkLabelDuration.Text = "0 sec";
            this.linkLabelDuration.Visible = false;
            this.linkLabelDuration.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.linkLabelDuration_LinkClicked);
            // 
            // timerRunningDuration
            // 
            this.timerRunningDuration.Interval = 1000;
            this.timerRunningDuration.Tick += new System.EventHandler(this.timerRunningDuration_Tick);
            // 
            // linkLabelProject
            // 
            this.linkLabelProject.AutoSize = true;
            this.linkLabelProject.Location = new System.Drawing.Point(3, 32);
            this.linkLabelProject.Name = "linkLabelProject";
            this.linkLabelProject.Size = new System.Drawing.Size(79, 13);
            this.linkLabelProject.TabIndex = 7;
            this.linkLabelProject.TabStop = true;
            this.linkLabelProject.Text = "NEW - TOGGL";
            this.linkLabelProject.Visible = false;
            this.linkLabelProject.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.linkLabelProject_LinkClicked);
            // 
            // TimerEditViewController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Control;
            this.Controls.Add(this.linkLabelProject);
            this.Controls.Add(this.linkLabelDuration);
            this.Controls.Add(this.linkLabelDescription);
            this.Controls.Add(this.buttonStart);
            this.Controls.Add(this.textBoxDuration);
            this.Controls.Add(this.comboBoxDescription);
            this.Name = "TimerEditViewController";
            this.Size = new System.Drawing.Size(305, 55);
            this.Load += new System.EventHandler(this.TimerEditViewController_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox comboBoxDescription;
        private System.Windows.Forms.TextBox textBoxDuration;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.LinkLabel linkLabelDescription;
        private System.Windows.Forms.LinkLabel linkLabelDuration;
        private System.Windows.Forms.Timer timerRunningDuration;
        private System.Windows.Forms.LinkLabel linkLabelProject;
    }
}
