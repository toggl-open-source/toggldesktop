using System;
using System.Windows;
using System.Windows.Controls;

namespace TogglDesktop.WPF
{
    public partial class TimeEntryList
    {
        private TimeEntryCell highlightedCell;

        public TimeEntryList()
        {
            InitializeComponent();
        }

        public UIElementCollection Children
        {
            get { return panel.Children; }
        }


        private void onSizeChanged(object sender, SizeChangedEventArgs e)
        {
            if(this.highlightedCell != null)
                this.RefreshHighLight();
        }

        public void RefreshHighLight()
        {
            this.HighlightCell(this.highlightedCell);
        }

        public void HighlightCell(TimeEntryCell cell)
        {
            if (cell != null && this.panel.IsAncestorOf(cell))
            {
                // y will be 0 if the time entry list hasnt rendered yet
                var y = cell.TransformToAncestor(this.panel).Transform(new Point(0, 0)).Y + cell.ActualHeight;
                this.highlightRectangleTop.Height = Math.Max(0, y - 53);
                this.highlightRectangleBottom.Height = this.panel.ActualHeight - y;
                this.highlightRectangleTop.Visibility = Visibility.Visible;
                this.highlightRectangleBottom.Visibility = Visibility.Visible;
            }
            else
            {
                this.highlightRectangleTop.Height = 0;
                this.highlightRectangleBottom.Height = 0;
                this.highlightRectangleTop.Visibility = Visibility.Collapsed;
                this.highlightRectangleBottom.Visibility = Visibility.Collapsed;
                cell = null;
            }

            if (this.highlightedCell != null)
            {
                this.highlightedCell.Selected = false;
            }
            this.highlightedCell = cell;
            if (this.highlightedCell != null)
            {
                this.highlightedCell.Selected = true;
            }
        }

        public void DisableHighlight()
        {
            this.HighlightCell(null);
        }
    }
}
