using System;
using System.Collections.Generic;
using System.IO;
using System.Windows;

namespace TogglDesktop
{
    public partial class TimeEntryCellDayHeader
    {
        public TimeEntryCellDayHeader()
        {
            this.InitializeComponent();
        }

        public void Display(List<Toggl.TogglTimeEntryView> items, Action<string, TimeEntryCell> registerCellByGUID)
        {
            var item = items[0];

            if (!item.IsHeader)
            {
                throw new InvalidDataException("Can only create day header from header time entry view.");
            }

            this.labelFormattedDate.Text = item.DateHeader;
            this.labelDateDuration.Text = item.DateDuration;

            this.fillCells(items, registerCellByGUID);
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
                cell.Display(entry);

                registerCellByGUID(entry.GUID, cell);
            }

            // add additional cells
            for (; i < list.Count; i++)
            {
                var entry = list[i];

                var cell = new TimeEntryCell();
                cell.Display(entry);

                registerCellByGUID(entry.GUID, cell);

                children.Add(cell);
            }
        }

        private void onHeaderClick(object sender, RoutedEventArgs e)
        {
            if (this.panel.Visibility == Visibility.Visible)
            {
                this.panel.Visibility = Visibility.Collapsed;
            }
            else
            {
                this.panel.Visibility = Visibility.Visible;
            }
        }
    }
}
