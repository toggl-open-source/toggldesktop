using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reactive.Disposables;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media.Animation;
using ReactiveUI;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public partial class TimeEntryList
    {
        public event EventHandler FocusTimer;
        public event EventHandler CloseEditPopup;

        private readonly Storyboard loadMoreSpinnerAnimation;

        private EditViewPopup editPopup;
        private TimeEntryCellViewModel _selectedCell;

        private bool IsAnyCellFocused => FocusedCellIndex >= 0;

        private TimeEntryCellViewModel GetFocusedCell()
        {
            Debug.Assert(IsAnyCellFocused);
            return GetCell((FocusedDayIndex, FocusedCellIndex));
        }

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

        public int EntriesCount => _cellsDictionary.Count;

        public void DeselectCells() => SelectCell(null);

        private void SelectCell(TimeEntryCellViewModel cell)
        {
            if (_selectedCell != null)
            {
                _selectedCell.IsSelected = false;
            }

            _selectedCell = cell;

            if (_selectedCell != null)
            {
                _selectedCell.IsSelected = true;
                if (_cellsDictionary.TryGetValue(cell.Id, out var focusedCellPosition))
                {
                    FocusCell(focusedCellPosition.dayIndex, focusedCellPosition.cellIndex);
                }
            }
        }

        public void SelectEntry(string guid)
        {
            if (guid == null)
            {
                SelectCell(null);
            }
            else if (!_cellsDictionary.TryGetValue(guid, out var cellPosition))
            {
                SelectCell(null);
            }
            else
            {
                var cell = GetCell(cellPosition);
                SelectCell(cell);
            }
        }

        private TimeEntryCellViewModel GetCell((int dayIndex, int cellIndex) cellPosition) =>
            _days[cellPosition.dayIndex].GetCell(cellPosition.cellIndex);

        public void Focus(bool selectKeyboard)
        {
            this.Focus();
            if (selectKeyboard)
            {
                if (FocusedDayIndex < 0)
                {
                    FocusFirst();
                }
                else
                {
                    RefreshFocus();
                }
            }
        }

        private void RefreshFocus()
        {
            if (FocusedDayIndex >= 0)
            {
                var dayToFocus = _days[FocusedDayIndex];
                if (FocusedCellIndex >= 0)
                {
                    GetFocusedCell().Focus();
                }
                else
                {
                    dayToFocus.Focus();
                }
            }
        }

        private void RefreshCellFocusIfListContainsFocus()
        {
            if (IsKeyboardFocusWithin)
            {
                GetFocusedCell().Focus();
            }
        }

        private void RefreshDayFocusIfListContainsFocus()
        {
            if (IsKeyboardFocusWithin)
            {
                _days[FocusedDayIndex].Focus();
            }
        }

        private int _focusedDayIndex = -1;
        public int FocusedDayIndex
        {
            get => _focusedDayIndex;
            private set
            {
                if (_focusedDayIndex != value)
                {
                    _updateFocusOnDayExpandCollapse.Dispose();
                    _focusedDayIndex = value;
                    _updateFocusOnDayExpandCollapse =
                        _focusedDayIndex >= 0
                            ? _days[_focusedDayIndex]
                                .ObservableForProperty(x => x.IsExpanded)
                                .Subscribe(isExpanded => FocusDayOrItsFirstCell(_focusedDayIndex))
                            : Disposable.Empty;
                }
            }
        }
        public int FocusedCellIndex { get; private set; }
        private DayHeaderViewModel[] _days = new DayHeaderViewModel[0];

        private Dictionary<string, (int dayIndex, int cellIndex)> _cellsDictionary = new Dictionary<string, (int dayIndex, int cellIndex)>();

        private Dictionary<string, int> _daysDictionary = new Dictionary<string, int>();

        private (string focusedEntryId, string focusedDayHeader) GetCurrentFocusedItemId()
        {
            string focusedEntryId = null;
            string focusedDayHeader = null;
            if (FocusedDayIndex >= 0)
            {
                var focusedDay = _days[FocusedDayIndex];
                if (FocusedCellIndex >= 0)
                {
                    var focusedEntry = focusedDay.GetCell(FocusedCellIndex);
                    focusedEntryId = focusedEntry.Id;
                }
                else
                {
                    focusedDayHeader = focusedDay.DateHeader;
                }
            }

            return (focusedEntryId, focusedDayHeader);
        }

        public void ClearSavedFocus()
        {
            FocusedCellIndex = -1;
            FocusedDayIndex = -1;
        }

        public void SetDayHeaderViewModels(DayHeaderViewModel[] dayHeaderViewModels)
        {
            var (focusedEntryId, focusedDayHeader) = GetCurrentFocusedItemId();

            _days = dayHeaderViewModels;
            _cellsDictionary = dayHeaderViewModels
                .WithIndex()
                .SelectMany(tuple => tuple.item.CellsMutable.Items
                    .Select((cell, cellIndex) => (cell.Id, dayIndex: tuple.index, cellIndex)))
                .ToDictionary(tuple => tuple.Id, tuple => (tuple.dayIndex, tuple.cellIndex));
            _daysDictionary = dayHeaderViewModels
                .WithIndex()
                .ToDictionary(tuple => tuple.item.DateHeader, tuple => tuple.index);

            ClearSavedFocus();
            if (focusedEntryId != null && _cellsDictionary.TryGetValue(focusedEntryId, out var focusedCellPosition))
            {
                FocusCell(focusedCellPosition.dayIndex, focusedCellPosition.cellIndex);
            }
            else if (focusedDayHeader != null && _daysDictionary.TryGetValue(focusedDayHeader, out var focusedDayIndex))
            {
                FocusDay(focusedDayIndex);
            }
        }

        private void FocusFirst()
        {
            if (_days.Any())
            {
                FocusDayOrItsFirstCell(0);
            }
        }

        private void FocusDayOrItsFirstCell(int dayIndex)
        {
            Debug.Assert(dayIndex >= 0);
            Debug.Assert(dayIndex < _days.Length);
            var dayViewModel = _days[dayIndex];
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
            Debug.Assert(dayIndex < _days.Length);
            var dayViewModel = _days[dayIndex];
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
            if (FocusedCellIndex < 0 && FocusedDayIndex == dayIndex)
            {
                return;
            }
            Debug.Assert(dayIndex >= 0);
            Debug.Assert(dayIndex < _days.Length);
            var dayViewModel = _days[dayIndex];
            Debug.Assert(dayViewModel.IsCollapsed);

            FocusedDayIndex = dayIndex;
            FocusedCellIndex = -1;
            RefreshDayFocusIfListContainsFocus();
        }

        private IDisposable _updateFocusOnDayExpandCollapse = Disposable.Empty;

        private void FocusCell(int dayIndex, int cellIndex)
        {
            if (FocusedCellIndex == cellIndex && FocusedDayIndex == dayIndex)
            {
                return;
            }

            Debug.Assert(dayIndex >= 0);
            Debug.Assert(dayIndex < _days.Length);
            Debug.Assert(cellIndex >= 0);
            var dayViewModel = _days[dayIndex];
            Debug.Assert(dayViewModel.IsExpanded);
            if (cellIndex >= dayViewModel.CellsCount)
            {
                Debug.Assert(cellIndex < dayViewModel.CellsCount);
            }
            FocusedDayIndex = dayIndex;
            FocusedCellIndex = cellIndex;
            RefreshCellFocusIfListContainsFocus();
        }

        private void onHighlightDown(object sender, ExecutedRoutedEventArgs e)
        {
            if (FocusedDayIndex < 0)
            {
                FocusFirst();
            }
            else
            {
                if (FocusedCellIndex >= 0 && FocusedCellIndex + 1 < _days[FocusedDayIndex].CellsCount)
                {
                    FocusCell(FocusedDayIndex, FocusedCellIndex + 1);
                }
                else if (FocusedDayIndex + 1 < _days.Length)
                {
                    FocusDayOrItsFirstCell(FocusedDayIndex + 1);
                }
            }
        }

        private void onHighlightUp(object sender, ExecutedRoutedEventArgs e)
        {
            if (FocusedDayIndex < 0)
            {
                FocusFirst();
            }
            else
            {
                if (FocusedCellIndex >= 1)
                {
                    FocusCell(FocusedDayIndex, FocusedCellIndex - 1);
                }
                else if (FocusedDayIndex >= 1)
                {
                    FocusDayOrItsLastCell(FocusedDayIndex - 1);
                }
            }
        }

        protected override void OnGotKeyboardFocus(KeyboardFocusChangedEventArgs e)
        {
            base.OnGotKeyboardFocus(e);
            if (IsKeyboardFocusWithin)
            {
                RefreshFocus();
            }
        }

        private bool TryExpandFocusedDay()
        {
            if (FocusedCellIndex >= 0) return false; // cell is selected
            if (FocusedDayIndex < 0) return false; // nothing is selected
            var dayToExpand = _days[FocusedDayIndex];
            dayToExpand.Expand();
            return true;
        }

        private void onHighlightEdit(object sender, ExecutedRoutedEventArgs e)
        {
            if (TryExpandFocusedDay())
                return;

            if (!IsAnyCellFocused)
                return;

            var focusedCell = GetFocusedCell();
            if (focusedCell.TryToggleExpandCollapse())
            {
                return;
            }

            var guidOfCellToEdit = focusedCell.Guid;
            Toggl.Edit(guidOfCellToEdit, false, "");
        }

        private void onExpandSelectedItem(object sender, ExecutedRoutedEventArgs e)
        {
            if (!TryExpandFocusedGroup())
            {
                TryExpandFocusedDay();
            }
        }

        private void onCollapseSelectedItem(object sender, ExecutedRoutedEventArgs e)
        {
            if (!TryCollapseFocusedGroup())
            {
                TryCollapseFocusedDay();
            }
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
        }

        private void onHighlightDelete(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsAnyCellFocused)
            {
                var cell = GetFocusedCell();
                cell.DeleteTimeEntry();
            }
        }

        private void onFocusTimer(object sender, ExecutedRoutedEventArgs e)
        {
            if (this.editPopup.IsVisible)
            {
                CloseEditPopup?.Invoke(this, e);
            }
            else
            {
                FocusTimer?.Invoke(this, e);
            }
        }

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

            var dayIndex = FocusedDayIndex;
            var focusedDay = _days[dayIndex];
            var cellIndex = FocusedCellIndex;
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

        private bool TryCollapseFocusedDay()
        {
            if (FocusedDayIndex < 0)
                return false;
            _days[FocusedDayIndex].Collapse();
            return true;
        }

        public void CollapseAllDays()
        {
            _days.ForEach(day => day.Collapse());
            Toggl.ViewTimeEntryList();
        }

        public void ExpandAllDays()
        {
            _days.ForEach(day => day.Expand());
            Toggl.ViewTimeEntryList();
        }

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
