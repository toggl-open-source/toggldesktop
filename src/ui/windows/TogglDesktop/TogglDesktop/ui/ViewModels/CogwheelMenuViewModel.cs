using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace TogglDesktop.ViewModels
{
    public class CogwheelMenuViewModel : ReactiveObject
    {
        [Reactive]
        public bool IsInManualMode { get; set; }

        [Reactive]
        public bool IsMiniTimerUsed { get; set; }

        [Reactive]
        public string Email { get; set; }
    }
}
