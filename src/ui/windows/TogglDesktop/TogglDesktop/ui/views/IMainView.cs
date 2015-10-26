namespace TogglDesktop
{
    public interface IMainView
    {
        void Activate(bool allowAnimation);
        void Deactivate(bool allowAnimation);

        double MinWidth { get; }
        double MinHeight { get; }
    }
}