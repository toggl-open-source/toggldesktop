using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TogglDesktop
{
    public partial class IdleNotificationWindowController : Form
    {
        private UInt64 idle_started_at;

        public IdleNotificationWindowController()
        {
            InitializeComponent();

            KopsikApi.OnIdleNotification += OnIdleNotification;
            KopsikApi.OnTimerState += OnTimerState;
        }

        private void buttonKeepTime_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void buttonDiscardTime_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_stop_running_time_entry_at(KopsikApi.ctx, idle_started_at);
            Close();
        }

        void OnIdleNotification(UInt64 started, UInt64 finished, UInt64 seconds)
        {
            DisplayIdleNotification(started, finished, seconds);
        }

        void DisplayIdleNotification(UInt64 started, UInt64 finished, UInt64 seconds)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayIdleNotification(started, finished, seconds); });
                return;
            }
            idle_started_at = started;
        }

        void OnTimerState(IntPtr te)
        {
            DisplayTimerState(te != IntPtr.Zero);
        }

        void DisplayTimerState(bool is_tracking)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayTimerState(is_tracking); });
                return;
            }
            if (!is_tracking && Visible)
            {
                Close();
            }
        }

        private void IdleNotificationWindowController_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (!TogglDesktop.Program.ShuttingDown)
            {
                this.Hide();
                e.Cancel = true;
            }
        }
    }
}
