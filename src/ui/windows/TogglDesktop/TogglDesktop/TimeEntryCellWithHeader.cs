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
        private Toggl.TimeEntry TimeEntry;

        public TimeEntryCellWithHeader(int y, int w)
        {
            InitializeComponent();
            Location = new Point(0, y);
            Width = w;
            Anchor = (AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top);
        }

        internal void Setup(Toggl.TimeEntry item)
        {
            TimeEntry = item;

            labelDescription.Text = TimeEntry.Description;
            labelProject.ForeColor = ColorTranslator.FromHtml(TimeEntry.Color);
            labelProject.Text = TimeEntry.ProjectAndTaskLabel;
            labelFormattedDate.Text = TimeEntry.DateHeader;
            labelDateDuration.Text = TimeEntry.DateDuration;
            labelDuration.Text = TimeEntry.Duration;
            labelBillable.Visible = TimeEntry.Billable;
            labelTag.Visible = (TimeEntry.Tags != null && TimeEntry.Tags.Length > 0);
        }

        private void TimeEntryCellWithHeader_MouseClick(object sender, MouseEventArgs e)
        {
            Toggl.Edit(TimeEntry.GUID, false, "");
        }

        private void labelDescription_Click(object sender, EventArgs e)
        {
            Toggl.Edit(TimeEntry.GUID, false, Toggl.Description);
        }

        private void labelDuration_Click(object sender, EventArgs e)
        {
            Toggl.Edit(TimeEntry.GUID, false, Toggl.Duration);
        }

        private void labelProject_Click(object sender, EventArgs e)
        {
            Toggl.Edit(TimeEntry.GUID, false, Toggl.Project);
        }

        private void labelContinue_Click(object sender, EventArgs e)
        {
            Toggl.Continue(TimeEntry.GUID);
        }
    }
}