using System;
using System.Reactive;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using TogglDesktop.Resources;

namespace TogglDesktop.ViewModels
{
    public class TimelineBlockViewModel : ReactiveObject
    {
        [Reactive]
        public double HorizontalOffset { get; set; }
        [Reactive]
        public double Height { get; set; }
        [Reactive]
        public double VerticalOffset { get; set; }

        public double Bottom => VerticalOffset + Height;
    }

    public class GapTimeEntryBlock : TimelineBlockViewModel
    {
        public ReactiveCommand<Unit, string> CreateTimeEntryFromBlock { get; }

        public GapTimeEntryBlock(Func<double, double, string> addNewTimeEntry)
        {
            CreateTimeEntryFromBlock = ReactiveCommand.Create(() =>
            {
                var id = addNewTimeEntry(VerticalOffset, Height);
                Toggl.Edit(id, false, Toggl.Description);
                return id;
            });
        }
    }

    public class TimeEntryBlock : TimelineBlockViewModel
    {
        [Reactive]
        public bool IsOverlapping { get; set; }
        public bool ShowDescription { [ObservableAsProperty] get; }
        public string Color => _timeEntry.Color;
        public string Description => _timeEntry.Description.IsNullOrEmpty() ? "No Description" : _timeEntry.Description;
        public string ProjectName => _timeEntry.ProjectLabel;
        public string ClientName => _timeEntry.ClientLabel;
        public string TaskName => _timeEntry.TaskLabel;
        public bool HasTag => !_timeEntry.Tags.IsNullOrEmpty();
        public bool IsBillable => _timeEntry.Billable;
        public long DurationInSeconds => _timeEntry.DurationInSeconds;
        public string Duration { [ObservableAsProperty]get; }
        public string StartEndCaption { [ObservableAsProperty]get; }
        public ReactiveCommand<Unit, Unit> OpenEditView { get; }
        public string TimeEntryId => _timeEntry.GUID;

        [Reactive]
        public bool IsEditViewOpened { get; set; }

        public bool IsResizable { [ObservableAsProperty] get; }

        [Reactive]
        public bool IsDragged { get; set; }

        public ulong Started => _timeEntry.Started;
        public ulong Ended => _timeEntry.Ended;

        private DateTime DateCreated { get; }

        private readonly double _hourHeight;
        private readonly Toggl.TogglTimeEntryView _timeEntry;

        public TimeEntryBlock(Toggl.TogglTimeEntryView te, int hourHeight, DateTime date)
        {
            _hourHeight = hourHeight;
            DateCreated = date;
            _timeEntry = te;
            OpenEditView = ReactiveCommand.Create(() => Toggl.Edit(TimeEntryId, false, Toggl.Description));
            var startEndObservable = this.WhenAnyValue(x => x.VerticalOffset, x => x.Height, (offset, height) =>
                (Started: TimelineUtils.ConvertOffsetToDateTime(offset, date, _hourHeight), Ended: TimelineUtils.ConvertOffsetToDateTime(offset + height, date, _hourHeight)));
            startEndObservable.Select(tuple => $"{tuple.Started:HH:mm tt} - {tuple.Ended:HH:mm tt}")
                .ToPropertyEx(this, x => x.StartEndCaption);
            startEndObservable.Select(tuple =>
                {
                    var duration = tuple.Ended.Subtract(tuple.Started);
                    return duration.Hours + " h " + duration.Minutes + " min";
                })
                .ToPropertyEx(this, x => x.Duration);
            this.WhenAnyValue(x => x.Height)
                .Select(h => h >= TimelineConstants.MinResizableTimeEntryBlockHeight)
                .Where(_ => !IsDragged)
                .ToPropertyEx(this, x => x.IsResizable);
            this.WhenAnyValue(x => x.IsOverlapping, x => x.Height,
                (isOverlapping, height) => !isOverlapping && height >= TimelineConstants.MinShowTEDescriptionHeight)
                .ToPropertyEx(this, x => x.ShowDescription);
        }

        public void ChangeStartTime()
        {
            Toggl.SetTimeEntryStartTimeStampWithOption(TimeEntryId,
                (long)TimelineUtils.ConvertOffsetToUnixTime(VerticalOffset, DateCreated, _hourHeight), true);
        }

        public void ChangeEndTime()
        {
            Toggl.SetTimeEntryEndTimeStamp(TimeEntryId,
                (long)TimelineUtils.ConvertOffsetToUnixTime(VerticalOffset + Height, DateCreated, _hourHeight));
        }

        public void ChangeStartEndTime()
        {
            ChangeStartTime();
            ChangeEndTime();
        }
    }
}
