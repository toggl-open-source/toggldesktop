using System;
using System.Reactive;
using System.Reactive.Linq;
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
    }

    public class GapTimeEntryBlock : TimelineBlockViewModel
    {
        public ReactiveCommand<Unit, string> CreateTimeEntryFromBlock { get; }

        public GapTimeEntryBlock(Func<double, double, string> addNewTimeEntry)
        {
            CreateTimeEntryFromBlock = ReactiveCommand.Create(() => addNewTimeEntry(VerticalOffset, Height));
        }
    }

    public class TimeEntryBlock : TimelineBlockViewModel
    {
        [Reactive]
        public string Color { get; set; }
        [Reactive]
        public bool ShowDescription { get; set; }
        [Reactive]
        public string Description { get; set; }
        [Reactive]
        public string ProjectName { get; set; }
        [Reactive]
        public string ClientName { get; set; }
        public string TaskName { get; set; }
        [Reactive]
        public ulong Started { get; set; } = 0;

        [Reactive]
        public ulong Ended { get; set; } = 0;
        [Reactive]
        public bool HasTag { get; set; }
        [Reactive]
        public bool IsBillable { get; set; }
        public string Duration { [ObservableAsProperty]get; }
        public string StartEndCaption { [ObservableAsProperty]get; }
        public ReactiveCommand<Unit, Unit> OpenEditView { get; }
        public string TimeEntryId { get; private set; }

        [Reactive]
        public bool IsEditViewOpened { get; set; }

        public bool IsResizable { [ObservableAsProperty] get; }

        private readonly double _hourHeight;

        public TimeEntryBlock(string timeEntryId, int hourHeight)
        {
            _hourHeight = hourHeight;
            TimeEntryId = timeEntryId;
            OpenEditView = ReactiveCommand.Create(() => Toggl.Edit(TimeEntryId, false, Toggl.Description));
            this.WhenAnyValue(x => x.VerticalOffset)
                .Select(h => TimelineUtils.ConvertOffsetToTime(h, Toggl.DateTimeFromUnix(Started).Date, _hourHeight))
                .Subscribe(next => Started = next);
            this.WhenAnyValue(x => x.Height, x => x.VerticalOffset)
                .Select(h => TimelineUtils.ConvertOffsetToTime(h.Item1 + h.Item2, Toggl.DateTimeFromUnix(Ended).Date, _hourHeight))
                .Subscribe(next => Ended = next);
            this.WhenAnyValue(x => x.Started, x => x.Ended)
                .Select(pair => $"{Toggl.DateTimeFromUnix(pair.Item1):HH:mm tt} - {Toggl.DateTimeFromUnix(pair.Item2):HH:mm tt}")
                .ToPropertyEx(this, x => x.StartEndCaption);
            this.WhenAnyValue(x => x.Started, x => x.Ended)
                .Select(pair =>
                {
                    var (start, end) = pair;
                    var duration = Toggl.DateTimeFromUnix(end).Subtract(Toggl.DateTimeFromUnix(start));
                    return duration.Hours + " h " + duration.Minutes + " min";
                })
                .ToPropertyEx(this, x => x.Duration);
            this.WhenAnyValue(x => x.Height)
                .Select(h => h >= TimelineConstants.MinResizableTimeEntryBlockHeight)
                .ToPropertyEx(this, x => x.IsResizable);
        }

        public void ChangeStartTime()
        {
            Toggl.SetTimeEntryStartTimeStamp(TimeEntryId,
                (long)TimelineUtils.ConvertOffsetToTime(VerticalOffset, Toggl.DateTimeFromUnix(Started).Date, _hourHeight));
        }

        public void ChangeEndTime()
        {
            Toggl.SetTimeEntryEndTimeStamp(TimeEntryId,
                (long)TimelineUtils.ConvertOffsetToTime(VerticalOffset + Height, Toggl.DateTimeFromUnix(Ended).Date, _hourHeight));
        }

        public void ChangeStartEndTime()
        {
            ChangeStartTime();
            ChangeEndTime();
        }
    }
}
