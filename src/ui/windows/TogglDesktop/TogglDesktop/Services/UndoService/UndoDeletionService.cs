using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TogglDesktop.Services.UndoService
{
    public class UndoDeletionService
    {
        private Stack<TimeEntrySnapshot> undoStack;
        public UndoDeletionService()
        {
            undoStack = new Stack<TimeEntrySnapshot>();
        }

        public bool CanUndo => undoStack.Any();

        public void SnapshotTimeEntry(TimeEntrySnapshot timeEntry)
        {
            undoStack.Push(timeEntry);
        }

        public void UndoDeletion()
        {
            if (CanUndo)
            {
                var te = undoStack.Pop();
                Toggl.Start(te.Description, te.Duration, te.TaskId, te.ProjectId, "", te.Tags, true, te.Started, te.Ended, false);
                if (te.Billable)
                    Toggl.SetTimeEntryBillable(te.GUID, te.Billable);
            }
        }
    }
}
