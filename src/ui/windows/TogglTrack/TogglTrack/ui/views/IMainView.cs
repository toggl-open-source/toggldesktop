using System.Windows.Media;

namespace TogglTrack
{
    public interface IMainView
    {
        void Activate(bool allowAnimation);
        void Deactivate(bool allowAnimation);
        bool TryShowErrorInsideView(string errorMessage);

        double MinWidth { get; }
        double MinHeight { get; }
        Brush TitleBarBrush { get; }
    }
}