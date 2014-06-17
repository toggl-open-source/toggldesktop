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
        private string GUID = "";
        private KopsikApi.KopsikTimeEntryViewItem timeEntry;
        private List<KopsikApi.KopsikAutocompleteItem> timeEntryAutocompleteUpdate = null;
        private List<KopsikApi.KopsikAutocompleteItem> projectAutocompleteUpdate = null;

        public TimeEntryEditViewController()
        {
            InitializeComponent();

            KopsikApi.OnTimeEntryEditor += OnTimeEntryEditor;
            KopsikApi.OnWorkspaceSelect += OnWorkspaceSelect;
            KopsikApi.OnClientSelect += OnClientSelect;
            KopsikApi.OnTags += OnTags;
            KopsikApi.OnTimeEntryAutocomplete += OnTimeEntryAutocomplete;
            KopsikApi.OnProjectAutocomplete += OnProjectAutocomplete;
            this.checkedListBoxTags.DisplayMember = "Name";
            this.checkedListBoxTags.ValueMember = "Name";
        }

        private void TimeEntryEditViewController_Load(object sender, EventArgs e)
        {
            this.Dock = DockStyle.Fill;
        }

        public void SetAcceptButton(Form frm)
        {
            frm.AcceptButton = buttonDone;
        }

        public void SetFocus(string focusedFieldName)
        {
            if (KopsikApi.Project == focusedFieldName)
            {
                comboBoxProject.Focus();
            }
            else if (KopsikApi.Duration == focusedFieldName)
            {
                textBoxDuration.Focus();
            }
            else if (KopsikApi.Description == focusedFieldName)
            {
                comboBoxDescription.Focus();
            }
        }

        private void buttonDone_Click(object sender, EventArgs e)
        {
            if (applyAddProject())
            {
                KopsikApi.ViewTimeEntryList();
            }
        }

        void OnTimeEntryEditor(
            bool open,
            KopsikApi.KopsikTimeEntryViewItem te,
            string focused_field_name)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnTimeEntryEditor(open, te, focused_field_name); });
                return;
            }

            timeEntry = te;
            if (!open || GUID == te.GUID)
            {
                return;
            }
            GUID = te.GUID;

            resetForms();
            Boolean can_see_billable = false;
            if (!KopsikApi.CanUserSeeBillableFlag(GUID, ref can_see_billable))
            {
                return;
            }
            checkBoxBillable.Visible = can_see_billable;

            checkBoxBillable.Tag = this;
            try
            {
                checkBoxBillable.Checked = te.Billable;
            }
            finally
            {
                checkBoxBillable.Tag = null;
            }

            Boolean can_add_projects = false;
            if (!KopsikApi.CanUserAddProjects(timeEntry.WID, ref can_add_projects))
            {
                return;
            }
            if (!can_add_projects)
            {
                this.linkAddProject.Visible = !can_add_projects;
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
                dateTimePickerStartDate.Value = KopsikApi.DateTimeFromUnix(te.Started);
            }

            this.panelStartEndTime.Visible = !timeEntry.DurOnly;
            if (timeEntry.DurOnly)
            {
                panelBottom.Height = 150;
            } else {
                panelBottom.Height = 175;
            }

            if (te.UpdatedAt >= 0)
            {
                DateTime updatedAt = KopsikApi.DateTimeFromUnix(te.UpdatedAt);
                toolStripStatusLabelLastUpdate.Text = "Last update: " + updatedAt.ToString();
                toolStripStatusLabelLastUpdate.Visible = true;
            }
            else
            {
                toolStripStatusLabelLastUpdate.Visible = false;
            }
            textBoxEndTime.Enabled = (te.DurationInSeconds >= 0);

            for (int i = 0; i < this.checkedListBoxTags.Items.Count; i++)
            {
                this.checkedListBoxTags.SetItemChecked(i, false);
            }

            if ( te.Tags != null) {
                string[] tags = te.Tags.Split('|');

                // Tick selected Tags
                for (int i = 0; i < tags.Length; i++)
                {
                    int index = this.checkedListBoxTags.Items.IndexOf(tags[i]);
                    if (index != -1)
                    {
                        this.checkedListBoxTags.SetItemChecked(index, true);
                    }
                }

            }
        }

        private void buttonDelete_Click(object sender, EventArgs e)
        {
            DialogResult dr = MessageBox.Show("Delete time entry?", "Please confirm",
                MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            if (DialogResult.Yes == dr)
            {
                KopsikApi.DeleteTimeEntry(GUID);
            }
        }

        private void buttonContinue_Click(object sender, EventArgs e)
        {
            KopsikApi.Continue(GUID);
        }

        void OnClientSelect(List<KopsikApi.KopsikViewItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnClientSelect(list); });
                return;
            }

            comboBoxClient.Items.Clear();
            foreach (KopsikApi.KopsikViewItem o in list)
            {
                comboBoxClient.Items.Add(o);
            }
        }

        void OnTags(List<KopsikApi.KopsikViewItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnTags(list); });
                return;
            }
            checkedListBoxTags.Items.Clear();
            foreach (KopsikApi.KopsikViewItem o in list)
            {
                checkedListBoxTags.Items.Add(o.Name);
            }
        }

        void OnWorkspaceSelect(List<KopsikApi.KopsikViewItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnWorkspaceSelect(list); });
                return;
            }
            comboBoxWorkspace.Items.Clear();
            foreach (KopsikApi.KopsikViewItem o in list)
            {
                comboBoxWorkspace.Items.Add(o);
            }
        }

        void OnTimeEntryAutocomplete(List<KopsikApi.KopsikAutocompleteItem> list)
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
            foreach (KopsikApi.KopsikAutocompleteItem o in timeEntryAutocompleteUpdate)
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
            KopsikApi.KopsikAutocompleteItem item = (KopsikApi.KopsikAutocompleteItem)o;
            comboBoxDescription.Text = item.Description;
            KopsikApi.SetTimeEntryProject(
                GUID,
                item.TaskID,
                item.ProjectID,
                null);
        }

        void OnProjectAutocomplete(List<KopsikApi.KopsikAutocompleteItem> list)
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
            foreach (KopsikApi.KopsikAutocompleteItem o in projectAutocompleteUpdate)
            {
                comboBoxProject.Items.Add(o);
            }
            projectAutocompleteUpdate = null;
        }

        private void comboBoxProject_Leave(object sender, EventArgs e)
        {
            if (comboBoxProject.Text.Length == 0)
            {
                KopsikApi.SetTimeEntryProject(GUID, 0, 0, "");
            }
        }

        private void comboBoxProject_SelectedIndexChanged(object sender, EventArgs e)
        {
            object o = comboBoxProject.SelectedItem;
            if (null == o)
            {
                return;
            }
            KopsikApi.KopsikAutocompleteItem item = (KopsikApi.KopsikAutocompleteItem)o;
            KopsikApi.SetTimeEntryProject(GUID, 0, item.ProjectID, "");
        }

        private void checkBoxBillable_CheckedChanged(object sender, EventArgs e)
        {
            if (null == checkBoxBillable.Tag)
            {
                KopsikApi.SetTimeEntryBillable(GUID, checkBoxBillable.Checked);
            }
        }

        private void comboBoxDescription_Leave(object sender, EventArgs e)
        {
            if (comboBoxDescription.Text == timeEntry.Description)
            {
                return;
            }
            KopsikApi.SetTimeEntryDescription(GUID, comboBoxDescription.Text);
        }

        private void textBoxStartTime_Leave(object sender, EventArgs e)
        {
            if (timeEntry.Equals(null))
            {
                Console.WriteLine("Cannot apply end time change. this.TimeEntry is null");
                return;
            }

            this.applyTimeChange(this.textBoxStartTime);
        }

        private void textBoxDuration_Leave(object sender, EventArgs e)
        {
            if (timeEntry.Equals(null))
            {
                Console.WriteLine("Cannot apply duration change. this.TimeEntry is null");
                return;
            }
            KopsikApi.SetTimeEntryDuration(GUID, this.textBoxDuration.Text);
        }

        private void textBoxEndTime_Leave(object sender, EventArgs e)
        {
            if (timeEntry.Equals(null))
            {
                Console.WriteLine("Cannot apply end time change. this.TimeEntry is null");
                return;
            }

            this.applyTimeChange(this.textBoxEndTime);
        }

        private void applyTimeChange(TextBox textbox)
        {
            DateTime date = this.parseTime(textbox);
            String iso8601String = date.ToString("yyyy-MM-ddTHH:mm:sszzz");
            String utf8String = iso8601String;
            if (textbox == this.textBoxStartTime)
            {
                KopsikApi.SetTimeEntryStart(timeEntry.GUID, utf8String);
            }
            else if (textbox == this.textBoxEndTime)
            {
                KopsikApi.SetTimeEntryEnd(timeEntry.GUID, utf8String);
            }            
        }

        private DateTime parseTime(TextBox field) 
        {
            DateTime date = this.dateTimePickerStartDate.Value;
            int hours = 0;
            int minutes = 0;
            if (!KopsikApi.ParseTime(field.Text, ref hours, ref minutes))
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
            this.applyTimeChange(this.textBoxStartTime);
            this.applyTimeChange(this.textBoxEndTime);
        }

        private void checkedListBoxTags_Leave(object sender, EventArgs e)
        {
            String tags = "";
            foreach (object item in this.checkedListBoxTags.CheckedItems)
            {
                if (tags.Length > 0)
                {
                    tags += "|";
                }
                tags += item.ToString();
            }

            KopsikApi.SetTimeEntryTags(timeEntry.GUID, tags);
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
            string s = KopsikApi.FormatDurationInSecondsHHMMSS(timeEntry.DurationInSeconds);
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
            panelBottom.Top = boxHeight+37;
            panelAddProject.Height = boxHeight;

            labelProject.Visible = true;
            comboBoxProject.Visible = true;

            panelAddProject.Visible = true;
        }

        private void resetForms()
        {
            if (panelAddProject.Visible)
            {
                panelAddProject.Visible = false;
                panelBottom.Top = 77;
                labelWorkspace.Visible = false;
                comboBoxWorkspace.Visible = false;
                linkAddProject.Visible = true;
                labelProject.Visible = true;
                comboBoxProject.Visible = true;
                checkBoxPublic.Checked = false;
                comboBoxWorkspace.SelectedIndex = -1;
                comboBoxClient.SelectedIndex = -1;
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
            ulong workspaceID = ((KopsikApi.KopsikViewItem)comboBoxWorkspace.Items[0]).ID;
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
            bool projectAdded = KopsikApi.AddProject(
                GUID, workspaceID, clientID, textBoxProjectName.Text, !is_public);
            if (projectAdded && isBillable)
            {
                KopsikApi.SetTimeEntryBillable(GUID, isBillable);
            }
            return projectAdded;
        }

        private ulong selectedItemID(ComboBox combobox)
        {
            for (int i = 0; i < combobox.Items.Count; i++)
            {
                KopsikApi.KopsikViewItem item = (KopsikApi.KopsikViewItem)combobox.Items[i];
                if (item.Name == combobox.Text)
                {
                    return item.ID;
                }
            }
            return 0;
        }
    }
}
