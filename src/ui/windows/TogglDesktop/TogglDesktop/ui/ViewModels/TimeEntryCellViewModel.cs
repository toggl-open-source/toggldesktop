using ReactiveUI;

namespace TogglDesktop.ViewModels
{
    public class TimeEntryCellViewModel : ReactiveObject
    {
        private bool _isFocused;
        public bool IsFocused
        {
            get => _isFocused;
            set => this.RaiseAndSetIfChanged(ref _isFocused, value);
        }
    }
}