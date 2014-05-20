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
    public partial class TimeEntryEditViewController : UserControl
    {
        public TimeEntryEditViewController()
        {
            InitializeComponent();
        }

        private void TimeEntryEditViewController_Load(object sender, EventArgs e)
        {

        }

        public void SetAcceptButton(Form frm)
        {
            frm.AcceptButton = buttonDone;
        }

        private void buttonDone_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_view_time_entry_list(KopsikApi.ctx);
        }

    }
}
