using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media.Animation;
using System.Windows.Threading;

namespace TogglDesktop
{
    public partial class TimeEntryList
    {
        public event EventHandler FocusTimer;
        public event EventHandler CloseEditPopup;

        private readonly Storyboard loadMoreSpinnerAnimation;

        private TimeEntryCell highlightedCell;

        private List<Tuple<string, TimeEntryCell>> cells;
        private int keyboardSelectedId;
        private TimeEntryCell cellAboutToKeyboardHighlight;
        private TimeEntryCellDayHeader selectedDay;
        private bool imposterVisible;
        private EditViewPopup editPopup;

        private bool hasKeyboardSelection
        {
            get { return this.keyboardSelectedId != -1 && this.imposterVisible; }
        }

        private string keyboardHighlightedGUID
        {
            get { return this.cells[this.keyboardSelectedId].Item1; }
        }
        private TimeEntryCell keyboardHighlightedCell
        {
            get { return this.cells[this.keyboardSelectedId].Item2; }
        }

        public TimeEntryList()
        {
            this.InitializeComponent();

            this.loadMoreSpinnerAnimation = (Storyboard)this.Resources["RotateLoadMoreSpinner"];

            this.resizeBackground.Width = SystemParameters.VerticalScrollBarWidth;
            this.resizeBackground.Height = SystemParameters.VerticalScrollBarWidth;
        }

        public UIElementCollection Children
        {
            get { return this.panel.Children; }
        }

        public void SetEditPopup(EditViewPopup editPopup)
        {
            this.editPopup = editPopup;
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
                this.refreshHighLight();
        }

        #region editing highlight

        private void refreshHighLight()
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
                this.highlightRectangleBottom.Height = Math.Max(0, this.panel.ActualHeight - y);
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
            this.hideSelection();
        }

        #endregion

        #region keyboard controls

        public void Focus(bool selectKeyboard)
        {
            this.Focus();
            if (selectKeyboard && !this.hasKeyboardSelection)
            {
                this.highlightKeyboard(0);
            }
        }

        public void SetTimeEntryCellList(List<Tuple<string, TimeEntryCell>> cells)
        {
            if (!this.hasKeyboardSelection || this.cells == null)
            {
                this.cells = cells;
                return;
            }

            var guid = this.keyboardHighlightedGUID;

            this.cells = cells;

            this.HighlightKeyboard(guid, true);
        }

        protected override void OnLostKeyboardFocus(KeyboardFocusChangedEventArgs e)
        {
            base.OnLostKeyboardFocus(e);

            if(this.highlightedCell == null)
                this.highlightKeyboard(-1);
        }

        private void onHighlightDown(object sender, ExecutedRoutedEventArgs e)
        {
            if (this.selectedDay != null)
            {
                this.trySelectAfterCurrentDay();
            }
            else
            {
                this.tryHighlightKeyboard(this.keyboardSelectedId + 1);
            }
        }

        private void onHighlightUp(object sender, ExecutedRoutedEventArgs e)
        {
            if (this.selectedDay != null)
            {
                this.trySelectBeforeCurrentDay();
            }
            else
            {
                this.tryHighlightKeyboard(this.keyboardSelectedId - 1);
            }
        }

        private void trySelectBeforeCurrentDay()
        {
            var i = this.cells.FindIndex(t => t.Item2.DayHeader == this.selectedDay);
            this.tryHighlightKeyboard(i - 1);
        }

        private void trySelectAfterCurrentDay()
        {
            var i = this.cells.FindLastIndex(t => t.Item2.DayHeader == this.selectedDay);
            this.tryHighlightKeyboard(i + 1);
        }

        private void onHighlightEdit(object sender, ExecutedRoutedEventArgs e)
        {
            if (this.tryExpandSelectedDay())
                return;

            if (!this.hasKeyboardSelection)
                return;

            Toggl.Edit(this.keyboardHighlightedGUID, false, "");
        }

        private void onExpandDay(object sender, ExecutedRoutedEventArgs e)
        {
            this.tryExpandSelectedDay();
        }

        private void onCollapseDay(object sender, ExecutedRoutedEventArgs e)
        {
            this.tryCollapseCurrentDay();
        }

        private void onExpandAllDays(object sender, ExecutedRoutedEventArgs e)
        {
            this.ExpandAllDays();
        }

        private void onCollapseAllDays(object sender, ExecutedRoutedEventArgs e)
        {
            this.CollapseAllDays();
        }

        private void onHighlightContinue(object sender, ExecutedRoutedEventArgs e)
        {
            if (!this.hasKeyboardSelection)
                return;

            Toggl.Continue(this.keyboardHighlightedGUID);
        }

        private void onHighlightDelete(object sender, ExecutedRoutedEventArgs e)
        {
            if (!this.hasKeyboardSelection)
                return;

            TimeEntryCell item = this.cells[this.keyboardSelectedId].Item2;

            if (item.confirmlessDelete)
            {
                Toggl.DeleteTimeEntry(this.keyboardHighlightedGUID);
                return;
            }

            Toggl.AskToDeleteEntry(this.keyboardHighlightedGUID);
        }

        private void onFocusTimer(object sender, ExecutedRoutedEventArgs e)
        {
            if (this.editPopup.IsVisible)
            {
                if (this.CloseEditPopup != null)
                    this.CloseEditPopup(this, e);
            }
            else
            {
                if (this.FocusTimer != null)
                    this.FocusTimer(this, e);
            }
        }


