
namespace TogglDesktop.Experiments
{
    sealed class Experiment101 : ExperimentBase
    {
        private ExperimentParameters parameters;

        public Experiment101()
            : base(101)
        {
        }

        protected override void runIncluded(ExperimentParameters parameters)
        {
            this.parameters = parameters;
            Toggl.OnRunningTimerState += this.onRunningTimerState;
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            ExperimentHacks.RemoveEmptyStateFirstLine(this, this.parameters);
            this.parameters.TutorialManager.ActivateScreen<Experiment101Screen>();
            Toggl.SendObmAction(this.Id, "seen");

            Toggl.OnRunningTimerState -= this.onRunningTimerState;
        }

    }
}