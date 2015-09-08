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
            this.InitializeComponent();

            this.LayoutUpdated += this.initResizeBackground;
        }

        private void initResizeBackground(object sender, EventArgs eventArgs)
        {
            var scrollBarWidth = this.ActualWidth - this.panel.ActualWidth;

            this.resizeBackground.Width = scrollBarWidth;
            this.resizeBackground.Height = scrollBarWidth;

            this.LayoutUpdated -= this.initResizeBackground;
        }

        public UIElementCollection Children
        {
            get { return this.panel.Children; }
        }

        public void FinishedFillingList()
        {
            this.emptyListText.Visibility = this.panel.Children.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
        }

        private void onEmptyListTextClick(object sender, RoutedEventArgs e)
        {
            Toggl.OpenInBrowser();
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
