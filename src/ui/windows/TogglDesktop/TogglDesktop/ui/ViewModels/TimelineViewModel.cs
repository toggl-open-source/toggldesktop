using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace TogglDesktop.ViewModels
{
    public class TimelineViewModel : ReactiveObject
    {
        private DateTime _lastDateLoaded;
        private readonly Dictionary<int, int> _scaleModes = new Dictionary<int, int>()
        {
            {0, 200},
            {1, 100},
            {2, 50},
            {3, 25}
        };

        private List<Toggl.TimelineChunkView> _timelineChunks;
        private List<Toggl.TogglTimeEntryView> _timeEntries;

        public TimelineViewModel()
        {
            this.WhenAnyValue(x => x.RecordActivity).ObserveOn(RxApp.TaskpoolScheduler)
                .Subscribe(value => Toggl.SetTimelineRecordingEnabled(value));
            OpenTogglHelpUri = ReactiveCommand.Create(() =>
                Toggl.OpenInBrowser("https://support.toggl.com/en/articles/3836325-toggl-desktop-for-windows"));
            this.WhenAnyValue(x => x.SelectedDate).Subscribe(HandleSelectedDateChanged);
            SelectPreviousDay = ReactiveCommand.Create(Toggl.ViewTimelinePreviousDay);
            SelectNextDay = ReactiveCommand.Create(Toggl.ViewTimelineNextDay);
            IncreaseScale = ReactiveCommand.Create(() => ChangeScale(-1));
            DecreaseScale = ReactiveCommand.Create(() => ChangeScale(1));
            this.WhenAnyValue(x => x.SelectedScaleMode).Subscribe(_ =>
                HourHeightView = _scaleModes[SelectedScaleMode] * GetHoursInLine(SelectedScaleMode));
            Toggl.OnTimeline += HandleDisplayTimeline;
            Toggl.OnTimeEntryList += HandleTimeEntryListChanged;
            UpdateHourViews();
        }

        private void ChangeScale(int value)
        {
            var newSelectedScaleMode = SelectedScaleMode + value < 0 || SelectedScaleMode + value > _scaleModes.Count - 1
                ? SelectedScaleMode
                : SelectedScaleMode + value;
            ScaleRatio = 1.0*_scaleModes[newSelectedScaleMode] / _scaleModes[SelectedScaleMode];
            SelectedScaleMode = newSelectedScaleMode;
            HandleScaleChanged(SelectedScaleMode);
        }

        private void UpdateHourViews()
        {
            int inc = GetHoursInLine(SelectedScaleMode);
            var hourViews = new List<DateTime>();
            for (int i = 0; i < 24; i+=inc)
            {
                hourViews.Add(new DateTime(1, 1, 1, i, 0, 0));
            }

            HourViews = null;
            HourViews = hourViews;
        }

        private int GetHoursInLine(int scaleMode) => scaleMode != 3 ? 1 : 2;

        private void HandleSelectedDateChanged(DateTime date)
        {
            if (date < _lastDateLoaded)
            {
                Toggl.LoadMore();
            }
            Toggl.SetViewTimelineDay(Toggl.UnixFromDateTime(SelectedDate));
        }

        private void HandleDisplayTimeline(bool open, string date, List<Toggl.TimelineChunkView> first, List<Toggl.TogglTimeEntryView> firstTimeEntry, ulong startDay, ulong endDay)
        {
            SelectedDate = Toggl.DateTimeFromUnix(startDay);
            _timeEntries = firstTimeEntry;
            _timelineChunks = first;
            ConvertChunksToActivityBlocks(first);
            ConvertTimeEntriesToBlocks(firstTimeEntry);
        }

        private void HandleTimeEntryListChanged(bool open, List<Toggl.TogglTimeEntryView> timeEntries, bool showLoadMore)
        {
            Toggl.SetViewTimelineDay(Toggl.UnixFromDateTime(SelectedDate)); //called in case if user changed some of the selected date TEs
            if (_lastDateLoaded == default)
            {
                _lastDateLoaded = timeEntries.Any() 
                    ? Toggl.DateTimeFromUnix(timeEntries.Select(te => te.Started).Min())
                    : DateTime.Today;
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
                    long duration = 0;
                    foreach (var eventDesc in chunk.Events)
                    {
                        var activity = new ActivityDescription()
                        {
                            ActivityTitle = eventDesc.Header ? Path.GetFileName(eventDesc.Filename) : eventDesc.Title
                        };
                        activity.SubActivities = eventDesc.Header
                            ? eventDesc.SubEvents.Select(e => e.DurationString + " " + e.Title).ToList()
                            : new List<string>() {eventDesc.DurationString + " " + eventDesc.Title}; 
                        block.ActivityDescriptions.Add(activity);
                        duration += eventDesc.Duration;
                    }
                    block.Height = (1.0 * duration * _scaleModes[SelectedScaleMode]) / (60 * 60); ;
                    if (block.ActivityDescriptions.Any())
                        blocks.Add(block);
                }
            }
            ActivityBlocks = blocks;
        }

        private enum TimeStampType
        {
            Start,
            End,
            Empty
        }
        private void ConvertTimeEntriesToBlocks(List<Toggl.TogglTimeEntryView> timeEntries)
        {
            var timeStampsList = new List<(TimeStampType Type, TimeEntryBlock Block)>();
            var blocks = new List<TimeEntryBlock>();
            //The idea is to place all the starts and ends in sorted order and then assign an offset to each time entry block from the list:
            // - if it's a start time stamp, then pick up the minimum available offset, if none is available assign a new one.
            // - if it's an end time stamp, then release the offset which it occupied.
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
                    ShowDescription = true,
                    Started = entry.Started,
                    Ended = entry.Ended
                };
                if (entry.Started != entry.Ended)
                {
                    timeStampsList.Add((TimeStampType.Start, block));
                    timeStampsList.Add((TimeStampType.End, block));
                }
                else
                {
                    timeStampsList.Add((TimeStampType.Empty, block));
                }
                blocks.Add(block);
            }
            //There can be a situation that next time entry starts exactly at the same moment, the previous one ended.
            //This situation must not be considered as overlap. So the comparison logic if time stamps are the same:
            // - always place the end time stamps first
            // - prefer empty time stamps to start time stamps
            // (otherwise if we discover a start then an empty, this will be considered as overlap, which we want to avoid)
            timeStampsList.Sort((te1, te2) =>
            {
                var time1 = te1.Type == TimeStampType.End ? te1.Block.Ended : te1.Block.Started;
                var time2 = te2.Type == TimeStampType.End ? te2.Block.Ended : te2.Block.Started;
                var res = time1.CompareTo(time2);
                if (res == 0)
                {
                    var getPriority = new Func<TimeStampType, int>(t =>
                        t == TimeStampType.End ? 0 : t == TimeStampType.Empty ? 1 : 2);
                    return getPriority(te1.Type) - getPriority(te2.Type);
                }
                return res;
            });
            var offsets = new HashSet<double>();
            var curOffset = 0;
            var usedNumOfOffsets = 0;
            foreach (var item in timeStampsList)
            {
                if (item.Type == TimeStampType.Start || item.Type == TimeStampType.Empty)
                {
                    if (!offsets.Any())
                    {
                        offsets.Add(curOffset);
                        curOffset += 25;
                    }
                    if (usedNumOfOffsets > 0 || item.Block.Height < 20)
                        item.Block.ShowDescription = false;
                    item.Block.HorizontalOffset = offsets.Min();
                    offsets.Remove(offsets.Min());
                    usedNumOfOffsets++;
                }
                if (item.Type == TimeStampType.End || item.Type == TimeStampType.Empty)
                {
                    offsets.Add(item.Block.HorizontalOffset);
                    if (usedNumOfOffsets > 1 || item.Block.Height < 20)
                        item.Block.ShowDescription = false;
                    usedNumOfOffsets--;
                }
            }
            TimeEntryBlocks = null;
            TimeEntryBlocks = blocks;

            GenerateGapTimeEntryBlocks(timeEntries);
        }

        private double ConvertTimeIntervalToHeight(DateTime start, DateTime end)
        {
            var timeInterval = (end - start).TotalMinutes;
            return timeInterval * _scaleModes[SelectedScaleMode] / 60;
        }

        private void GenerateGapTimeEntryBlocks(List<Toggl.TogglTimeEntryView> timeEntries)
        {
            var gaps = new List<TimeEntryBlock>();
            timeEntries.Sort((te1,te2) => te1.Started.CompareTo(te2.Started));
            ulong? prevEnd = null;
            foreach (var entry in timeEntries)
            {
                if (prevEnd != null && entry.Started > prevEnd.Value + 5 *60)
                {
                    var start = Toggl.DateTimeFromUnix(prevEnd.Value+1);
                    gaps.Add(new TimeEntryBlock()
                    {
                        Height = ConvertTimeIntervalToHeight(start, Toggl.DateTimeFromUnix(entry.Started-1)),
                        VerticalOffset = ConvertTimeIntervalToHeight(new DateTime(start.Year, start.Month, start.Day), start),
                        HorizontalOffset = 0,
                        Started = prevEnd.Value+1,
                        Ended = entry.Started-1
                    });
                }
                prevEnd = !prevEnd.HasValue || entry.Ended > prevEnd ? entry.Ended : prevEnd;
            }

            GapTimeEntryBlocks = null;
            GapTimeEntryBlocks = gaps;
        }

        private void HandleScaleChanged(int newScaleMode)
        {
            if (_timelineChunks != null)
                ConvertChunksToActivityBlocks(_timelineChunks);
            if (_timeEntries != null)
                ConvertTimeEntriesToBlocks(_timeEntries);
            UpdateHourViews();
        }

        [Reactive]
        public double ScaleRatio { get; set; }

        [Reactive] 
        public int SelectedScaleMode { get; private set; } = 0;
        [Reactive] 
        public int HourHeightView { get; private set; }
        [Reactive] 
        public bool RecordActivity { get; set; } = Toggl.IsTimelineRecordingEnabled();

        public ReactiveCommand<Unit, Unit> OpenTogglHelpUri { get; }

        [Reactive]
        public DateTime SelectedDate { get; set; } = DateTime.Today;

        public ReactiveCommand<Unit,Unit> SelectPreviousDay { get; }

        public ReactiveCommand<Unit, Unit> SelectNextDay { get; }

        [Reactive]
        public List<DateTime> HourViews { get; private set; }

        [Reactive]
        public List<ActivityBlock> ActivityBlocks { get; private set; }

        [Reactive]
        public ActivityBlock SelectedActivityBlock { get; set; }

        [Reactive]
        public List<TimeEntryBlock> TimeEntryBlocks { get; private set; }

        [Reactive]
        public List<TimeEntryBlock> GapTimeEntryBlocks { get; private set; }

        public ReactiveCommand<Unit, Unit> IncreaseScale { get; }
        public ReactiveCommand<Unit, Unit> DecreaseScale { get; }

        public class ActivityBlock
        {
            public double Offset { get; set; }
            public string TimeInterval { get; set; }
            public double Height { get; set; }
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
        public ulong Started { get; set; }
        public ulong Ended { get; set; }
        public ReactiveCommand<Unit, Unit> CreateTimeEntryFromBlock { get; }

        public TimeEntryBlock()
        {
            CreateTimeEntryFromBlock = ReactiveCommand.Create(AddNewTimeEntry);
        }

        private void AddNewTimeEntry()
        {
            Toggl.CreateEmptyTimeEntry(Started, Ended);
        }
    }
}
