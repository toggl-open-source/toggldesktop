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
                item.ToProjectLabelViewModel());
        }

        public static TimeEntryLabelViewModel ToTrayToolTipTimeEntryLabelViewModel(this Toggl.TogglTimeEntryView item)
        {
            return new TimeEntryLabelViewModel(
                item.Description,
                item.ToProjectLabelViewModel(),
                false);
        }
    }
}