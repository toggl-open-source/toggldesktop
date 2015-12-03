namespace TogglDesktop.Experiments
{
    interface IExperiment
    {
        ulong Id { get; }
        bool OnlyRunOnce { get; }
        void Run(ExperimentParameters parameters);
    }
}