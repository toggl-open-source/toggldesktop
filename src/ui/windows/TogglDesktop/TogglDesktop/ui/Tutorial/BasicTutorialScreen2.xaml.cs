using System;

namespace TogglDesktop.Tutorial
{
    public partial class BasicTutorialScreen2
    {
        public BasicTutorialScreen2()
        {
            this.InitializeComponent();
        }

        private IDisposable _runningTimeEntryObservable;
        protected override void initialise()
        {
            _runningTimeEntryObservable = Toggl.RunningTimerState.Subscribe(this.onRunningTimerState);
            this.tutorialManager.Timer.DescriptionTextBoxTextChanged += this.onDescriptionTextChanged;
        }

        protected override void cleanup()
        {
            _runningTimeEntryObservable.Dispose();
            this.tutorialManager.Timer.DescriptionTextBoxTextChanged -= this.onDescriptionTextChanged;
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            this.tutorialManager.ActivateScreen<BasicTutorialScreen4>();
        }

        private void onDescriptionTextChanged(object sender, string text)
        {
            if (string.IsNullOrWhiteSpace(text))
                return;

            this.activateScreen<BasicTutorialScreen3>();
        }

    }
}
