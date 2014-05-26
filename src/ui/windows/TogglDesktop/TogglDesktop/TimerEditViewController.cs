using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace TogglDesktop
{
    public partial class TimerEditViewController : UserControl
    {
        private Int64 duration_in_seconds = 0;
        private UInt64 task_id = 0;
        private UInt64 project_id = 0;
        private List<KopsikApi.KopsikAutocompleteItem> timeEntryAutocompleteUpdate;

        public TimerEditViewController()
        {
            InitializeComponent();

            KopsikApi.OnTimeEntryAutocomplete += OnTimeEntryAutocomplete;
            KopsikApi.OnTimerState += OnTimerState;

            comboBoxDescription.DisplayMember = "Text";

        }

        private void comboBoxDescription_SelectedIndexChanged(object sender, EventArgs e)
        {
            object o = comboBoxDescription.SelectedItem;
            if (o == null)
            {
                task_id = 0;
                project_id = 0;
                linkLabelProject.Visible = false;
                linkLabelProject.Text = "";
                return;
            }
            KopsikApi.KopsikAutocompleteItem item = (KopsikApi.KopsikAutocompleteItem)o;
            comboBoxDescription.Text = item.Description;
            if (item.ProjectID > 0)
            {
                linkLabelProject.Text = item.ProjectAndTaskLabel;
                linkLabelProject.BackColor = ColorTranslator.FromHtml(item.ProjectColor);
                linkLabelProject.Visible = true;
            }
            else
            {
                linkLabelProject.Visible = false;
            }
            task_id = item.TaskID;
            project_id = item.ProjectID;
        }

        private const string defaultDescription = "What are you doing?";
        private const string defaultDuration = "0 sec";

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (buttonStart.Text == "Start") {
                string description = comboBoxDescription.Text;
                if (defaultDescription == description)
                {
                    description = "";
                }

                string duration = textBoxDuration.Text;
                if (defaultDuration == duration)
                {
                    duration = "";
                }

                KopsikApi.kopsik_start(KopsikApi.ctx,
                    description,
                    duration,
                    task_id,
                    project_id);
            }
            else
            {
                KopsikApi.kopsik_stop(KopsikApi.ctx);
            }
        }

        public void SetAcceptButton(Form frm)
        {
            frm.AcceptButton = buttonStart;
        }

        void OnTimerState(IntPtr te)
        {
            if (te == IntPtr.Zero)
            {
                DisplayStoppedTimerState();
                return;
            }
            KopsikApi.KopsikTimeEntryViewItem view =
                (KopsikApi.KopsikTimeEntryViewItem)Marshal.PtrToStructure(
                te, typeof(KopsikApi.KopsikTimeEntryViewItem));
            KopsikApi.KopsikTimeEntryViewItem copy = view;
            DisplayRunningTimerState(copy);
        }

        void DisplayStoppedTimerState()
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayStoppedTimerState(); });
                return;
            }

            timerRunningDuration.Enabled = false;

            duration_in_seconds = 0;

            buttonStart.Text = "Start";

            comboBoxDescription.Text = defaultDescription;
            comboBoxDescription.Visible = true;

            linkLabelDescription.Visible = false;
            linkLabelDescription.Text = "";

            textBoxDuration.Text = defaultDuration;
            textBoxDuration.Visible = true;

            linkLabelDuration.Visible = false;
            linkLabelDuration.Text = "";
        }

        void DisplayRunningTimerState(KopsikApi.KopsikTimeEntryViewItem te)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayRunningTimerState(te); });
                return;
            }
            duration_in_seconds = te.DurationInSeconds;

            timerRunningDuration.Enabled = true;

            buttonStart.Text = "Stop";

            linkLabelDescription.Top = comboBoxDescription.Top;
            linkLabelDescription.Left = comboBoxDescription.Left;
            linkLabelDescription.Text = te.Description;
            if (linkLabelDescription.Text == "")
            {
                linkLabelDescription.Text = "(no description)";
            }
            linkLabelDescription.Visible = true;

            comboBoxDescription.Visible = false;
            comboBoxDescription.Text = "";

            linkLabelDuration.Top = textBoxDuration.Top;
            linkLabelDuration.Left = textBoxDuration.Left;
            linkLabelDuration.Text = te.Duration;
            linkLabelDuration.Visible = true;

            textBoxDuration.Visible = false;
            textBoxDuration.Text = "";

            linkLabelProject.Text = te.ProjectAndTaskLabel;
            linkLabelProject.Visible = true;
            task_id = 0;
            project_id = 0;
        }

        void OnTimeEntryAutocomplete(ref KopsikApi.KopsikAutocompleteItem first)
        {
            List<KopsikApi.KopsikAutocompleteItem> list =
                KopsikApi.ConvertToAutocompleteList(ref first);
            DisplayTimeEntryAutocomplete(list);
        }

        void DisplayTimeEntryAutocomplete(List<KopsikApi.KopsikAutocompleteItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayTimeEntryAutocomplete(list); });
                return;
            }
            timeEntryAutocompleteUpdate = list;
            if (comboBoxDescription.DroppedDown || comboBoxDescription.Focused)
            {
                return;
            }
            comboBoxDescription.Items.Clear();
            foreach (object o in timeEntryAutocompleteUpdate)
            {
                comboBoxDescription.Items.Add(o);
            }
            timeEntryAutocompleteUpdate = null;
        }

        private void linkLabelDescription_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            KopsikApi.kopsik_edit(KopsikApi.ctx, "", true, KopsikApi.Description);
        }

        private void linkLabelDuration_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            KopsikApi.kopsik_edit(KopsikApi.ctx, "", true, KopsikApi.Duration);
        }

        private void timerRunningDuration_Tick(object sender, EventArgs e)
        {
            if (duration_in_seconds >= 0)
            {
                // Timer is not running
                return;
            }
            const int duration_len = 20;
            StringBuilder sb = new StringBuilder(duration_len);
            KopsikApi.kopsik_format_duration_in_seconds_hhmmss(
                duration_in_seconds, sb, duration_len);
            string s = sb.ToString();
            if (s != linkLabelDuration.Text) {
                linkLabelDuration.Text = s;
            }
        }

        private void linkLabelProject_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            KopsikApi.kopsik_edit(KopsikApi.ctx, "", true, KopsikApi.Project);
        }

        private void comboBoxDescription_DropDownClosed(object sender, EventArgs e)
        {
            if (timeEntryAutocompleteUpdate != null)
            {
                DisplayTimeEntryAutocomplete(timeEntryAutocompleteUpdate);
            }
        }

        private void TimerEditViewController_Load(object sender, EventArgs e)
        {

        }

        private void comboBoxDescription_Enter(object sender, EventArgs e)
        {
            if (comboBoxDescription.Text == defaultDescription)
            {
                comboBoxDescription.Text = "";
            }
        }

        private void comboBoxDescription_Leave(object sender, EventArgs e)
        {
            if (comboBoxDescription.Text == "")
            {
                comboBoxDescription.Text = defaultDescription;
            }
        }

        private void textBoxDuration_Enter(object sender, EventArgs e)
        {
            if (textBoxDuration.Text == defaultDuration)
            {
                textBoxDuration.Text = "";
            }
        }

        private void textBoxDuration_Leave(object sender, EventArgs e)
        {
            if (textBoxDuration.Text == "")
            {
                textBoxDuration.Text = defaultDuration;
            }
        }
    }
}
