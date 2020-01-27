using ReactiveUI;

namespace TogglDesktop.ViewModels
{
    public class TimeEntryCellViewModel : ReactiveObject
    {
        public TimeEntryCellViewModel(TimeEntryLabelViewModel timeEntryLabel)
        {
            TimeEntryLabel = timeEntryLabel;
        }
        private TimeEntryLabelViewModel _timeEntryLabel;
        public TimeEntryLabelViewModel TimeEntryLabel
        {
            get => _timeEntryLabel;
            set => this.RaiseAndSetIfChanged(ref _timeEntryLabel, value);
        }

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

        private bool _isSelected;
        public bool IsSelected
        {
            get => _isSelected;
            set => this.RaiseAndSetIfChanged(ref _isSelected, value);
        }

        private bool _isGroup;
        // immutable per Guid
        public bool IsGroup
        {
            get => _isGroup;
            set => this.RaiseAndSetIfChanged(ref _isGroup, value);
        }

        private bool _isGroupExpanded;
        public bool IsGroupExpanded
        {
            get => _isGroupExpanded;
            set => this.RaiseAndSetIfChanged(ref _isGroupExpanded, value);
        }

        private string _guid;
        // immutable per Guid
        public string Guid
        {
            get => _guid;
            set => this.RaiseAndSetIfChanged(ref _guid, value);
        }

        private string _groupName;
        // immutable per Guid
        public string GroupName
        {
            get => _groupName;
            set => this.RaiseAndSetIfChanged(ref _groupName, value);
        }

        private bool _isSubItem;
        // immutable per Guid
        public bool IsSubItem
        {
            get => _isSubItem;
            set => this.RaiseAndSetIfChanged(ref _isSubItem, value);
        }

        private ulong _groupItemCount;
        public ulong GroupItemCount
        {
            get => _groupItemCount;
            set => this.RaiseAndSetIfChanged(ref _groupItemCount, value);
        }

        public long DurationInSeconds { get; set; }

        public bool TryExpand()
        {
            var canExpand = IsGroup && !IsGroupExpanded;
            if (!canExpand) return false;
            Toggl.ToggleEntriesGroup(GroupName);
            return true;
        }

        public bool TryCollapse()
        {
            var canCollapse = IsGroup && IsGroupExpanded;
            if (!canCollapse) return false;
            Toggl.ToggleEntriesGroup(GroupName);
            return true;
        }

        public bool TryToggleExpandCollapse()
        {
            var canToggleExpandCollapse = IsGroup;
            if (!canToggleExpandCollapse) return false;
            Toggl.ToggleEntriesGroup(GroupName);
            return true;
        }
    }
}