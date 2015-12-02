namespace TogglDesktop.Experiments
{
    sealed class Experiment80 : ExperimentBase
    {
        public Experiment80()
            : base(80)
        {
        }

        protected override void runIncluded(ExperimentParameters parameters)
        {
            parameters.TutorialManager.ActivateScreen<BasicOverviewScreen>();
        }
    }
}