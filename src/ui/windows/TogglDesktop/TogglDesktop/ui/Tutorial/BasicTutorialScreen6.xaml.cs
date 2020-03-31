using System;
namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen6
    {
        public BasicTutorialScreen6()
        {
            this.InitializeComponent();
        }

        private IDisposable _subscription;
        protected override void initialise()
        {
            _subscription = Toggl.OnStoppedTimerState.Subscribe(_ => this.activateScreen<BasicTutorialScreen7>());
        }

        protected override void cleanup()
        {
            _subscription.Dispose();
        }
    }
}
