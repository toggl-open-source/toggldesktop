using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media.Animation;
using System.Windows.Threading;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public partial class TimeEntryList
    {
        public event EventHandler FocusTimer;
        public event EventHandler CloseEditPopup;

        private readonly Storyboard loadMoreSpinnerAnimation;

        // private TimeEntryCell highlightedCell;

        // private TimeEntryCell focusedCell;
        // private TimeEntryCell selectedCell;

        // private List<Tuple<string, TimeEntryCell>> cells;
        // private int keyboardSelectedId;
        // private TimeEntryCell cellAboutToKeyboardHighlight;
        // private TimeEntryCellDayHeader selectedDay;
        // private bool imposterVisible;
        private EditViewPopup editPopup;

        private TimeEntryCellViewModel _selectedCell;

        // private bool hasKeyboardSelection
        // {
        //     get { return this.keyboardSelectedId != -1; }
        // }

        private bool IsAnyCellFocused => _focusedCellIndex >= 0;

        private TimeEntryCellViewModel GetFocusedCell()
        {
            Debug.Assert(IsAnyCellFocused);
            return GetCell((_focusedDayIndex, _focusedCellIndex));
        }

        // private string keyboardHighlightedGUID
        // {
        //     get { return this.cells[this.keyboardSelectedId].Item1; }
        // }
        // private TimeEntryCell keyboardHighlightedCell
        // {
        //     get { return this.cells[this.keyboardSelectedId].Item2; }
        // }

        // private TimeEntryCell cellAt(int index) => this.cells[index].Item2;

        public TimeEntryList()
        {
            this.InitializeComponent();

            this.loadMoreSpinnerAnimation = (Storyboard)this.Resources["RotateLoadMoreSpinner"];
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
            this.emptyListText.ShowOnlyIf(this.panel.Children.Count == 0 && !loadMoreButton.IsVisible);
        }

        private void onEmptyListTextClick(object sender, RoutedEventArgs e)
        {
            Toggl.OpenInBrowser();
        }

        public int EntriesCount => _cellsDictionary2.Count;

        #region editing highlight

        // private void refreshHighLight()
        // {
        //     this.HighlightCell(this.highlightedCell);
        // }

        public void DeselectCells() => SelectCell(null);
        // public void SelectCell(TimeEntryCell cell)
        // {
        //     if (cell != null && !this.panel.IsAncestorOf(cell))
        //     {
        //         // the time entry list hasnt rendered yet
        //         cell = null;
        //     }
        //
        //     if (this.selectedCell != cell)
        //     {
        //         if (this.selectedCell != null)
        //         {
        //             this.selectedCell.ViewModel.IsSelected = false;
        //         }
        //
        //         this.selectedCell = cell;
        //         if (this.selectedCell != null)
        //         {
        //             this.selectedCell.ViewModel.IsSelected = true;
        //         }
        //     }
        // }

        public void SelectCell(TimeEntryCellViewModel cell)
        {
            if (_selectedCell != null)
            {
                _selectedCell.IsSelected = false;
            }

            _selectedCell = cell;
            if (_selectedCell != null)
            {
                _selectedCell.IsSelected = true;
            }
        }

        public void SelectEntry(string guid)
        {
            if (guid == null)
                SelectCell(null);
            if (!_cellsDictionary2.TryGetValue(guid, out var cellPosition))
                SelectCell(null);

            var cell = GetCell(cellPosition);
            SelectCell(cell);
        }

        private TimeEntryCellViewModel GetCell((int dayIndex, int cellIndex) cellPosition) =>
            _dayHeaderViewModels[cellPosition.dayIndex].GetCell(cellPosition.cellIndex);

        // public void HighlightCell(TimeEntryCell cell)
        // {
        //     if (cell != null && !this.panel.IsAncestorOf(cell))
        //     {
        //         // the time entry list hasnt rendered yet
        //         cell = null;
        //     }
        //
        //     if (this.highlightedCell != cell)
        //     {
        //         if (this.highlightedCell != null)
        //         {
        //             this.highlightedCell.ViewModel.IsFocused = false;
        //         }
        //         this.highlightedCell = cell;
        //         if (this.highlightedCell != null)
        //         {
        //             this.highlightedCell.ViewModel.IsFocused = true;
        //         }
        //     }
        // }

        // public void DisableHighlight()
        // {
        //     this.HighlightCell(null);
        //     this.cellAboutToKeyboardHighlight = null;
        // }

        #endregion

        #region keyboard controls

        public void Focus(bool selectKeyboard)
        {
            this.Focus();
            if (selectKeyboard && _focusedDayIndex < 0)
            {
                FocusFirst();
            }
            // if (selectKeyboard && !this.hasKeyboardSelection)
            // {
            //     this.highlightKeyboard(0);
            //     if (hasKeyboardSelection)
            //         HighlightCell(this.keyboardHighlightedCell);
            // }
        }

        // public void SetTimeEntryCellList(List<Tuple<string, TimeEntryCell>> cells)
        // {
        //     if (!this.hasKeyboardSelection || this.cells == null)
        //     {
        //         this.cells = cells;
        //         return;
        //     }
        //
        //     var guid = this.keyboardHighlightedGUID;
        //
        //     this.cells = cells;
        //
        //     this.HighlightKeyboard(guid, true);
        // }

        private int _focusedDayIndex = -1;
        private int _focusedCellIndex = -1;
        private DayHeaderViewModel[] _dayHeaderViewModels = new DayHeaderViewModel[0];

        private Dictionary<string, (int dayIndex, int cellIndex)> _cellsDictionary2 = new Dictionary<string, (int dayIndex, int cellIndex)>();

        private Dictionary<string, int> _daysDictionary2 = new Dictionary<string, int>();

        // private (string focusedEntryGuid, string focusedDayHeader) RemoveFocus()
        // {
        //     string focusedEntryGuid = null;
        //     string focusedDayHeader = null;
        //     if (_focusedDayIndex >= 0)
        //     {
        //         var focusedDay = _dayHeaderViewModels[_focusedDayIndex];
        //         if (_focusedCellIndex >= 0)
        //         {
        //             var focusedEntry = focusedDay.CellsBindable[_focusedCellIndex];
        //             focusedEntry.IsFocused = false;
        //             focusedEntryGuid = focusedEntry.Guid;
        //         }
        //         else
        //         {
        //             focusedDay.IsFocused = false;
        //             focusedDayHeader = focusedDay.DateHeader;
        //         }
        //     }
        //
        //     return (focusedEntryGuid, focusedDayHeader);
        // }
        private (string focusedEntryGuid, string focusedDayHeader) GetCurrentFocusedItemGuid()
        {
            string focusedEntryGuid = null;
            string focusedDayHeader = null;
            if (_focusedDayIndex >= 0)
            {
                var focusedDay = _dayHeaderViewModels[_focusedDayIndex];
                if (_focusedCellIndex >= 0)
                {
                    var focusedEntry = focusedDay.GetCell(_focusedCellIndex);
                    focusedEntryGuid = focusedEntry.Guid;
                }
                else
                {
                    focusedDayHeader = focusedDay.DateHeader;
                }
            }

            return (focusedEntryGuid, focusedDayHeader);
        }

        public void RemoveFocus()
        {
            _focusedCellIndex = -1;
            _focusedDayIndex = -1;
        }

        public void SetDayHeaderViewModels(DayHeaderViewModel[] dayHeaderViewModels)
        {
            // remove focus from old day header or cell
            var (focusedEntryGuid, focusedDayHeader) = GetCurrentFocusedItemGuid();

            _dayHeaderViewModels = dayHeaderViewModels;
            _cellsDictionary2 = dayHeaderViewModels
                .WithIndex()
                .SelectMany(tuple => tuple.item.CellsMutable.Items
                    .Select((cell, cellIndex) => (cell.Guid, dayIndex: tuple.index, cellIndex)))
                .ToDictionary(tuple => tuple.Guid, tuple => (tuple.dayIndex, tuple.cellIndex));
            _daysDictionary2 = dayHeaderViewModels
                .WithIndex()
                .ToDictionary(tuple => tuple.item.DateHeader, tuple => tuple.index);

            // set the focus back
            if (focusedEntryGuid != null && _cellsDictionary2.TryGetValue(focusedEntryGuid, out var focusedCellPosition))
            {
                FocusCell(focusedCellPosition.dayIndex, focusedCellPosition.cellIndex);
            }
            else if (focusedDayHeader != null && _daysDictionary2.TryGetValue(focusedDayHeader, out var focusedDayIndex))
            {
                FocusDay(focusedDayIndex);
            }
            else
            {
                RemoveFocus();
            }
        }

        private void FocusFirst()
        {
            if (_dayHeaderViewModels.Any())
            {
                FocusDayOrItsFirstCell(0);
            }
        }

        private void FocusDayOrItsFirstCell(int dayIndex)
        {
            Debug.Assert(dayIndex >= 0);
            Debug.Assert(dayIndex < _dayHeaderViewModels.Length);
            var dayViewModel = _dayHeaderViewModels[dayIndex];
            if (dayViewModel.IsExpanded)
            {
                FocusCell(dayIndex, 0);
            }
            else
            {
                FocusDay(dayIndex);
            }
        }

        private void FocusDayOrItsLastCell(int dayIndex)
        {
            Debug.Assert(dayIndex >= 0);
            Debug.Assert(dayIndex < _dayHeaderViewModels.Length);
            var dayViewModel = _dayHeaderViewModels[dayIndex];
            if (dayViewModel.IsExpanded)
            {
                FocusCell(dayIndex, dayViewModel.CellsCount - 1);
            }
            else
            {
                FocusDay(dayIndex);
            }
        }

        private void FocusDay(int dayIndex)
        {
            if (_focusedCellIndex < 0 && _focusedDayIndex == dayIndex)
            {
                return;
            }
            Debug.Assert(dayIndex >= 0);
            Debug.Assert(dayIndex < _dayHeaderViewModels.Length);
            var dayViewModel = _dayHeaderViewModels[dayIndex];
            Debug.Assert(dayViewModel.IsCollapsed);

            _focusedDayIndex = dayIndex;
            _focusedCellIndex = -1;
            dayViewModel.Focus();
        }

        private void FocusCell(int dayIndex, int cellIndex)
        {
            if (_focusedCellIndex == cellIndex && _focusedDayIndex == dayIndex)
            {
                return;
            }

            Debug.Assert(dayIndex >= 0);
            Debug.Assert(dayIndex < _dayHeaderViewModels.Length);
            Debug.Assert(cellIndex >= 0);
            var dayViewModel = _dayHeaderViewModels[dayIndex];
            Debug.Assert(dayViewModel.IsExpanded);
            if (cellIndex >= dayViewModel.CellsCount)
            {
                Debug.Assert(cellIndex < dayViewModel.CellsCount);
            }
            var cellViewModel = dayViewModel.GetCell(cellIndex);

            _focusedDayIndex = dayIndex;
            _focusedCellIndex = cellIndex;
            cellViewModel.Focus();
        }

        // protected override void OnLostKeyboardFocus(KeyboardFocusChangedEventArgs e)
        // {
        //     base.OnLostKeyboardFocus(e);
        //
        //     if(this.highlightedCell == null)
        //         this.highlightKeyboard(-1);
        // }

        private void onHighlightDown(object sender, ExecutedRoutedEventArgs e)
        {
            if (_focusedDayIndex < 0)
            {
                FocusFirst();
            }
            else
            {
                if (_focusedCellIndex >= 0 && _focusedCellIndex + 1 < _dayHeaderViewModels[_focusedDayIndex].CellsCount)
                {
                    FocusCell(_focusedDayIndex, _focusedCellIndex + 1);
                }
                else if (_focusedDayIndex + 1 < _dayHeaderViewModels.Length)
                {
                    FocusDayOrItsFirstCell(_focusedDayIndex + 1);
                }
            }
            // if (this.selectedDay != null)
            // {
            //     this.trySelectAfterCurrentDay();
            // }
            // else
            // {
            //     this.tryHighlightKeyboard(this.keyboardSelectedId + 1);
            // }
        }

        private void onHighlightUp(object sender, ExecutedRoutedEventArgs e)
        {
            if (_focusedDayIndex < 0)
            {
                FocusFirst();
            }
            else
            {
                if (_focusedCellIndex >= 1)
                {
                    FocusCell(_focusedDayIndex, _focusedCellIndex - 1);
                }
                else if (_focusedDayIndex >= 1)
                {
                    FocusDayOrItsLastCell(_focusedDayIndex - 1);
                }
            }
            // if (this.selectedDay != null)
            // {
            //     this.trySelectBeforeCurrentDay();
            // }
            // else
            // {
            //     this.tryHighlightKeyboard(this.keyboardSelectedId - 1);
            // }
        }

        // private void trySelectBeforeCurrentDay()
        // {
        //     var i = this.cells.FindIndex(t => t.Item2.DayHeader == this.selectedDay);
        //     this.tryHighlightKeyboard(i - 1);
        // }
        //
        // private void trySelectAfterCurrentDay()
        // {
        //     var i = this.cells.FindLastIndex(t => t.Item2.DayHeader == this.selectedDay);
        //     this.tryHighlightKeyboard(i + 1);
        // }

        private bool TryExpandFocusedDay()
        {
            if (_focusedCellIndex >= 0) return false; // cell is selected
            if (_focusedDayIndex < 0) return false; // nothing is selected
            var dayToExpand = _dayHeaderViewModels[_focusedDayIndex];
            dayToExpand.Expand();
            FocusCell(_focusedDayIndex, 0);
            return true;
        }

        private void onHighlightEdit(object sender, ExecutedRoutedEventArgs e)
        {
            if (TryExpandFocusedDay())
                return;

            if (!IsAnyCellFocused)
                return;

            var guidOfCellToEdit = GetFocusedCell().Guid;

            Toggl.Edit(guidOfCellToEdit, false, "");

            // if (this.tryExpandSelectedDay())
            //     return;
            //
            // if (!this.hasKeyboardSelection)
            //     return;

            // Toggl.Edit(this.keyboardHighlightedGUID, false, "");
        }

        private void onExpandSelectedItem(object sender, ExecutedRoutedEventArgs e)
        {
            if (!TryExpandFocusedGroup())
            {
                TryExpandFocusedDay();
            }
            // var expandedSelectedGroup = this.tryExpandSelectedGroup();
            // if (!expandedSelectedGroup)
            // {
            //     this.tryExpandSelectedDay();
            // }
        }

        private void onCollapseSelectedItem(object sender, ExecutedRoutedEventArgs e)
        {
            if (!TryCollapseFocusedGroup())
            {
                TryCollapseFocusedDay();
            }
            // var collapsedSelectedGroup = this.tryCollapseSelectedGroup();
            // if (!collapsedSelectedGroup)
            // {
            //     this.tryCollapseCurrentDay();
            // }
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
            if (IsAnyCellFocused)
            {
                Toggl.Continue(GetFocusedCell().Guid);
            }
            // if (!this.hasKeyboardSelection)
            //     return;
            //
            // Toggl.Continue(this.keyboardHighlightedGUID);
        }

        private void onHighlightDelete(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsAnyCellFocused)
            {
                var cell = GetFocusedCell();
                cell.DeleteTimeEntry();
            }

            // if (!this.hasKeyboardSelection)
            //     return;
            //
            // TimeEntryCell item = this.cells[this.keyboardSelectedId].Item2;
            //
            // if (item.confirmlessDelete())
            // {
            //     Toggl.DeleteTimeEntry(this.keyboardHighlightedGUID);
            //     return;
            // }
            //
            // Toggl.AskToDeleteEntry(this.keyboardHighlightedGUID);
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

        // private bool tryCollapseSelectedGroup()
        // {
        //     if (!this.hasKeyboardSelection)
        //     {
        //         return false;
        //     }
        //
        //     var idToHighlight = keyboardSelectedId;
        //     while (idToHighlight >= 0 && !cellAt(idToHighlight).IsGroup) // highlight the group that is being collapsed
        //     {
        //         idToHighlight--;
        //     }
        //
        //     if (!this.keyboardHighlightedCell.ViewModel.TryCollapse())
        //     {
        //         return false;
        //     }
        //
        //     this.tryHighlightKeyboard(idToHighlight);
        //     return true;
        // }

        private bool TryExpandFocusedGroup()
        {
            return IsAnyCellFocused && GetFocusedCell().TryExpand();
        }

        private bool TryCollapseFocusedGroup()
        {
            if (!IsAnyCellFocused)
            {
                return false;
            }

            var dayIndex = _focusedDayIndex;
            var focusedDay = _dayHeaderViewModels[_focusedDayIndex];
            var cellIndex = _focusedCellIndex;
            var focusedCell = focusedDay.GetCell(cellIndex);
            if (!focusedCell.IsGroup && !focusedCell.IsSubItem)
            {
                return false;
            }

            while (!focusedDay.GetCell(cellIndex).IsGroup)
            {
                cellIndex--;
            }

            var groupToCollapse = focusedDay.GetCell(cellIndex);
            if (!groupToCollapse.TryCollapse())
            {
                return false;
            }

            FocusCell(dayIndex, cellIndex);
            return true;
        }

        // private bool tryCollapseCurrentDay()
        // {
        //     if (this.selectedDay != null)
        //         return false;
        //
        //     if (this.keyboardSelectedId >= 0 && this.keyboardSelectedId < cells.Count)
        //     {
        //         this.keyboardHighlightedCell.DayHeader.Collapse();
        //     }
        //
        //     this.refreshKeyboardHighlight();
        //
        //     return true;
        // }

        private bool TryCollapseFocusedDay()
        {
            if (_focusedDayIndex < 0)
                return false;
            _dayHeaderViewModels[_focusedDayIndex].Collapse();
            FocusDay(_focusedDayIndex);
            return true;
        }

        // private bool tryExpandSelectedGroup()
        // {
        //     var expanded = this.hasKeyboardSelection && this.keyboardHighlightedCell.ViewModel.TryExpand();
        //     if (expanded)
        //     {
        //         this.refreshKeyboardHighlight();
        //     }
        //
        //     return expanded;
        // }
        //
        // private bool tryExpandSelectedDay()
        // {
        //     if (this.selectedDay == null)
        //         return false;
        //
        //     this.selectedDay.Expand();
        //     this.tryHighlightKeyboard(this.cells.FindIndex(t => t.Item2.DayHeader == this.selectedDay));
        //
        //     return true;
        // }

        public void CollapseAllDays()
        {
            _dayHeaderViewModels.ForEach(day => day.Collapse());
            if (_focusedDayIndex >= 0)
            {
                FocusDay(_focusedDayIndex);
            }
            // foreach (var day in this.Children.Cast<TimeEntryCellDayHeader>())
            // {
            //     day.Collapse();
            // }
            // this.refreshKeyboardHighlight();
            Toggl.ViewTimeEntryList();
        }

        public void ExpandAllDays()
        {
            _dayHeaderViewModels.ForEach(day => day.Expand());
            if (_focusedDayIndex >= 0)
            {
                FocusCell(_focusedDayIndex, 0);
            }
            // foreach (var day in this.Children.Cast<TimeEntryCellDayHeader>())
            // {
            //     day.Expand();
            // }
            // this.refreshKeyboardHighlight();
            Toggl.ViewTimeEntryList();
        }

        // private void refreshKeyboardHighlight()
        // {
        //     this.tryHighlightKeyboard(this.keyboardSelectedId);
        // }

        #region updating highlight

        // private void tryHighlightKeyboard(int id)
        // {
        //     if (id < 0)
        //     {
        //         if(!this.hasKeyboardSelection)
        //             this.highlightKeyboard(0);
        //         return;
        //     }
        //
        //     if (id >= this.cells.Count)
        //     {
        //         if (!this.hasKeyboardSelection)
        //             this.highlightKeyboard(this.cells.Count - 1);
        //         return;
        //     }
        //
        //     this.highlightKeyboard(id);
        // }

        // public void HighlightKeyboard(string guid, bool async = false)
        // {
        //     this.highlightKeyboard(
        //         this.cells.FindIndex(t => t.Item1 == guid),
        //         async
        //         );
        // }

        // private void highlightKeyboard(int id, bool async = false)
        // {
        //     if (id < 0 || id >= this.cells.Count)
        //     {
        //         this.cellAboutToKeyboardHighlight = null;
        //         this.keyboardSelectedId = -1;
        //         return;
        //     }
        //
        //     this.keyboardSelectedId = id;
        //
        //     this.setKeyboardHighlight(async);
        // }

        // private void setKeyboardHighlight(bool async)
        // {
        //     var cell = this.keyboardHighlightedCell;
        //     this.cellAboutToKeyboardHighlight = cell ?? throw new InvalidOperationException();
        //
        //     if (async)
        //     {
        //         this.Dispatcher.BeginInvoke(
        //             new Action(this.updateKeyboardHighlightImposter),
        //             DispatcherPriority.Background
        //             );
        //     }
        //     else
        //     {
        //         this.updateKeyboardHighlightImposter();
        //
        //     }
        // }

        // private void updateKeyboardHighlightImposter()
        // {
        //     var cell = this.cellAboutToKeyboardHighlight;
        //     this.cellAboutToKeyboardHighlight = null;
        //
        //     if (cell == null || !this.panel.IsAncestorOf(cell))
        //     {
        //         return;
        //     }
        //
        //     if (cell.DayHeader.IsCollapsed)
        //     {
        //         this.selectDay(cell.DayHeader);
        //         return;
        //     }
        //     this.selectDay(null);
        //     cell.BringIntoView();
        // }

        // private void selectDay(TimeEntryCellDayHeader dayHeader)
        // {
        //     if (this.selectedDay != null)
        //     {
        //         this.selectedDay.IsSelected = false;
        //     }
        //
        //     this.selectedDay = dayHeader;
        //
        //     if (dayHeader == null)
        //     {
        //         return;
        //     }
        //
        //     dayHeader.IsSelected = true;
        //     dayHeader.BringIntoView();
        //     this.cellAboutToKeyboardHighlight = null;
        // }

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

        #endregion

        private void UIElement_OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (!e.Handled)
            {
                e.Handled = true;
                var args = new KeyEventArgs(e.KeyboardDevice, e.InputSource, e.Timestamp, e.Key);
                args.RoutedEvent = KeyDownEvent;
                args.Source = sender;
                this.RaiseEvent(args);
            }
        }
    }
}
