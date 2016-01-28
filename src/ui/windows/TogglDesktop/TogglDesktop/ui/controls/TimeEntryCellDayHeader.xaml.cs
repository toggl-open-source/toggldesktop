using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Media;
using System.Windows.Shapes;

namespace TogglDesktop
{
    public partial class TimeEntryCellDayHeader
    {
        private bool isSelected;
        private DateTime date;

        public bool IsDummy
        {
            set
            {
                if (value == false)
                    return;

                this.IsEnabled = false;
            }
        }

        public bool IsCollapsed
        {
            get { return this.panel.Visibility == Visibility.Collapsed; }
            private set { this.panel.Visibility = value ? Visibility.Collapsed : Visibility.Visible; }
        }
       
        public bool IsSelected
        {
            get { return this.isSelected; }
            set
            {
                if (this.isSelected == value)
                    return;
                this.isSelected = value;

                this.updateBackground();
            }
        }

        public TimeEntryCellDayHeader()
        {
            this.InitializeComponent();
        }

        public void DisplayDummy(string dateText, string durationText = "")
        {
            this.labelFormattedDate.Text = dateText;
            this.labelDateDuration.Text = durationText;
        }

        public void Display(List<Toggl.TogglTimeEntryView> items, Action<string, TimeEntryCell> registerCellByGUID)
        {
            var item = items[0];

            if (!item.IsHeader)
            {
                throw new InvalidDataException("Can only create day header from header time entry view.");
            }

            this.date = Toggl.DateTimeFromUnix(item.Started);

            if (this.labelFormattedDate.Text != item.DateHeader)
            {
                this.panel.Visibility = Visibility.Visible;
            }

            this.labelFormattedDate.Text = item.DateHeader;
            this.labelDateDuration.Text = item.DateDuration;

            this.fillTimeline(items);

            this.fillCells(items, registerCellByGUID);
        }

        private void fillTimeline(List<Toggl.TogglTimeEntryView> items)
        {
            const double rowHeight = 3;
            const double rowMargin = 4;

            this.timelineStartTime.Text = items.Last().StartTimeString;
            this.timelineEndTime.Text = items.First().EndTimeString;

            var startTime = items.Last().Started;
            var endTime = items.First().Ended;

            var rows = new List<ulong>();

            this.timelinePanel.Children.Clear();
            for (int i = items.Count; i-- > 0;)
            {
                var item = items[i];

                var row = this.insertIntoTimeLineRow(rows, item.Started, item.Ended);

                var rect = new Rectangle
                {
                    Fill = Utils.ProjectColorBrushFromString(item.Color),
                    Margin = new Thickness(
                        item.Started - startTime, 0,
                        endTime - item.Ended, row * (rowHeight + rowMargin)
                        ),
                    Height = rowHeight,
                    Width = item.DurationInSeconds,
                    ToolTip = getTimeLineItemTooltip(item),
                };

                this.timelinePanel.Children.Add(rect);
            }

            this.timelineViewbox.Height = rows.Count * (rowHeight + rowMargin) - rowMargin;
        }

        private int insertIntoTimeLineRow(List<ulong> rows, ulong started, ulong ended)
        {
            for (int i = 0; i < rows.Count; i++)
            {
                if (rows[i] < started + 1)
                {
                    rows[i] = ended;
                    return i;
                }
            }

            rows.Add(ended);
            return rows.Count - 1;
        }

        private static string getTimeLineItemTooltip(Toggl.TogglTimeEntryView item)
        {
            var hasDescription = !item.Description.IsNullOrEmpty();
            var hasProject = !item.ProjectAndTaskLabel.IsNullOrEmpty();

            if (hasDescription)
            {
                if (hasProject)
                {
                    return item.Description + " - " + item.ProjectAndTaskLabel;
                }

                return item.Description;
            }

            if (hasProject)
            {
                return "(no description) - " + item.ProjectAndTaskLabel;
            }

            return null;
        }

        private void fillCells(List<Toggl.TogglTimeEntryView> list, Action<string, TimeEntryCell> registerCellByGUID)
        {
            var children = this.panel.Children;

            // remove superfluous cells
            if (children.Count > list.Count)
            {
                children.RemoveRange(list.Count, children.Count - list.Count);
            }

            // update existing cells
            var i = 0;
            for (; i < children.Count; i++)
            {
                var entry = list[i];

                var cell = (TimeEntryCell)children[i];
                cell.Display(entry, this);

                registerCellByGUID(entry.GUID, cell);
            }

            // add additional cells
            for (; i < list.Count; i++)
            {
                var entry = list[i];

                var cell = new TimeEntryCell();
                cell.Display(entry, this);

                registerCellByGUID(entry.GUID, cell);

                children.Add(cell);
            }
        }

        private void onHeaderClick(object sender, RoutedEventArgs e)
        {
            this.IsCollapsed = !this.IsCollapsed;
            Toggl.ViewTimeEntryList();
        }

        public void Expand(bool supressTimeEntryListEvent = false)
        {
            if (!this.IsCollapsed)
                return;

            this.IsCollapsed = false;

            if(!supressTimeEntryListEvent)
                Toggl.ViewTimeEntryList();
        }
        public void Collapse(bool supressTimeEntryListEvent = false)
        {
            if (this.IsCollapsed)
                return;

            this.IsCollapsed = true;

            if (!supressTimeEntryListEvent)
                Toggl.ViewTimeEntryList();
        }

        private void updateBackground()
        {
            this.Background = new SolidColorBrush(
                this.isSelected ? Color.FromRgb(200, 200, 200) : Color.FromRgb(247, 247, 247)
                );
        }


        protected override void OnDrop(DragEventArgs e)
        {
            if (e.Data.GetDataPresent("time-entry-cell"))
            {
                var cell = (TimeEntryCell)e.Data.GetData("time-entry-cell");
                cell.MoveToDay(this.date);
            }
        }
    }
}
