using System;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public static class ModelExtensions
    {
        public static bool ConfirmlessDelete(this TimeEntryCellViewModel cell)
        {
            if (cell.DurationInSeconds < 0)
            {
                var epoch = (int)(DateTime.UtcNow - new DateTime(1970, 1, 1)).TotalSeconds;
                var actualDuration = cell.DurationInSeconds + epoch;
                return actualDuration < 15;
            }
            else
            {
                return cell.DurationInSeconds < 15;
            }
        }

        public static void DeleteTimeEntry(this TimeEntryCellViewModel cell)
        {
            if (cell.ConfirmlessDelete())
            {
                Toggl.DeleteTimeEntry(cell.Guid);
                return;
            }
            Toggl.AskToDeleteEntry(cell.Guid);
        }
    }
}