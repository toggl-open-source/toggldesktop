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
        public string GUID;

        public TimeEntryCell()
        {
            InitializeComponent();
        }

        internal void Display(Toggl.TimeEntry item)
        {
            GUID = item.GUID;

            labelDescription.Text = item.Description;
            labelProject.ForeColor = ColorTranslator.FromHtml(item.Color);
            labelProject.Text = item.ProjectLabel;
            labelClient.Text = item.ClientLabel;
            labelTask.Text = item.TaskLabel;
            labelDuration.Text = item.Duration;
            labelBillable.Visible = item.Billable;
            labelTag.Visible = (item.Tags != null && item.Tags.Length > 0);

            if (item.IsHeader)
            {
                labelFormattedDate.Text = item.DateHeader;
                labelDateDuration.Text = item.DateDuration;
            }
            if (item.IsHeader)
            {
                Height = headerPanel.Height + panel.Height;
                panel.Top = headerPanel.Bottom;
            }
            else
            {
                Height = panel.Height;
                panel.Top = 0;
            }
            headerPanel.Visible = item.IsHeader;
            toolTip.SetToolTip(labelDescription, item.Description);
            toolTip.SetToolTip(labelTask, item.ProjectAndTaskLabel);
            toolTip.SetToolTip(labelProject, item.ProjectAndTaskLabel);
            toolTip.SetToolTip(labelClient, item.ProjectAndTaskLabel);
            if (labelTag.Visible)
            {
                toolTip.SetToolTip(labelTag, item.Tags.Replace("|", ", "));
            }
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

        private void panel_Click(object sender, EventArgs e)
        {
            Toggl.Edit(GUID, false, "");
        }

        private void labelBillable_Click(object sender, EventArgs e)
        {
            Toggl.Edit(GUID, false, "");
        }

        private void labelTag_Click(object sender, EventArgs e)
        {
            Toggl.Edit(GUID, false, "");
        }

        private void labelTask_Click(object sender, EventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Project);
        }

        private void labelClient_Click(object sender, EventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Project);
        }

        private void taskProjectPanel_Click(object sender, EventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Project);
        }
    }
}