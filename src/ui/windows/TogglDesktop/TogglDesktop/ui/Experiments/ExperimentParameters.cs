using TogglDesktop.Tutorial;

namespace TogglDesktop.Experiments
{
    sealed class ExperimentParameters
    {
        public ExperimentParameters(bool included, TutorialManager tutorialManager)
        {
            this.Included = included;
            this.TutorialManager = tutorialManager;
        }

        public bool Included { get; private set; }
        public TutorialManager TutorialManager { get; private set; }
    }
}