using System;
using System.Collections.Generic;
using TogglDesktop.ViewModels;
using Xunit;

namespace TogglDesktop.Tests
{
    public class TimelineGenerateGapTimeEntryBlocksTests
    {
        private static readonly DateTime _selectedDate = new DateTime(2020, 12, 3);

        [Theory]
        [MemberData(nameof(GetData))]
        public void TestGenerateGapTimeEntryBlocks_CorrectOffsets(List<TimeEntryBlock> timeEntries,
            List<(double VerticalOffset, double Height)> expectedValues)
        {
            var gaps = TimelineViewModel.GenerateGapTimeEntryBlocks(timeEntries);

            Assert.Equal(gaps.Count, expectedValues.Count);
            for (var i = 0; i < gaps.Count; i++)
            {
                Assert.Equal(expectedValues[i].VerticalOffset, gaps[i].VerticalOffset);
                Assert.Equal(expectedValues[i].Height, gaps[i].Height);
            }
        }

        public static IEnumerable<object[]> GetData()
        {
            yield return TestCase1();
            yield return TestCase2();
        }

        //Time entries scheme:
        //  |
        //  |
        //   
        //  |
        //  |
        //   
        //  |
        //  |
        private static object[] TestCase1()
        {
            var timeEntryList = new List<TimeEntryBlock>()
            {
                new TimeEntryBlock(new Toggl.TogglTimeEntryView(), 100, _selectedDate)
                    {VerticalOffset = 0, Height = 100},
                new TimeEntryBlock(new Toggl.TogglTimeEntryView(), 100, _selectedDate)
                    {VerticalOffset = 150, Height = 100},
                new TimeEntryBlock(new Toggl.TogglTimeEntryView(), 100, _selectedDate)
                    {VerticalOffset = 300, Height = 100}
            };
            
            return new object[]
            {
                timeEntryList,
                new List<(double VerticalOffset, double Height)>()
                {
                    (100, 50),
                    (250, 50)
                }
            };
        }

        //Time entries scheme:
        //  |
        //  ||
        //   |
        // 
        //  |
        //  |
        private static object[] TestCase2()
        {
            var timeEntryList = new List<TimeEntryBlock>()
            {
                new TimeEntryBlock(new Toggl.TogglTimeEntryView(), 100, _selectedDate)
                    {VerticalOffset = 0, Height = 100},
                new TimeEntryBlock(new Toggl.TogglTimeEntryView(), 100, _selectedDate)
                    {VerticalOffset = 50, Height = 100},
                new TimeEntryBlock(new Toggl.TogglTimeEntryView(), 100, _selectedDate)
                    {VerticalOffset = 200, Height = 100}
            };

            return new object[]
            {
                timeEntryList,
                new List<(double VerticalOffset, double Height)>()
                {
                    (150, 50)
                }
            };
        }
    }
}
