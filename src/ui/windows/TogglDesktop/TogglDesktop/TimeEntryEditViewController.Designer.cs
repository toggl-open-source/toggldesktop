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
            this.labelDescription = new System.Windows.Forms.Label();
            this.labelProject = new System.Windows.Forms.Label();
            this.linkAddProject = new System.Windows.Forms.LinkLabel();
            this.labelDuration = new System.Windows.Forms.Label();
            this.textBoxDuration = new System.Windows.Forms.TextBox();
            this.labelStartEnd = new System.Windows.Forms.Label();
            this.textBoxStartTime = new System.Windows.Forms.TextBox();
            this.labelDash = new System.Windows.Forms.Label();
            this.textBoxEndTime = new System.Windows.Forms.TextBox();
            this.labelDate = new System.Windows.Forms.Label();
            this.dateTimePickerStartDate = new System.Windows.Forms.DateTimePicker();
            this.labelTags = new System.Windows.Forms.Label();
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
            this.panelDateTag = new System.Windows.Forms.Panel();
            this.panelStartEndTime = new System.Windows.Forms.Panel();
            this.panelAddProject = new System.Windows.Forms.Panel();
            this.labelWorkspace = new System.Windows.Forms.Label();
            this.comboBoxWorkspace = new System.Windows.Forms.ComboBox();
            this.comboBoxClient = new System.Windows.Forms.ComboBox();
            this.checkBoxPublic = new System.Windows.Forms.CheckBox();
            this.textBoxProjectName = new System.Windows.Forms.TextBox();
            this.labelClient = new System.Windows.Forms.Label();
            this.labelProjectName = new System.Windows.Forms.Label();
            this.descriptionButton = new System.Windows.Forms.Button();
            this.projectButton = new System.Windows.Forms.Button();
            this.panelDuration = new System.Windows.Forms.Panel();
            this.panelBillable = new System.Windows.Forms.Panel();
            this.comboBoxProject = new TogglDesktop.CustomComboBox();
            this.comboBoxDescription = new TogglDesktop.CustomComboBox();
            this.statusStripLastUpdate.SuspendLayout();
            this.buttonsPanel.SuspendLayout();
            this.panelBottom.SuspendLayout();
            this.panelDateTag.SuspendLayout();
            this.panelStartEndTime.SuspendLayout();
            this.panelAddProject.SuspendLayout();
            this.panelDuration.SuspendLayout();
            this.panelBillable.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelDescription
            // 
            this.labelDescription.AutoSize = true;
            this.labelDescription.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelDescription.Location = new System.Drawing.Point(22, 20);
            this.labelDescription.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.labelDescription.Name = "labelDescription";
            this.labelDescription.Size = new System.Drawing.Size(110, 21);
            this.labelDescription.TabIndex = 0;
            this.labelDescription.Text = "Description:";
            // 
            // labelProject
            // 
            this.labelProject.AutoSize = true;
            this.labelProject.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelProject.Location = new System.Drawing.Point(22, 62);
            this.labelProject.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.labelProject.Name = "labelProject";
            this.labelProject.Size = new System.Drawing.Size(72, 21);
            this.labelProject.TabIndex = 2;
            this.labelProject.Text = "Project:";
            // 
            // linkAddProject
            // 
            this.linkAddProject.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.linkAddProject.AutoSize = true;
            this.linkAddProject.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.linkAddProject.Location = new System.Drawing.Point(315, 94);
            this.linkAddProject.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.linkAddProject.Name = "linkAddProject";
            this.linkAddProject.Size = new System.Drawing.Size(146, 21);
            this.linkAddProject.TabIndex = 2;
            this.linkAddProject.TabStop = true;
            this.linkAddProject.Text = "Add new project";
            this.linkAddProject.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.linkAddProject_LinkClicked);
            // 
            // labelDuration
            // 
            this.labelDuration.AutoSize = true;
            this.labelDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelDuration.Location = new System.Drawing.Point(22, 9);
            this.labelDuration.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.labelDuration.Name = "labelDuration";
            this.labelDuration.Size = new System.Drawing.Size(87, 21);
            this.labelDuration.TabIndex = 5;
            this.labelDuration.Text = "Duration:";
            // 
            // textBoxDuration
            // 
            this.textBoxDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.textBoxDuration.Location = new System.Drawing.Point(165, 5);
            this.textBoxDuration.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.textBoxDuration.Name = "textBoxDuration";
            this.textBoxDuration.Size = new System.Drawing.Size(208, 33);
            this.textBoxDuration.TabIndex = 4;
            this.textBoxDuration.Leave += new System.EventHandler(this.textBoxDuration_Leave);
            // 
            // labelStartEnd
            // 
            this.labelStartEnd.AutoSize = true;
            this.labelStartEnd.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelStartEnd.Location = new System.Drawing.Point(22, 12);
            this.labelStartEnd.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.labelStartEnd.Name = "labelStartEnd";
            this.labelStartEnd.Size = new System.Drawing.Size(136, 21);
            this.labelStartEnd.TabIndex = 7;
            this.labelStartEnd.Text = "Start-end time:";
            // 
            // textBoxStartTime
            // 
            this.textBoxStartTime.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.textBoxStartTime.Location = new System.Drawing.Point(165, 8);
            this.textBoxStartTime.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.textBoxStartTime.Name = "textBoxStartTime";
            this.textBoxStartTime.Size = new System.Drawing.Size(88, 33);
            this.textBoxStartTime.TabIndex = 6;
            this.textBoxStartTime.Leave += new System.EventHandler(this.textBoxStartTime_Leave);
            // 
            // labelDash
            // 
            this.labelDash.AutoSize = true;
            this.labelDash.Location = new System.Drawing.Point(262, 12);
            this.labelDash.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.labelDash.Name = "labelDash";
            this.labelDash.Size = new System.Drawing.Size(14, 20);
            this.labelDash.TabIndex = 9;
            this.labelDash.Text = "-";
            // 
            // textBoxEndTime
            // 
            this.textBoxEndTime.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.textBoxEndTime.Location = new System.Drawing.Point(285, 8);
            this.textBoxEndTime.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.textBoxEndTime.Name = "textBoxEndTime";
            this.textBoxEndTime.Size = new System.Drawing.Size(88, 33);
            this.textBoxEndTime.TabIndex = 7;
            this.textBoxEndTime.Leave += new System.EventHandler(this.textBoxEndTime_Leave);
            // 
            // labelDate
            // 
            this.labelDate.AutoSize = true;
            this.labelDate.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelDate.Location = new System.Drawing.Point(22, 12);
            this.labelDate.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.labelDate.Name = "labelDate";
            this.labelDate.Size = new System.Drawing.Size(54, 21);
            this.labelDate.TabIndex = 11;
            this.labelDate.Text = "Date:";
            // 
            // dateTimePickerStartDate
            // 
            this.dateTimePickerStartDate.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.dateTimePickerStartDate.Location = new System.Drawing.Point(165, 8);
            this.dateTimePickerStartDate.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.dateTimePickerStartDate.Name = "dateTimePickerStartDate";
            this.dateTimePickerStartDate.Size = new System.Drawing.Size(208, 33);
            this.dateTimePickerStartDate.TabIndex = 9;
            this.dateTimePickerStartDate.Leave += new System.EventHandler(this.dateTimePickerStartDate_Leave);
            // 
            // labelTags
            // 
            this.labelTags.AutoSize = true;
            this.labelTags.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelTags.Location = new System.Drawing.Point(22, 58);
            this.labelTags.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.labelTags.Name = "labelTags";
            this.labelTags.Size = new System.Drawing.Size(55, 21);
            this.labelTags.TabIndex = 13;
            this.labelTags.Text = "Tags:";
            // 
            // statusStripLastUpdate
            // 
            this.statusStripLastUpdate.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabelLastUpdate});
            this.statusStripLastUpdate.Location = new System.Drawing.Point(0, 589);
            this.statusStripLastUpdate.Name = "statusStripLastUpdate";
            this.statusStripLastUpdate.Padding = new System.Windows.Forms.Padding(2, 0, 21, 0);
            this.statusStripLastUpdate.Size = new System.Drawing.Size(476, 26);
            this.statusStripLastUpdate.TabIndex = 14;
            // 
            // toolStripStatusLabelLastUpdate
            // 
            this.toolStripStatusLabelLastUpdate.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.toolStripStatusLabelLastUpdate.Margin = new System.Windows.Forms.Padding(6, 3, 0, 2);
            this.toolStripStatusLabelLastUpdate.Name = "toolStripStatusLabelLastUpdate";
            this.toolStripStatusLabelLastUpdate.Size = new System.Drawing.Size(238, 21);
            this.toolStripStatusLabelLastUpdate.Text = "Last update: Today 2:03AM";
            // 
            // buttonsPanel
            // 
            this.buttonsPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonsPanel.Controls.Add(this.buttonContinue);
            this.buttonsPanel.Controls.Add(this.buttonDelete);
            this.buttonsPanel.Controls.Add(this.buttonDone);
            this.buttonsPanel.Location = new System.Drawing.Point(0, 537);
            this.buttonsPanel.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.buttonsPanel.Name = "buttonsPanel";
            this.buttonsPanel.Size = new System.Drawing.Size(476, 48);
            this.buttonsPanel.TabIndex = 16;
            // 
            // buttonContinue
            // 
            this.buttonContinue.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonContinue.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.buttonContinue.Location = new System.Drawing.Point(345, 5);
            this.buttonContinue.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.buttonContinue.Name = "buttonContinue";
            this.buttonContinue.Size = new System.Drawing.Size(112, 35);
            this.buttonContinue.TabIndex = 11;
            this.buttonContinue.Text = "Continue";
            this.buttonContinue.UseVisualStyleBackColor = true;
            this.buttonContinue.Click += new System.EventHandler(this.buttonContinue_Click);
            // 
            // buttonDelete
            // 
            this.buttonDelete.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.buttonDelete.Location = new System.Drawing.Point(142, 5);
            this.buttonDelete.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.buttonDelete.Name = "buttonDelete";
            this.buttonDelete.Size = new System.Drawing.Size(112, 35);
            this.buttonDelete.TabIndex = 10;
            this.buttonDelete.Text = "Delete";
            this.buttonDelete.UseVisualStyleBackColor = true;
            this.buttonDelete.Click += new System.EventHandler(this.buttonDelete_Click);
            // 
            // buttonDone
            // 
            this.buttonDone.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.buttonDone.Location = new System.Drawing.Point(15, 5);
            this.buttonDone.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.buttonDone.Name = "buttonDone";
            this.buttonDone.Size = new System.Drawing.Size(112, 35);
            this.buttonDone.TabIndex = 9;
            this.buttonDone.Text = "Done";
            this.buttonDone.UseVisualStyleBackColor = true;
            this.buttonDone.Click += new System.EventHandler(this.buttonDone_Click);
            // 
            // checkBoxBillable
            // 
            this.checkBoxBillable.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.checkBoxBillable.AutoSize = true;
            this.checkBoxBillable.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.checkBoxBillable.Location = new System.Drawing.Point(165, 12);
            this.checkBoxBillable.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.checkBoxBillable.Name = "checkBoxBillable";
            this.checkBoxBillable.Size = new System.Drawing.Size(97, 25);
            this.checkBoxBillable.TabIndex = 11;
            this.checkBoxBillable.Text = "Billable";
            this.checkBoxBillable.UseVisualStyleBackColor = true;
            this.checkBoxBillable.CheckedChanged += new System.EventHandler(this.checkBoxBillable_CheckedChanged);
            // 
            // checkedListBoxTags
            // 
            this.checkedListBoxTags.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.checkedListBoxTags.CheckOnClick = true;
            this.checkedListBoxTags.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.checkedListBoxTags.FormattingEnabled = true;
            this.checkedListBoxTags.Location = new System.Drawing.Point(165, 54);
            this.checkedListBoxTags.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.checkedListBoxTags.Name = "checkedListBoxTags";
            this.checkedListBoxTags.Size = new System.Drawing.Size(286, 172);
            this.checkedListBoxTags.TabIndex = 10;
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
            this.panelBottom.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panelBottom.Controls.Add(this.panelBillable);
            this.panelBottom.Controls.Add(this.panelDuration);
            this.panelBottom.Controls.Add(this.panelDateTag);
            this.panelBottom.Controls.Add(this.panelStartEndTime);
            this.panelBottom.Location = new System.Drawing.Point(0, 118);
            this.panelBottom.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.panelBottom.Name = "panelBottom";
            this.panelBottom.Size = new System.Drawing.Size(476, 406);
            this.panelBottom.TabIndex = 3;
            this.panelBottom.TabStop = true;
            // 
            // panelDateTag
            // 
            this.panelDateTag.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panelDateTag.Controls.Add(this.checkedListBoxTags);
            this.panelDateTag.Controls.Add(this.labelDate);
            this.panelDateTag.Controls.Add(this.dateTimePickerStartDate);
            this.panelDateTag.Controls.Add(this.labelTags);
            this.panelDateTag.Location = new System.Drawing.Point(0, 92);
            this.panelDateTag.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.panelDateTag.MinimumSize = new System.Drawing.Size(0, 154);
            this.panelDateTag.Name = "panelDateTag";
            this.panelDateTag.Size = new System.Drawing.Size(476, 269);
            this.panelDateTag.TabIndex = 8;
            this.panelDateTag.TabStop = true;
            // 
            // panelStartEndTime
            // 
            this.panelStartEndTime.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panelStartEndTime.Controls.Add(this.labelStartEnd);
            this.panelStartEndTime.Controls.Add(this.textBoxEndTime);
            this.panelStartEndTime.Controls.Add(this.labelDash);
            this.panelStartEndTime.Controls.Add(this.textBoxStartTime);
            this.panelStartEndTime.Location = new System.Drawing.Point(0, 46);
            this.panelStartEndTime.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.panelStartEndTime.Name = "panelStartEndTime";
            this.panelStartEndTime.Size = new System.Drawing.Size(476, 46);
            this.panelStartEndTime.TabIndex = 5;
            this.panelStartEndTime.TabStop = true;
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
            this.panelAddProject.Location = new System.Drawing.Point(0, 858);
            this.panelAddProject.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.panelAddProject.Name = "panelAddProject";
            this.panelAddProject.Size = new System.Drawing.Size(474, 187);
            this.panelAddProject.TabIndex = 1;
            this.panelAddProject.Visible = false;
            // 
            // labelWorkspace
            // 
            this.labelWorkspace.AutoSize = true;
            this.labelWorkspace.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelWorkspace.Location = new System.Drawing.Point(22, 98);
            this.labelWorkspace.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.labelWorkspace.Name = "labelWorkspace";
            this.labelWorkspace.Size = new System.Drawing.Size(106, 21);
            this.labelWorkspace.TabIndex = 6;
            this.labelWorkspace.Text = "Workspace:";
            this.labelWorkspace.Visible = false;
            // 
            // comboBoxWorkspace
            // 
            this.comboBoxWorkspace.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxWorkspace.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.comboBoxWorkspace.FormattingEnabled = true;
            this.comboBoxWorkspace.Location = new System.Drawing.Point(165, 89);
            this.comboBoxWorkspace.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.comboBoxWorkspace.Name = "comboBoxWorkspace";
            this.comboBoxWorkspace.Size = new System.Drawing.Size(286, 29);
            this.comboBoxWorkspace.TabIndex = 4;
            this.comboBoxWorkspace.TabStop = false;
            this.comboBoxWorkspace.Visible = false;
            // 
            // comboBoxClient
            // 
            this.comboBoxClient.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxClient.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.comboBoxClient.FormattingEnabled = true;
            this.comboBoxClient.Location = new System.Drawing.Point(165, 140);
            this.comboBoxClient.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.comboBoxClient.Name = "comboBoxClient";
            this.comboBoxClient.Size = new System.Drawing.Size(286, 29);
            this.comboBoxClient.TabIndex = 5;
            this.comboBoxClient.TabStop = false;
            // 
            // checkBoxPublic
            // 
            this.checkBoxPublic.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.checkBoxPublic.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.checkBoxPublic.Location = new System.Drawing.Point(165, 48);
            this.checkBoxPublic.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.checkBoxPublic.Name = "checkBoxPublic";
            this.checkBoxPublic.Size = new System.Drawing.Size(292, 29);
            this.checkBoxPublic.TabIndex = 3;
            this.checkBoxPublic.TabStop = false;
            this.checkBoxPublic.Text = "Public (visible to the whole team)";
            this.checkBoxPublic.UseVisualStyleBackColor = true;
            // 
            // textBoxProjectName
            // 
            this.textBoxProjectName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxProjectName.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.textBoxProjectName.Location = new System.Drawing.Point(165, 8);
            this.textBoxProjectName.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.textBoxProjectName.Name = "textBoxProjectName";
            this.textBoxProjectName.Size = new System.Drawing.Size(286, 33);
            this.textBoxProjectName.TabIndex = 2;
            this.textBoxProjectName.TabStop = false;
            // 
            // labelClient
            // 
            this.labelClient.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.labelClient.AutoSize = true;
            this.labelClient.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelClient.Location = new System.Drawing.Point(27, 145);
            this.labelClient.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.labelClient.Name = "labelClient";
            this.labelClient.Size = new System.Drawing.Size(63, 21);
            this.labelClient.TabIndex = 1;
            this.labelClient.Text = "Client:";
            // 
            // labelProjectName
            // 
            this.labelProjectName.AutoSize = true;
            this.labelProjectName.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelProjectName.Location = new System.Drawing.Point(22, 12);
            this.labelProjectName.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.labelProjectName.Name = "labelProjectName";
            this.labelProjectName.Size = new System.Drawing.Size(123, 21);
            this.labelProjectName.TabIndex = 0;
            this.labelProjectName.Text = "Project name:";
            // 
            // descriptionButton
            // 
            this.descriptionButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.descriptionButton.BackColor = System.Drawing.Color.Transparent;
            this.descriptionButton.FlatAppearance.BorderSize = 0;
            this.descriptionButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.descriptionButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(0)));
            this.descriptionButton.Location = new System.Drawing.Point(418, 17);
            this.descriptionButton.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.descriptionButton.Name = "descriptionButton";
            this.descriptionButton.Size = new System.Drawing.Size(33, 34);
            this.descriptionButton.TabIndex = 17;
            this.descriptionButton.Text = "▼";
            this.descriptionButton.UseVisualStyleBackColor = false;
            // 
            // projectButton
            // 
            this.projectButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.projectButton.BackColor = System.Drawing.Color.Transparent;
            this.projectButton.FlatAppearance.BorderSize = 0;
            this.projectButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.projectButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Pixel, ((byte)(0)));
            this.projectButton.Location = new System.Drawing.Point(418, 58);
            this.projectButton.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.projectButton.Name = "projectButton";
            this.projectButton.Size = new System.Drawing.Size(33, 34);
            this.projectButton.TabIndex = 18;
            this.projectButton.Text = "▼";
            this.projectButton.UseVisualStyleBackColor = false;
            // 
            // panelDuration
            // 
            this.panelDuration.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panelDuration.Controls.Add(this.textBoxDuration);
            this.panelDuration.Controls.Add(this.labelDuration);
            this.panelDuration.Location = new System.Drawing.Point(0, 0);
            this.panelDuration.Name = "panelDuration";
            this.panelDuration.Size = new System.Drawing.Size(476, 46);
            this.panelDuration.TabIndex = 9;
            // 
            // panelBillable
            // 
            this.panelBillable.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panelBillable.Controls.Add(this.checkBoxBillable);
            this.panelBillable.Location = new System.Drawing.Point(0, 359);
            this.panelBillable.Name = "panelBillable";
            this.panelBillable.Size = new System.Drawing.Size(476, 47);
            this.panelBillable.TabIndex = 10;
            // 
            // comboBoxProject
            // 
            this.comboBoxProject.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxProject.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.comboBoxProject.Location = new System.Drawing.Point(165, 57);
            this.comboBoxProject.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.comboBoxProject.Name = "comboBoxProject";
            this.comboBoxProject.Size = new System.Drawing.Size(286, 33);
            this.comboBoxProject.TabIndex = 1;
            this.comboBoxProject.KeyUp += new System.Windows.Forms.KeyEventHandler(this.comboBoxProject_KeyUp);
            this.comboBoxProject.Leave += new System.EventHandler(this.comboBoxProject_Leave);
            this.comboBoxProject.PreviewKeyDown += new System.Windows.Forms.PreviewKeyDownEventHandler(this.comboBoxProject_PreviewKeyDown);
            // 
            // comboBoxDescription
            // 
            this.comboBoxDescription.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxDescription.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.Suggest;
            this.comboBoxDescription.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.comboBoxDescription.Location = new System.Drawing.Point(165, 15);
            this.comboBoxDescription.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.comboBoxDescription.Name = "comboBoxDescription";
            this.comboBoxDescription.Size = new System.Drawing.Size(286, 33);
            this.comboBoxDescription.TabIndex = 0;
            this.comboBoxDescription.KeyUp += new System.Windows.Forms.KeyEventHandler(this.comboBoxDescription_KeyUp);
            this.comboBoxDescription.Leave += new System.EventHandler(this.comboBoxDescription_Leave);
            this.comboBoxDescription.PreviewKeyDown += new System.Windows.Forms.PreviewKeyDownEventHandler(this.comboBoxDescription_PreviewKeyDown);
            // 
            // TimeEntryEditViewController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.projectButton);
            this.Controls.Add(this.descriptionButton);
            this.Controls.Add(this.panelAddProject);
            this.Controls.Add(this.panelBottom);
            this.Controls.Add(this.buttonsPanel);
            this.Controls.Add(this.statusStripLastUpdate);
            this.Controls.Add(this.linkAddProject);
            this.Controls.Add(this.comboBoxProject);
            this.Controls.Add(this.labelProject);
            this.Controls.Add(this.comboBoxDescription);
            this.Controls.Add(this.labelDescription);
            this.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.Name = "TimeEntryEditViewController";
            this.Size = new System.Drawing.Size(476, 615);
            this.Load += new System.EventHandler(this.TimeEntryEditViewController_Load);
            this.statusStripLastUpdate.ResumeLayout(false);
            this.statusStripLastUpdate.PerformLayout();
            this.buttonsPanel.ResumeLayout(false);
            this.panelBottom.ResumeLayout(false);
            this.panelDateTag.ResumeLayout(false);
            this.panelDateTag.PerformLayout();
            this.panelStartEndTime.ResumeLayout(false);
            this.panelStartEndTime.PerformLayout();
            this.panelAddProject.ResumeLayout(false);
            this.panelAddProject.PerformLayout();
            this.panelDuration.ResumeLayout(false);
            this.panelDuration.PerformLayout();
            this.panelBillable.ResumeLayout(false);
            this.panelBillable.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelDescription;
        private CustomComboBox comboBoxDescription;
        private System.Windows.Forms.Label labelProject;
        private CustomComboBox comboBoxProject;
        private System.Windows.Forms.LinkLabel linkAddProject;
        private System.Windows.Forms.Label labelDuration;
        private System.Windows.Forms.TextBox textBoxDuration;
        private System.Windows.Forms.Label labelStartEnd;
        private System.Windows.Forms.TextBox textBoxStartTime;
        private System.Windows.Forms.Label labelDash;
        private System.Windows.Forms.TextBox textBoxEndTime;
        private System.Windows.Forms.Label labelDate;
        private System.Windows.Forms.DateTimePicker dateTimePickerStartDate;
        private System.Windows.Forms.Label labelTags;
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
        private System.Windows.Forms.Panel panelStartEndTime;
        private System.Windows.Forms.Panel panelDateTag;
        private System.Windows.Forms.Button descriptionButton;
        private System.Windows.Forms.Button projectButton;
        private System.Windows.Forms.Panel panelDuration;
        private System.Windows.Forms.Panel panelBillable;
    }
}
