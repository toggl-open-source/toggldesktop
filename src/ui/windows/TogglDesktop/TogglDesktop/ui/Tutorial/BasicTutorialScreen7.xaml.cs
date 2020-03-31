
using System;
using System.Reactive.Disposables;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen7
    {
        private IDisposable _subscription;
        public BasicTutorialScreen7()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            _subscription = new CompositeDisposable(
                Toggl.OnRunningTimerState.Subscribe(this.onRunningTimerState),
                Toggl.OnTimeEntryEditor.Subscribe(_ => this.quitTutorial()));
        }

        protected override void cleanup()
        {
            _subscription.Dispose();
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            this.quitTutorial();
        }
    }
}
