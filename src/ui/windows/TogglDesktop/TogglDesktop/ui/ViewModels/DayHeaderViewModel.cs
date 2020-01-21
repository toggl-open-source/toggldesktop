using ReactiveUI;

namespace TogglDesktop.ViewModels
{
    public class DayHeaderViewModel : ReactiveObject
    {
        public DayHeaderViewModel(string dateHeader, string dateDuration)
        {
            DateHeader = dateHeader;
            DateDuration = dateDuration;
        }

        public string DateHeader { get; }
        public string DateDuration { get; }

        private bool _isExpanded;
        public bool IsExpanded
        {
            get => _isExpanded;
            set => this.RaiseAndSetIfChanged(ref _isExpanded, value);
        }

        private bool _isSelected;
        public bool IsSelected
        {
            get => _isSelected;
            set => this.RaiseAndSetIfChanged(ref _isSelected, value);
        }

        public void Expand() => IsExpanded = true;
        public void Collapse() => IsExpanded = false;
    }
}