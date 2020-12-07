
using System;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen6
    {
        private IDisposable _timerStateObservable;

        public BasicTutorialScreen6()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            _timerStateObservable = Toggl.StoppedTimerState.Subscribe(_ => this.onStoppedTimerState());
        }

        protected override void cleanup()
        {
            _timerStateObservable?.Dispose();
        }

        private void onStoppedTimerState()
        {
            this.activateScreen<BasicTutorialScreen7>();
        }
    }
}
