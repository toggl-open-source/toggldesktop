using System;
using System.Collections.Generic;
using System.IO;
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
        private DateTime _lastDateLoaded;

        public TimelineViewModel()
        {
            this.WhenAnyValue(x => x.RecordActivity).ObserveOn(RxApp.TaskpoolScheduler)
                .Subscribe(value => Toggl.SetTimelineRecordingEnabled(value));
            OpenTogglHelpUri = ReactiveCommand.Create(() =>
                Toggl.OpenInBrowser("https://support.toggl.com/en/articles/3836325-toggl-desktop-for-windows"));

            Toggl.TimelineSelectedDate.Subscribe(date => SelectedDate = date);

            this.WhenAnyValue(x => x.SelectedDate)
                .Where(date => date != Toggl.TimelineSelectedDate.Value)
                .ObserveOn(RxApp.TaskpoolScheduler)
                .Subscribe(date => Toggl.SetViewTimelineDay(Toggl.UnixFromDateTime(date)));

            this.WhenAnyValue(x => x.SelectedDate)
                .ObserveOn(RxApp.TaskpoolScheduler)
                .Subscribe(LoadMoreIfNeeded);

            this.WhenAnyValue(x => x.SelectedDate)
                .ObserveOn(RxApp.TaskpoolScheduler)
                .Subscribe(_ => HideEditViewIfNeeded());

            Toggl.TimelineSelectedDate
                .Select(dateTime => dateTime.Date == DateTime.Today.Date)
                .ToPropertyEx(this, x => x.IsTodaySelected);

            SelectPreviousDay = ReactiveCommand.Create(Toggl.ViewTimelinePreviousDay);
            SelectNextDay = ReactiveCommand.Create(Toggl.ViewTimelineNextDay);
            IncreaseScale = ReactiveCommand.Create(() => SelectedScaleMode = ChangeScaleMode(-1));
            DecreaseScale = ReactiveCommand.Create(() => SelectedScaleMode = ChangeScaleMode(1));
            var scaleModeObservable = this.WhenAnyValue(x => x.SelectedScaleMode);
            scaleModeObservable.Subscribe(_ =>
                HourHeightView = ScaleModes[SelectedScaleMode] * GetHoursInLine(SelectedScaleMode));
            scaleModeObservable.Select(GetHoursListFromScale).ToPropertyEx(this, x => x.HourViews);
            scaleModeObservable.Select(mode => ConvertTimeIntervalToHeight(DateTime.Today, DateTime.Now, mode))
                .Subscribe(h => CurrentTimeOffset = h);

            Toggl.TimelineChunks.CombineLatest(scaleModeObservable, ConvertChunksToActivityBlocks)
                .ToPropertyEx(this, x => x.ActivityBlocks);
            Toggl.TimelineTimeEntries.CombineLatest(scaleModeObservable, ConvertTimeEntriesToBlocks)
                .ToPropertyEx(this, x => x.TimeEntryBlocks);
            Toggl.TimelineTimeEntries.CombineLatest(scaleModeObservable, GenerateGapTimeEntryBlocks)
                .ToPropertyEx(this, x => x.GapTimeEntryBlocks);

            this.WhenAnyValue(x => x.TimeEntryBlocks)
                .Where(blocks => blocks != null && blocks.Any())
                .Select(blocks => blocks.Min(te => te.VerticalOffset))
                .ToPropertyEx(this, x => x.FirstTimeEntryOffset);

            Toggl.OnTimeEntryList += HandleTimeEntryListChanged;
            Toggl.OnTimeEntryEditor += (open, te, field) =>
                SelectedForEditTEId = open ? te.GUID : SelectedForEditTEId;
            this.WhenAnyValue(x => x.SelectedForEditTEId, x => x.TimeEntryBlocks)
                .ObserveOn(RxApp.TaskpoolScheduler).Subscribe(_ =>
                TimeEntryBlocks?.ForEach(te => te.IsEditViewOpened = SelectedForEditTEId == te.TimeEntryId));
            Observable.Timer(TimeSpan.Zero,TimeSpan.FromMinutes(1))
                .Select(_ => ConvertTimeIntervalToHeight(DateTime.Today, DateTime.Now, SelectedScaleMode))
                .Subscribe(h => CurrentTimeOffset = h);
        }

        private int ChangeScaleMode(int value) =>
            SelectedScaleMode + value < 0 || SelectedScaleMode + value > ScaleModes.Count - 1
                ? SelectedScaleMode
                : SelectedScaleMode + value;

        private static List<DateTime> GetHoursListFromScale(int scale)
        {
            int inc = GetHoursInLine(scale);
            var hourViews = new List<DateTime>();
            for (int i = 0; i < 24; i+=inc)
            {
                hourViews.Add(new DateTime(1, 1, 1, i, 0, 0));
            }

            return hourViews;
        }

        private static int GetHoursInLine(int scaleMode) => scaleMode != 3 ? 1 : 2;

        private void LoadMoreIfNeeded(DateTime date)
        {
            if (date < _lastDateLoaded)
            {
                Toggl.LoadMore();
            }
        }

        private void HideEditViewIfNeeded()
        {
            if (SelectedForEditTEId != null)
                Toggl.Edit(SelectedForEditTEId, false, "");
        }

        private void HandleTimeEntryListChanged(bool open, List<Toggl.TogglTimeEntryView> timeEntries, bool showLoadMore)
        {
            Task.Run(() => Toggl.SetViewTimelineDay(Toggl.UnixFromDateTime(SelectedDate))); //called in case if user changed some of the selected date TEs
            if (_lastDateLoaded == default)
            {
                _lastDateLoaded = timeEntries.Any() 
                    ? Toggl.DateTimeFromUnix(timeEntries.Select(te => te.Started).Min())
                    : DateTime.Today;
            }
        }

        private const int MaxActivityBlockDurationInSec = 900;
        private static List<ActivityBlock> ConvertChunksToActivityBlocks(List<Toggl.TimelineChunkView> chunks, int selectedScaleMode)
        {
            var blocks = new List<ActivityBlock>();
            foreach (var chunk in chunks)
            {
                if (chunk.Events.Any())
                {
                    var start = Toggl.DateTimeFromUnix(chunk.Started);
                    var block = new ActivityBlock()
                    {
                        Offset = ConvertTimeIntervalToHeight(new DateTime(start.Year, start.Month, start.Day), start, selectedScaleMode),
                        TimeInterval = chunk.StartTimeString+" - "+chunk.EndTimeString,
                        ActivityDescriptions = new List<ActivityDescription>()
                    };
                    long duration = 0;
                    foreach (var eventDesc in chunk.Events)
                    {
                        var activity = new ActivityDescription()
                        {
                            ActivityTitle = eventDesc.DurationString + " " + (eventDesc.Header ? Path.GetFileName(eventDesc.Filename) : eventDesc.Title)
                        };
                        activity.SubActivities = eventDesc.Header
                            ? eventDesc.SubEvents.Select(e => e.DurationString + " " + e.Title).ToList()
                            : new List<string>() {eventDesc.DurationString + " " + eventDesc.Title}; 
                        block.ActivityDescriptions.Add(activity);
                        duration += eventDesc.Duration;
                    }
                    block.Height = (1.0 * Math.Min(duration, MaxActivityBlockDurationInSec) * ScaleModes[selectedScaleMode]) / (60 * 60);
                    if (block.ActivityDescriptions.Any())
                        blocks.Add(block);
                }
            }
            return blocks;
        }

        private enum TimeStampType
        {
            Start,
            End,
            Empty
        }
        private static List<TimeEntryBlock> ConvertTimeEntriesToBlocks(List<Toggl.TogglTimeEntryView> timeEntries, int selectedScaleMode)
        {
            var timeStampsList = new List<(TimeStampType Type, TimeEntryBlock Block)>();
            var blocks = new List<TimeEntryBlock>();
            //The idea is to place all the starts and ends in sorted order and then assign an offset to each time entry block from the list:
            // - if it's a start time stamp, then pick up the minimum available offset, if none is available assign a new one.
            // - if it's an end time stamp, then release the offset which it occupied.
            foreach (var entry in timeEntries)
            {
                var startTime = Toggl.DateTimeFromUnix(entry.Started);
                var height = ConvertTimeIntervalToHeight(startTime, Toggl.DateTimeFromUnix(entry.Ended), selectedScaleMode);
                var block = new TimeEntryBlock(entry.GUID)
                {
                    Height = height < 2 ? 2 : height,
                    VerticalOffset = ConvertTimeIntervalToHeight(new DateTime(startTime.Year, startTime.Month, startTime.Day), startTime, selectedScaleMode),
                    Color = entry.Color,
                    Description = entry.Description.IsNullOrEmpty() ? "No Description" : entry.Description,
                    ProjectName = entry.ProjectLabel,
                    ClientName = entry.ClientLabel,
                    ShowDescription = true,
                    Started = entry.Started,
                    Ended = entry.Ended,
                    HasTag = !entry.Tags.IsNullOrEmpty(),
                    IsBillable = entry.Billable,
                    Duration = entry.DateDuration,
                    StartEndCaption = entry.StartTimeString + " - " + entry.EndTimeString
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
            TimeEntryBlock prevLayerBlock = null;
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
                    {
                        item.Block.ShowDescription = false;
                        if (prevLayerBlock != null)
                            prevLayerBlock.ShowDescription = false;
                    }
                    item.Block.HorizontalOffset = offsets.Min();
                    offsets.Remove(offsets.Min());
                    usedNumOfOffsets++;
                    prevLayerBlock = item.Block;
                }
                if (item.Type == TimeStampType.End || item.Type == TimeStampType.Empty)
                {
                    offsets.Add(item.Block.HorizontalOffset);
                    usedNumOfOffsets--;
                    prevLayerBlock = null;
                }
            }

            return blocks;
        }

        public static double ConvertTimeIntervalToHeight(DateTime start, DateTime end, int scaleMode)
        {
            var timeInterval = (end - start).TotalMinutes;
            return timeInterval * ScaleModes[scaleMode] / 60;
        }

        private static List<TimeEntryBlock> GenerateGapTimeEntryBlocks(List<Toggl.TogglTimeEntryView> timeEntries, int selectedScaleMode)
        {
            var gaps = new List<TimeEntryBlock>();
            timeEntries.Sort((te1,te2) => te1.Started.CompareTo(te2.Started));
            ulong? prevEnd = null;
            foreach (var entry in timeEntries)
            {
                if (prevEnd != null && entry.Started > prevEnd.Value)
                {
                    var start = Toggl.DateTimeFromUnix(prevEnd.Value+1);
                    var block = new TimeEntryBlock()
                    {
                        Height = ConvertTimeIntervalToHeight(start, Toggl.DateTimeFromUnix(entry.Started - 1), selectedScaleMode),
                        VerticalOffset =
                            ConvertTimeIntervalToHeight(new DateTime(start.Year, start.Month, start.Day), start, selectedScaleMode),
                        HorizontalOffset = 0,
                        Started = prevEnd.Value + 1,
                        Ended = entry.Started - 1
                    };
                    if (block.Height > 10) // Don't display to small gaps not to obstruct the view
                        gaps.Add(block);
                }
                prevEnd = !prevEnd.HasValue || entry.Ended > prevEnd ? entry.Ended : prevEnd;
            }
            
            return gaps;
        }

        public static IReadOnlyDictionary<int, int> ScaleModes { get; } = new Dictionary<int, int>()
        {
            {0, 200},
            {1, 100},
            {2, 50},
            {3, 25}
        };

        [Reactive] 
        public int SelectedScaleMode { get; private set; } = 0;
        [Reactive] 
        public int HourHeightView { get; private set; }
        [Reactive] 
        public bool RecordActivity { get; set; } = Toggl.IsTimelineRecordingEnabled();

        public ReactiveCommand<Unit, Unit> OpenTogglHelpUri { get; }

        [Reactive]
        public DateTime SelectedDate { get; set; }

        public ReactiveCommand<Unit,Unit> SelectPreviousDay { get; }

        public ReactiveCommand<Unit, Unit> SelectNextDay { get; }

        public List<DateTime> HourViews { [ObservableAsProperty] get;  }
        
        public List<ActivityBlock> ActivityBlocks { [ObservableAsProperty] get; }

        [Reactive]
        public ActivityBlock SelectedActivityBlock { get; set; }

        [Reactive]
        public TimeEntryBlock SelectedTimeEntryBlock { get; set; }
        
        public List<TimeEntryBlock> TimeEntryBlocks { [ObservableAsProperty] get; }
        
        public List<TimeEntryBlock> GapTimeEntryBlocks { [ObservableAsProperty] get; }

        [Reactive]
        public string SelectedForEditTEId { get; set; }
        public ReactiveCommand<Unit, int> IncreaseScale { get; }
        public ReactiveCommand<Unit, int> DecreaseScale { get; }

        public double FirstTimeEntryOffset { [ObservableAsProperty] get; }

        [Reactive]
        public double CurrentTimeOffset { get; set; }

        public bool IsTodaySelected { [ObservableAsProperty] get; }

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

    public class TimeEntryBlock : ReactiveObject
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
        public bool HasTag { get; set; }
        public bool IsBillable { get; set; }
        public string Duration { get; set; }
        public string StartEndCaption { get; set; }
        public ReactiveCommand<Unit, Unit> CreateTimeEntryFromBlock { get; }
        public ReactiveCommand<Unit,Unit> OpenEditView { get; }
        public string TimeEntryId { get; private set; }

        [Reactive]
        public bool IsEditViewOpened { get; set; }

        public TimeEntryBlock(string timeEntryId)
        {
            TimeEntryId = timeEntryId;
            CreateTimeEntryFromBlock = ReactiveCommand.Create(() => AddNewTimeEntry());
            OpenEditView = ReactiveCommand.Create(() => Toggl.Edit(TimeEntryId, false, Toggl.Description));
            CreateTimeEntryFromBlock = ReactiveCommand.Create(AddNewTimeEntry);
        }

        public TimeEntryBlock() : this(null) { }

        private void AddNewTimeEntry()
        {
            TimeEntryId = Toggl.CreateEmptyTimeEntry(Started, Ended);
            OpenEditView.Execute().Subscribe();
        }
    }
}
