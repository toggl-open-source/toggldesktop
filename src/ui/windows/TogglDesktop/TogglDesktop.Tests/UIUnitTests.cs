using System;
using System.Linq;
using Xunit;

namespace TogglDesktop.Tests
{
    public class UIUnitTests
    {
        [Theory]
        [InlineData(new[] { false, true,  false, false, false, false, false}, DayOfWeek.Monday, "on Monday")]
        [InlineData(new[] { false, true,  true,  false, false, false, false}, DayOfWeek.Monday, "on Monday, Tuesday")]
        [InlineData(new[] { true,  false, false, false, false, false, true},  DayOfWeek.Monday, "on weekend")]
        [InlineData(new[] { false, true,  true,  true,  false, false, false}, DayOfWeek.Monday, "on Monday, Tuesday, Wednesday")]
        [InlineData(new[] { false, true,  true,  true,  false, false, true},  DayOfWeek.Monday, "on Monday, Tuesday, Wednesday, Saturday")]
        [InlineData(new[] { false, true,  true,  true,  true,  false, false}, DayOfWeek.Monday, "on weekdays except Friday")]
        [InlineData(new[] { false, true,  true,  true,  true,  false, true},  DayOfWeek.Monday, "every day except Friday, Sunday")]
        [InlineData(new[] { false, true,  true,  true,  true,  true,  false}, DayOfWeek.Monday, "on weekdays")]
        [InlineData(new[] { true,  true,  true,  true,  true,  false, true},  DayOfWeek.Monday, "every day except Friday")]
        [InlineData(new[] { true,  true,  true,  true,  true,  true,  false}, DayOfWeek.Monday, "on weekdays and Sunday")]
        [InlineData(new[] { true,  true,  true,  true,  true,  true,  true},  DayOfWeek.Monday, "every day")]
        [InlineData(new[] { true,  true,  false, false, false, false, false}, DayOfWeek.Monday, "on Monday, Sunday")]
        [InlineData(new[] { true,  true,  false, false, false, false, false}, DayOfWeek.Sunday, "on Sunday, Monday")]
        public void TestSelectedDaysOfWeekText(bool[] daysChecked, DayOfWeek beginningOfWeek, string expectedText)
        {
            var isDayChecked = daysChecked.WithIndex()
                .ToDictionary(x => (DayOfWeek) x.index, x => x.item);
            var text = DayOfWeekExtensions.GetText(isDayChecked, beginningOfWeek);
            Assert.Equal(expectedText, text);
        }
    }
}