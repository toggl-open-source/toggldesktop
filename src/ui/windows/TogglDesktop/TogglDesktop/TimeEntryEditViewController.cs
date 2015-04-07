using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TogglDesktop
{
    public partial class TimeEntryEditViewController : UserControl
    {
        private Toggl.TimeEntry timeEntry;
        private Boolean firstLoad = true;
        private int bottomPanelTop;
        private List<Toggl.AutocompleteItem> autoCompleteEntryList;
        private List<Toggl.AutocompleteItem> autoCompleteProjectList;
        private Boolean overTags = false;
        private List<Toggl.Model> tagsList;
        private string newestClient = "";

        public TimeEntryEditViewController()
        {
            InitializeComponent();            

            Toggl.OnTimeEntryEditor += OnTimeEntryEditor;
            Toggl.OnWorkspaceSelect += OnWorkspaceSelect;
            Toggl.OnClientSelect += OnClientSelect;
            Toggl.OnTags += OnTags;
            Toggl.OnTimeEntryAutocomplete += OnTimeEntryAutocomplete;
            Toggl.OnProjectAutocomplete += OnProjectAutocomplete;

            checkedListBoxTags.DisplayMember = "Name";
            checkedListBoxTags.ValueMember = "Name";

            comboBoxDescription.MouseWheel += new MouseEventHandler(ignoreMouseWheel);
            comboBoxProject.MouseWheel += new MouseEventHandler(ignoreMouseWheel);
            textBoxDuration.MouseWheel += new MouseEventHandler(ignoreMouseWheel);
            textBoxStartTime.MouseWheel += new MouseEventHandler(ignoreMouseWheel);
            textBoxEndTime.MouseWheel += new MouseEventHandler(ignoreMouseWheel);
            textBoxProjectName.MouseWheel += new MouseEventHandler(ignoreMouseWheel);
            dateTimePickerStartDate.MouseWheel += new MouseEventHandler(ignoreMouseWheel);
            tagTextBox.MouseWheel += new MouseEventHandler(ignoreMouseWheel);

            descriptionButton.Click += descriptionButton_Click;
            projectButton.Click += projectButton_Click;
        }

        private void TimeEntryEditViewController_Load(object sender, EventArgs e)
        {
            Dock = DockStyle.Fill;

            comboBoxDescription.autoCompleteListBox.KeyDown += autoCompleteEntryListBox_KeyDown;
            comboBoxDescription.autoCompleteListBox.Click += autoCompleteEntryListBox_Click;
            comboBoxDescription.autoCompleteListBox.Leave += autoCompleteEntryListBox_Leave;

            comboBoxProject.autoCompleteListBox.KeyDown += autoCompleteProjectListBox_KeyDown;
            comboBoxProject.autoCompleteListBox.Click += autoCompleteProjectListBox_Click;
            comboBoxProject.autoCompleteListBox.Leave += autoCompleteProjectListBox_Leave;
            updateTimeBoxes();
        }

        void autoCompleteEntryListBox_Leave(object sender, EventArgs e)
        {
            if (!comboBoxDescription.Focused && !descriptionButton.Focused)
            {
                comboBoxDescription.ResetListBox();
            }
        }

        void autoCompleteProjectListBox_Leave(object sender, EventArgs e)
        {
            if (!comboBoxProject.Focused && !projectButton.Focused)
            {
                comboBoxProject.ResetListBox();
            }
        }

        void projectButton_Click(object sender, EventArgs e)
        {
            if (!comboBoxProject.autoCompleteListBox.Visible)
            {
                comboBoxProject.openFullList(autoCompleteProjectList);
            }
            else if (comboBoxProject.fullListOpened)
            {
                comboBoxProject.ResetListBox();
            }            
        }

        void descriptionButton_Click(object sender, EventArgs e)
        {
            if (!comboBoxDescription.autoCompleteListBox.Visible)
            {
                comboBoxDescription.openFullList(autoCompleteEntryList);
            }
            else if (comboBoxDescription.fullListOpened)
            {
                comboBoxDescription.ResetListBox();
            }
        }

        private void autoCompleteEntryListBox_Click(object sender, EventArgs e)
        {
            selectEntryAutoComplete();
        }

        private void autoCompleteEntryListBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                if (autoCompleteEntryList == null)
                {
                    return;
                }

                selectEntryAutoComplete();
            }
        }

        private void autoCompleteProjectListBox_Click(object sender, EventArgs e)
        {
            selectProjectAutoComplete();
        }

        private void autoCompleteProjectListBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                if (autoCompleteProjectList == null)
                {
                    return;
                }

                selectProjectAutoComplete();
            }
        }

        private void ignoreMouseWheel(object sender, MouseEventArgs args)
        {
            ((HandledMouseEventArgs)args).Handled = true;
            if (overTags)
            {
                checkedListBoxTags.Focus();
            }
        }

        public void setupView(Form frm, string focusedFieldName)
        {
            SetFocus(focusedFieldName);
            checkFirstLoad();
        }

        public void checkFirstLoad() {
            if (firstLoad)
            {
                comboBoxDescription.SelectionLength = 0;
                comboBoxProject.SelectionLength = 0;
                if (comboBoxDescription.Text != timeEntry.Description)
                {
                    comboBoxDescription.Text = timeEntry.Description;
                }
                firstLoad = false;
            }
        }

        public void SetFocus(string focusedFieldName)
        {
            if (Toggl.Project == focusedFieldName)
            {
                comboBoxProject.Focus();
            }
            else if (Toggl.Duration == focusedFieldName)
            {
                textBoxDuration.Focus();
            }
            else if (Toggl.Description == focusedFieldName)
            {
                comboBoxDescription.Focus();
            }
        }

        public void buttonDone_Click(object sender, EventArgs e)
        {
            if (applyAddProject())
            {
                resetForms();
                Toggl.ViewTimeEntryList();
            }
        }

        void OnTimeEntryEditor(
            bool open,
            Toggl.TimeEntry te,
            string focused_field_name)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnTimeEntryEditor(open, te, focused_field_name); });
                return;
            }
            //
            // Custom Combobox Buttons
            //
            projectButton.Height = projectButton.Width = descriptionButton.Height = descriptionButton.Width = comboBoxDescription.Height;
            projectButton.Left = descriptionButton.Left = comboBoxProject.Left + comboBoxProject.Width - 1;
            descriptionButton.Top = comboBoxDescription.Top;
            projectButton.Top = comboBoxProject.Top;

            timeEntry = te;

            panelBillable.Visible = te.CanSeeBillable;

            checkBoxBillable.Tag = this;
            try
            {
                checkBoxBillable.Checked = te.Billable;
            }
            finally
            {
                checkBoxBillable.Tag = null;
            }

            if (!te.CanAddProjects)
            {
                linkAddProject.Visible = !te.CanAddProjects;
            }

            if (open)
            {
                comboBoxDescription.ResetListBox();
                comboBoxProject.ResetListBox();
                comboBoxDescription.Text = te.Description;
                comboBoxProject.Text = te.ProjectAndTaskLabel;
                textBoxDuration.Text = te.Duration;
                setTimeValue(textBoxStartTime, te.StartTimeString);
                setTimeValue(textBoxEndTime, te.EndTimeString);
                dateTimePickerStartDate.Value = Toggl.DateTimeFromUnix(te.Started);
                resetTagFilter();
            }
            else
            {
                if (!comboBoxDescription.Focused)
                {
                    comboBoxDescription.Text = te.Description;
                }
                if (!comboBoxProject.Focused)
                {
                    comboBoxProject.Text = te.ProjectAndTaskLabel;
                }
                if (!textBoxDuration.Focused)
                {
                    textBoxDuration.Text = te.Duration;
                }
                if (!textBoxStartTime.Focused)
                {
                    setTimeValue(textBoxStartTime, te.StartTimeString);
                }
                if (!textBoxEndTime.Focused)
                {
                    setTimeValue(textBoxEndTime, te.EndTimeString);
                }
                if (!dateTimePickerStartDate.Focused)
                {
                    dateTimePickerStartDate.Value = Toggl.DateTimeFromUnix(te.Started);
                }
            }

            panelStartEndTime.Visible = !timeEntry.DurOnly;
            
            if (timeEntry.DurOnly)
            {
                panelDateTag.Top = panelDuration.Top + panelDuration.Height;
            } else {
                panelDateTag.Top = panelStartEndTime.Top + panelStartEndTime.Height;
            }
            panelDateTag.Height = panelBottom.Height - panelDateTag.Top;
            
            if (te.UpdatedAt > 0)
            {
                DateTime updatedAt = Toggl.DateTimeFromUnix(te.UpdatedAt);
                toolStripStatusLabelLastUpdate.Text = "Last update: " + updatedAt.ToString();
                toolStripStatusLabelLastUpdate.Visible = true;
            }
            else
            {
                toolStripStatusLabelLastUpdate.Visible = false;
            }
            textBoxEndTime.Enabled = (te.DurationInSeconds >= 0);

            if (!checkedListBoxTags.Focused || open)
            {
                for (int i = 0; i < checkedListBoxTags.Items.Count; i++)
                {
                    checkedListBoxTags.SetItemChecked(i, false);
                }
                setCheckedTags(te);
            }

            workspaceNameLabel.Text = te.WorkspaceName;
        }

        private void setTimeValue(TextBox control, string value)
        {
            control.Tag = value;
            control.Text = value;
        }

        private void buttonDelete_Click(object sender, EventArgs e)
        {
            DialogResult dr;
            try
            {
                MainWindowController.DisableTop();
                dr = MessageBox.Show("Delete time entry?", "Please confirm",
                    MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            }
            finally
            {
                MainWindowController.EnableTop();
            }
            if (DialogResult.Yes == dr)
            {
                resetForms();
                Toggl.DeleteTimeEntry(timeEntry.GUID);
            }
        }

        void OnClientSelect(List<Toggl.Model> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnClientSelect(list); });
                return;
            }

            comboBoxClient.Items.Clear();
            foreach (Toggl.Model o in list)
            {
                comboBoxClient.Items.Add(o);
            }
            if (newestClient.Length > 0)
            {
                comboBoxClient.Text = newestClient;
                newestClient = "";
            }
        }

        void OnTags(List<Toggl.Model> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnTags(list); });
                return;
            }
            tagsList = list;
            checkedListBoxTags.Items.Clear();
            foreach (Toggl.Model o in list)
            {
                checkedListBoxTags.Items.Add(o.Name);
            }
            setCheckedTags(timeEntry);
        }

        void OnWorkspaceSelect(List<Toggl.Model> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnWorkspaceSelect(list); });
                return;
            }
            comboBoxWorkspace.Items.Clear();
            foreach (Toggl.Model o in list)
            {
                comboBoxWorkspace.Items.Add(o);
            }
        }

        void OnTimeEntryAutocomplete(List<Toggl.AutocompleteItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnTimeEntryAutocomplete(list); });
                return;
            }
            autoCompleteEntryList = list;
        }

        void OnProjectAutocomplete(List<Toggl.AutocompleteItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnProjectAutocomplete(list); });
                return;
            }
            autoCompleteProjectList = list;
        }

        private void comboBoxProject_Leave(object sender, EventArgs e)
        {
            if (comboBoxProject.Text.Length == 0)
            {
                Toggl.SetTimeEntryProject(timeEntry.GUID, 0, 0, "");
            }
            if (!projectButton.Focused)
            {
                comboBoxProject.handleLeave();
            }
        }

        private void checkBoxBillable_CheckedChanged(object sender, EventArgs e)
        {
            if (null == checkBoxBillable.Tag)
            {
                Toggl.SetTimeEntryBillable(timeEntry.GUID, checkBoxBillable.Checked);
            }
        }

        private void comboBoxDescription_Leave(object sender, EventArgs e)
        {
            if (!descriptionButton.Focused)
            {
                comboBoxDescription.handleLeave();
            }
            if (comboBoxDescription.Text == timeEntry.Description || comboBoxDescription.autoCompleteListBox.Visible)
            {
                return;
            }
            Toggl.SetTimeEntryDescription(timeEntry.GUID, comboBoxDescription.Text);
        }

        private void textBoxStartTime_Leave(object sender, EventArgs e)
        {
            if (timeEntry.Equals(null))
            {
                Console.WriteLine("Cannot apply end time change. this.TimeEntry is null");
                return;
            }

            applyTimeChange(textBoxStartTime);
        }

        private void textBoxDuration_Leave(object sender, EventArgs e)
        {
            if (timeEntry.Equals(null))
            {           
                Console.WriteLine("Cannot apply duration change. this.TimeEntry is null");
                return;
            }
            Toggl.SetTimeEntryDuration(timeEntry.GUID, textBoxDuration.Text);
        }

        private void textBoxEndTime_Leave(object sender, EventArgs e)
        {
            if (timeEntry.Equals(null))
            {
                Console.WriteLine("Cannot apply end time change. this.TimeEntry is null");
                return;
            }

            applyTimeChange(textBoxEndTime);
        }

        private void applyTimeChange(TextBox textbox)
        {
            // If textbox value is same as it was before user started
            // don't apply the change. User cannot enter seconds,
            // only hours and minutes. But we don't want to change the value
            // if user tabs over the controls.
            if (textbox.Tag != null && textbox.Tag.ToString() == textbox.Text) {
                return;
            }
            if (textbox == textBoxStartTime)
            {
                Toggl.SetTimeEntryStart(timeEntry.GUID, textbox.Text);
            }
            else if (textbox == textBoxEndTime)
            {
                Toggl.SetTimeEntryEnd(timeEntry.GUID, textbox.Text);
            }
        }

        private void dateTimePickerStartDate_Leave(object sender, EventArgs e)
        {
            if (timeEntry.Equals(null))
            {
                Console.WriteLine("Cannot apply end time change. this.TimeEntry is null");
                return;
            }
            Toggl.SetTimeEntryDate(timeEntry.GUID, dateTimePickerStartDate.Value);
        }

        private void checkedListBoxTags_Leave(object sender, EventArgs e)
        {
            saveTimeEntryTags();
        }

        private void timerRunningDuration_Tick(object sender, EventArgs e)
        {
            if (timeEntry.Equals(null) || timeEntry.DurationInSeconds >= 0)
            {
                return;
            }
            if (textBoxDuration.Focused)
            {
                return;
            }
            string s = Toggl.FormatDurationInSecondsHHMMSS(timeEntry.DurationInSeconds);
            if (s != textBoxDuration.Text)
            {
                textBoxDuration.Text = s;
            }
        }

        private void linkAddProject_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            panelAddProject.Top = comboBoxProject.Top - 4;
            projectButton.Visible = false;
            bottomPanelTop = panelBottom.Top;
            
            textBoxProjectName.Text = "";
            comboBoxClient.Text = "";
            comboBoxWorkspace.Text = "";
            linkAddProject.Visible = false;
            int boxHeight = panelDuration.Height*3;
            if (comboBoxWorkspace.Items.Count > 1)
            {
                comboBoxWorkspace.SelectedIndex = 0;
                labelWorkspace.Visible = true;
                comboBoxWorkspace.Visible = true;
                boxHeight = panelDuration.Height * 4;
            }
            boxHeight += 10;
            panelBottom.Top = panelAddProject.Top + boxHeight;
            panelBottom.Height = buttonsPanel.Top - panelBottom.Top;
            panelAddProject.Height = boxHeight;

            labelProject.Visible = true;
            comboBoxProject.Visible = true;

            panelAddProject.Visible = true;
            recalculateTabIndexes(true);
            textBoxProjectName.Focus();
        }

        private void recalculateTabIndexes(Boolean openAddProject)
        {
            panelAddProject.TabStop = openAddProject;
            textBoxProjectName.TabStop = openAddProject;
            checkBoxPublic.TabStop = openAddProject;
            comboBoxWorkspace.TabStop = openAddProject;
            comboBoxClient.TabStop = openAddProject;
            comboBoxProject.TabStop = !openAddProject;

            int addition = openAddProject ? 3 : -3;
            panelBottom.TabIndex += addition;
            textBoxDuration.TabIndex += addition;
            panelStartEndTime.TabIndex += addition;
            textBoxStartTime.TabIndex += addition;
            textBoxEndTime.TabIndex += addition;
            panelDateTag.TabIndex += addition;
            dateTimePickerStartDate.TabIndex += addition;
            checkedListBoxTags.TabIndex += addition;
        }

        public void resetForms()
        {
            if (panelAddProject.Visible)
            {
                panelAddProject.Visible = false;
                panelBottom.Height = buttonsPanel.Top - bottomPanelTop;
                panelBottom.Top = bottomPanelTop;
                projectButton.Visible = true;
                labelWorkspace.Visible = false;
                comboBoxWorkspace.Visible = false;
                linkAddProject.Visible = true;
                labelProject.Visible = true;
                comboBoxProject.Visible = true;
                checkBoxPublic.Checked = false;
                comboBoxWorkspace.SelectedIndex = -1;
                comboBoxClient.SelectedIndex = -1;
                recalculateTabIndexes(false);
                if (addClientLinkLabel.Text == "cancel") {
                    addClientLinkLabel_Click(null, null);
                }
            }
            comboBoxProject.ResetListBox();
            comboBoxDescription.ResetListBox();
        }

        private Boolean applyAddProject()
        {
            if (!panelAddProject.Visible)
            {
                return true;
            }

            if (textBoxProjectName.Text.Length == 0)
            {
                return true;
            }

            bool is_public = checkBoxPublic.Checked;
            ulong workspaceID = timeEntry.WID;
            if (comboBoxWorkspace.Items.Count == 1)
            {
                workspaceID = ((Toggl.Model)comboBoxWorkspace.Items[0]).ID;
            }
            if (comboBoxWorkspace.Items.Count > 1)
            {
                workspaceID = selectedItemID(comboBoxWorkspace);
            }
            if (workspaceID == 0)
            {
                comboBoxWorkspace.Focus();
                return false;
            }
            ulong clientID = selectedItemID(comboBoxClient);
            bool isBillable = timeEntry.Billable;
            bool projectAdded = Toggl.AddProject(
                timeEntry.GUID,
                workspaceID,
                clientID,
                textBoxProjectName.Text,
                !is_public);
            if (projectAdded && isBillable)
            {
                Toggl.SetTimeEntryBillable(timeEntry.GUID, isBillable);
            }
            return projectAdded;
        }

        private ulong selectedItemID(ComboBox combobox)
        {
            for (int i = 0; i < combobox.Items.Count; i++)
            {
                Toggl.Model item = (Toggl.Model)combobox.Items[i];
                if (item.Name == combobox.Text)
                {
                    return item.ID;
                }
            }
            return 0;
        }

        private void comboBoxDescription_KeyUp(object sender, KeyEventArgs e)
        {
            if (comboBoxDescription.Text.Length == 0)
            {
                comboBoxDescription.ResetListBox();
            }
            comboBoxDescription.UpdateListBox(autoCompleteEntryList, e);
        }

        private void comboBoxDescription_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            comboBoxDescription.parseKeyDown(e, autoCompleteEntryList);
            if (e.KeyCode == Keys.Enter)
            {
                selectEntryAutoComplete();
            }
        }

        private void comboBoxProject_KeyUp(object sender, KeyEventArgs e)
        {
            if (comboBoxProject.Text.Length == 0)
            {
                comboBoxProject.ResetListBox();
            }
            comboBoxProject.UpdateListBox(autoCompleteProjectList, e);
        }

        private void comboBoxProject_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            comboBoxProject.parseKeyDown(e, autoCompleteProjectList);
            if (e.KeyCode == Keys.Enter)
            {
                selectProjectAutoComplete();
            }
        }

        private void selectEntryAutoComplete()
        {
            comboBoxDescription.ResetListBox();

            object selected = comboBoxDescription.autoCompleteListBox.SelectedItem;
            if (null == selected) {
                return;
            }

            Toggl.AutocompleteItem item = (Toggl.AutocompleteItem)selected;
            comboBoxDescription.Text = item.Description;

            if (item.ProjectID != 0)
            {
                foreach (object obj in comboBoxProject.autoCompleteListBox.Items)
                {
                    Toggl.AutocompleteItem projectItem = (Toggl.AutocompleteItem)obj;
                    if (item.ProjectID == projectItem.ProjectID)
                    {
                        comboBoxProject.Text = projectItem.Text;
                    }
                }
            }

            comboBoxDescription.ResetListBox();

            Toggl.SetTimeEntryProject(timeEntry.GUID, item.TaskID, item.ProjectID, "");            
        }

        private void selectProjectAutoComplete()
        {
            object selected = comboBoxProject.autoCompleteListBox.SelectedItem;
            if (null == selected)
            {
                return;
            }

            Toggl.AutocompleteItem item = (Toggl.AutocompleteItem)selected;
            if (item.ProjectID == 0)
            {
                return;
            }
            comboBoxProject.ResetListBox();
            comboBoxProject.Text = item.Text;

            comboBoxProject.ResetListBox();

            Toggl.SetTimeEntryProject(
                timeEntry.GUID,
                item.TaskID,
                item.ProjectID,
                null);           
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            if (keyData == Keys.Enter)
            {
                if (!comboBoxDescription.autoCompleteListBox.Visible && !comboBoxProject.autoCompleteListBox.Visible)
                {
                    SendKeys.Send("{TAB}+{TAB}");
                    return true;
                }
            }
            return base.ProcessCmdKey(ref msg, keyData);
        }

        internal bool dropDownsClosed()
        {
            if (comboBoxDescription.autoCompleteListBox.Visible)
            {
                comboBoxDescription.ResetListBox();
                return false;
            }

            if (comboBoxProject.autoCompleteListBox.Visible)
            {
                comboBoxProject.ResetListBox();
                return false;
            }
            return true;
        }

        private void checkedListBoxTags_MouseLeave(object sender, EventArgs e)
        {
            overTags = false;
        }

        private void checkedListBoxTags_MouseEnter(object sender, EventArgs e)
        {
            if (noFieldFocused(this))
            {
                checkedListBoxTags.Focus();
            }
            else
            {
                overTags = true;
            }
        }

        private bool noFieldFocused(Control parent)
        {
            foreach (Control c in parent.Controls){
                if (c.Focused)
                {
                    return false;
                }
                if (c.Controls.Count > 0)
                {
                    if (!noFieldFocused(c))
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        private void TimeEntryEditViewController_SizeChanged(object sender, EventArgs e)
        {
            updateTimeBoxes();
        }

        private void updateTimeBoxes()
        {
            int width = (dateTimePickerStartDate.Width / 2) - 10;
            if (width > textBoxStartTime.MaximumSize.Width)
            {
                return;
            }
            textBoxStartTime.Width = textBoxEndTime.Width = width;
            labelDash.Left = textBoxStartTime.Left + textBoxStartTime.Width + 5;
            textBoxEndTime.Left = labelDash.Left + 15;
        }

        private void tagTextBox_KeyUp(object sender, KeyEventArgs e)
        {
            checkedListBoxTags.Items.Clear();
            foreach (Toggl.Model item in tagsList)
            {
                if (tagTextBox.Text.Length == 0 || item.Name.IndexOf(tagTextBox.Text, StringComparison.OrdinalIgnoreCase) >= 0)
                {
                    checkedListBoxTags.Items.Add(item.Name);
                }
            }

            setCheckedTags(timeEntry);
        }

        private void setCheckedTags(Toggl.TimeEntry te)
        {
            if (te.Tags != null)
            {
                int count = 0;
                string[] tags = te.Tags.Split(Toggl.TagSeparator.ToCharArray());

                // Tick selected Tags
                for (int i = 0; i < tags.Length; i++)
                {
                    int index = checkedListBoxTags.Items.IndexOf(tags[i]);
                    if (index != -1)
                    {
                        checkedListBoxTags.Items.RemoveAt(index);
                        checkedListBoxTags.Items.Insert(count, tags[i]);
                        checkedListBoxTags.SetItemChecked(count, true);
                        count++;
                    }
                }
            }
        }

        private void resetTagFilter()
        {
            tagTextBox.Text = "";
            tagTextBox_KeyUp(null, null);
        }

        private void addTagButton_Click(object sender, EventArgs e)
        {
            String word = tagTextBox.Text;
            if (word.Length == 0)
            {
                return;
            }
            resetTagFilter();
            int index = checkedListBoxTags.Items.IndexOf(word);
            if (index != -1)
            {
                checkedListBoxTags.SetItemChecked(index, true);
            }
            else
            {
                checkedListBoxTags.Items.Insert(0,word);
                checkedListBoxTags.SetItemChecked(0, true);
                saveTimeEntryTags();
            }
        }

        private void saveTimeEntryTags()
        {
            List<String> tags = new List<String>();
            foreach (object item in checkedListBoxTags.CheckedItems)
            {
                tags.Add(item.ToString());
            }      
            Toggl.SetTimeEntryTags(timeEntry.GUID,
                String.Join(Toggl.TagSeparator, tags));
        }

        private void tagTextBox_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                addTagButton_Click(null, null);
            }
        }

        private void addClientLinkLabel_Click(object sender, EventArgs e)
        {
            bool showCombobox = (addClientLinkLabel.Text == "cancel");
            comboBoxClient.Visible = showCombobox;
            addClientTextBox.Visible = !showCombobox;
            addClientButton.Visible = !showCombobox;
            if (showCombobox)
            {
                comboBoxClient.Focus();
                addClientLinkLabel.Text = "Add new client";
                addClientTextBox.Text = "";
            }
            else
            {
                addClientLinkLabel.Text = "cancel";
                addClientTextBox.Focus();
            }            
        }

        private void addClientButton_Click(object sender, EventArgs e)
        {
            if (addClientTextBox.Text.Length == 0)
            {
                addClientLinkLabel_Click(null, null);
                return;
            }

            ulong workspaceID = timeEntry.WID;
            if (comboBoxWorkspace.Items.Count == 1)
            {
                workspaceID = ((Toggl.Model)comboBoxWorkspace.Items[0]).ID;
            }
            if (comboBoxWorkspace.Items.Count > 1)
            {
                workspaceID = selectedItemID(comboBoxWorkspace);
            }
            if (workspaceID == 0)
            {
                comboBoxWorkspace.Focus();
                return;
            }

            // Client with the same name already in list
            if (comboBoxClient.FindStringExact(addClientTextBox.Text) != -1)
            {
                comboBoxClient.Text = addClientTextBox.Text;
                addClientLinkLabel_Click(null, null);
                return;
            }

            bool clientCreated = Toggl.AddClient(workspaceID, addClientTextBox.Text);
            if (clientCreated)
            {
                comboBoxClient.Text = addClientTextBox.Text;
                newestClient = addClientTextBox.Text;
            }
            addClientLinkLabel_Click(null, null);
        }

        private void TimeEntryEditViewController_Paint(object sender, PaintEventArgs e)
        {
            comboBoxWorkspace.Select(0, 0);
        }
    }
}
