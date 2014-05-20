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

        public TimerEditViewController()
        {
            InitializeComponent();

            KopsikApi.OnAutocomplete += OnAutocomplete;
            KopsikApi.OnTimerState += OnTimerState;
        }

        private void comboBoxDescription_SelectedIndexChanged(object sender, EventArgs e)
        {
            // FIXME: apply autocomplete selection
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (buttonStart.Text == "Start") {
                string description = comboBoxDescription.Text;
                if ("What are you doing?" == description)
                {
                    description = "";
                }

                string duration = textBoxDuration.Text;
                if ("0 sec" == duration)
                {
                    duration = "";
                }

                KopsikApi.kopsik_start(KopsikApi.ctx,
                    description,
                    duration,
                    0,
                    0);
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

            comboBoxDescription.Visible = true;
            linkLabelDescription.Visible = !comboBoxDescription.Visible;

            labelProject.Visible = false;

            textBoxDuration.Visible = true;
            linkLabelDuration.Visible = !textBoxDuration.Visible;

            labelProject.Visible = false;
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
            linkLabelDescription.Visible = true;
            comboBoxDescription.Visible = !linkLabelDescription.Visible;

            linkLabelDuration.Top = textBoxDuration.Top;
            linkLabelDuration.Left = textBoxDuration.Left;
            linkLabelDuration.Text = te.Duration;
            linkLabelDuration.Visible = true;
            textBoxDuration.Visible = !linkLabelDuration.Visible;

            labelProject.Text = te.ProjectAndTaskLabel;
            labelProject.Visible = true;
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
            comboBoxDescription.Items.Clear();
            foreach (KopsikApi.KopsikAutocompleteItem item in list) {
                comboBoxDescription.Items.Add(item.Text);
            }
        }

        private void labelProject_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_edit(KopsikApi.ctx, "", true, "project");
        }

        private void linkLabelDescription_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            KopsikApi.kopsik_edit(KopsikApi.ctx, "", true, "description");
        }

        private void linkLabelDuration_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            KopsikApi.kopsik_edit(KopsikApi.ctx, "", true, "duration");
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
    }
}
