
using System;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen3
    {
        public BasicTutorialScreen3()
        {
            this.InitializeComponent();
        }

        private IDisposable _runningTimeEntryObservable;
        protected override void initialise()
        {
            _runningTimeEntryObservable = Toggl.RunningTimerState.Subscribe(this.onRunningTimerState);
        }

        protected override void cleanup()
        {
            _runningTimeEntryObservable.Dispose();
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            this.activateScreen<BasicTutorialScreen4>();
        }

    }
}
