using System;
using DynamicData.Binding;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace TogglDesktop.ViewModels
{
    public class TimerEntryListViewViewModel : ReactiveObject
    {
        public TimerEntryListViewViewModel()
        {
            this.WhenValueChanged(x => SelectedTab)
                .Subscribe(value => Toggl.SetActiveTab(value));
            Toggl.OnDisplayTimelineUI += isEnabled => IsTimelineViewEnabled = isEnabled;
        }

        [Reactive] 
        public byte SelectedTab { get; set; } = Toggl.GetActiveTab();

        [Reactive]
        public bool IsTimelineViewEnabled { get; private set; }

        public TimelineViewModel TimelineViewModel { get; } = new TimelineViewModel();
    }
}
