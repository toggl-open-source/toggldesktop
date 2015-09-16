using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;

namespace TogglDesktop.WPF
{
    public partial class TimeEntryList
    {
        private TimeEntryCell highlightedCell;

        private List<Tuple<string, TimeEntryCell>> cells;
        private int keyboardSelectedId;
        private TimeEntryCell highlightingCell;

        public TimeEntryList()
        {
            this.InitializeComponent();

            this.resizeBackground.Width = SystemParameters.VerticalScrollBarWidth;
            this.resizeBackground.Height = SystemParameters.VerticalScrollBarWidth;
        }

        public UIElementCollection Children
        {
            get { return this.panel.Children; }
        }

        public void FinishedFillingList()
        {
            this.emptyListText.ShowOnlyIf(this.panel.Children.Count == 0);
        }

        private void onEmptyListTextClick(object sender, RoutedEventArgs e)
        {
            Toggl.OpenInBrowser();
        }

        private void onSizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (this.highlightedCell != null)
                this.RefreshHighLight();
        }

        #region editing highlight

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
                if (this.highlightedCell == this.keyboardHighlightCellImposter)
                {
                    this.keyboardHighlightCellImposter.Selected = false;
                }
            }
            this.highlightedCell = cell;
            if (this.highlightedCell != null)
            {
                this.highlightedCell.Selected = true;
                if (this.highlightedCell == this.keyboardHighlightCellImposter)
                {
                    this.keyboardHighlightCellImposter.Selected = true;
                }
            }
        }

        public void DisableHighlight()
        {
            this.HighlightCell(null);
        }

        #endregion

        #region keyboard controls

        public void SetTimeEntryCellList(List<Tuple<string, TimeEntryCell>> cells)
        {
            if (this.keyboardSelectedId == -1 || this.cells == null)
            {
                this.cells = cells;
                return;
            }

            var guid = this.cells[this.keyboardSelectedId].Item1;

            this.cells = cells;

            this.HighlightKeyboard(guid);
        }

        protected override void OnLostKeyboardFocus(KeyboardFocusChangedEventArgs e)
        {
            base.OnLostKeyboardFocus(e);

            if(this.highlightedCell == null)
                this.highlightKeyboard(-1);
        }

        private void onHighlightDown(object sender, ExecutedRoutedEventArgs e)
        {
            this.tryHighlightKeyboard(this.keyboardSelectedId + 1);
        }

        private void onHighlightUp(object sender, ExecutedRoutedEventArgs e)
        {
            this.tryHighlightKeyboard(this.keyboardSelectedId - 1);
        }

        private void onHighlightEdit(object sender, ExecutedRoutedEventArgs e)
        {
            Toggl.Edit(this.cells[this.keyboardSelectedId].Item1, false, "");
        }

        private void onHighlightContinue(object sender, ExecutedRoutedEventArgs e)
        {
        }

        private void onHighlightDelete(object sender, ExecutedRoutedEventArgs e)
        {
        }

        #region updating highlight

        private void tryHighlightKeyboard(int id)
        {
            if (id >= 0 && id < this.cells.Count)
            {
                this.highlightKeyboard(id);
            }
        }

        public void HighlightKeyboard(string guid)
        {
            this.highlightKeyboard(
                this.cells.FindIndex(t => t.Item1 == guid)
                );
        }

        private void highlightKeyboard(int id)
        {
            this.removeKeyboardHighlight();

            if (id < 0 || id >= this.cells.Count)
            {
                this.keyboardSelectedId = -1;
                return;
            }

            this.keyboardSelectedId = id;

            this.setKeyboardHighlight();
        }

        private void removeKeyboardHighlight()
        {
            this.keyboardHighlight.Visibility = Visibility.Collapsed;
        }

        private void setKeyboardHighlight()
        {
            var cell = this.cells[this.keyboardSelectedId].Item2;

            this.highlightingCell = cell;

            this.Dispatcher.BeginInvoke(new Action(() =>
            {
                if(this.highlightingCell != cell || !this.panel.IsAncestorOf(cell))
                {
                    return;
                }

                this.keyboardHighlightCellImposter.Imitate(cell);

                var cellYTop = cell.TransformToAncestor(this.panel).Transform(new Point(0, 0)).Y;

                var y = Math.Max(0, cellYTop + cell.ActualHeight - 53);

                this.keyboardHighlight.Margin = new Thickness(-10, y, -10, 0);
                this.keyboardHighlight.Visibility = Visibility.Visible;

                this.highlightingCell = null;
            }
            ), DispatcherPriority.Render);


        }

        #endregion

        #endregion
    }
}
