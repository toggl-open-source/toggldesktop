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
            KopsikApi.OnRunningTimerState += OnRunningTimerState;
            KopsikApi.OnStoppedTimerState += OnStoppedTimerState;

            comboBoxDescription.DisplayMember = "Text";
            this.Anchor = (AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top);
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
                comboBoxDescription.Top = 16;
                return;
            }
            KopsikApi.KopsikAutocompleteItem item = (KopsikApi.KopsikAutocompleteItem)o;
            comboBoxDescription.Text = item.Description;
            
            if (item.ProjectID > 0)
            {
                linkLabelProject.Text = item.ProjectAndTaskLabel;
                linkLabelProject.Visible = true;
                comboBoxDescription.Top = 10;
            }
            else
            {
                linkLabelProject.Visible = false;
                comboBoxDescription.Top = 16;
            }
            task_id = item.TaskID;
            project_id = item.ProjectID;
        }

        private const string defaultDescription = "What are you doing?";
        private const string defaultDuration = "00:00:00";

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

                KopsikApi.Start(
                    description,
                    duration,
                    task_id,
                    project_id);
            }
            else
            {
                KopsikApi.Stop();
            }
        }

        public void SetAcceptButton(Form frm)
        {
            frm.AcceptButton = buttonStart;
        }

        void OnStoppedTimerState()
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnStoppedTimerState(); });
                return;
            }

            timerRunningDuration.Enabled = false;

            duration_in_seconds = 0;

            buttonStart.Text = "Start";
            buttonStart.BackColor = ColorTranslator.FromHtml("#47bc00");
            buttonStart.FlatAppearance.BorderColor = ColorTranslator.FromHtml("#47bc00");

            if (!comboBoxDescription.Focused)
            {
                comboBoxDescription.Text = defaultDescription;
            }
            comboBoxDescription.Visible = true;

            linkLabelDescription.Visible = false;
            linkLabelDescription.Text = "";

            if (!textBoxDuration.Focused)
            {
                textBoxDuration.Text = defaultDuration;
            }
            textBoxDuration.Visible = true;

            linkLabelDuration.Visible = false;
            linkLabelDuration.Text = "";

            linkLabelProject.Text = "";
            linkLabelProject.Visible = false;
            comboBoxDescription.Top = 16;
        }

        void OnRunningTimerState(KopsikApi.KopsikTimeEntryViewItem te)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnRunningTimerState(te); });
                return;
            }
            duration_in_seconds = te.DurationInSeconds;

            timerRunningDuration.Enabled = true;

            buttonStart.Text = "Stop";
            buttonStart.BackColor = ColorTranslator.FromHtml("#e20000");
            buttonStart.FlatAppearance.BorderColor = ColorTranslator.FromHtml("#e20000");
            if (te.ProjectAndTaskLabel.Length > 0)
            {
                linkLabelProject.Text = te.ProjectAndTaskLabel;
                linkLabelProject.Visible = true;
                comboBoxDescription.Top = 10;
            }

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
            linkLabelDuration.Text = te.Duration;
            linkLabelDuration.Visible = true;

            textBoxDuration.Visible = false;
            textBoxDuration.Text = "";

            task_id = 0;
            project_id = 0;
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
            foreach (object o in timeEntryAutocompleteUpdate)
            {
                comboBoxDescription.Items.Add(o);
            }
            timeEntryAutocompleteUpdate = null;
        }

        private void linkLabelDescription_Click(object sender, EventArgs e)
        {
            KopsikApi.Edit("", true, KopsikApi.Description);
        }

        private void linkLabelDuration_Click(object sender, EventArgs e)
        {
            KopsikApi.Edit("", true, KopsikApi.Duration);
        }

        private void timerRunningDuration_Tick(object sender, EventArgs e)
        {
            if (duration_in_seconds >= 0)
            {
                // Timer is not running
                return;
            }
            string s = KopsikApi.FormatDurationInSecondsHHMMSS(duration_in_seconds);
            if (s != linkLabelDuration.Text) {
                linkLabelDuration.Text = s;
            }
        }

        private void linkLabelProject_Click(object sender, EventArgs e)
        {
            KopsikApi.Edit("", true, KopsikApi.Project);
        }

        private void comboBoxDescription_DropDownClosed(object sender, EventArgs e)
        {
            if (timeEntryAutocompleteUpdate != null)
            {
                OnTimeEntryAutocomplete(timeEntryAutocompleteUpdate);
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
