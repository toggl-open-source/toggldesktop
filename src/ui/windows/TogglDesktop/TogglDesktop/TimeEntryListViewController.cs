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
            this.Dock = DockStyle.Fill;
            int y = 0;
            this.EntriesList.Controls.Clear();
            this.EntriesList.SuspendLayout();

            foreach (TogglApi.KopsikTimeEntryViewItem item in list)
            {
                if (item.IsHeader)
                {
                    TimeEntryCellWithHeader cell = new TimeEntryCellWithHeader(y, this.Width);
                    cell.Setup(item);
                    this.EntriesList.Controls.Add(cell);
                    y += cell.Height;
                }
                else
                {
                    TimeEntryCell cell = new TimeEntryCell(y, this.Width);
                    cell.Setup(item);
                    this.EntriesList.Controls.Add(cell);
                    y += cell.Height;
                }

            }
            this.EntriesList.ResumeLayout(false);
            this.EntriesList.PerformLayout();
        }

        private void TimeEntryListViewController_Load(object sender, EventArgs e)
        {
            // FIXME:
            //regular 50 header 100
        }
    }
}
