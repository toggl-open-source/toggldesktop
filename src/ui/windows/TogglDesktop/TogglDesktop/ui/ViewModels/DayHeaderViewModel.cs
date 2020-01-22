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
        }

        public string DateHeader { get; }
        public string DateDuration { get; }

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
            set => this.RaiseAndSetIfChanged(ref _isFocused, value);
        }

        public void Expand() => IsExpanded = true;
        public void Collapse() => IsExpanded = false;
    }
}