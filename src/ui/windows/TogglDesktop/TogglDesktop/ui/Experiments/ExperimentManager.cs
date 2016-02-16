
using System;
using System.Collections.Generic;
using System.Linq;

namespace TogglDesktop.Experiments
{
    sealed class ExperimentManager
    {
        // this is where the currently supported experiments are defined
        private readonly IExperiment[] experiments =
            {
                new Experiment87(),
                new Experiment98(),
                new Experiment99(),
                new Experiment101(),
            };

        private readonly MainWindow mainWindow;

        public ExperimentManager(MainWindow mainWindow)
        {
            this.mainWindow = mainWindow;

            Toggl.OnDisplayObmExperiment += this.onDisplayObmExperiment;

            if (this.experiments.Any(e => e.Id == 0))
            {
                throw new Exception("Experiment with id 0 is invalid.");
            }

            if (this.experiments.GroupBy(e => e.Id).Any(g => g.Count() != 1))
            {
                throw new Exception("Multiple experiments with the same id are invalid.");
            }
        }

        public IEnumerable<ulong> CurrentExperumentIds
        {
            get { return this.experiments.Select(e => e.Id); }
        }

        private void onDisplayObmExperiment(ulong id, bool included, bool seenBefore)
        {
            if (this.mainWindow.TryBeginInvoke(this.onDisplayObmExperiment, id, included, seenBefore))
                return;

            var experiment = this.experiments.FirstOrDefault(e => e.Id == id);

            if (experiment == null)
            {
                Toggl.Debug("Unknown experiment: {0}", id);
                return;
            }

            if (seenBefore && experiment.OnlyRunOnce)
            {
                Toggl.Debug("Already ran experiment: {0}", id);
                return;
            }

            Toggl.Debug("Running experiment: {0} (included: {1})", id, included);
            experiment.Run(
                new ExperimentParameters(included, this.mainWindow.TutorialManager, !seenBefore)
                );
        }
    }
}