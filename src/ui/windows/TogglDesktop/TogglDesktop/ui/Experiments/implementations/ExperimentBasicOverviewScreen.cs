namespace TogglDesktop.Experiments
{
    sealed class ExperimentBasicOverviewScreen : ExperimentBase
    {
        public ExperimentBasicOverviewScreen()
            : base(87)
        {
        }

        protected override void runIncluded(ExperimentParameters parameters)
        {
            parameters.TutorialManager.ActivateScreen<BasicOverviewScreen>();
        }
    }
}