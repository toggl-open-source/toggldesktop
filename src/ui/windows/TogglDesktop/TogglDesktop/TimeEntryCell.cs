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
    public partial class TimeEntryCell : UserControl
    {
        private KopsikApi.KopsikTimeEntryViewItem TimeEntry;
        public TimeEntryCell(int y)
        {
            InitializeComponent();
            Location = new Point(0, y);
        }

        internal void Setup(KopsikApi.KopsikTimeEntryViewItem item)
        {
            this.TimeEntry = item;
            this.Render();
        }

        private void Render()
        {
            this.labelDescription.Text = this.TimeEntry.Description;
            this.labelProject.Text = this.TimeEntry.ProjectAndTaskLabel;
            this.labelDuration.Text = this.TimeEntry.Duration;
        }

        private void buttonContinue_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_continue(KopsikApi.ctx, TimeEntry.GUID);
        }
    }
}
