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
        public bool header = false;
        private TimeEntryListViewController list;
        public bool opened = false;
        private Color hoverColor = System.Drawing.Color.FromArgb(((int)(((byte)(244)))), ((int)(((byte)(244)))), ((int)(((byte)(244)))));
        private Color defaultColor = Color.White;

        public TimeEntryCell(TimeEntryListViewController listContainer)
        {
            list = listContainer;
            InitializeComponent();

            this.defaultColor = this.panel.BackColor;

            TogglForm.ApplyFont("roboto", this.labelFormattedDate);
            TogglForm.ApplyFont("roboto", this.labelDateDuration);
            TogglForm.ApplyFont("roboto-medium", this.labelProject);
            TogglForm.ApplyFont("roboto-medium", this.labelClient);
            TogglForm.ApplyFont("roboto", this.labelTask);
            TogglForm.ApplyFont("roboto", this.labelDescription);
            TogglForm.ApplyFont("roboto-light", this.labelDuration);
        }

        internal void Display(Toggl.TimeEntry item)
        {
            GUID = item.GUID;
            string Color = (item.Color.Length > 0) ? item.Color : "#999999";

            if (item.Description.Length == 0)
            {
                labelDescription.Text = "(no description)";
            }
            else
            {
                labelDescription.Text = item.Description;
            }

            labelProject.ForeColor = ProjectColorPanel.BackColor = ColorTranslator.FromHtml(Color);
            labelProject.Text = (item.ClientLabel.Length > 0) ? "• " + item.ProjectLabel : item.ProjectLabel;
            labelClient.Text = item.ClientLabel;
            labelTask.Text = item.TaskLabel;
            labelDuration.Text = item.Duration;
            labelBillable.Visible = item.Billable;
            labelTag.Visible = (item.Tags != null && item.Tags.Length > 0);

            if (item.IsHeader)
            {
                labelFormattedDate.Text = item.DateHeader;
                labelDateDuration.Text = item.DateDuration;
                Height = headerPanel.Height + panel.Height;
                panel.Top = headerPanel.Bottom;
                header = true;
            }
            else
            {
                Height = panel.Height;
                panel.Top = 0;
                header = false;
            }
            headerPanel.Visible = item.IsHeader;
            toolTip.SetToolTip(labelDescription, item.Description);
            toolTip.SetToolTip(labelTask, item.ProjectAndTaskLabel);
            toolTip.SetToolTip(labelProject, item.ProjectAndTaskLabel);
            toolTip.SetToolTip(labelClient, item.ProjectAndTaskLabel);
            if (!item.DurOnly)
            {
                toolTip.SetToolTip(labelDuration, item.StartTimeString + " - " + item.EndTimeString);
            }
            if (labelTag.Visible)
            {
                toolTip.SetToolTip(labelTag, item.Tags.Replace(Toggl.TagSeparator, ", "));
            }
        }

        private void checkLabelHeights()
        {
            if (labelDateDuration.Height != headerPanel.Height)
            {
                labelDateDuration.Height = headerPanel.Height;
                labelDateDuration.MaximumSize = new System.Drawing.Size(0, headerPanel.Height);
                labelDateDuration.MinimumSize = new System.Drawing.Size(0, headerPanel.Height);
            }
            if (labelDuration.Height != panel.Height)
            {
                labelDuration.Height = panel.Height;
                labelDuration.MaximumSize = new System.Drawing.Size(0, panel.Height);
                labelDuration.MinimumSize = new System.Drawing.Size(0, panel.Height);
            }
        }

        public int getTopLocation()
        {
            if (header)
            {
                return headerPanel.Height + (panel.Height / 2);
            }
            return Height / 2;
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

        private void labelContinue_Click(object sender, EventArgs e)
        {
            Toggl.Continue(GUID);
        }

        private void edit_Click(object sender, EventArgs e)
        {
            Toggl.Edit(GUID, false, "");
        }

        private void project_Click(object sender, EventArgs e)
        {
            Toggl.Edit(GUID, false, Toggl.Project);
        }

        private void item_MouseMove(object sender, MouseEventArgs e)
        {
            if (list.currentEntry != this)
            {
                if (list.currentEntry != null)
                {
                    if (list.currentEntry.opened)
                    {
                        return;
                    }
                    list.currentEntry.toggleBackground(defaultColor);
                }
                toggleBackground(hoverColor);
                list.currentEntry = this;
            }
        }

        private void toggleBackground(Color color)
        {
            panel.BackColor = color;
            tagBillableContinuePanel.BackColor = color;
            labelDuration.BackColor = color;
        }

        private void tagBillableContinuePanel_MouseUp(object sender, MouseEventArgs e)
        {
            edit_Click(null, null);
        }

        private void labelContinue_MouseEnter(object sender, EventArgs e)
        {
            labelContinue.Image = ((System.Drawing.Image)(Properties.Resources.continue_hover));
        }

        private void labelContinue_MouseLeave(object sender, EventArgs e)
        {
            labelContinue.Image = ((System.Drawing.Image)(Properties.Resources._continue));
        }
    }
}