
using System.Collections.Generic;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen5
    {
        public BasicTutorialScreen5()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.OnTimeEntryList += this.onTimeEntryList;
            Toggl.OnStoppedTimerState += this.onStoppedTimerState;
        }

        protected override void cleanup()
        {
            Toggl.OnTimeEntryList -= this.onTimeEntryList;
            Toggl.OnStoppedTimerState -= this.onStoppedTimerState;
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
