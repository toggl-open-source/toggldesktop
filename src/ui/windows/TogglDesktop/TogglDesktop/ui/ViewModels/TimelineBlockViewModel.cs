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
        public bool HasTag { get; set; }
        [Reactive]
        public bool IsBillable { get; set; }
        public string Duration { [ObservableAsProperty]get; }
        public string StartEndCaption { [ObservableAsProperty]get; }
        public ReactiveCommand<Unit, Unit> OpenEditView { get; }
        public string TimeEntryId { get; }

        [Reactive]
        public bool IsEditViewOpened { get; set; }

        public bool IsResizable { [ObservableAsProperty] get; }

        [Reactive]
        public bool IsDragged { get; set; }

        public DateTime DateCreated { get; }

        private readonly double _hourHeight;

        public TimeEntryBlock(string timeEntryId, int hourHeight, DateTime date)
        {
            _hourHeight = hourHeight;
            DateCreated = date;
            TimeEntryId = timeEntryId;
            OpenEditView = ReactiveCommand.Create(() => Toggl.Edit(TimeEntryId, false, Toggl.Description));
            this.WhenAnyValue(x => x.VerticalOffset, x => x.Height,
                    (offset, height) => 
                        $"{TimelineUtils.ConvertOffsetToDateTime(offset, date, _hourHeight):HH:mm tt} - {TimelineUtils.ConvertOffsetToDateTime(offset+height, date, _hourHeight):HH:mm tt}")
                .ToPropertyEx(this, x => x.StartEndCaption);
            this.WhenAnyValue(x => x.VerticalOffset, x => x.Height, (offset, height) =>
                {
                    var duration = TimelineUtils.ConvertOffsetToDateTime(offset+height, date, _hourHeight)
                        .Subtract(TimelineUtils.ConvertOffsetToDateTime(offset, date, _hourHeight));
                    return duration.Hours + " h " + duration.Minutes + " min";
                })
                .ToPropertyEx(this, x => x.Duration);
            this.WhenAnyValue(x => x.Height)
                .Select(h => h >= TimelineConstants.MinResizableTimeEntryBlockHeight)
                .Where(_ => !IsDragged)
                .ToPropertyEx(this, x => x.IsResizable);
        }

        public void ChangeStartTime()
        {
            Toggl.SetTimeEntryStartTimeStamp(TimeEntryId,
                (long)TimelineUtils.ConvertOffsetToUnixTime(VerticalOffset, DateCreated, _hourHeight));
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
