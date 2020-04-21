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

        public static int DaysSince(this DayOfWeek day1, DayOfWeek day2) => (day1 - day2 + 7) % 7;

        public static string GetText(Dictionary<DayOfWeek, bool> isDayChecked, DayOfWeek beginningOfWeek)
        {
            var checkedCount = isDayChecked.Count(kvp => kvp.Value);
            return isDayChecked switch
            {
                _ when checkedCount == 7
                => "every day",
                var x when checkedCount == 5 && !x[DayOfWeek.Sunday] && !x[DayOfWeek.Saturday]
                => "on weekdays",
                var x when checkedCount == 6 && (!x[DayOfWeek.Sunday] || !x[DayOfWeek.Saturday])
                => "on weekdays and " + (x[DayOfWeek.Sunday] ? "Sunday" : "Saturday"),
                var x when checkedCount >= 5
                => "every day except " + string.Join(", ",
                    x.Where(kvp => !kvp.Value)
                        .OrderBy(kvp => kvp.Key.DaysSince(beginningOfWeek))
                        .Select(kvp => Enum.GetName(typeof(DayOfWeek), kvp.Key))),
                var x when checkedCount == 4 && !x[DayOfWeek.Sunday] && !x[DayOfWeek.Saturday]
                => "on weekdays except " +
                   Enum.GetName(typeof(DayOfWeek), x.First(kvp => kvp.Key.IsWeekday() && !kvp.Value).Key),
                var x when checkedCount == 2 && x[DayOfWeek.Sunday] && x[DayOfWeek.Saturday]
                => "on weekend",
                var x => "on " + string.Join(", ",
                    x.Where(kvp => kvp.Value)
                        .OrderBy(kvp => kvp.Key.DaysSince(beginningOfWeek))
                        .Select(kvp => Enum.GetName(typeof(DayOfWeek), kvp.Key)))
            };
        }

    }
}