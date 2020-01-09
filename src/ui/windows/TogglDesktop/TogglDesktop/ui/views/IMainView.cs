namespace TogglDesktop
{
    public interface IMainView
    {
        void Activate(bool allowAnimation);
        void Deactivate(bool allowAnimation);
        bool HandlesError(string errorMessage);

        double MinWidth { get; }
        double MinHeight { get; }
    }
}