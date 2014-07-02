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
        private string time_entry_guid;

        public IdleNotificationWindowController()
        {
            InitializeComponent();

            Toggl.OnIdleNotification += OnIdleNotification;
            Toggl.OnStoppedTimerState += OnStoppedTimerState;
        }

        private void buttonKeepTime_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void buttonDiscardTime_Click(object sender, EventArgs e)
        {
            Toggl.DiscardTimeAt(time_entry_guid, idle_started_at);
            Close();
        }

        void OnIdleNotification(string guid,
            string since, string duration, UInt64 started)
        {
            DisplayIdleNotification(guid, since, duration, started);
        }

        void DisplayIdleNotification(
            string guid, string since, string duration, UInt64 started)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayIdleNotification(guid, since, duration, started); });
                return;
            }

            time_entry_guid = guid;

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
