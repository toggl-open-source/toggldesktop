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
        }

        public void SetAcceptButton(Form frm)
        {
            timerEditViewController.SetAcceptButton(frm);
        }

        internal void DrawEntriesList(List<KopsikApi.KopsikTimeEntryViewItem> list)
        {
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
    }
}
