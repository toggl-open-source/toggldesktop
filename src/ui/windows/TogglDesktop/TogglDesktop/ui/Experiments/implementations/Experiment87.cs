namespace TogglDesktop.Experiments
{
    sealed class Experiment87 : ExperimentBase
    {
        public Experiment87()
            : base(87)
        {
        }

        protected override void runIncluded(ExperimentParameters parameters)
        {
            parameters.TutorialManager.ActivateScreen<Experiment87Screen>();
        }
    }
}