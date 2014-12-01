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
        private SizeF currentFactor;
        private bool scaled = false;
        private TimeEntryListViewController list;
        public bool opened = false;
        private Color hoverColor = Color.WhiteSmoke;
        private Color defaultColor = System.Drawing.Color.FromArgb(((int)(((byte)(250)))), ((int)(((byte)(250)))), ((int)(((byte)(250)))));

        public TimeEntryCell(SizeF factor, TimeEntryListViewController listContainer)
        {
            currentFactor = factor;
            list = listContainer;
            InitializeComponent();   
        }

        protected override void ScaleControl(SizeF factor, BoundsSpecified specified)
        {
            base.ScaleControl(factor, specified);
            if (factor.Height > 1 && currentFactor != factor)
            {
                currentFactor = factor;
                reScale();
            }
            if (currentFactor.Height > 1 && !scaled)
            {
                reScale();
                scaled = true;
            }
        }

        private void reScale()
        {
            scaleChild(labelDescription);
            scaleChild(labelProject);
            scaleChild(labelClient);
            scaleChild(labelTask);
            scaleChild(labelDuration);
            scaleChild(labelFormattedDate);
            scaleChild(labelDateDuration);
        }

        private void scaleChild(Control child)
        {
            float scaledFontSize = (float)(int)(child.Font.Size * currentFactor.Height);
            child.Font = new Font(child.Font.Name, 20, GraphicsUnit.Pixel);
        }

        internal void Display(Toggl.TimeEntry item)
        {
            GUID = item.GUID;

            if (item.Description.Length == 0)
            {
                labelDescription.Text = "(no description)";
            }
            else
            {
                labelDescription.Text = item.Description;
            }
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
            if (scaled)
            {
                checkLabelHeights();
            }
            headerPanel.Visible = item.IsHeader;
            toolTip.SetToolTip(labelDescription, item.Description);
            toolTip.SetToolTip(labelTask, item.ProjectAndTaskLabel);
            toolTip.SetToolTip(labelProject, item.ProjectAndTaskLabel);
            toolTip.SetToolTip(labelClient, item.ProjectAndTaskLabel);
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
                this.labelDateDuration.MaximumSize = new System.Drawing.Size(0, headerPanel.Height);
                this.labelDateDuration.MinimumSize = new System.Drawing.Size(0, headerPanel.Height);
            }
            if (labelDuration.Height != panel.Height)
            {
                labelDuration.Height = panel.Height;
                this.labelDuration.MaximumSize = new System.Drawing.Size(0, panel.Height);
                this.labelDuration.MinimumSize = new System.Drawing.Size(0, panel.Height);
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