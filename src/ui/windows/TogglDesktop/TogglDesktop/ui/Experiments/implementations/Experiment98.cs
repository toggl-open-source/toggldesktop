
namespace TogglDesktop.Experiments
{
    sealed class Experiment98 : ExperimentBase
    {
        public Experiment98()
            : base(98, false)
        {
        }

        protected override void runIncluded(ExperimentParameters parameters) 
        {
            if (parameters.IsFirstRun)
            {
                parameters.TutorialManager.ActivateScreen<Experiment98Screen1>();
                ExperimentHacks.RemoveEmptyStateFirstLine(this, parameters);
            }

            Toggl.OnUserTimeEntryStart += this.sendStartAction;
        }

        private void sendStartAction()
        {
            Toggl.SendObmAction(this.Id, "start");

            Toggl.OnUserTimeEntryStart -= this.sendStartAction;
        }
    }
}