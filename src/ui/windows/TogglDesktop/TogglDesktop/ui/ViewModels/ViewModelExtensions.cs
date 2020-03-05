using System;

namespace TogglDesktop.ViewModels
{
    public static class ViewModelExtensions
    {
        public static DayHeaderViewModel ToDayHeaderViewModel(this Toggl.TogglTimeEntryView item)
        {
            if (!item.IsHeader)
            {
                throw new InvalidOperationException("Can only create day header from header time entry view.");
            }

            return new DayHeaderViewModel(item.DateHeader, item.DateDuration);
        }

        public static ProjectLabelViewModel ToProjectLabelViewModel(this Toggl.TogglTimeEntryView item)
        {
            return new ProjectLabelViewModel(
                item.ProjectLabel,
                item.TaskLabel,
                item.ClientLabel,
                item.Color,
                item.WorkspaceName,
                item.PID,
                item.TID);
        }
        public static ProjectLabelViewModel ToProjectLabelViewModel(this Toggl.TogglAutocompleteView item)
        {
            return new ProjectLabelViewModel(
                item.ProjectLabel,
                item.TaskLabel,
                item.ClientLabel,
                item.ProjectColor,
                item.WorkspaceName,
                item.ProjectID,
                item.TaskID);
        }

        public static TimeEntryLabelViewModel ToTimeEntryLabelViewModel(this Toggl.TogglTimeEntryView item)
        {
            return new TimeEntryLabelViewModel(
                item.Description,
                item.ToProjectLabelViewModel(),
                item.Tags,
                item.Billable);
        }

        public static TimeEntryLabelViewModel ToTrayToolTipTimeEntryLabelViewModel(this Toggl.TogglTimeEntryView item)
        {
            return new TimeEntryLabelViewModel(
                item.Description,
                item.ToProjectLabelViewModel(),
                item.Tags,
                item.Billable,
                false);
        }

        public static TimeEntryCellViewModel ToTimeEntryCellViewModel(this Toggl.TogglTimeEntryView item)
        {
            return new TimeEntryCellViewModel().UpdateWith(item);
        }

        public static TimeEntryCellViewModel UpdateWith(this TimeEntryCellViewModel vm, Toggl.TogglTimeEntryView item)
        {
            vm.Guid = item.GUID;
            vm.IsGroup = item.Group;
            if (vm.IsGroup)
            {
                vm.IsGroupExpanded = item.GroupOpen;
                vm.GroupName = item.GroupName;
                vm.GroupItemCount = item.GroupItemCount;
            }

            vm.IsSubItem = !item.Group && item.GroupOpen;
            vm.DurationInSeconds = item.DurationInSeconds;
            vm.TimeEntryLabel = item.ToTimeEntryLabelViewModel();
            vm.Duration = item.Duration;
            vm.DurationToolTip = $"{item.StartTimeString} - {item.EndTimeString}";

            vm.Unsynced = item.Unsynced;
            vm.Locked = item.Locked;
            return vm;
        }
    }
}