namespace TogglDesktop.Experiments
{
    sealed class Experiment99 : ExperimentBase
    {
        public Experiment99()
            : base(99, false)
        {
        }

        protected override void runIncluded(ExperimentParameters parameters) 
        {
            if (parameters.IsFirstRun)
            {
                parameters.TutorialManager.ActivateScreen<Experiment99Screen>();
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