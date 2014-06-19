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
        private List<KopsikApi.KopsikAutocompleteItem> autoCompleteList;

        public TimerEditViewController()
        {
            InitializeComponent();

            KopsikApi.OnTimeEntryAutocomplete += OnTimeEntryAutocomplete;
            KopsikApi.OnRunningTimerState += OnRunningTimerState;
            KopsikApi.OnStoppedTimerState += OnStoppedTimerState;

            this.Anchor = (AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top);
        }

        private const string defaultDescription = "What are you doing?";
        private const string defaultDuration = "00:00:00";
        private const int defaultDescriptionTop = 20;
        private const int projectDescriptionTop = 10;

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (buttonStart.Text == "Start") {
                string description = descriptionTextBox.Text;
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

            if (!descriptionTextBox.Focused)
            {
                descriptionTextBox.Text = defaultDescription;
            }
            descriptionTextBox.Visible = true;

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
            descriptionTextBox.Top = defaultDescriptionTop;
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
                descriptionTextBox.Top = projectDescriptionTop+1;
            }

            linkLabelDescription.Top = descriptionTextBox.Top-1;
            linkLabelDescription.Left = descriptionTextBox.Left-3;
            linkLabelDescription.Text = te.Description;
            if (linkLabelDescription.Text == "")
            {
                linkLabelDescription.Text = "(no description)";
            }
            linkLabelDescription.Visible = true;

            descriptionTextBox.Visible = false;
            descriptionTextBox.Text = "";

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
            autoCompleteList = list;
            descriptionTextBox.AutoCompleteCustomSource.Clear();

            if (descriptionTextBox.Focused)
            {
                return;
            }
            AutoCompleteStringCollection collection = new AutoCompleteStringCollection();
            foreach (object o in timeEntryAutocompleteUpdate)
            {
                collection.Add(o.ToString());
            }
            timeEntryAutocompleteUpdate = null;
            
            descriptionTextBox.AutoCompleteCustomSource = collection;
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

        private void descriptionTextBox_Enter(object sender, EventArgs e)
        {
            if (descriptionTextBox.Text == defaultDescription)
            {
                descriptionTextBox.Text = "";
            }
        }

        private void descriptionTextBox_Leave(object sender, EventArgs e)
        {
            if (descriptionTextBox.Text == "")
            {
                descriptionTextBox.Text = defaultDescription;
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

        private void descriptionTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.KeyCode == Keys.Enter)
            {
                if (autoCompleteList == null)
                {
                    return;
                }
                foreach (KopsikApi.KopsikAutocompleteItem item in autoCompleteList)
                {
                    if (item.ToString() == descriptionTextBox.Text)
                    {
                        descriptionTextBox.Text = item.Description;

                        if (item.ProjectID > 0)
                        {
                            linkLabelProject.Text = item.ProjectAndTaskLabel;
                            linkLabelProject.Visible = true;
                            descriptionTextBox.Top = projectDescriptionTop;
                        }
                        else
                        {
                            linkLabelProject.Visible = false;
                            descriptionTextBox.Top = defaultDescriptionTop;
                        }
                        task_id = item.TaskID;
                        project_id = item.ProjectID;
                        break;
                    }
                }

                
            }
        }
    }
}
