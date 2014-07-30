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
        private UInt64 TEUpdatedAt;
        private Toggl.TimeEntry timeEntry;
        private List<Toggl.AutocompleteItem> timeEntryAutocompleteUpdate = null;
        private List<Toggl.AutocompleteItem> projectAutocompleteUpdate = null;
        private Boolean firstLoad = true;

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
            comboBoxClient.MouseWheel += new MouseEventHandler(ignoreMouseWheel);
            comboBoxWorkspace.MouseWheel += new MouseEventHandler(ignoreMouseWheel);
        }

        private void ignoreMouseWheel(object sender, MouseEventArgs args)
        {
            ((HandledMouseEventArgs)args).Handled = true;
        }

        private void TimeEntryEditViewController_Load(object sender, EventArgs e)
        {
            Dock = DockStyle.Fill;
        }

        public void setupView(Form frm, string focusedFieldName)
        {
            SetAcceptButton(frm);
            SetFocus(focusedFieldName);
            checkFirstLoad();
        }

        public void SetAcceptButton(Form frm)
        {
            frm.AcceptButton = buttonDone;
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

        private void buttonDone_Click(object sender, EventArgs e)
        {
            if (applyAddProject())
            {
                Toggl.ViewTimeEntryList();
                resetForms();
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

            timeEntry = te;

            checkBoxBillable.Visible = te.CanSeeBillable;

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
            Console.WriteLine("Te.duration = {0}", te.Duration);
            if (!textBoxStartTime.Focused)
            {
                textBoxStartTime.Text = te.StartTimeString;
            }
            if (!textBoxEndTime.Focused)
            {
                textBoxEndTime.Text = te.EndTimeString;
            }
            if (!dateTimePickerStartDate.Focused)
            {
                dateTimePickerStartDate.Value = Toggl.DateTimeFromUnix(te.Started);
            }

            panelStartEndTime.Visible = !timeEntry.DurOnly;
            
            if (timeEntry.DurOnly)
            {
                panelDateTag.Top = 30;
                panelDateTag.Height = panelBottom.Height - 30;
            } else {
                panelDateTag.Top = 60;
                panelDateTag.Height = panelBottom.Height - 60;
            }
            
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

            for (int i = 0; i < checkedListBoxTags.Items.Count; i++)
            {
                checkedListBoxTags.SetItemChecked(i, false);
            }

            if ( te.Tags != null) {
                string[] tags = te.Tags.Split('|');

                // Tick selected Tags
                for (int i = 0; i < tags.Length; i++)
                {
                    int index = checkedListBoxTags.Items.IndexOf(tags[i]);
                    if (index != -1)
                    {
                        checkedListBoxTags.SetItemChecked(index, true);
                    }
                }
            }

            buttonContinue.Visible = te.DurationInSeconds >= 0;
        }

        private void buttonDelete_Click(object sender, EventArgs e)
        {
            DialogResult dr = MessageBox.Show("Delete time entry?", "Please confirm",
                MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            if (DialogResult.Yes == dr)
            {
                resetForms();
                Toggl.DeleteTimeEntry(timeEntry.GUID);
            }
        }

        private void buttonContinue_Click(object sender, EventArgs e)
        {
            resetForms();
            Toggl.Continue(timeEntry.GUID);
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
        }

        void OnTags(List<Toggl.Model> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnTags(list); });
                return;
            }
            checkedListBoxTags.Items.Clear();
            foreach (Toggl.Model o in list)
            {
                checkedListBoxTags.Items.Add(o.Name);
            }
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
            timeEntryAutocompleteUpdate = list;
            if (comboBoxDescription.DroppedDown || comboBoxDescription.Focused)
            {
                return;
            }
            comboBoxDescription.Items.Clear();
            foreach (Toggl.AutocompleteItem o in timeEntryAutocompleteUpdate)
            {
                comboBoxDescription.Items.Add(o);
            }
            timeEntryAutocompleteUpdate = null;
        }

        private void comboBoxDescription_SelectedIndexChanged(object sender, EventArgs e)
        {
            object o = comboBoxDescription.SelectedItem;
            if (o == null)
            {
                return;
            }

            Toggl.AutocompleteItem item = (Toggl.AutocompleteItem)o;
            comboBoxDescription.Text = item.Description;
            if (item.ProjectID != 0) {
                foreach (object obj in comboBoxProject.Items)
                {
                    Toggl.AutocompleteItem projectItem = (Toggl.AutocompleteItem)obj;
                    if (item.ProjectID == projectItem.ProjectID)
                    {
                        comboBoxProject.Text = projectItem.Text;
                    }
                }
            }

            Toggl.SetTimeEntryProject(
                timeEntry.GUID,
                item.TaskID,
                item.ProjectID,
                null);
        }

        void OnProjectAutocomplete(List<Toggl.AutocompleteItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnProjectAutocomplete(list); });
                return;
            }
            projectAutocompleteUpdate = list;
            if (comboBoxProject.DroppedDown || comboBoxProject.Focused)
            {
                return;
            }
            comboBoxProject.Items.Clear();
            foreach (Toggl.AutocompleteItem o in projectAutocompleteUpdate)
            {
                comboBoxProject.Items.Add(o);
            }
            projectAutocompleteUpdate = null;
        }

        private void comboBoxProject_Leave(object sender, EventArgs e)
        {
            if (comboBoxProject.Text.Length == 0)
            {
                Toggl.SetTimeEntryProject(timeEntry.GUID, 0, 0, "");
            }
        }

        private void comboBoxProject_SelectedIndexChanged(object sender, EventArgs e)
        {
            object o = comboBoxProject.SelectedItem;
            if (null == o)
            {
                return;
            }
            Toggl.AutocompleteItem item = (Toggl.AutocompleteItem)o;
            Toggl.SetTimeEntryProject(timeEntry.GUID, item.TaskID, item.ProjectID, "");
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
            if (comboBoxDescription.Text == timeEntry.Description)
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
            DateTime date = parseTime(textbox);
            String utf8String = date.ToString("yyyy-MM-ddTHH:mm:sszzz");
            if (textbox == textBoxStartTime)
            {
                Toggl.SetTimeEntryStart(timeEntry.GUID, utf8String);
            }
            else if (textbox == textBoxEndTime)
            {
                Toggl.SetTimeEntryEnd(timeEntry.GUID, utf8String);
            }
        }

        private DateTime parseTime(TextBox field) 
        {
            DateTime date = dateTimePickerStartDate.Value;
            int hours = 0;
            int minutes = 0;
            if (!Toggl.ParseTime(field.Text, ref hours, ref minutes))
            {
                return date;
            }

            return date.Date + new TimeSpan(hours, minutes, 0);
        }

        private void dateTimePickerStartDate_Leave(object sender, EventArgs e)
        {
            if (timeEntry.Equals(null))
            {
                Console.WriteLine("Cannot apply end time change. this.TimeEntry is null");
                return;
            }
            applyTimeChange(textBoxStartTime);
            applyTimeChange(textBoxEndTime);
        }

        private void checkedListBoxTags_Leave(object sender, EventArgs e)
        {
            List<String> tags = new List<String>();
            foreach (object item in checkedListBoxTags.CheckedItems)
            {
                tags.Add(item.ToString());
            }
            Toggl.SetTimeEntryTags(timeEntry.GUID, String.Join("|", tags));
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
            panelAddProject.Top = 37;
            
            textBoxProjectName.Text = "";
            comboBoxClient.Text = "";
            comboBoxWorkspace.Text = "";
            linkAddProject.Visible = false;
            int boxHeight = 89;
            if (comboBoxWorkspace.Items.Count > 1)
            {
                labelWorkspace.Visible = true;
                comboBoxWorkspace.Visible = true;
                boxHeight = 122;
            }
            panelBottom.Height -= (boxHeight-40);
            panelBottom.Top = boxHeight+37;            
            panelAddProject.Height = boxHeight;

            labelProject.Visible = true;
            comboBoxProject.Visible = true;

            panelAddProject.Visible = true;
            recalculateTabIndexes(true);
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

        private void resetForms()
        {
            if (panelAddProject.Visible)
            {
                panelAddProject.Visible = false;
                panelBottom.Height += panelBottom.Top - 77;
                panelBottom.Top = 77;
                labelWorkspace.Visible = false;
                comboBoxWorkspace.Visible = false;
                linkAddProject.Visible = true;
                labelProject.Visible = true;
                comboBoxProject.Visible = true;
                checkBoxPublic.Checked = false;
                comboBoxWorkspace.SelectedIndex = -1;
                comboBoxClient.SelectedIndex = -1;
                recalculateTabIndexes(false);
            }           
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
            ulong workspaceID = ((Toggl.Model)comboBoxWorkspace.Items[0]).ID;
            if (comboBoxWorkspace.Items.Count > 1)
            {
                workspaceID = selectedItemID(comboBoxWorkspace);
            }
            if (workspaceID == 0){
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
    }
}
