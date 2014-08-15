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
            this.panelBillable = new System.Windows.Forms.Panel();
            this.panelDuration = new System.Windows.Forms.Panel();
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
            this.comboBoxProject = new TogglDesktop.CustomComboBox();
            this.comboBoxDescription = new TogglDesktop.CustomComboBox();
            this.statusStripLastUpdate.SuspendLayout();
            this.buttonsPanel.SuspendLayout();
            this.panelBottom.SuspendLayout();
            this.panelBillable.SuspendLayout();
            this.panelDuration.SuspendLayout();
            this.panelDateTag.SuspendLayout();
            this.panelStartEndTime.SuspendLayout();
            this.panelAddProject.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelDescription
            // 
            this.labelDescription.AutoSize = true;
            this.labelDescription.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelDescription.Location = new System.Drawing.Point(15, 13);
            this.labelDescription.Name = "labelDescription";
            this.labelDescription.Size = new System.Drawing.Size(72, 15);
            this.labelDescription.TabIndex = 0;
            this.labelDescription.Text = "Description:";
            // 
            // labelProject
            // 
            this.labelProject.AutoSize = true;
            this.labelProject.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelProject.Location = new System.Drawing.Point(15, 43);
            this.labelProject.Name = "labelProject";
            this.labelProject.Size = new System.Drawing.Size(47, 15);
            this.labelProject.TabIndex = 2;
            this.labelProject.Text = "Project:";
            // 
            // linkAddProject
            // 
            this.linkAddProject.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.linkAddProject.AutoSize = true;
            this.linkAddProject.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.linkAddProject.Location = new System.Drawing.Point(210, 64);
            this.linkAddProject.Name = "linkAddProject";
            this.linkAddProject.Size = new System.Drawing.Size(94, 15);
            this.linkAddProject.TabIndex = 2;
            this.linkAddProject.TabStop = true;
            this.linkAddProject.Text = "Add new project";
            this.linkAddProject.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.linkAddProject_LinkClicked);
            // 
            // labelDuration
            // 
            this.labelDuration.AutoSize = true;
            this.labelDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelDuration.Location = new System.Drawing.Point(15, 6);
            this.labelDuration.Name = "labelDuration";
            this.labelDuration.Size = new System.Drawing.Size(57, 15);
            this.labelDuration.TabIndex = 5;
            this.labelDuration.Text = "Duration:";
            // 
            // textBoxDuration
            // 
            this.textBoxDuration.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.textBoxDuration.Location = new System.Drawing.Point(110, 3);
            this.textBoxDuration.Name = "textBoxDuration";
            this.textBoxDuration.Size = new System.Drawing.Size(140, 24);
            this.textBoxDuration.TabIndex = 4;
            this.textBoxDuration.Leave += new System.EventHandler(this.textBoxDuration_Leave);
            // 
            // labelStartEnd
            // 
            this.labelStartEnd.AutoSize = true;
            this.labelStartEnd.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelStartEnd.Location = new System.Drawing.Point(15, 8);
            this.labelStartEnd.Name = "labelStartEnd";
            this.labelStartEnd.Size = new System.Drawing.Size(88, 15);
            this.labelStartEnd.TabIndex = 7;
            this.labelStartEnd.Text = "Start-end time:";
            // 
            // textBoxStartTime
            // 
            this.textBoxStartTime.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.textBoxStartTime.Location = new System.Drawing.Point(110, 5);
            this.textBoxStartTime.Name = "textBoxStartTime";
            this.textBoxStartTime.Size = new System.Drawing.Size(60, 24);
            this.textBoxStartTime.TabIndex = 6;
            this.textBoxStartTime.Leave += new System.EventHandler(this.textBoxStartTime_Leave);
            // 
            // labelDash
            // 
            this.labelDash.AutoSize = true;
            this.labelDash.Location = new System.Drawing.Point(175, 8);
            this.labelDash.Name = "labelDash";
            this.labelDash.Size = new System.Drawing.Size(10, 13);
            this.labelDash.TabIndex = 9;
            this.labelDash.Text = "-";
            // 
            // textBoxEndTime
            // 
            this.textBoxEndTime.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.textBoxEndTime.Location = new System.Drawing.Point(190, 5);
            this.textBoxEndTime.Name = "textBoxEndTime";
            this.textBoxEndTime.Size = new System.Drawing.Size(60, 24);
            this.textBoxEndTime.TabIndex = 7;
            this.textBoxEndTime.Leave += new System.EventHandler(this.textBoxEndTime_Leave);
            // 
            // labelDate
            // 
            this.labelDate.AutoSize = true;
            this.labelDate.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelDate.Location = new System.Drawing.Point(15, 8);
            this.labelDate.Name = "labelDate";
            this.labelDate.Size = new System.Drawing.Size(34, 15);
            this.labelDate.TabIndex = 11;
            this.labelDate.Text = "Date:";
            // 
            // dateTimePickerStartDate
            // 
            this.dateTimePickerStartDate.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.dateTimePickerStartDate.Location = new System.Drawing.Point(110, 5);
            this.dateTimePickerStartDate.Name = "dateTimePickerStartDate";
            this.dateTimePickerStartDate.Size = new System.Drawing.Size(140, 24);
            this.dateTimePickerStartDate.TabIndex = 9;
            this.dateTimePickerStartDate.Leave += new System.EventHandler(this.dateTimePickerStartDate_Leave);
            // 
            // labelTags
            // 
            this.labelTags.AutoSize = true;
            this.labelTags.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelTags.Location = new System.Drawing.Point(15, 38);
            this.labelTags.Name = "labelTags";
            this.labelTags.Size = new System.Drawing.Size(36, 15);
            this.labelTags.TabIndex = 13;
            this.labelTags.Text = "Tags:";
            // 
            // statusStripLastUpdate
            // 
            this.statusStripLastUpdate.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabelLastUpdate});
            this.statusStripLastUpdate.Location = new System.Drawing.Point(0, 378);
            this.statusStripLastUpdate.Name = "statusStripLastUpdate";
            this.statusStripLastUpdate.Size = new System.Drawing.Size(317, 22);
            this.statusStripLastUpdate.TabIndex = 14;
            // 
            // toolStripStatusLabelLastUpdate
            // 
            this.toolStripStatusLabelLastUpdate.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.toolStripStatusLabelLastUpdate.Margin = new System.Windows.Forms.Padding(6, 3, 0, 2);
            this.toolStripStatusLabelLastUpdate.Name = "toolStripStatusLabelLastUpdate";
            this.toolStripStatusLabelLastUpdate.Size = new System.Drawing.Size(152, 17);
            this.toolStripStatusLabelLastUpdate.Text = "Last update: Today 2:03AM";
            // 
            // buttonsPanel
            // 
            this.buttonsPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonsPanel.Controls.Add(this.buttonContinue);
            this.buttonsPanel.Controls.Add(this.buttonDelete);
            this.buttonsPanel.Controls.Add(this.buttonDone);
            this.buttonsPanel.Location = new System.Drawing.Point(0, 349);
            this.buttonsPanel.Name = "buttonsPanel";
            this.buttonsPanel.Size = new System.Drawing.Size(317, 31);
            this.buttonsPanel.TabIndex = 16;
            // 
            // buttonContinue
            // 
            this.buttonContinue.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonContinue.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.buttonContinue.Location = new System.Drawing.Point(230, 3);
            this.buttonContinue.Name = "buttonContinue";
            this.buttonContinue.Size = new System.Drawing.Size(75, 23);
            this.buttonContinue.TabIndex = 11;
            this.buttonContinue.Text = "Continue";
            this.buttonContinue.UseVisualStyleBackColor = true;
            this.buttonContinue.Click += new System.EventHandler(this.buttonContinue_Click);
            // 
            // buttonDelete
            // 
            this.buttonDelete.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.buttonDelete.Location = new System.Drawing.Point(95, 3);
            this.buttonDelete.Name = "buttonDelete";
            this.buttonDelete.Size = new System.Drawing.Size(75, 23);
            this.buttonDelete.TabIndex = 10;
            this.buttonDelete.Text = "Delete";
            this.buttonDelete.UseVisualStyleBackColor = true;
            this.buttonDelete.Click += new System.EventHandler(this.buttonDelete_Click);
            // 
            // buttonDone
            // 
            this.buttonDone.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.buttonDone.Location = new System.Drawing.Point(10, 3);
            this.buttonDone.Name = "buttonDone";
            this.buttonDone.Size = new System.Drawing.Size(75, 23);
            this.buttonDone.TabIndex = 9;
            this.buttonDone.Text = "Back";
            this.buttonDone.UseVisualStyleBackColor = true;
            this.buttonDone.Click += new System.EventHandler(this.buttonDone_Click);
            // 
            // checkBoxBillable
            // 
            this.checkBoxBillable.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.checkBoxBillable.AutoSize = true;
            this.checkBoxBillable.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.checkBoxBillable.Location = new System.Drawing.Point(110, 5);
            this.checkBoxBillable.Name = "checkBoxBillable";
            this.checkBoxBillable.Size = new System.Drawing.Size(63, 19);
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
            this.checkedListBoxTags.Location = new System.Drawing.Point(110, 35);
            this.checkedListBoxTags.Name = "checkedListBoxTags";
            this.checkedListBoxTags.Size = new System.Drawing.Size(192, 99);
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
            this.panelBottom.Location = new System.Drawing.Point(0, 80);
            this.panelBottom.Name = "panelBottom";
            this.panelBottom.Size = new System.Drawing.Size(317, 264);
            this.panelBottom.TabIndex = 3;
            this.panelBottom.TabStop = true;
            // 
            // panelBillable
            // 
            this.panelBillable.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panelBillable.Controls.Add(this.checkBoxBillable);
            this.panelBillable.Location = new System.Drawing.Point(0, 233);
            this.panelBillable.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.panelBillable.Name = "panelBillable";
            this.panelBillable.Size = new System.Drawing.Size(317, 31);
            this.panelBillable.TabIndex = 10;
            // 
            // panelDuration
            // 
            this.panelDuration.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.panelDuration.Controls.Add(this.textBoxDuration);
            this.panelDuration.Controls.Add(this.labelDuration);
            this.panelDuration.Location = new System.Drawing.Point(0, 0);
            this.panelDuration.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.panelDuration.Name = "panelDuration";
            this.panelDuration.Size = new System.Drawing.Size(317, 30);
            this.panelDuration.TabIndex = 9;
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
            this.panelDateTag.Location = new System.Drawing.Point(0, 60);
            this.panelDateTag.MinimumSize = new System.Drawing.Size(67, 100);
            this.panelDateTag.Name = "panelDateTag";
            this.panelDateTag.Size = new System.Drawing.Size(317, 175);
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
            this.panelStartEndTime.Location = new System.Drawing.Point(0, 30);
            this.panelStartEndTime.Name = "panelStartEndTime";
            this.panelStartEndTime.Size = new System.Drawing.Size(317, 30);
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
            this.panelAddProject.Location = new System.Drawing.Point(0, 558);
            this.panelAddProject.Name = "panelAddProject";
            this.panelAddProject.Size = new System.Drawing.Size(318, 122);
            this.panelAddProject.TabIndex = 1;
            this.panelAddProject.Visible = false;
            // 
            // labelWorkspace
            // 
            this.labelWorkspace.AutoSize = true;
            this.labelWorkspace.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelWorkspace.Location = new System.Drawing.Point(14, 64);
            this.labelWorkspace.Name = "labelWorkspace";
            this.labelWorkspace.Size = new System.Drawing.Size(68, 15);
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
            this.comboBoxWorkspace.Location = new System.Drawing.Point(109, 58);
            this.comboBoxWorkspace.Name = "comboBoxWorkspace";
            this.comboBoxWorkspace.Size = new System.Drawing.Size(193, 23);
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
            this.comboBoxClient.Location = new System.Drawing.Point(109, 91);
            this.comboBoxClient.Name = "comboBoxClient";
            this.comboBoxClient.Size = new System.Drawing.Size(193, 23);
            this.comboBoxClient.TabIndex = 5;
            this.comboBoxClient.TabStop = false;
            // 
            // checkBoxPublic
            // 
            this.checkBoxPublic.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.checkBoxPublic.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.checkBoxPublic.Location = new System.Drawing.Point(110, 31);
            this.checkBoxPublic.Name = "checkBoxPublic";
            this.checkBoxPublic.Size = new System.Drawing.Size(196, 19);
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
            this.textBoxProjectName.Location = new System.Drawing.Point(109, 5);
            this.textBoxProjectName.Name = "textBoxProjectName";
            this.textBoxProjectName.Size = new System.Drawing.Size(193, 24);
            this.textBoxProjectName.TabIndex = 2;
            this.textBoxProjectName.TabStop = false;
            // 
            // labelClient
            // 
            this.labelClient.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.labelClient.AutoSize = true;
            this.labelClient.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelClient.Location = new System.Drawing.Point(14, 94);
            this.labelClient.Name = "labelClient";
            this.labelClient.Size = new System.Drawing.Size(41, 15);
            this.labelClient.TabIndex = 1;
            this.labelClient.Text = "Client:";
            // 
            // labelProjectName
            // 
            this.labelProjectName.AutoSize = true;
            this.labelProjectName.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.labelProjectName.Location = new System.Drawing.Point(14, 8);
            this.labelProjectName.Name = "labelProjectName";
            this.labelProjectName.Size = new System.Drawing.Size(80, 15);
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
            this.descriptionButton.Location = new System.Drawing.Point(279, 11);
            this.descriptionButton.Name = "descriptionButton";
            this.descriptionButton.Size = new System.Drawing.Size(22, 22);
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
            this.projectButton.Location = new System.Drawing.Point(279, 41);
            this.projectButton.Name = "projectButton";
            this.projectButton.Size = new System.Drawing.Size(22, 22);
            this.projectButton.TabIndex = 18;
            this.projectButton.Text = "▼";
            this.projectButton.UseVisualStyleBackColor = false;
            // 
            // comboBoxProject
            // 
            this.comboBoxProject.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxProject.Font = new System.Drawing.Font("Lucida Sans Unicode", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.comboBoxProject.Location = new System.Drawing.Point(110, 40);
            this.comboBoxProject.Name = "comboBoxProject";
            this.comboBoxProject.Size = new System.Drawing.Size(192, 24);
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
            this.comboBoxDescription.Location = new System.Drawing.Point(110, 10);
            this.comboBoxDescription.Name = "comboBoxDescription";
            this.comboBoxDescription.Size = new System.Drawing.Size(192, 24);
            this.comboBoxDescription.TabIndex = 0;
            this.comboBoxDescription.KeyUp += new System.Windows.Forms.KeyEventHandler(this.comboBoxDescription_KeyUp);
            this.comboBoxDescription.Leave += new System.EventHandler(this.comboBoxDescription_Leave);
            this.comboBoxDescription.PreviewKeyDown += new System.Windows.Forms.PreviewKeyDownEventHandler(this.comboBoxDescription_PreviewKeyDown);
            // 
            // TimeEntryEditViewController
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
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
            this.Name = "TimeEntryEditViewController";
            this.Size = new System.Drawing.Size(317, 400);
            this.Load += new System.EventHandler(this.TimeEntryEditViewController_Load);
            this.statusStripLastUpdate.ResumeLayout(false);
            this.statusStripLastUpdate.PerformLayout();
            this.buttonsPanel.ResumeLayout(false);
            this.panelBottom.ResumeLayout(false);
            this.panelBillable.ResumeLayout(false);
            this.panelBillable.PerformLayout();
            this.panelDuration.ResumeLayout(false);
            this.panelDuration.PerformLayout();
            this.panelDateTag.ResumeLayout(false);
            this.panelDateTag.PerformLayout();
            this.panelStartEndTime.ResumeLayout(false);
            this.panelStartEndTime.PerformLayout();
            this.panelAddProject.ResumeLayout(false);
            this.panelAddProject.PerformLayout();
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
