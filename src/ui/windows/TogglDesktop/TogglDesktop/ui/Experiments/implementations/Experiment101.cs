using TogglDesktop.Tutorial;

namespace TogglDesktop.Experiments
{
    sealed class Experiment101 : ExperimentBase
    {
        private TutorialManager tutorialManager;

        public Experiment101()
            : base(101)
        {
        }

        protected override void runIncluded(ExperimentParameters parameters)
        {
            this.tutorialManager = parameters.TutorialManager;
            Toggl.OnRunningTimerState += this.onRunningTimerState;
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            this.tutorialManager.ActivateScreen<Experiment101Screen>();
            Toggl.SendObmAction(this.Id, "seen");

            Toggl.OnRunningTimerState -= this.onRunningTimerState;
        }

    }
}