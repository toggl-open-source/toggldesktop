using System;
using System.Reactive;
using System.Threading.Tasks;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace TogglDesktop.ViewModels
{
    public class TimelineViewModel : ReactiveObject
    {
        public TimelineViewModel()
        {
            this.WhenAnyValue(x => x.RecordActivity).Subscribe(x => SaveRecordActivity());
            OpenTogglHelpUri = ReactiveCommand.Create(() =>
                Toggl.OpenInBrowser("https://support.toggl.com/en/articles/3836325-toggl-desktop-for-windows"));
        }

        [Reactive] 
        public bool RecordActivity { get; set; } = Toggl.IsTimelineRecordingEnabled();

        private void SaveRecordActivity()
        {
            Task.Run(() => Toggl.SetTimelineRecordingEnabled(RecordActivity));
        }

        public ReactiveCommand<Unit, Unit> OpenTogglHelpUri { get; set; }
    }
}
