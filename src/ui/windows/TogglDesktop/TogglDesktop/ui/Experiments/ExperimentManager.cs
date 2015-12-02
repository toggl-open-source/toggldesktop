
namespace TogglDesktop.Experiments
{
    sealed class ExperimentManager
    {
        // this is where the currently supported experiment is defined
        private readonly IExperiment experiment = new Experiment80();

        private readonly MainWindow mainWindow;

        public ExperimentManager(MainWindow mainWindow)
        {
            this.mainWindow = mainWindow;

            // TODO: use OBM event instead
            //Toggl.OnDisplayPromotion += this.onDisplayPromotion;
        }

        public int? CurrentExperumentId
        {
            get { return this.experiment == null ? (int?)null : this.experiment.Id; }
        }

        private void onDisplayObmExperiment(int id, bool included, bool firstStart)
        {
            if (this.mainWindow.TryBeginInvoke(this.onDisplayObmExperiment, id, included, firstStart))
                return;

            if (this.experiment == null)
                return;

            if (this.experiment.Id != id)
            {
                Toggl.Debug("Unknown experiment: {0}", id);
                return;
            }

            if (!firstStart && this.experiment.OnlyRunOnce)
            {
                Toggl.Debug("Already ran experiment: {0}", id);
                return;
            }

            Toggl.Debug("Running experiment: {0} (included: {1})", id, included);
            this.experiment.Run(
                new ExperimentParameters(included, this.mainWindow.TutorialManager)
                );
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