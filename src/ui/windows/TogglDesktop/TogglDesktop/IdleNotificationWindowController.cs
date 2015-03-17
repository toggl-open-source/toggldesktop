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
    public partial class IdleNotificationWindowController : TogglForm
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
            Toggl.ShowApp();
            Close();
        }

        private void buttonDiscardTime_Click(object sender, EventArgs e)
        {
            Toggl.DiscardTimeAt(time_entry_guid, idle_started_at, false);
            Close();
        }

        void OnIdleNotification(
            string guid,
            string since,
            string duration,
            UInt64 started,
            string description)
        {
            DisplayIdleNotification(guid, since, duration, started, description);
        }

        void DisplayIdleNotification(
            string guid,
            string since,
            string duration,
            UInt64 started,
            string description)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayIdleNotification(guid, since, duration, started, description); });
                return;
            }

            time_entry_guid = guid;

            labelIdleSince.Text = since;
            labelIdleDuration.Text = duration;
            idle_started_at = started;
            labelDescription.Text = description;
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
                Hide();
                e.Cancel = true;
            }
        }

        private void IdleNotificationWindowController_Load(object sender, EventArgs e)
        {

        }

        internal void ShowWindow()
        {
            Show();
            BringToFront();
        }

        private void buttonSplitIdleTimeIntoNewEntry_Click(object sender, EventArgs e)
        {
            Toggl.DiscardTimeAt(time_entry_guid, idle_started_at, true);
            Close();
        }
    }
}
