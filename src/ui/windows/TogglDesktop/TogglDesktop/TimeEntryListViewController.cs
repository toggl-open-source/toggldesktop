using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Threading;

namespace TogglDesktop
{
    public partial class TimeEntryListViewController : UserControl
    {
        private Object rendering = new Object();
        private DateTime lastRender;

        public TimeEntryListViewController()
        {
            InitializeComponent();

            Dock = DockStyle.Fill;

            Toggl.OnTimeEntryList += OnTimeEntryList;
            Toggl.OnLogin += OnLogin;
        }

        public void SetAcceptButton(Form frm)
        {
            timerEditViewController.SetAcceptButton(frm);
        }

        void OnTimeEntryList(bool open, List<Toggl.TimeEntry> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnTimeEntryList(open, list); });
                return;
            }
            lastRender = DateTime.Now;

            lock (rendering)
            {
                renderTimeEntryList(list);
            }

            TimeSpan spent = DateTime.Now.Subtract(lastRender);
            Console.WriteLine(String.Format(
                "Time entries list view rendered in {0} ms",
                spent.TotalMilliseconds));
        }

        private void renderTimeEntryList(List<Toggl.TimeEntry> list)
        {
            emptyLabel.Visible = (list.Count == 0);
            entries.SuspendLayout();

            for (int i = 0; i < list.Count; i++)
            {
                Toggl.TimeEntry te = list.ElementAt(i);

                TimeEntryCell cell = null;
                if (entries.Controls.Count > i)
                {
                    cell = entries.Controls[i] as TimeEntryCell;
                }

                if (cell == null)
                {
                    cell = new TimeEntryCell();
                    entries.Controls.Add(cell);
                    if (i == 0)
                    {
                        cell.Width = entries.Width;
                    }
                    else
                    {
                        cell.Dock = DockStyle.Top;
                    }
                }

                cell.Display(te);
                entries.Controls.SetChildIndex(cell, i);
            }

            while (entries.Controls.Count > list.Count)
            {
                entries.Controls.RemoveAt(list.Count);
            }

            entries.ResumeLayout();
            entries.PerformLayout();
        }

        private void TimeEntryListViewController_Load(object sender, EventArgs e)
        {
        }

        void OnLogin(bool open, UInt64 user_id)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnLogin(open, user_id); });
                return;
            }
            if (open || user_id == 0)
            {
                entries.SuspendLayout();
                entries.Controls.Clear();
                entries.ResumeLayout();
                entries.PerformLayout();
            }
        }

        private void entries_ClientSizeChanged(object sender, EventArgs e)
        {
            if (entries.Controls.Count > 0)
            {
                entries.SuspendLayout();
                entries.Controls[0].Width = entries.ClientSize.Width;
                entries.ResumeLayout();
            }
        }

        private void entries_MouseEnter(object sender, EventArgs e)
        {
            entries.Focus();
        }

        private void emptyLabel_Click(object sender, EventArgs e)
        {
            Toggl.OpenInBrowser();
        }
    }
}
