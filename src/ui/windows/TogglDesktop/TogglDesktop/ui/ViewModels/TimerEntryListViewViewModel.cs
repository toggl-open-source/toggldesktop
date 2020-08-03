using ReactiveUI;

namespace TogglDesktop.ViewModels
{
    public class TimerEntryListViewViewModel : ReactiveObject
    {
        private byte? _selectedTab;
        public byte SelectedTab
        {
            get
            {
                _selectedTab ??= Toggl.GetActiveTab();
                return _selectedTab.Value;
            }
            set
            {
                this.RaiseAndSetIfChanged(ref _selectedTab, value);
                Toggl.SetActiveTab(SelectedTab);
            }
        }

        public bool IsTimelineViewEnabled => Toggl.IsTimelineUiEnabled();
    }
}
