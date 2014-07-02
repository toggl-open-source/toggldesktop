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
        public string GUID;

        public TimeEntryCellWithHeader()
        {
            InitializeComponent();
            Anchor = (AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top);
        }

        internal void Display(Toggl.TimeEntry item)
        {
            GUID = item.GUID;

            labelDescription.Text = item.Description;
            labelProject.ForeColor = ColorTranslator.FromHtml(item.Color);
            labelProject.Text = item.ProjectAndTaskLabel;
            labelFormattedDate.Text = item.DateHeader;
            labelDateDuration.Text = item.DateDuration;
            labelDuration.Text = item.Duration;
            labelBillable.Visible = item.Billable;
            labelTag.Visible = (item.Tags != null && item.Tags.Length > 0);
        }

        private void TimeEntryCellWithHeader_MouseClick(object sender, MouseEventArgs e)
        {
            Toggl.Edit(GUID, false, "");
        }

        private void labelDescription_Click(object sender, EventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Description);
        }

        private void labelDuration_Click(object sender, EventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Duration);
        }

        private void labelProject_Click(object sender, EventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Project);
        }

        private void labelContinue_Click(object sender, EventArgs e)
        {
            Toggl.Continue(GUID);
        }
    }
}