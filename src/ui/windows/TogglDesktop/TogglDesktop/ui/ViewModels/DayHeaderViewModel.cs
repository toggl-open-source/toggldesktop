using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive.Linq;
using DynamicData;
using ReactiveUI;

namespace TogglDesktop.ViewModels
{
    public class DayHeaderViewModel : ReactiveObject
    {
        public DayHeaderViewModel(string dateHeader, string dateDuration)
        {
            DateHeader = dateHeader;
            DateDuration = dateDuration;
            this.WhenAnyValue(x => x.IsExpanded, x => !x)
                .ToProperty(this, nameof(IsCollapsed), out _isCollapsed);
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

        private bool _isExpanded;
        public bool IsExpanded
        {
            get => _isExpanded;
            set => this.RaiseAndSetIfChanged(ref _isExpanded, value);
        }

        private readonly ObservableAsPropertyHelper<bool> _isCollapsed;
        public bool IsCollapsed => _isCollapsed.Value;

        private bool _isFocused;
        public bool IsFocused
        {
            get => _isFocused;
            private set => this.RaiseAndSetIfChanged(ref _isFocused, value);
        }
        public void Focus()
        {
            IsFocused = false;
            IsFocused = true;
        }

        public void Expand() => IsExpanded = true;
        public void Collapse() => IsExpanded = false;
    }
}