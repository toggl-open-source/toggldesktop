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

        public void FillCells(List<Toggl.TogglTimeEntryView> list)
        {
            var children = ViewModel.CellsMutable;

            // remove superfluous cells
            if (children.Count > list.Count)
            {
                children.RemoveRange(list.Count, children.Count - list.Count);
            }

            // update existing cells
            var i = 0;
            foreach (var cellViewModel in children.Items)
            {
                var entry = list[i++];
                cellViewModel.UpdateWith(entry);
            }

            // add additional cells
            if (list.Count > children.Count)
            {
                var newCells = list.Skip(i).Select(entry => entry.ToTimeEntryCellViewModel());
                children.AddRange(newCells);
            }
        }
    }
}
