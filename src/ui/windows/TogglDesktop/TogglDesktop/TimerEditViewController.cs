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
        public TimerEditViewController()
        {
            InitializeComponent();

            KopsikApi.OnTimerState += OnTimerState;
        }

        private void comboBoxDescription_SelectedIndexChanged(object sender, EventArgs e)
        {
            // FIXME: apply autocomplete selection
        }

        private void textBoxDuration_TextChanged(object sender, EventArgs e)
        {
            // FIXME: apply duration change
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (buttonStart.Text == "Start") {
                KopsikApi.kopsik_start(KopsikApi.ctx,
                    comboBoxDescription.Text,
                    textBoxDuration.Text,
                    0,
                    0);
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
                DisplayEmptyTimerState();
                return;
            }
            KopsikApi.KopsikTimeEntryViewItem view =
                (KopsikApi.KopsikTimeEntryViewItem)Marshal.PtrToStructure(
                te, typeof(KopsikApi.KopsikTimeEntryViewItem));
            KopsikApi.KopsikTimeEntryViewItem copy = view;
            DisplayTimerState(copy);
        }

        void DisplayEmptyTimerState()
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayEmptyTimerState(); });
                return;
            }
            buttonStart.Text = "Start";
        }

        void DisplayTimerState(KopsikApi.KopsikTimeEntryViewItem te)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayTimerState(te); });
                return;
            }
            buttonStart.Text = "Stop";
        }
    }
}
