using System;
using System.Collections.Generic;
using System.Linq;
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
        }

        public bool IsDummy
        {
            set => this.IsEnabled = !value;
        }

        public bool IsCollapsed => !this.ViewModel.IsExpanded;

        public bool IsSelected
        {
            get => ViewModel.IsSelected;
            set => ViewModel.IsSelected = value;
        }

        public void DisplayDummy(string dateText, string durationText = "")
        {
            ViewModel = new DayHeaderViewModel(dateText, durationText);
        }

        public void Display(List<Toggl.TogglTimeEntryView> items, bool isExpanded)
        {
            ViewModel = items[0].ToDayHeaderViewModel();
            ViewModel.IsExpanded = isExpanded;
            this.fillCells(items);
        }

        public IEnumerable<TimeEntryCell> Children => this.panel.Children.Cast<TimeEntryCell>();

        private void fillCells(List<Toggl.TogglTimeEntryView> list)
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
    }
}
