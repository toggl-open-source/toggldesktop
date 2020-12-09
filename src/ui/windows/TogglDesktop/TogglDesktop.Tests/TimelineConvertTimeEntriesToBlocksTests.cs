using System;
using System.Collections.Generic;
using TogglDesktop.Resources;
using TogglDesktop.ViewModels;
using Xunit;

namespace TogglDesktop.Tests
{
    public class TimelineConvertTimeEntriesToBlocksTests
    {
        private static readonly DateTime _selectedDate = new DateTime(2020, 12, 3);
        private static readonly ulong _selectedDateUnix = (ulong)Toggl.UnixFromDateTime(_selectedDate);
        private const double _horizontalTimeEntryOffset =
            TimelineConstants.GapBetweenOverlappingTEs + TimelineConstants.TimeEntryBlockWidth;

        private const ulong T1 = 0;
        private const ulong T2 = 50;
        private const ulong T3 = 100;
        private const ulong T4 = 150;
        private const ulong T5 = 200;
        private const ulong T6 = 300;

        [Theory]
        [MemberData(nameof(GetData))]
        public void TestConvertTimeEntriesToBlocks_CorrectOffsets(List<Toggl.TogglTimeEntryView> timeEntries,
            Toggl.TogglTimeEntryView? runningEntry,
            int selectedScaleMode,
            DateTime now,
            List<(string Guid, double VerticalOffset, double Height, double HorizontalOffset, bool IsOverlapping)> expectedValues)
        {
            var blocks = TimelineViewModel.ConvertTimeEntriesToBlocks(timeEntries,
                runningEntry, selectedScaleMode, _selectedDate, now);

            Assert.Equal(expectedValues.Count, blocks.Count);
            foreach (var (guid, verticalOffset, height, horizontalOffset, isOverlapping) in expectedValues)
            {
                Assert.True(blocks.ContainsKey(guid));
                Assert.Equal(verticalOffset, blocks[guid].VerticalOffset, 5);
                Assert.Equal(height, blocks[guid].Height, 5);
                Assert.Equal(horizontalOffset, blocks[guid].HorizontalOffset, 5);
                Assert.Equal(isOverlapping, blocks[guid].IsOverlapping);
            }
        }

        public static IEnumerable<object[]> GetData()
        {
            yield return TestCase1();
            yield return TestCase2();
            yield return TestCase_MustIncludeRunningEntry();
            yield return TestCase_MustNotIncludeRunningEntry();
            yield return TestCase_NoOverlap();
            yield return TestCase_MustIncludeMoreThan24HoursEntry();
            yield return TestCase_SameGuid();
        }

        //Time entries scheme:
        //  |
        //  ||
        //   |
        //  ||
        //  |
        private static object[] TestCase1()
        {
            var timeEntryList = new List<Toggl.TogglTimeEntryView>()
            {
                new Toggl.TogglTimeEntryView()
                    {GUID = "1", Started = _selectedDateUnix + T1, Ended = _selectedDateUnix + T3},
                new Toggl.TogglTimeEntryView()
                    {GUID = "2", Started = _selectedDateUnix + T2, Ended = _selectedDateUnix + T5},
                new Toggl.TogglTimeEntryView()
                    {GUID = "3", Started = _selectedDateUnix + T4, Ended = _selectedDateUnix + T5}
            };
            var scaleMode = 0;
            return new object[]
            {
                timeEntryList,
                null,
                scaleMode,
                _selectedDate,
                new List<(string Guid, double VerticalOffset, double Height, double HorizontalOffset, bool IsOverlapping)>()
                {
                    (timeEntryList[0].GUID, TimeToHeight(T1, scaleMode),
                        TimeToHeight(T3 - T1, scaleMode), 0, true),
                    (timeEntryList[1].GUID, TimeToHeight(T2, scaleMode),
                        TimeToHeight(T5 - T2, scaleMode), 0 + _horizontalTimeEntryOffset, true),
                    (timeEntryList[2].GUID, TimeToHeight(T4, scaleMode),
                        TimeToHeight(T5 - T4, scaleMode), 0, true)
                }
            };
        }

        //Time entries scheme:
        //  |
        //  ||
        //  |||
        //    |
        private static object[] TestCase2()
        {
            var timeEntryList = new List<Toggl.TogglTimeEntryView>()
            {
                new Toggl.TogglTimeEntryView()
                    {GUID = "1", Started = _selectedDateUnix + T1, Ended = _selectedDateUnix + T4},
                new Toggl.TogglTimeEntryView()
                    {GUID = "2", Started = _selectedDateUnix + T2, Ended = _selectedDateUnix + T4},
                new Toggl.TogglTimeEntryView()
                    {GUID = "3", Started = _selectedDateUnix + T3, Ended = _selectedDateUnix + T5}
            };
            var scaleMode = 1;
            return new object[]
            {
                timeEntryList,
                null,
                scaleMode,
                _selectedDate,
                new List<(string Guid, double VerticalOffset, double Height, double HorizontalOffset, bool IsOverlapping)>()
                {
                    (timeEntryList[0].GUID, TimeToHeight(T1, scaleMode),
                        TimeToHeight(T4 - T1, scaleMode), 0, true),
                    (timeEntryList[1].GUID, TimeToHeight(T2, scaleMode),
                        TimeToHeight(T4 - T2, scaleMode), 0 + _horizontalTimeEntryOffset, true),
                    (timeEntryList[2].GUID, TimeToHeight(T3, scaleMode),
                        TimeToHeight(T5 - T3, scaleMode), 0 + 2*_horizontalTimeEntryOffset, true)
                }
            };
        }

