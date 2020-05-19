using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Text;
using System.Threading.Tasks;
using static TogglDesktop.Toggl;

namespace TogglDesktop.Services
{
    public class UndoDeletionService
    {
        private Stack<TogglTimeEntryView> undoStack;
        public UndoDeletionService()
        {
            undoStack = new Stack<TogglTimeEntryView>();
        }

        public bool CanUndo => undoStack.Any();

        public void SnapshotTimeEntry(TogglTimeEntryView timeEntry)
        {
            undoStack.Push(timeEntry);
        }

        public void UndoDeletion()
        {
            if (CanUndo)
            {
                var te = undoStack.Pop();
                Toggl.Start(te.Description, te.Duration, te.TID, te.PID, "", te.Tags, true, te.Started, te.Ended, false);
                if (te.Billable)
                    Toggl.SetTimeEntryBillable(te.GUID, te.Billable);
            }
        }
    }
}
