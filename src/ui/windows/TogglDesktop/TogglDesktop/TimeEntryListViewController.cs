using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TogglDesktop
{
    public partial class TimeEntryListViewController : UserControl
    {
        public TimeEntryListViewController()
        {
            InitializeComponent();

            Dock = DockStyle.Fill;

            Toggl.OnTimeEntryList += OnTimeEntryList;
        }

        public void SetAcceptButton(Form frm)
        {
            timerEditViewController.SetAcceptButton(frm);
        }

        private int addTableRow()
        {
            int index = entries.RowCount++;
            RowStyle style = new RowStyle(SizeType.AutoSize);
            entries.RowStyles.Add(style);
            return index;
        }

        void OnTimeEntryList(bool open, List<Toggl.TimeEntry> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnTimeEntryList(open, list); });
                return;
            }
            DateTime start = DateTime.Now;

            entries.SuspendLayout();

            foreach (Toggl.TimeEntry item in list)
            {
                bool existing = false;

                // Find existing time entry
                foreach(UserControl c in entries.Controls)
                {
                    if (c is TimeEntryCell)
                    {
                        TimeEntryCell cell = c as TimeEntryCell;
                        if (cell.GUID == item.GUID)
                        {
                            cell.Display(item);
                            existing = true;
                            break;
                        }
                    }
                    if (c is TimeEntryCellWithHeader)
                    {
                        TimeEntryCellWithHeader cell = c as TimeEntryCellWithHeader;
                        if (cell.GUID == item.GUID)
                        {
                            cell.Display(item);
                            existing = true;
                            break;
                        }
                    }
                }
                if (existing)
                {
                    continue;
                }
                int row = addTableRow();
                Console.WriteLine("Row {0}", row);
                if (item.IsHeader)
                {
                    TimeEntryCellWithHeader cell = new TimeEntryCellWithHeader();
                    cell.Display(item);
                    entries.Controls.Add(cell, 0, row);
                }
                else
                {
                    TimeEntryCell cell = new TimeEntryCell();
                    cell.Display(item);
                    entries.Controls.Add(cell, 0, row);
                }
            }

            entries.ResumeLayout();

            TimeSpan spent = DateTime.Now.Subtract(start);
            Console.WriteLine(String.Format(
                "Time entries list view rendered in {0} ms",
                spent.TotalMilliseconds));
        }

        private void TimeEntryListViewController_Load(object sender, EventArgs e)
        {
        }
    }
}
