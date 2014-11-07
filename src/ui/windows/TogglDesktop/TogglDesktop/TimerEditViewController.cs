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
        private int defaultDescriptionTop = 20;
        private int projectDescriptionTop = 10;
        private SizeF currentFactor;
        public Boolean durationFocused = false;

        private List<Toggl.AutocompleteItem> timeEntryAutocompleteUpdate;
        private List<Toggl.AutocompleteItem> autoCompleteList;

        public TimerEditViewController()
        {
            InitializeComponent();

            descriptionTextBox.autoCompleteListBox.KeyDown += autoCompleteListBox_KeyDown;
            descriptionTextBox.autoCompleteListBox.Click += autoCompleteListBox_Click;
            textBoxDuration.Top = (Height / 2) - (linkLabelDuration.Height / 2);
            descriptionTextBox.Top = (Height / 2) - (descriptionTextBox.Height / 2);
            defaultDescriptionTop = descriptionTextBox.Top;
            projectDescriptionTop = defaultDescriptionTop - (descriptionTextBox.Height / 2);

            Toggl.OnTimeEntryAutocomplete += OnTimeEntryAutocomplete;
            Toggl.OnRunningTimerState += OnRunningTimerState;
            Toggl.OnStoppedTimerState += OnStoppedTimerState;

            Anchor = (AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top);
        }

        public AutoCompleteTextBox getDescriptionTextBox() {
            return descriptionTextBox;
        }

        public TextBox getDurationTextBox()
        {
            return textBoxDuration;
        }

        public bool areFieldsFocused()
        {
            return descriptionTextBox.Focused || textBoxDuration.Focused;
        }

        protected override void ScaleControl(SizeF factor, BoundsSpecified specified)
        {
            base.ScaleControl(factor, specified);
            this.descriptionTextBox.scaleList(factor);
            if (factor.Height > 1 && currentFactor != factor)
            {
                currentFactor = factor;
                textBoxDuration.Top = (Height / 2) - (textBoxDuration.Height / 2);
                reScale();
            }
        }

        private void reScale()
        {
            scaleChild(descriptionTextBox);
            scaleChild(linkLabelDescription);
            scaleChild(textBoxDuration);
            scaleChild(linkLabelDuration);
            scaleChild(linkLabelProject);
            scaleChild(buttonStart);
        }

        private void scaleChild(Control child)
        {
            float scaledFontSize = (float)(int)(child.Font.Size * currentFactor.Height);
            child.Font = new Font(child.Font.Name, scaledFontSize, GraphicsUnit.Pixel);
        }

        public bool focusList()
        {
            return !(!isAutocompleteOpened() && !descriptionTextBox.Focused && !textBoxDuration.Focused);
        }

        public bool isAutocompleteOpened()
        {
            return descriptionTextBox.autoCompleteListBox.Visible;
        }

        private void autoCompleteListBox_Click(object sender, EventArgs e)
        {
            selectAutoComplete();
        }

        private void autoCompleteListBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                if (autoCompleteList == null)
                {
                    return;
                }

                selectAutoComplete();
            }
            else if (e.KeyCode == Keys.Escape)
            {
                descriptionTextBox.ResetListBox();
                descriptionTextBox.Focus();
            }
        }

        private const string defaultDescription = "What are you doing?";
        private const string defaultDuration = "00:00:00";

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (buttonStart.Text != "Start")
            {
                Toggl.Stop();
                return;
            }

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

            if (Toggl.Start(
                description,
                duration,
                task_id,
                project_id) == null)
            {
                task_id = 0;
                project_id = 0;
                return;
            }

            descriptionTextBox.Text = defaultDescription;

            if (!textBoxDuration.Focused)
            {
                textBoxDuration.Text = defaultDuration;
            }
            else
            {
                textBoxDuration.Text = "";
            }
            labelClearProject.Visible = false;

            task_id = 0;
            project_id = 0;
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

        void OnRunningTimerState(Toggl.TimeEntry te)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnRunningTimerState(te); });
                return;
            }
            duration_in_seconds = te.DurationInSeconds;

            timerRunningDuration.Enabled = true;

            labelClearProject.Visible = false;

            buttonStart.Text = "Stop";
            buttonStart.BackColor = ColorTranslator.FromHtml("#e20000");
            buttonStart.FlatAppearance.BorderColor = ColorTranslator.FromHtml("#e20000");
            if (te.ProjectAndTaskLabel.Length > 0)
            {
                linkLabelProject.Text = te.ProjectAndTaskLabel;
                linkLabelProject.Visible = true;
                descriptionTextBox.Top = projectDescriptionTop+1;
            }
            else
            {
                linkLabelProject.Visible = false;
                descriptionTextBox.Top = defaultDescriptionTop;
            }

            linkLabelDescription.Top = descriptionTextBox.Top;
            linkLabelDescription.Left = descriptionTextBox.Left-3;
            linkLabelDescription.Text = te.Description;
            if (linkLabelDescription.Text.Length == 0)
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

        void OnTimeEntryAutocomplete(List<Toggl.AutocompleteItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnTimeEntryAutocomplete(list); });
                return;
            }
            timeEntryAutocompleteUpdate = list;
            autoCompleteList = list;
        }

        private void linkLabelDescription_Click(object sender, EventArgs e)
        {
            Toggl.Edit("", true, Toggl.Description);
        }

        private void linkLabelDuration_Click(object sender, EventArgs e)
        {
            Toggl.Edit("", true, Toggl.Duration);
        }

        private void timerRunningDuration_Tick(object sender, EventArgs e)
        {
            if (duration_in_seconds >= 0)
            {
                // Timer is not running
                return;
            }
            string s = Toggl.FormatDurationInSecondsHHMMSS(duration_in_seconds);
            if (s != linkLabelDuration.Text) {
                linkLabelDuration.Text = s;
            }
        }

        private void linkLabelProject_Click(object sender, EventArgs e)
        {
            Toggl.Edit("", true, Toggl.Project);
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
            if (!descriptionTextBox.autoCompleteListBox.Focused)
            {
                descriptionTextBox.ResetListBox();
            }
        }

        private void textBoxDuration_Enter(object sender, EventArgs e)
        {
            if (textBoxDuration.Text == defaultDuration)
            {
                textBoxDuration.Text = "";
            }
            durationFocused = true;
        }

        private void textBoxDuration_Leave(object sender, EventArgs e)
        {
            if (textBoxDuration.Text == "")
            {
                textBoxDuration.Text = defaultDuration;
            }
            durationFocused = false;
        }

        private void selectAutoComplete()
        {
            descriptionTextBox.ResetListBox();
            object selectedItem = descriptionTextBox.autoCompleteListBox.SelectedItem;
            if (null == selectedItem) {
                return;
            }
            Toggl.AutocompleteItem item = (Toggl.AutocompleteItem)selectedItem;
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

            labelClearProject.Visible = linkLabelProject.Visible;
        }

        private void labelClearProject_Click(object sender, EventArgs e)
        {
            project_id = 0;
            task_id = 0;
            linkLabelProject.Text = "";
            labelClearProject.Visible = false;
            labelClearProject.Enabled = false;
            descriptionTextBox.Top = defaultDescriptionTop;
        }

        private void linkLabelProject_Enter(object sender, EventArgs e)
        {
            if (linkLabelProject.Text.Length > 0 && buttonStart.Text == "Start")
            {
                labelClearProject.Visible = true;
                labelClearProject.Enabled = true;
            }
        }

        private void descriptionTextBox_KeyUp(object sender, KeyEventArgs e)
        {
            if (descriptionTextBox.Text.Length == 0)
            {
                descriptionTextBox.ResetListBox();
            }
            descriptionTextBox.UpdateListBox(autoCompleteList);
        }

        private void descriptionTextBox_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            if (descriptionTextBox.parseKeyDown(e, autoCompleteList))
            {
                selectAutoComplete();
            }
        }

        private void textBoxDuration_Click(object sender, EventArgs e)
        {
            durationFocused = true;
            string descriptionText = "";
            if (descriptionTextBox.Text != defaultDescription)
            {
                descriptionText = descriptionTextBox.Text;
            }
            string GUID = Toggl.Start(
                descriptionText,
                defaultDuration,
                task_id,
                project_id);

            if (GUID != null)
            {
                task_id = 0;
                project_id = 0;
                labelClearProject.Visible = false;
                Toggl.Edit(GUID, false, Toggl.Duration);
            }
        }
    }
}
