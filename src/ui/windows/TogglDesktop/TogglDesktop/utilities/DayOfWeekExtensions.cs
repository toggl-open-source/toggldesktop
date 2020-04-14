using System;
using System.Collections.Generic;
using System.Linq;

namespace TogglDesktop
{
    public static class DayOfWeekExtensions
    {
        public static IEnumerable<DayOfWeek> DaysOfWeek() => Enum.GetValues(typeof(DayOfWeek)).Cast<DayOfWeek>();

        public static int PositionRelativeTo(this DayOfWeek day, DayOfWeek beginningOfWeek) =>
            (day - beginningOfWeek + 7) % 7;

        public static bool IsWeekday(this DayOfWeek day) => day >= DayOfWeek.Monday && day <= DayOfWeek.Friday;

        public static DayOfWeek Add(this DayOfWeek day, int numberOfDays) => (DayOfWeek) (((int)day + numberOfDays) % 7);

        public static int DaysBetween(DayOfWeek dayFrom, DayOfWeek dayUntil) => (dayUntil - dayUntil + 7) % 7;
        public static int DaysSince(this DayOfWeek day1, DayOfWeek day2) => (day1 - day2 + 7) % 7;
    }
}