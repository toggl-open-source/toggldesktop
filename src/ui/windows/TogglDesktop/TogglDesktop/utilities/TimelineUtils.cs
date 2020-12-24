using System;
using System.Collections.Generic;
using System.Linq;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public static class TimelineUtils
    {
        public static ulong ConvertOffsetToUnixTime(double height, DateTime date, double hourHeight)
        {
            var dateTime = ConvertOffsetToDateTime(height, date, hourHeight);
            var unixTime = Toggl.UnixFromDateTime(dateTime);
            return unixTime >= 0 ? (ulong)unixTime : 0;
        }

        public static DateTime ConvertOffsetToDateTime(double height, DateTime date, double hourHeight)
        {
            var hours = 1.0 * height / hourHeight;
            var dateTime = date.AddHours(hours);
            return dateTime;
        }

        public static void CreateAndEditRunningTimeEntryFrom(ulong started)
        {
            var teId = Toggl.Start("", "", 0, 0, "", "");
            Toggl.SetTimeEntryStartTimeStampWithOption(teId, (long)started, true);
            Toggl.Edit(teId, true, Toggl.Description);
        }

        public static void CreateAndEditTimeEntry(ulong started, ulong ended)
        {
            var teId = Toggl.CreateEmptyTimeEntry(started, ended);
            Toggl.Edit(teId, false, Toggl.Description);
        }

        public static DateTime StartTime(this Toggl.TogglTimeEntryView te) => Toggl.DateTimeFromUnix(te.Started);

        public static DateTime EndTime(this Toggl.TogglTimeEntryView te) => Toggl.DateTimeFromUnix(te.Ended);

        public static DateTime StartTime(this Toggl.TimelineChunkView chunk) => Toggl.DateTimeFromUnix(chunk.Started);

        public static DateTime EndTime(this Toggl.TimelineChunkView chunk) => Toggl.DateTimeFromUnix(chunk.Ended);

        public static bool IsOverlappingWith(this TimeEntryBlock first, TimeEntryBlock second) =>
            first.Started < second.Ended && second.Started < first.Ended;

        public static double? FirstTimeEntryVerticalOffset(this IDictionary<string, TimeEntryBlock> blocks) =>
            blocks == null || !blocks.Any()
                ? (double?) null
                : blocks.Min(te => te.Value.VerticalOffset);
    }
}
