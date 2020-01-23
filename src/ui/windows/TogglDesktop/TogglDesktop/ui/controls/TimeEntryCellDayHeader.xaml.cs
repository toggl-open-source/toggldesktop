using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using DynamicData;
using ReactiveUI;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public partial class TimeEntryCellDayHeader : IViewFor<DayHeaderViewModel>
    {
        public DayHeaderViewModel ViewModel
        {
            get => (DayHeaderViewModel)DataContext;
            set => DataContext = value;
        }

        object IViewFor.ViewModel
        {
            get => ViewModel;
            set => ViewModel = (DayHeaderViewModel) value;
        }

        public TimeEntryCellDayHeader()
        {
            this.InitializeComponent();
            this.SetupKeyboardFocusedReverseBinding(nameof(ViewModel.IsFocused));
        }

        public bool IsDummy
        {
            set => this.IsEnabled = !value;
        }

        public void DisplayDummy(string dateText, string durationText = "")
        {
            ViewModel = new DayHeaderViewModel(dateText, durationText);
        }

        public void Display(DayHeaderViewModel viewModel, List<Toggl.TogglTimeEntryView> items)
        {
            ViewModel = viewModel;
            this.fillCells(items);
            ViewModel.CellsMutable.AddRange(this.panel.Children.Cast<TimeEntryCell>().Select(cell => cell.ViewModel)); // ???
        }

        public void Display(List<Toggl.TogglTimeEntryView> items, bool isExpanded)
        {
            ViewModel = items[0].ToDayHeaderViewModel();
            ViewModel.IsExpanded = isExpanded;
            this.fillCells(items);
            ViewModel.CellsMutable.AddRange(this.panel.Children.Cast<TimeEntryCell>().Select(cell => cell.ViewModel));
        }

        public IEnumerable<TimeEntryCell> Children => this.panel.Children.Cast<TimeEntryCell>();

        private void fillCells(List<Toggl.TogglTimeEntryView> list)
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
                // maybe also update ViewModel.DaysMutable here?
            }

            // add additional cells
            for (; i < list.Count; i++)
            {
                var entry = list[i];

                var cell = new TimeEntryCell();
                cell.Display(entry, this);
                children.Add(cell);
            }
        }

        private void fillCells2(List<Toggl.TogglTimeEntryView> list)
        {
            var children = this.panel.Children;
            string guidString;

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
            }

            // add additional cells
            for (; i < list.Count; i++)
            {
                var entry = list[i];

                var cell = new TimeEntryCell();
                cell.Display(entry, this);

                children.Add(cell);
            }
        }

        public void Expand() => ViewModel.Expand();
        public void Collapse() => ViewModel.Collapse();

        private void TimeEntryCellDayHeader_OnKeyDown(object sender, KeyEventArgs e)
        {
            // var isShiftPressed = (Keyboard.Modifiers & ModifierKeys.Shift) == ModifierKeys.Shift;
            // var direction = e.Key switch
            // {
            //     Key.Down when isShiftPressed => FocusNavigationDirection.Down,
            //     Key.Down => FocusNavigationDirection.Next,
            //     Key.Up when isShiftPressed => FocusNavigationDirection.Up,
            //     Key.Up => FocusNavigationDirection.Previous,
            //     _ => (FocusNavigationDirection?)null
            // };
            // if (direction != null)
            // {
            //     var request = new TraversalRequest(direction.Value);
            //     var sourceElement = e.OriginalSource as FrameworkElement;
            //     sourceElement?.MoveFocus(request);
            //     e.Handled = true;
            // }
            // else
            // {
            //     switch (e.Key)
            //     {
            //         case Key.Right when !isShiftPressed:
            //             ViewModel.IsExpanded = true;
            //             this.panel.Children[0].Focus();
            //             e.Handled = true;
            //             break;
            //         case Key.Left when !isShiftPressed:
            //             ViewModel.IsExpanded = false;
            //             ViewModel.IsFocused = true;
            //             e.Handled = true;
            //             break;
            //     }
            // }
        }
    }
}
