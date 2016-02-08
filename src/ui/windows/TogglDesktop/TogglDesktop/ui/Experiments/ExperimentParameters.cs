using TogglDesktop.Tutorial;

namespace TogglDesktop.Experiments
{
    sealed class ExperimentParameters
    {
        public ExperimentParameters(bool included, TutorialManager tutorialManager, bool isFirstRun)
        {
            this.IsFirstRun = isFirstRun;
            this.Included = included;
            this.TutorialManager = tutorialManager;
        }

        public bool IsFirstRun { get; private set; }
        public bool Included { get; private set; }
        public TutorialManager TutorialManager { get; private set; }
    }
}