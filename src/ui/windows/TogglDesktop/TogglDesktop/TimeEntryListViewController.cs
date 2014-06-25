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

            TogglApi.OnTimeEntryList += OnTimeEntryList;
        }

        public void SetAcceptButton(Form frm)
        {
            timerEditViewController.SetAcceptButton(frm);
        }

        void OnTimeEntryList(bool open, List<TogglApi.KopsikTimeEntryViewItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { OnTimeEntryList(open, list); });
                return;
            }
            DateTime start = DateTime.Now;

            int y = 0;
            
            List<Control> controls = new List<Control>();

            foreach (TogglApi.KopsikTimeEntryViewItem item in list)
            {
                if (item.IsHeader)
                {
                    TimeEntryCellWithHeader cell = new TimeEntryCellWithHeader(y, Width);
                    cell.Setup(item);
                    controls.Add(cell);
                    y += cell.Height;
                }
                else
                {
                    TimeEntryCell cell = new TimeEntryCell(y, Width);
                    cell.Setup(item);
                    controls.Add(cell);
                    y += cell.Height;
                }

            }

            Dock = DockStyle.Fill;
            EntriesList.SuspendLayout();
            EntriesList.Controls.Clear();
            EntriesList.Controls.AddRange(controls.ToArray());
            EntriesList.ResumeLayout(false);
            EntriesList.PerformLayout();

            TimeSpan spent = DateTime.Now.Subtract(start);
            Console.WriteLine(String.Format(
                "Time entries list view rendered in {0} ms",
                spent.TotalMilliseconds));
        }

        private void TimeEntryListViewController_Load(object sender, EventArgs e)
        {
            // FIXME:
            //regular 50 header 100
        }
    }
}
