
using System;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen4
    {
        private IDisposable _timerStateObservable;

        public BasicTutorialScreen4()
        {
            this.InitializeComponent();
        }

        protected override void initialise()
        {
            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
            _timerStateObservable = Toggl.StoppedTimerState.Subscribe(_ => this.onStoppedTimerState());
        }

        protected override void cleanup()
        {
            Toggl.OnTimeEntryEditor -= this.onTimeEntryEditor;
            _timerStateObservable?.Dispose();
        }

        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView te, string focusedFieldName)
        {
            this.activateScreen<BasicTutorialScreen5>();
        }

        private void onStoppedTimerState()
        {
            this.activateScreen<BasicTutorialScreen7>();
        }


    }
}
