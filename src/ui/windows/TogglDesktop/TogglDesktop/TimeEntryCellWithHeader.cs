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
    public partial class TimeEntryCellWithHeader : UserControl
    {
        private KopsikApi.KopsikTimeEntryViewItem TimeEntry;
        public TimeEntryCellWithHeader(int y)
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
            this.labelFormattedDate.Text = this.TimeEntry.DateHeader;
            this.labelDateDuration.Text = this.TimeEntry.DateDuration;
            this.labelDuration.Text = this.TimeEntry.Duration;
            this.labelBillable.Visible = this.TimeEntry.Billable;
        }

        private void buttonContinue_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_continue(KopsikApi.ctx, TimeEntry.GUID);
        }

        private void TimeEntryCellWithHeader_MouseClick(object sender, MouseEventArgs e)
        {
            KopsikApi.kopsik_edit(KopsikApi.ctx, TimeEntry.GUID, false, "");
        }

        private void labelDescription_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_edit(KopsikApi.ctx, TimeEntry.GUID, false, KopsikApi.Description);
        }

        private void labelDuration_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_edit(KopsikApi.ctx, TimeEntry.GUID, false, KopsikApi.Duration);
        }

        private void labelProject_Click(object sender, EventArgs e)
        {
            KopsikApi.kopsik_edit(KopsikApi.ctx, TimeEntry.GUID, false, KopsikApi.Project);
        }

        private void TimeEntryCellWithHeader_Load(object sender, EventArgs e)
        {

        }
    }
}