using System;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen3
    {
        public BasicTutorialScreen3()
        {
            this.InitializeComponent();
        }

        private IDisposable _subscription;
        protected override void initialise()
        {
            _subscription = Toggl.OnRunningTimerState.Subscribe(onRunningTimerState);
        }

        protected override void cleanup()
        {
            _subscription.Dispose();
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            this.activateScreen<BasicTutorialScreen4>();
        }

    }
}
