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
        private TogglApi.KopsikTimeEntryViewItem TimeEntry;
        public TimeEntryCell(int y, int w)
        {
            InitializeComponent();
            Location = new Point(0, y);
            this.Width = w;
            this.Anchor = (AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top);
        }

        internal void Setup(TogglApi.KopsikTimeEntryViewItem item)
        {
            this.TimeEntry = item;
            this.Render();
        }

        private void Render()
        {
            this.labelDescription.Text = this.TimeEntry.Description;
            this.labelProject.ForeColor = ColorTranslator.FromHtml(this.TimeEntry.Color);
            this.labelProject.Text = this.TimeEntry.ProjectAndTaskLabel;
            this.labelDuration.Text = this.TimeEntry.Duration;
            this.labelBillable.Visible = this.TimeEntry.Billable;
            this.labelTag.Visible = (this.TimeEntry.Tags != null && this.TimeEntry.Tags.Length > 0);
        }

        private void TimeEntryCell_MouseClick(object sender, MouseEventArgs e)
        {
            TogglApi.Edit(TimeEntry.GUID, false, "");
        }

        private void labelDescription_Click(object sender, EventArgs e)
        {
            TogglApi.Edit(TimeEntry.GUID, false, TogglApi.Description);
        }

        private void labelDuration_Click(object sender, EventArgs e)
        {
            TogglApi.Edit(TimeEntry.GUID, false, TogglApi.Duration);
        }

        private void labelProject_Click(object sender, EventArgs e)
        {
            TogglApi.Edit(TimeEntry.GUID, false, TogglApi.Project);
        }

        private void labelContinue_Click(object sender, EventArgs e)
        {
            TogglApi.Continue(TimeEntry.GUID);
        }

    }
}
