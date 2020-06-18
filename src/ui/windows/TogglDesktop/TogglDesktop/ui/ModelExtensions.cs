using System;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public static class ModelExtensions
    {
        public static bool ConfirmlessDelete(this TimeEntryCellViewModel cell)
        {
            return IsDurationLessThan15Seconds(cell.DurationInSeconds);
        }

        public static bool ConfirmlessDelete(this Toggl.TogglTimeEntryView timeEntry)
        {
            return IsDurationLessThan15Seconds(timeEntry.DurationInSeconds);
        }

        public static void DeleteTimeEntry(this TimeEntryCellViewModel cell)
        {
            if (cell.ConfirmlessDelete())
            {
                Toggl.DeleteTimeEntry(cell.TimeEntrySnapshot);
                return;
            }
            Toggl.AskToDeleteEntry(cell.TimeEntrySnapshot);
        }

        private static bool IsDurationLessThan15Seconds(long durationInSeconds)
        {
            if (durationInSeconds < 0)
            {
                var epoch = (int)(DateTime.UtcNow - new DateTime(1970, 1, 1)).TotalSeconds;
                var actualDuration = durationInSeconds + epoch;
                return actualDuration < 15;
            }
            else
            {
                return durationInSeconds < 15;
            }
        }

        public static string GetProjectAndTaskString(this Toggl.TogglAutocompleteView item)
        {
            return item.ProjectLabel + (item.TaskLabel.IsNullOrEmpty() ? "" : $" - {item.TaskLabel}");
        }

        public static string GetFullProjectString(this Toggl.TogglAutocompleteView item)
        {
            return item.GetProjectAndTaskString() + (item.ClientLabel.IsNullOrEmpty() ? "" : $". {item.ClientLabel}");
        }
    }
}