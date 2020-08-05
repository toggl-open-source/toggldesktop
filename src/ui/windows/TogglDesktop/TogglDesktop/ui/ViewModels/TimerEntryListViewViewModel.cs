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
        }

        [Reactive] 
        public byte SelectedTab { get; set; } = Toggl.GetActiveTab();

        public bool IsTimelineViewEnabled => Toggl.IsTimelineUiEnabled();
    }
}
