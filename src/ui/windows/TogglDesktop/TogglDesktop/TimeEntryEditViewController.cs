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
        private List<KopsikApi.KopsikAutocompleteItem> autocompleteUpdate = null;

        public TimeEntryEditViewController()
        {
            InitializeComponent();

            KopsikApi.OnTimeEntryEditor += OnTimeEntryEditor;
            KopsikApi.OnWorkspaceSelect += OnWorkspaceSelect;
            KopsikApi.OnClientSelect += OnClientSelect;
            KopsikApi.OnTags += OnTags;
            KopsikApi.OnAutocomplete += OnAutocomplete;
        }

        private void TimeEntryEditViewController_Load(object sender, EventArgs e)
        {

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
            KopsikApi.kopsik_view_time_entry_list(KopsikApi.ctx);
        }

        void OnTimeEntryEditor(
            bool open,
            ref KopsikApi.KopsikTimeEntryViewItem te,
            string focused_field_name)
        {
            KopsikApi.KopsikTimeEntryViewItem n = te;
            DisplayTimeEntryEditor(open, n, focused_field_name);
        }

        void DisplayTimeEntryEditor(
            bool open,
            KopsikApi.KopsikTimeEntryViewItem te,
            string focused_field_name)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayTimeEntryEditor(open, te, focused_field_name); });
                return;
            }
            GUID = te.GUID;
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
            checkBoxBillable.Checked = te.Billable;
            if (te.UpdatedAt >= 0)
            {
                DateTime updatedAt = KopsikApi.DateTimeFromUnix(te.UpdatedAt);
                toolStripStatusLabelLastUpdate.Text = updatedAt.ToString();
                toolStripStatusLabelLastUpdate.Visible = true;
            }
            else
            {
                toolStripStatusLabelLastUpdate.Visible = false;
            }
            textBoxEndTime.Enabled = (te.DurationInSeconds >= 0);
        }

        private void buttonDelete_Click(object sender, EventArgs e)
        {
            DialogResult dr = MessageBox.Show("Delete time entry?", "Please confirm",
                MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            if (DialogResult.Yes == dr)
            {
                KopsikApi.kopsik_delete_time_entry(KopsikApi.ctx, GUID);
            }
        }

        private void buttonContinue_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_continue(KopsikApi.ctx, GUID);
        }

        void OnClientSelect(ref KopsikApi.KopsikViewItem first)
        {
            List<KopsikApi.KopsikViewItem> list = KopsikApi.ConvertToViewItemList(ref first);
            DisplayClientSelect(list);
        }

        void DisplayClientSelect(List<KopsikApi.KopsikViewItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayClientSelect(list); });
                return;
            }
            // FIXME: 
        }

        void OnTags(ref KopsikApi.KopsikViewItem first)
        {
            List<KopsikApi.KopsikViewItem> list = KopsikApi.ConvertToViewItemList(ref first);
            DisplayTags(list);
        }

        void DisplayTags(List<KopsikApi.KopsikViewItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayTags(list); });
                return;
            }
            checkedListBoxTags.Items.Clear();
            foreach (object o in list) {
                checkedListBoxTags.Items.Add(o);
            }
        }

        void OnWorkspaceSelect(ref KopsikApi.KopsikViewItem first)
        {
            List<KopsikApi.KopsikViewItem> list = KopsikApi.ConvertToViewItemList(ref first);
            DisplayWorkspaceSelect(list);
        }

        void DisplayWorkspaceSelect(List<KopsikApi.KopsikViewItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayWorkspaceSelect(list); });
                return;
            }
            // FIXME:
        }

        void OnAutocomplete(ref KopsikApi.KopsikAutocompleteItem first)
        {
            List<KopsikApi.KopsikAutocompleteItem> list =
                KopsikApi.ConvertToAutocompleteList(ref first);
            DisplayAutocomplete(list);
        }

        void DisplayAutocomplete(List<KopsikApi.KopsikAutocompleteItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayAutocomplete(list); });
                return;
            }
            autocompleteUpdate = list;
            if (comboBoxDescription.DroppedDown || comboBoxDescription.Focused)
            {
                return;
            }
            comboBoxDescription.Items.Clear();
            foreach (object o in autocompleteUpdate)
            {
                comboBoxDescription.Items.Add(o);
            }
            autocompleteUpdate = null;
        }

        private void comboBoxDescription_TextChanged(object sender, EventArgs e)
        {
            KopsikApi.kopsik_set_time_entry_description(KopsikApi.ctx,
                GUID, comboBoxDescription.Text);
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
            KopsikApi.kopsik_set_time_entry_project(KopsikApi.ctx,
                GUID,
                item.TaskID,
                item.ProjectID,
                null);
        }

        private void comboBoxProject_SelectedIndexChanged(object sender, EventArgs e)
        {
            object o = comboBoxProject.SelectedItem;
            if (null == o)
            {
                return;
            }
            KopsikApi.KopsikViewItem item = (KopsikApi.KopsikViewItem)o;
            KopsikApi.kopsik_set_time_entry_project(KopsikApi.ctx,
                GUID, 0, item.ID, "");
        }

        private void checkBoxBillable_CheckedChanged(object sender, EventArgs e)
        {
            KopsikApi.kopsik_set_time_entry_billable(KopsikApi.ctx,
                GUID, checkBoxBillable.Checked);
        }

    }
}
