using System;

namespace TogglDesktop
{
    public class TimelineUtils
    {
        public static ulong ConvertOffsetToTime(double height, DateTime date, double hourHeight)
        {
            var hours = 1.0 * height / hourHeight;
            var dateTime = date.AddHours(hours);
            var unixTime = Toggl.UnixFromDateTime(dateTime);
            return unixTime >= 0 ? (ulong)unixTime : 0;
        }
    }
}
