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

            KopsikApi.OnTimeEntryList += OnTimeEntryList;
        }

        public void SetAcceptButton(Form frm)
        {
            timerEditViewController.SetAcceptButton(frm);
        }

        void OnTimeEntryList(bool open, IntPtr first)
        {
            List<KopsikApi.KopsikTimeEntryViewItem> list = KopsikApi.ConvertToTimeEntryList(first);
            DisplayTimeEntryList(open, list);
        }

        void DisplayTimeEntryList(bool open, List<KopsikApi.KopsikTimeEntryViewItem> list)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { DisplayTimeEntryList(open, list); });
                return;
            }
            int y = 0;
            this.panelContent.Controls.Clear();
            foreach (KopsikApi.KopsikTimeEntryViewItem item in list)
            {
                if (item.IsHeader)
                {
                    TimeEntryCellWithHeader cell = new TimeEntryCellWithHeader(y);
                    cell.Setup(item);
                    this.panelContent.Controls.Add(cell);
                    y += 82;
                }
                else
                {
                    TimeEntryCell cell = new TimeEntryCell(y);
                    cell.Setup(item);
                    this.panelContent.Controls.Add(cell);
                    y += 49;
                }                 
            }
        }

        private void TimeEntryListViewController_Load(object sender, EventArgs e)
        {
            // FIXME:
            //regular 49 header 82
        }

        private void panelContent_Paint(object sender, PaintEventArgs e)
        {

        }
    }
}
