using System;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public static class TimelineUtils
    {
        public static ulong ConvertOffsetToTime(double height, DateTime date, double hourHeight)
        {
            var hours = 1.0 * height / hourHeight;
            var dateTime = date.AddHours(hours);
            var unixTime = Toggl.UnixFromDateTime(dateTime);
            return unixTime >= 0 ? (ulong)unixTime : 0;
        }

        public static DateTime StartTime(this TimeEntryBlock block) => Toggl.DateTimeFromUnix(block.Started);

        public static DateTime EndTime(this TimeEntryBlock block) => Toggl.DateTimeFromUnix(block.Ended);

        public static DateTime StartTime(this Toggl.TogglTimeEntryView te) => Toggl.DateTimeFromUnix(te.Started);

        public static DateTime EndTime(this Toggl.TogglTimeEntryView te) => Toggl.DateTimeFromUnix(te.Ended);

        public static DateTime StartTime(this Toggl.TimelineChunkView chunk) => Toggl.DateTimeFromUnix(chunk.Started);

        public static DateTime EndTime(this Toggl.TimelineChunkView chunk) => Toggl.DateTimeFromUnix(chunk.Ended);
    }
}
