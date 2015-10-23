using System;
using System.Windows.Controls;

namespace TogglDesktop.Tutorial
{
    public abstract class TutorialScreen : UserControl, IDisposable
    {
        protected TutorialManager tutorialManager { get; private set; }

        public void Initialise(TutorialManager tutorialManager)
        {
            this.tutorialManager = tutorialManager;
            this.initialise();
        }

        public void Dispose()
        {
            this.cleanup();
        }

        protected void activateScreen<T>()
            where T : TutorialScreen, new()
        {
            this.tutorialManager.ActivateScreen<T>();
        }

        protected void quitTutorial()
        {
            this.tutorialManager.QuitTutorial();
        }

        protected abstract void initialise();

        protected abstract void cleanup();
    }
}