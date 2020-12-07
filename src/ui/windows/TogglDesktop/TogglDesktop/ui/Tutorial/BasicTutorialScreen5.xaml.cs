
using System;
using System.Collections.Generic;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen5
    {
        private IDisposable _timerStateObservable;

        public BasicTutorialScreen5()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.OnTimeEntryList += this.onTimeEntryList;
            _timerStateObservable = Toggl.StoppedTimerState.Subscribe(_ => this.onStoppedTimerState());
        }

        protected override void cleanup()
        {
            Toggl.OnTimeEntryList -= this.onTimeEntryList;
            _timerStateObservable?.Dispose();
        }

        private void onTimeEntryList(bool open, List<Toggl.TogglTimeEntryView> list, bool showLoadMoreButton)
        {
            if (!open)
                return;

            this.activateScreen<BasicTutorialScreen6>();
        }


        private void onStoppedTimerState()
        {
            this.activateScreen<BasicTutorialScreen7>();
        }

    }
}
