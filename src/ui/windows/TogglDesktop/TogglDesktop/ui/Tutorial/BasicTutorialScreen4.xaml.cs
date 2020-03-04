
using System;
using System.Reactive.Disposables;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen4
    {
        private IDisposable _subscription;
        public BasicTutorialScreen4()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            _subscription = new CompositeDisposable(
                Toggl.OnTimeEntryEditor.Subscribe(_ => this.activateScreen<BasicTutorialScreen5>()),
                Toggl.OnStoppedTimerState.Subscribe(_ => this.activateScreen<BasicTutorialScreen7>()));
        }

        protected override void cleanup()
        {
            _subscription.Dispose();
        }
    }
}
