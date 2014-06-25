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
            Width = w;
            Anchor = (AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top);
        }

        internal void Setup(TogglApi.KopsikTimeEntryViewItem item)
        {
            TimeEntry = item;

            labelDescription.Text = TimeEntry.Description;
            labelProject.ForeColor = ColorTranslator.FromHtml(TimeEntry.Color);
            labelProject.Text = TimeEntry.ProjectAndTaskLabel;
            labelDuration.Text = TimeEntry.Duration;
            labelBillable.Visible = TimeEntry.Billable;
            labelTag.Visible = (TimeEntry.Tags != null && TimeEntry.Tags.Length > 0);
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
