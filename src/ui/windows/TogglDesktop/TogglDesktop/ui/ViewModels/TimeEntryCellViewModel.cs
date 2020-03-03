using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace TogglDesktop.ViewModels
{
    public class TimeEntryCellViewModel : ReactiveObject
    {
        public TimeEntryCellViewModel()
        {
        }

        [Reactive]
        public TimeEntryLabelViewModel TimeEntryLabel { get; set; }

        [Reactive]
        public bool IsFocused { get; private set; }

        public void Focus()
        {
            IsFocused = false;
            IsFocused = true;
        }

        [Reactive]
        public bool IsSelected { get; set; }

        // immutable per Guid
        [Reactive]
        public bool IsGroup { get; set; }

        [Reactive]
        public bool IsGroupExpanded { get; set; }

        // immutable per Guid
        [Reactive]
        public string Guid { get; set; }

        public string Id => IsGroup ? GroupName : Guid;

        // immutable per Guid
        [Reactive]
        public string GroupName { get; set; }

        // immutable per Guid
        [Reactive]
        public bool IsSubItem { get; set; }

        [Reactive]
        public ulong GroupItemCount { get; set; }

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