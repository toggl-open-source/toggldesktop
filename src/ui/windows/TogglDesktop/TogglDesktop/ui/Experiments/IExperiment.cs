namespace TogglDesktop.Experiments
{
    interface IExperiment
    {
        int Id { get; }
        bool OnlyRunOnce { get; }
        void Run(ExperimentParameters parameters);
    }
}