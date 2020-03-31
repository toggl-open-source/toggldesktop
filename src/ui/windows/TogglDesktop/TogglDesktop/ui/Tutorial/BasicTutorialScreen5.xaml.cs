using System;
using System.Reactive.Disposables;
using System.Reactive.Linq;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen5
    {
        private IDisposable _subscription;
        public BasicTutorialScreen5()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            _subscription = new CompositeDisposable(
                Toggl.OnTimeEntryList.Where(x => x.open).Subscribe(_ => this.activateScreen<BasicTutorialScreen6>()),
                Toggl.OnStoppedTimerState.Subscribe(_ => this.activateScreen<BasicTutorialScreen7>()));
        }

        protected override void cleanup()
        {
            _subscription.Dispose();
        }
    }
}
