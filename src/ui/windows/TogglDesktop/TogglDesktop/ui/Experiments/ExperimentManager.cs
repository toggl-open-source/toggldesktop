
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

            Toggl.OnDisplayObmExperiment += this.onDisplayObmExperiment;
        }

        public ulong? CurrentExperumentId
        {
            get { return this.experiment == null ? (ulong?)null : this.experiment.Id; }
        }

        private void onDisplayObmExperiment(ulong id, bool included, bool seenBefore)
        {
            if (this.mainWindow.TryBeginInvoke(this.onDisplayObmExperiment, id, included, seenBefore))
                return;

            if (this.experiment == null)
                return;

            if (this.experiment.Id != id)
            {
                Toggl.Debug("Unknown experiment: {0}", id);
                return;
            }

            if (seenBefore && this.experiment.OnlyRunOnce)
            {
                Toggl.Debug("Already ran experiment: {0}", id);
                return;
            }

            Toggl.Debug("Running experiment: {0} (included: {1})", id, included);
            this.experiment.Run(
                new ExperimentParameters(included, this.mainWindow.TutorialManager)
                );
        }
    }
}