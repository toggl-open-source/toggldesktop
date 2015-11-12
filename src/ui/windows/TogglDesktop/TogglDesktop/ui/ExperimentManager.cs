
using TogglDesktop.Experiments;

namespace TogglDesktop
{
    class ExperimentManager
    {
        private readonly MainWindow mainWindow;

        public ExperimentManager(MainWindow mainWindow)
        {
            this.mainWindow = mainWindow;

            Toggl.OnDisplayPromotion += this.onDisplayPromotion;
        }

        private void onDisplayPromotion(long id)
        {
            if (this.mainWindow.TryBeginInvoke(this.onDisplayPromotion, id))
                return;

            switch (id)
            {
                case 73:
                {
                    this.mainWindow.TutorialManager.ActivateScreen<BasicOverviewScreen>();
                    break;
                }
                default:
                {
                    Toggl.Debug("Tried to show unknown experiment: {0}", id);
                    break;
                }
            }
        }
    }
}