        //Now >= selected date, and TE is running
        private static object[] TestCase_MustIncludeRunningEntry()
        {
            var timeEntryList = new List<Toggl.TogglTimeEntryView>()
            {
                new Toggl.TogglTimeEntryView()
                    {GUID = "1", Started = _selectedDateUnix + T1, Ended = _selectedDateUnix + T4},
            };
            var scaleMode = 1;
            var runningEntry = new Toggl.TogglTimeEntryView() {GUID = "4", Started = _selectedDateUnix + T4};
            var now = _selectedDate.AddSeconds(300);
            return new object[]
            {
                timeEntryList,
                runningEntry, 
                scaleMode,
                now,
                new List<(string Guid, double VerticalOffset, double Height, double HorizontalOffset, bool IsOverlapping)>()
                {
                    (timeEntryList[0].GUID, TimeToHeight(T1, scaleMode),
                        TimeToHeight(T4 - T1, scaleMode), 0, false),
                    (runningEntry.GUID, TimeToHeight(T4, scaleMode),
                        TimeToHeight(300 - T4, scaleMode), 0, false)
                }
            };
        }

        //Now < selected date, and TE is running
        private static object[] TestCase_MustNotIncludeRunningEntry()
        {
            var timeEntryList = new List<Toggl.TogglTimeEntryView>()
            {
                new Toggl.TogglTimeEntryView()
                    {GUID = "1", Started = _selectedDateUnix + T1, Ended = _selectedDateUnix + T4},
            };
            var scaleMode = 1;
            var runningEntry = new Toggl.TogglTimeEntryView() { GUID = "4", Started = _selectedDateUnix - 600 };
            var now = _selectedDate.Subtract(TimeSpan.FromSeconds(300));
            return new object[]
            {
                timeEntryList,
                runningEntry,
                scaleMode,
                now,
                new List<(string Guid, double VerticalOffset, double Height, double HorizontalOffset, bool IsOverlapping)>()
                {
                    (timeEntryList[0].GUID, TimeToHeight(T1, scaleMode),
                        TimeToHeight(T4 - T1, scaleMode), 0, false)
                }
            };
        }

        //Now >= selected date, and TE is running
        private static object[] TestCase_MustIncludeMoreThan24HoursEntry()
        {
            var startTime = (ulong)Toggl.UnixFromDateTime(_selectedDate.Subtract(TimeSpan.FromHours(6)));
            var endTime = (ulong)Toggl.UnixFromDateTime(_selectedDate.AddHours(25));
            var timeEntryList = new List<Toggl.TogglTimeEntryView>()
            {
                new Toggl.TogglTimeEntryView()
                    {GUID = "1", Started = startTime, Ended = endTime},
            };
            var scaleMode = 1;
            return new object[]
            {
                timeEntryList,
                null,
                scaleMode,
                _selectedDate,
                new List<(string Guid, double VerticalOffset, double Height, double HorizontalOffset, bool IsOverlapping)>()
                {
                    (timeEntryList[0].GUID, -TimeToHeight(6*3600, scaleMode), // Vertical offset must be negative, as start time is six hours before selected date
                        TimeToHeight(endTime - startTime, scaleMode), 0, false)
                }
            };
        }

        //Time entries scheme:
        //  |
        //  |
        //  |
        //  |
        private static object[] TestCase_NoOverlap()
        {
            var timeEntryList = new List<Toggl.TogglTimeEntryView>()
            {
                new Toggl.TogglTimeEntryView()
                    {GUID = "1", Started = _selectedDateUnix + T1, Ended = _selectedDateUnix + T4},
                new Toggl.TogglTimeEntryView()
                    {GUID = "2", Started = _selectedDateUnix + T4, Ended = _selectedDateUnix + T6},
            };
            var scaleMode = 1;
            return new object[]
            {
                timeEntryList,
                null,
                scaleMode,
                _selectedDate,
                new List<(string Guid, double VerticalOffset, double Height, double HorizontalOffset, bool IsOverlapping)>()
                {
                    (timeEntryList[0].GUID, TimeToHeight(T1, scaleMode),
                        TimeToHeight(T4 - T1, scaleMode), 0, false),
                    (timeEntryList[1].GUID, TimeToHeight(timeEntryList[1].Started - _selectedDateUnix, scaleMode),
                        TimeToHeight(T6 - T4, scaleMode), 0, false)
                }
            };
        }

        //Running TE, and TE from the list have the same guid
        private static object[] TestCase_SameGuid()
        {
            var timeEntryList = new List<Toggl.TogglTimeEntryView>()
            {
                new Toggl.TogglTimeEntryView()
                    {GUID = "1", Started = _selectedDateUnix + T1, Ended = _selectedDateUnix + T4},
                new Toggl.TogglTimeEntryView()
                    {GUID = "2", Started = _selectedDateUnix + T4, Ended = _selectedDateUnix + T6},
            };
            var scaleMode = 1;
            var runningTE = new Toggl.TogglTimeEntryView()
                { GUID = "2", Started = _selectedDateUnix + T4};

            return new object[]
            {
                timeEntryList,
                runningTE,
                scaleMode,
                _selectedDate.AddSeconds(T6),
                new List<(string Guid, double VerticalOffset, double Height, double HorizontalOffset, bool IsOverlapping)>()
                {
                    (timeEntryList[0].GUID, TimeToHeight(T1, scaleMode),
                        TimeToHeight(T4 - T1, scaleMode), 0, false),
                    (timeEntryList[1].GUID, TimeToHeight(T4, scaleMode),
                        TimeToHeight(T6 - T4, scaleMode), 0, false)
                }
            };
        }

        private static double TimeToHeight(ulong time, int mode) =>
            TimelineConstants.ScaleModes[mode] * (1d*time / 3600);
    }
}
