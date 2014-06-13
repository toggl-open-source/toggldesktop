namespace TogglDesktop
{
    partial class TimeEntryEditViewController
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
            this.label1 = new System.Windows.Forms.Label();
            this.comboBoxDescription = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.comboBoxProject = new System.Windows.Forms.ComboBox();
            this.linkAddProject = new System.Windows.Forms.LinkLabel();
            this.label3 = new System.Windows.Forms.Label();
            this.textBoxDuration = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.textBoxStartTime = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.textBoxEndTime = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.dateTimePickerStartDate = new System.Windows.Forms.DateTimePicker();
            this.label7 = new System.Windows.Forms.Label();
            this.statusStripLastUpdate = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabelLastUpdate = new System.Windows.Forms.ToolStripStatusLabel();
            this.buttonsPanel = new System.Windows.Forms.Panel();
            this.buttonContinue = new System.Windows.Forms.Button();
            this.buttonDelete = new System.Windows.Forms.Button();
            this.buttonDone = new System.Windows.Forms.Button();
            this.checkBoxBillable = new System.Windows.Forms.CheckBox();
            this.checkedListBoxTags = new System.Windows.Forms.CheckedListBox();
            this.timerRunningDuration = new System.Windows.Forms.Timer(this.components);
            this.panelBottom = new System.Windows.Forms.Panel();
            this.panelAddProject = new System.Windows.Forms.Panel();
            this.labelProjectName = new System.Windows.Forms.Label();
            this.labelClient = new System.Windows.Forms.Label();
            this.textBoxProjectName = new System.Windows.Forms.TextBox();
            this.checkBoxPublic = new System.Windows.Forms.CheckBox();
            this.comboBoxClient = new System.Windows.Forms.ComboBox();
            this.comboBoxWorkspace = new System.Windows.Forms.ComboBox();
            this.labelWorkspace = new System.Windows.Forms.Label();
            this.statusStripLastUpdate.SuspendLayout();
            this.buttonsPanel.SuspendLayout();
            this.panelBottom.SuspendLayout();
            this.panelAddProject.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(63, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Description:";
            // 
            // comboBoxDescription
            // 
            this.comboBoxDescription.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxDescription.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxDescription.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxDescription.FormattingEnabled = true;
            this.comboBoxDescription.Location = new System.Drawing.Point(100, 10);
            this.comboBoxDescription.Name = "comboBoxDescription";
            this.comboBoxDescription.Size = new System.Drawing.Size(202, 21);
            this.comboBoxDescription.TabIndex = 1;
            this.comboBoxDescription.SelectedIndexChanged += new System.EventHandler(this.comboBoxDescription_SelectedIndexChanged);
            this.comboBoxDescription.Leave += new System.EventHandler(this.comboBoxDescription_Leave);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(15, 40);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(43, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "Project:";
            // 
            // comboBoxProject
            // 
            this.comboBoxProject.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxProject.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.comboBoxProject.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.comboBoxProject.FormattingEnabled = true;
            this.comboBoxProject.Location = new System.Drawing.Point(100, 37);
            this.comboBoxProject.Name = "comboBoxProject";
            this.comboBoxProject.Size = new System.Drawing.Size(202, 21);
            this.comboBoxProject.TabIndex = 3;
            this.comboBoxProject.SelectedIndexChanged += new System.EventHandler(this.comboBoxProject_SelectedIndexChanged);
            this.comboBoxProject.Leave += new System.EventHandler(this.comboBoxProject_Leave);
            // 
            // linkAddProject
            // 
            this.linkAddProject.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.linkAddProject.AutoSize = true;
            this.linkAddProject.Location = new System.Drawing.Point(218, 61);
            this.linkAddProject.Name = "linkAddProject";
            this.linkAddProject.Size = new System.Drawing.Size(84, 13);
            this.linkAddProject.TabIndex = 4;
            this.linkAddProject.TabStop = true;
            this.linkAddProject.Text = "Add new project";
            this.linkAddProject.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.linkAddProject_LinkClicked);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(15, 8);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(50, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "Duration:";
            // 
            // textBoxDuration
            // 
            this.textBoxDuration.Location = new System.Drawing.Point(100, 5);
            this.textBoxDuration.Name = "textBoxDuration";
            this.textBoxDuration.Size = new System.Drawing.Size(140, 20);
            this.textBoxDuration.TabIndex = 6;
            this.textBoxDuration.Leave += new System.EventHandler(this.textBoxDuration_Leave);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(15, 34);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(75, 13);
            this.label4.TabIndex = 7;
            this.label4.Text = "Start-end time:";
            // 
            // textBoxStartTime
            // 
            this.textBoxStartTime.Location = new System.Drawing.Point(100, 31);
            this.textBoxStartTime.Name = "textBoxStartTime";
            this.textBoxStartTime.Size = new System.Drawing.Size(60, 20);
            this.textBoxStartTime.TabIndex = 8;
            this.textBoxStartTime.Leave += new System.EventHandler(this.textBoxStartTime_Leave);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(165, 34);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(10, 13);
            this.label5.TabIndex = 9;
            this.label5.Text = "-";
            // 
            // textBoxEndTime
            // 
            this.textBoxEndTime.Location = new System.Drawing.Point(180, 31);
            this.textBoxEndTime.Name = "textBoxEndTime";
            this.textBoxEndTime.Size = new System.Drawing.Size(60, 20);
            this.textBoxEndTime.TabIndex = 10;
            this.textBoxEndTime.Leave += new System.EventHandler(this.textBoxEndTime_Leave);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(17, 63);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(33, 13);
            this.label6.TabIndex = 11;
            this.label6.Text = "Date:";
            // 
            // dateTimePickerStartDate
            // 
            this.dateTimePickerStartDate.Location = new System.Drawing.Point(100, 57);
            this.dateTimePickerStartDate.Name = "dateTimePickerStartDate";
            this.dateTimePickerStartDate.Size = new System.Drawing.Size(140, 20);
            this.dateTimePickerStartDate.TabIndex = 12;
            this.dateTimePickerStartDate.Leave += new System.EventHandler(this.dateTimePickerStartDate_Leave);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(17, 83);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(34, 13);
            this.label7.TabIndex = 13;
            this.label7.Text = "Tags:";
            // 
            // statusStripLastUpdate
            // 
            this.statusStripLastUpdate.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabelLastUpdate});
            this.statusStripLastUpdate.Location = new System.Drawing.Point(0, 428);
            this.statusStripLastUpdate.Name = "statusStripLastUpdate";
            this.statusStripLastUpdate.Size = new System.Drawing.Size(317, 22);
            this.statusStripLastUpdate.TabIndex = 14;
            // 
            // toolStripStatusLabelLastUpdate
            // 
            this.toolStripStatusLabelLastUpdate.Margin = new System.Windows.Forms.Padding(9, 3, 0, 2);
            this.toolStripStatusLabelLastUpdate.Name = "toolStripStatusLabelLastUpdate";
            this.toolStripStatusLabelLastUpdate.Size = new System.Drawing.Size(150, 17);
            this.toolStripStatusLabelLastUpdate.Text = "Last update: Today 2:03AM";
            // 
            // buttonsPanel
            // 
            this.buttonsPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonsPanel.Controls.Add(this.buttonContinue);
            this.buttonsPanel.Controls.Add(this.buttonDelete);
            this.buttonsPanel.Controls.Add(this.buttonDone);
            this.buttonsPanel.Location = new System.Drawing.Point(0, 399);
            this.buttonsPanel.Name = "buttonsPanel";
            this.buttonsPanel.Size = new System.Drawing.Size(317, 31);
            this.buttonsPanel.TabIndex = 16;
            // 
            // buttonContinue
            // 
            this.buttonContinue.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonContinue.Location = new System.Drawing.Point(227, 3);
            this.buttonContinue.Name = "buttonContinue";
            this.buttonContinue.Size = new System.Drawing.Size(75, 23);
            this.buttonContinue.TabIndex = 2;
            this.buttonContinue.Text = "Continue";
            this.buttonContinue.UseVisualStyleBackColor = true;
            this.buttonContinue.Click += new System.EventHandler(this.buttonContinue_Click);
            // 
            // buttonDelete
            // 
            this.buttonDelete.Location = new System.Drawing.Point(95, 3);
            this.buttonDelete.Name = "buttonDelete";
            this.buttonDelete.Size = new System.Drawing.Size(75, 23);
            this.buttonDelete.TabIndex = 1;
            this.buttonDelete.Text = "Delete";
            this.buttonDelete.UseVisualStyleBackColor = true;
            this.buttonDelete.Click += new System.EventHandler(this.buttonDelete_Click);
            // 
            // buttonDone
            // 
            this.buttonDone.Location = new System.Drawing.Point(10, 3);
            this.buttonDone.Name = "buttonDone";
            this.buttonDone.Size = new System.Drawing.Size(75, 23);
            this.buttonDone.TabIndex = 0;
            this.buttonDone.Text = "Done";
            this.buttonDone.UseVisualStyleBackColor = true;
            this.buttonDone.Click += new System.EventHandler(this.buttonDone_Click);
            // 
            // checkBoxBillable
            // 
            this.checkBoxBillable.AutoSize = true;
            this.checkBoxBillable.Location = new System.Drawing.Point(100, 153);
            this.checkBoxBillable.Name = "checkBoxBillable";
            this.checkBoxBillable.Size = new System.Drawing.Size(59, 17);
            this.checkBoxBillable.TabIndex = 14;
            this.checkBoxBillable.Text = "Billable";
            this.checkBoxBillable.UseVisualStyleBackColor = true;
            this.checkBoxBillable.CheckedChanged += new System.EventHandler(this.checkBoxBillable_CheckedChanged);
            // 
            // checkedListBoxTags
            // 
            this.checkedListBoxTags.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.checkedListBoxTags.CheckOnClick = true;
            this.checkedListBoxTags.FormattingEnabled = true;
            this.checkedListBoxTags.Location = new System.Drawing.Point(100, 83);
            this.checkedListBoxTags.Name = "checkedListBoxTags";
            this.checkedListBoxTags.Size = new System.Drawing.Size(202, 64);
            this.checkedListBoxTags.TabIndex = 13;
            this.checkedListBoxTags.Leave += new System.EventHandler(this.checkedListBoxTags_Leave);
            // 
            // timerRunningDuration
            // 
            this.timerRunningDuration.Enabled = true;
            this.timerRunningDuration.Interval = 1000;
            this.timerRunningDuration.Tick += new System.EventHandler(this.timerRunningDuration_Tick);
            // 
            // panelBottom
            // 
            this.panelBottom.Controls.Add(this.label3);
            this.panelBottom.Controls.Add(this.checkedListBoxTags);
            this.panelBottom.Controls.Add(this.textBoxDuration);
            this.panelBottom.Controls.Add(this.checkBoxBillable);
            this.panelBottom.Controls.Add(this.label4);
            this.panelBottom.Controls.Add(this.textBoxStartTime);
            this.panelBottom.Controls.Add(this.label5);
            this.panelBottom.Controls.Add(this.label7);
            this.panelBottom.Controls.Add(this.textBoxEndTime);
            this.panelBottom.Controls.Add(this.dateTimePickerStartDate);
            this.panelBottom.Controls.Add(this.label6);
            this.panelBottom.Location = new System.Drawing.Point(0, 77);
            this.panelBottom.Name = "panelBottom";
            this.panelBottom.Size = new System.Drawing.Size(317, 175);
            this.panelBottom.TabIndex = 17;
            // 
            // panelAddProject
            // 
            this.panelAddProject.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panelAddProject.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panelAddProject.Controls.Add(this.labelWorkspace);
            this.panelAddProject.Controls.Add(this.comboBoxWorkspace);
            this.panelAddProject.Controls.Add(this.comboBoxClient);
            this.panelAddProject.Controls.Add(this.checkBoxPublic);
            this.panelAddProject.Controls.Add(this.textBoxProjectName);
            this.panelAddProject.Controls.Add(this.labelClient);
            this.panelAddProject.Controls.Add(this.labelProjectName);
            this.panelAddProject.Location = new System.Drawing.Point(0, 258);
            this.panelAddProject.Name = "panelAddProject";
            this.panelAddProject.Size = new System.Drawing.Size(317, 122);
            this.panelAddProject.TabIndex = 18;
            this.panelAddProject.Visible = false;
            // 
            // labelProjectName
            // 
            this.labelProjectName.AutoSize = true;
            this.labelProjectName.Location = new System.Drawing.Point(15, 8);
            this.labelProjectName.Name = "labelProjectName";
            this.labelProjectName.Size = new System.Drawing.Size(72, 13);
            this.labelProjectName.TabIndex = 0;
            this.labelProjectName.Text = "Project name:";
            // 
            // labelClient
            // 
            this.labelClient.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.labelClient.AutoSize = true;
            this.labelClient.Location = new System.Drawing.Point(18, 94);
            this.labelClient.Name = "labelClient";
            this.labelClient.Size = new System.Drawing.Size(36, 13);
            this.labelClient.TabIndex = 1;
            this.labelClient.Text = "Client:";
            // 
            // textBoxProjectName
            // 
            this.textBoxProjectName.Location = new System.Drawing.Point(100, 5);
            this.textBoxProjectName.Name = "textBoxProjectName";
            this.textBoxProjectName.Size = new System.Drawing.Size(202, 20);
            this.textBoxProjectName.TabIndex = 2;
            // 
            // checkBoxPublic
            // 
            this.checkBoxPublic.AutoSize = true;
            this.checkBoxPublic.Location = new System.Drawing.Point(100, 31);
            this.checkBoxPublic.Name = "checkBoxPublic";
            this.checkBoxPublic.Size = new System.Drawing.Size(180, 17);
            this.checkBoxPublic.TabIndex = 3;
            this.checkBoxPublic.Text = "Public (visible to the whole team)";
            this.checkBoxPublic.UseVisualStyleBackColor = true;
            // 
            // comboBoxClient
            // 
            this.comboBoxClient.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.comboBoxClient.FormattingEnabled = true;
            this.comboBoxClient.Location = new System.Drawing.Point(100, 91);
            this.comboBoxClient.Name = "comboBoxClient";
            this.comboBoxClient.Size = new System.Drawing.Size(202, 21);
            this.comboBoxClient.TabIndex = 4;
            // 
            // comboBoxWorkspace
            // 
            this.comboBoxWorkspace.FormattingEnabled = true;
            this.comboBoxWorkspace.Location = new System.Drawing.Point(100, 58);
            this.comboBoxWorkspace.Name = "comboBoxWorkspace";
            this.comboBoxWorkspace.Size = new System.Drawing.Size(202, 21);
            this.comboBoxWorkspace.TabIndex = 5;
            this.comboBoxWorkspace.Visible = false;
            // 
            // labelWorkspace
            // 
            this.labelWorkspace.AutoSize = true;
            this.labelWorkspace.Location = new System.Drawing.Point(15, 64);
            this.labelWorkspace.Name = "labelWorkspace";
            this.labelWorkspace.Size = new System.Drawing.Size(65, 13);
            this.labelWorkspace.TabIndex = 6;
            this.labelWorkspace.Text = "Workspace:";
            this.labelWorkspace.Visible = false;
            // 
            // TimeEntryEditViewController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.panelAddProject);
            this.Controls.Add(this.panelBottom);
            this.Controls.Add(this.buttonsPanel);
            this.Controls.Add(this.statusStripLastUpdate);
            this.Controls.Add(this.linkAddProject);
            this.Controls.Add(this.comboBoxProject);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.comboBoxDescription);
            this.Controls.Add(this.label1);
            this.Name = "TimeEntryEditViewController";
            this.Size = new System.Drawing.Size(317, 450);
            this.Load += new System.EventHandler(this.TimeEntryEditViewController_Load);
            this.statusStripLastUpdate.ResumeLayout(false);
            this.statusStripLastUpdate.PerformLayout();
            this.buttonsPanel.ResumeLayout(false);
            this.panelBottom.ResumeLayout(false);
            this.panelBottom.PerformLayout();
            this.panelAddProject.ResumeLayout(false);
            this.panelAddProject.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox comboBoxDescription;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox comboBoxProject;
        private System.Windows.Forms.LinkLabel linkAddProject;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBoxDuration;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox textBoxStartTime;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox textBoxEndTime;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.DateTimePicker dateTimePickerStartDate;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.StatusStrip statusStripLastUpdate;
        private System.Windows.Forms.Panel buttonsPanel;
        private System.Windows.Forms.Button buttonDelete;
        private System.Windows.Forms.Button buttonDone;
        private System.Windows.Forms.Button buttonContinue;
        private System.Windows.Forms.CheckBox checkBoxBillable;
        private System.Windows.Forms.CheckedListBox checkedListBoxTags;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabelLastUpdate;
        private System.Windows.Forms.Timer timerRunningDuration;
        private System.Windows.Forms.Panel panelBottom;
        private System.Windows.Forms.Panel panelAddProject;
        private System.Windows.Forms.Label labelWorkspace;
        private System.Windows.Forms.ComboBox comboBoxWorkspace;
        private System.Windows.Forms.ComboBox comboBoxClient;
        private System.Windows.Forms.CheckBox checkBoxPublic;
        private System.Windows.Forms.TextBox textBoxProjectName;
        private System.Windows.Forms.Label labelClient;
        private System.Windows.Forms.Label labelProjectName;
    }
}
