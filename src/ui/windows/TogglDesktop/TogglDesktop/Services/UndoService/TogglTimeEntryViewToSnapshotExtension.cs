using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static TogglDesktop.Toggl;

namespace TogglDesktop.Services.UndoService
{
    public static class TogglTimeEntryViewToSnapshotExtension
    {
        public static TimeEntrySnapshot ToTimeEntrySnapshot(
            this TogglTimeEntryView timeEntryView)
        {
            return new TimeEntrySnapshot()
            {
                GUID = timeEntryView.GUID,
                Description = timeEntryView.Description,
                ProjectId = timeEntryView.PID,
                TaskId = timeEntryView.TID,
                Tags = timeEntryView.Tags,
                Billable = timeEntryView.Billable,
                Duration = timeEntryView.Duration,
                Started = timeEntryView.Started,
                Ended = timeEntryView.Ended
            };
        }
    }
}
