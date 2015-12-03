namespace TogglDesktop.Experiments
{
    abstract class ExperimentBase : IExperiment
    {
        protected ExperimentBase(ulong id, bool onlyRunOnce = true)
        {
            this.Id = id;
            this.OnlyRunOnce = onlyRunOnce;
        }

        public ulong Id { get; private set; }
        public bool OnlyRunOnce { get; private set; }

        public void Run(ExperimentParameters parameters)
        {
            if (parameters.Included)
            {
                this.runIncluded(parameters);
            }
            else
            {
                this.runExcluded(parameters);
            }
        }

        protected virtual void runIncluded(ExperimentParameters parameters) { }
        protected virtual void runExcluded(ExperimentParameters parameters) { }
    }
}