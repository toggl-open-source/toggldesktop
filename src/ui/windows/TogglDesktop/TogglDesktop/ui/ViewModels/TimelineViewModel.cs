using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using System.Threading.Tasks;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace TogglDesktop.ViewModels
{
    public class TimelineViewModel : ReactiveObject
    {
        public TimelineViewModel()
        {
            this.WhenAnyValue(x => x.RecordActivity).ObserveOn(RxApp.TaskpoolScheduler)
                .Subscribe(value => Toggl.SetTimelineRecordingEnabled(value));
            OpenTogglHelpUri = ReactiveCommand.Create(() =>
                Toggl.OpenInBrowser("https://support.toggl.com/en/articles/3836325-toggl-desktop-for-windows"));
            this.WhenAnyValue(x => x.SelectedDate).Subscribe(value => 
                Toggl.SetViewTimelineDay(Toggl.UnixFromDateTime(SelectedDate)));
            SelectPreviousDay = ReactiveCommand.Create(Toggl.ViewTimelinePreviousDay);
            SelectNextDay = ReactiveCommand.Create(Toggl.ViewTimelineNextDay);
            Toggl.OnTimeline += HandleDisplayTimeline;

            HourViews = new List<DateTime>();
            for (int i = 0; i < 24; i++)
            {
                HourViews.Add(new DateTime(1, 1, 1, i, 0, 0));
            }
        }

        private void HandleDisplayTimeline(bool open, string date, List<Toggl.TimelineChunkView> first, List<Toggl.TogglTimeEntryView> firstTimeEntry, ulong startDay, ulong endDay)
        {
            SelectedDate = Toggl.DateTimeFromUnix(startDay);
            TimeEntries = firstTimeEntry;
            ConvertChunksToActivityBlocks(first);
        }

        public void ConvertChunksToActivityBlocks(List<Toggl.TimelineChunkView> chunks)
        {
            var blocks = new List<ActivityBlock>();
            foreach (var chunk in chunks)
            {
                if (chunk.Events.Any())
                {
                    var start = Toggl.DateTimeFromUnix(chunk.Started);
                    var timeInterval = (start - new DateTime(start.Year, start.Month, start.Day)).TotalMinutes;
                    var offset = (timeInterval * 24 * 200) / (24 * 60);
                    var block = new ActivityBlock()
                    {
                        Offset = offset,
                        TimeInterval = chunk.StartTimeString+" - "+chunk.EndTimeString,
                        ActivityDescriptions = new List<ActivityDescription>()
                    };
                    foreach (var eventDesc in chunk.Events)
                    {
                        var activity = new ActivityDescription()
                        {
                            SubActivities = new List<string>()
                        };
                        var title = eventDesc.Title;
                        foreach (var subEvent in eventDesc.SubEvents)
                        {
                            if (subEvent.Title.IsNullOrEmpty()) continue;
                            activity.SubActivities.Add(eventDesc.DurationString + " " + subEvent.Title);
                            if (title.IsNullOrEmpty())
                                title = subEvent.Title;
                        }
                        if (!title.IsNullOrEmpty())
                        {
                            activity.ActivityTitle = eventDesc.DurationString + " " + title;
                            block.ActivityDescriptions.Add(activity);
                        }
                    }
                    if (block.ActivityDescriptions.Any())
                        blocks.Add(block);
                }
            }
            ActivityBlocks = blocks;
        }

        [Reactive] 
        public bool RecordActivity { get; set; } = Toggl.IsTimelineRecordingEnabled();

        public ReactiveCommand<Unit, Unit> OpenTogglHelpUri { get; }

        [Reactive]
        public DateTime SelectedDate { get; set; } = DateTime.Today;

        public ReactiveCommand<Unit,Unit> SelectPreviousDay { get; }

        public ReactiveCommand<Unit, Unit> SelectNextDay { get; }

        public List<DateTime> HourViews { get; }

        [Reactive]
        public List<Toggl.TogglTimeEntryView> TimeEntries { get; private set; }

        [Reactive]
        public List<ActivityBlock> ActivityBlocks { get; private set; }

        [Reactive]
        public ActivityBlock SelectedActivityBlock { get; set; }

        public class ActivityBlock
        {
            public double Offset { get; set; }
            public string TimeInterval { get; set; }
            public List<ActivityDescription> ActivityDescriptions { get; set; }
        }

        public class ActivityDescription
        {
            public string ActivityTitle { get; set; }

            public List<string> SubActivities { get; set; }
        }
    }
}