        private bool tryCollapseCurrentDay()
        {
            if (this.selectedDay != null)
                return false;

            this.keyboardHighlightedCell.DayHeader.Collapse();
            this.refreshKeyboardHighlight();

            return true;
        }

        private bool tryExpandSelectedDay()
        {
            if (this.selectedDay == null)
                return false;

            this.selectedDay.Expand();
            this.tryHighlightKeyboard(this.cells.FindIndex(t => t.Item2.DayHeader == this.selectedDay));

            return true;
        }

        public void CollapseAllDays()
        {
            foreach (var day in this.Children.Cast<TimeEntryCellDayHeader>())
            {
                day.Collapse(true);
            }
            this.refreshKeyboardHighlight();
            Toggl.ViewTimeEntryList();
        }

        public void ExpandAllDays()
        {
            foreach (var day in this.Children.Cast<TimeEntryCellDayHeader>())
            {
                day.Expand(true);
            }
            this.refreshKeyboardHighlight();
            Toggl.ViewTimeEntryList();
        }

        private void refreshKeyboardHighlight()
        {
            this.tryHighlightKeyboard(this.keyboardSelectedId);
        }

        #region updating highlight

        private void tryHighlightKeyboard(int id)
        {
            if (id < 0)
            {
                if(!this.hasKeyboardSelection)
                    this.highlightKeyboard(0);
                return;
            }

            if (id >= this.cells.Count)
            {
                if (!this.hasKeyboardSelection)
                    this.highlightKeyboard(this.cells.Count - 1);
                return;
            }

            this.highlightKeyboard(id);
        }

        public void HighlightKeyboard(string guid, bool async = false)
        {
            this.highlightKeyboard(
                this.cells.FindIndex(t => t.Item1 == guid),
                async
                );
        }

        private void highlightKeyboard(int id, bool async = false)
        {
            if (id < 0 || id >= this.cells.Count)
            {
                this.hideSelection();
                this.keyboardSelectedId = -1;
                return;
            }

            this.keyboardSelectedId = id;

            this.setKeyboardHighlight(async);
        }

        private void hideSelection()
        {
            this.keyboardHighlight.Visibility = Visibility.Collapsed;
            this.imposterVisible = false;
            this.cellAboutToKeyboardHighlight = null;   
        }

        private void setKeyboardHighlight(bool async)
        {
            var cell = this.keyboardHighlightedCell;

            this.cellAboutToKeyboardHighlight = cell;

            if (async)
            {
                this.Dispatcher.BeginInvoke(
                    new Action(this.updateKeyboardHighlightImposter),
                    DispatcherPriority.Background
                    );
            }
            else
            {
                this.updateKeyboardHighlightImposter();

            }
        }

        private void updateKeyboardHighlightImposter()
        {
            var cell = this.cellAboutToKeyboardHighlight;
            this.cellAboutToKeyboardHighlight = null;

            if (cell == null || !this.panel.IsAncestorOf(cell))
            {
                return;
            }

            if (cell.DayHeader.IsCollapsed)
            {
                this.selectDay(cell.DayHeader);
                return;
            }
            this.selectDay(null);

            this.keyboardHighlightCellImposter.Imitate(cell);

            var cellYTop = cell.TransformToAncestor(this.panel).Transform(new Point(0, 0)).Y;

            var y = Math.Max(0, cellYTop + cell.ActualHeight - 53 - 3);

            this.keyboardHighlight.Margin = new Thickness(-10, y, -10, 0);
            this.keyboardHighlight.Visibility = Visibility.Visible;
            this.imposterVisible = true;

            cell.BringIntoView();
        }

        private void selectDay(TimeEntryCellDayHeader dayHeader)
        {
            if (this.selectedDay != null)
            {
                this.selectedDay.IsSelected = false;
            }

            this.selectedDay = dayHeader;

            if (dayHeader == null)
            {
                return;
            }


            dayHeader.IsSelected = true;
            dayHeader.BringIntoView();
            this.hideSelection();
        }

        #endregion

        #endregion

        #region load more
        
        private void onLoadMoreButtonClick(object sender, RoutedEventArgs e)
        {
            this.loadMore();
        }

        private void loadMore()
        {
            this.loadMoreButton.Visibility = Visibility.Visible;
            this.loadMoreButton.IsEnabled = false;
            this.loadMoreButtonText.Visibility = Visibility.Collapsed;
            this.loadMoreSpinnerAnimation.Begin();
            this.loadMoreSpinner.Visibility = Visibility.Visible;

            Toggl.LoadMore();
        }

        private void showLoadMoreButton()
        {
            this.loadMoreButton.Visibility = Visibility.Visible;
            this.loadMoreButtonText.Visibility = Visibility.Visible;
            this.loadMoreSpinner.Visibility = Visibility.Collapsed;
            if (!this.loadMoreButton.IsEnabled)
            {
                this.loadMoreSpinnerAnimation.Stop();
            }
            this.loadMoreButton.IsEnabled = true;
        }

        private void hideLoadMoreButton()
        {
            this.loadMoreButton.Visibility = Visibility.Collapsed;
            if (!this.loadMoreButton.IsEnabled)
            {
                this.loadMoreSpinnerAnimation.Stop();
            }
        }

        #endregion

        public void SetLoadMoreButtonVisibility(bool showLoadMoreButton)
        {
            if (showLoadMoreButton)
            {
                this.showLoadMoreButton();
            }
            else
            {
                this.hideLoadMoreButton();
            }
        }
    }
}
