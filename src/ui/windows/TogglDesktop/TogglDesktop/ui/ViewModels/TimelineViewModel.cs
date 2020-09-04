using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using Priority_Queue;
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
            Toggl.OnTimeEntryList += HandleTimeEntryListChanged; 

            HourViews = new List<DateTime>();
            for (int i = 0; i < 24; i++)
            {
                HourViews.Add(new DateTime(1, 1, 1, i, 0, 0));
            }
        }

        private void HandleDisplayTimeline(bool open, string date, List<Toggl.TimelineChunkView> first, List<Toggl.TogglTimeEntryView> firstTimeEntry, ulong startDay, ulong endDay)
        {
            SelectedDate = Toggl.DateTimeFromUnix(startDay);
            ConvertChunksToActivityBlocks(first);
            ConvertTimeEntriesToBlocks(firstTimeEntry);
        }

        private void HandleTimeEntryListChanged(bool open, List<Toggl.TogglTimeEntryView> timeEntries, bool showLoadMore)
        {
            List<Toggl.TogglTimeEntryView> selectedDateTEs = new List<Toggl.TogglTimeEntryView>();
            foreach (var entry in timeEntries)
            {
                var startDate = Toggl.DateTimeFromUnix(entry.Started);
                if (startDate.Date == SelectedDate)
                    selectedDateTEs.Add(entry);
            }
            if (selectedDateTEs.Any())
            {
                ConvertTimeEntriesToBlocks(selectedDateTEs);
            }
        }

        private void ConvertChunksToActivityBlocks(List<Toggl.TimelineChunkView> chunks)
        {
            var blocks = new List<ActivityBlock>();
            foreach (var chunk in chunks)
            {
                if (chunk.Events.Any())
                {
                    var start = Toggl.DateTimeFromUnix(chunk.Started);
                    var block = new ActivityBlock()
                    {
                        Offset = ConvertTimeIntervalToHeight(new DateTime(start.Year, start.Month, start.Day), start),
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

        private void ConvertTimeEntriesToBlocks(List<Toggl.TogglTimeEntryView> timeEntries)
        {
            var queue = new SimplePriorityQueue<(bool IsEnd, TimeEntryBlock Block), ulong>();
            var blocks = new List<TimeEntryBlock>();
            foreach (var entry in timeEntries)
            {
                var startTime = Toggl.DateTimeFromUnix(entry.Started);
                var height = ConvertTimeIntervalToHeight(startTime, Toggl.DateTimeFromUnix(entry.Ended));
                var block = new TimeEntryBlock()
                {
                    Height = height < 2 ? 2 : height,
                    VerticalOffset = ConvertTimeIntervalToHeight(new DateTime(startTime.Year, startTime.Month, startTime.Day), startTime),
                    Color = entry.Color,
                    Description = entry.Description,
                    ProjectName = entry.ProjectLabel,
                    ClientName = entry.ClientLabel,
                    ShowDescription = true
                };
                queue.Enqueue((false, block), entry.Started);
                queue.Enqueue((true, block), entry.Ended);
                blocks.Add(block);
            }

            var offsets = new HashSet<double>();
            var curOffset = 0;
            var usedNumOfOffsets = 0;
            while (queue.Count>0)
            {
                var item = queue.Dequeue();
                if (item.IsEnd)
                {
                    offsets.Add(item.Block.HorizontalOffset);
                    if (usedNumOfOffsets > 1 || item.Block.Height<20)
                        item.Block.ShowDescription = false;
                    usedNumOfOffsets--;
                }
                else
                {
                    if (!offsets.Any())
                    {
                        offsets.Add(curOffset);
                        curOffset += 20;
                    }
                    if (usedNumOfOffsets > 0 || item.Block.Height < 20)
                        item.Block.ShowDescription = false;
                    item.Block.HorizontalOffset = offsets.Min();
                    offsets.Remove(offsets.Min());
                    usedNumOfOffsets++;
                }
            }

            TimeEntryBlocks = blocks;
        }

        private double ConvertTimeIntervalToHeight(DateTime start, DateTime end)
        {
            var timeInterval = (end - start).TotalMinutes;
            return timeInterval * _hourHeight / 60;
        }

        private static int _hourHeight = 200;

        [Reactive] 
        public bool RecordActivity { get; set; } = Toggl.IsTimelineRecordingEnabled();

        public ReactiveCommand<Unit, Unit> OpenTogglHelpUri { get; }

        [Reactive]
        public DateTime SelectedDate { get; set; } = DateTime.Today;

        public ReactiveCommand<Unit,Unit> SelectPreviousDay { get; }

        public ReactiveCommand<Unit, Unit> SelectNextDay { get; }

        public List<DateTime> HourViews { get; }

        [Reactive]
        public List<ActivityBlock> ActivityBlocks { get; private set; }

        [Reactive]
        public ActivityBlock SelectedActivityBlock { get; set; }

        [Reactive]
        public List<TimeEntryBlock> TimeEntryBlocks { get; private set; }

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

    public class TimeEntryBlock
    {
        public double VerticalOffset { get; set; }
        public double HorizontalOffset { get; set; }
        public double Height { get; set; }
        public string Color { get; set; }
        public bool ShowDescription { get; set; }
        public string Description { get; set; }
        public string ProjectName { get; set; }
        public string ClientName { get; set; }
    }
}
