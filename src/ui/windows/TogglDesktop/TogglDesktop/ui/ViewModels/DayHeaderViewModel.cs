using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive.Linq;
using DynamicData;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace TogglDesktop.ViewModels
{
    public class DayHeaderViewModel : ReactiveObject
    {
        public DayHeaderViewModel(string dateHeader, string dateDuration)
        {
            DateHeader = dateHeader;
            DateDuration = dateDuration;
            this.WhenAnyValue(x => x.IsExpanded, x => !x)
                .ToPropertyEx(this, x => x.IsCollapsed);
            _cells.Connect()
                .ObserveOnDispatcher()
                .Bind(out var daysReadOnlyObservableCollection)
                .DisposeMany()
                .Subscribe();
            Cells = daysReadOnlyObservableCollection;
        }

        public string DateHeader { get; }
        public string DateDuration { get; }

        private readonly SourceList<TimeEntryCellViewModel> _cells = new SourceList<TimeEntryCellViewModel>();
        public ReadOnlyObservableCollection<TimeEntryCellViewModel> Cells { get; }
        public ISourceList<TimeEntryCellViewModel> CellsMutable => _cells;

        public TimeEntryCellViewModel GetCell(int cellIndex) => _cells.Items.ElementAt(cellIndex);
        public int CellsCount => _cells.Count;

        [Reactive]
        public bool IsExpanded { get; set; }

        public bool IsCollapsed { [ObservableAsProperty] get; }

        [Reactive]
        public bool IsFocused { get; private set; }
        public void Focus()
        {
            IsFocused = false;
            IsFocused = true;
        }

        public void Expand() => IsExpanded = true;
        public void Collapse() => IsExpanded = false;
    }
}