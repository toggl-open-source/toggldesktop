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

        public static bool IsEqualTo(
            this TimeEntryLabelViewModel timeEntryLabelViewModel,
            Toggl.TogglTimeEntryView item)
        {
            if (timeEntryLabelViewModel == null) return false;
            return timeEntryLabelViewModel.Description == item.Description
                   && timeEntryLabelViewModel.Tags == item.Tags
                   && timeEntryLabelViewModel.IsBillable == item.Billable
                   && timeEntryLabelViewModel.ProjectLabel.IsEqualTo(item);
        }

        public static bool IsEqualTo(
            this ProjectLabelViewModel projectLabelViewModel,
            Toggl.TogglTimeEntryView item)
        {
            if (projectLabelViewModel == null) return false;
            return projectLabelViewModel.ProjectName == item.ProjectLabel
                   && projectLabelViewModel.TaskName == item.TaskLabel
                   && projectLabelViewModel.ClientName == item.ClientLabel
                   && projectLabelViewModel.ColorString == item.Color
                   && projectLabelViewModel.WorkspaceName == item.WorkspaceName
                   && projectLabelViewModel.ProjectInfo.ProjectId == item.PID
                   && projectLabelViewModel.ProjectInfo.TaskId == item.TID;
        }
    }
}