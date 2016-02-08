namespace TogglDesktop.Experiments
{
    sealed class ExperimentSimpleWelcomeScreen : ExperimentBase
    {
        public ExperimentSimpleWelcomeScreen()
            : base(0)
        {
        }

        protected override void runIncluded(ExperimentParameters parameters) 
        {
            parameters.TutorialManager.ActivateScreen<SimpleWelcomeScreen>();
        }
    }
}