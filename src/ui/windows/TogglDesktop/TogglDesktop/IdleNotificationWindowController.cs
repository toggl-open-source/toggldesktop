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

            TogglApi.OnIdleNotification += OnIdleNotification;
            TogglApi.OnStoppedTimerState += OnStoppedTimerState;
        }

        private void buttonKeepTime_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void buttonDiscardTime_Click(object sender, EventArgs e)
        {
            TogglApi.StopRunningTimeEntryAt(idle_started_at);
            Close();
        }

        void OnIdleNotification(string since, string duration, UInt64 started)
        {
            DisplayIdleNotification(since, duration, started);
        }

        void DisplayIdleNotification(string since, string duration, UInt64 started)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayIdleNotification(since, duration, started); });
                return;
            }
            labelIdleSince.Text = since;
            labelIdleDuration.Text = duration;
            idle_started_at = started;
        }

        void OnStoppedTimerState()
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnStoppedTimerState(); });
                return;
            }
            if (Visible)
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

        private void IdleNotificationWindowController_Load(object sender, EventArgs e)
        {

        }
    }
}
