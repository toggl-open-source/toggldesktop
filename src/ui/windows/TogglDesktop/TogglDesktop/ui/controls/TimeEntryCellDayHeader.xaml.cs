using System.Collections.Generic;
using System.Linq;
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
                cell.ViewModel.UpdateWith(entry);
                // maybe also update ViewModel.DaysMutable here?
            }

            // add additional cells
            for (; i < list.Count; i++)
            {
                var entry = list[i];
                var cell = new TimeEntryCell
                {
                    ViewModel = entry.ToTimeEntryCellViewModel()
                };
                children.Add(cell);
            }
        }
    }
}
