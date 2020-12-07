
using System;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen7
    {
        public BasicTutorialScreen7()
        {
            this.InitializeComponent();
        }

        private IDisposable _runningTimeEntryObservable;
        protected override void initialise()
        {
            _runningTimeEntryObservable = Toggl.RunningTimerState.Subscribe(this.onRunningTimerState);
            Toggl.OnTimeEntryEditor += this.onTimerEntryEditor;
        }

        protected override void cleanup()
        {
            _runningTimeEntryObservable.Dispose();
            Toggl.OnTimeEntryEditor -= this.onTimerEntryEditor;
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            this.quitTutorial();
        }
        private void onTimerEntryEditor(bool open, Toggl.TogglTimeEntryView te, string focusedFieldName)
        {
            this.quitTutorial();
        }
    }
}
