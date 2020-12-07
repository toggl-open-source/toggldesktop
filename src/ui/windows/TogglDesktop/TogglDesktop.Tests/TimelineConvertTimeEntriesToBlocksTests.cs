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
        private const double _timeEntryOffset =
            TimelineConstants.GapBetweenOverlappingTEs + TimelineConstants.TimeEntryBlockWidth;

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
                    {GUID = "1", Started = _selectedDateUnix + 0, Ended = _selectedDateUnix + 100},
                new Toggl.TogglTimeEntryView()
                    {GUID = "2", Started = _selectedDateUnix + 50, Ended = _selectedDateUnix + 200},
                new Toggl.TogglTimeEntryView()
                    {GUID = "3", Started = _selectedDateUnix + 150, Ended = _selectedDateUnix + 200}
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
                    (timeEntryList[0].GUID, TimeToHeight(timeEntryList[0].Started - _selectedDateUnix, scaleMode),
                        TimeToHeight(timeEntryList[0].Ended - timeEntryList[0].Started, scaleMode), 0, true),
                    (timeEntryList[1].GUID, TimeToHeight(timeEntryList[1].Started - _selectedDateUnix, scaleMode),
                        TimeToHeight(timeEntryList[1].Ended - timeEntryList[1].Started, scaleMode), 0 + _timeEntryOffset, true),
                    (timeEntryList[2].GUID, TimeToHeight(timeEntryList[2].Started - _selectedDateUnix, scaleMode),
                        TimeToHeight(timeEntryList[2].Ended - timeEntryList[2].Started, scaleMode), 0, true)
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
                    {GUID = "1", Started = _selectedDateUnix + 0, Ended = _selectedDateUnix + 150},
                new Toggl.TogglTimeEntryView()
                    {GUID = "2", Started = _selectedDateUnix + 50, Ended = _selectedDateUnix + 150},
                new Toggl.TogglTimeEntryView()
                    {GUID = "3", Started = _selectedDateUnix + 100, Ended = _selectedDateUnix + 200}
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
                    (timeEntryList[0].GUID, TimeToHeight(timeEntryList[0].Started - _selectedDateUnix, scaleMode),
                        TimeToHeight(timeEntryList[0].Ended - timeEntryList[0].Started, scaleMode), 0, true),
                    (timeEntryList[1].GUID, TimeToHeight(timeEntryList[1].Started - _selectedDateUnix, scaleMode),
                        TimeToHeight(timeEntryList[1].Ended - timeEntryList[1].Started, scaleMode), 0 + _timeEntryOffset, true),
                    (timeEntryList[2].GUID, TimeToHeight(timeEntryList[2].Started - _selectedDateUnix, scaleMode),
                        TimeToHeight(timeEntryList[2].Ended - timeEntryList[2].Started, scaleMode), 0 + 2*_timeEntryOffset, true)
                }
            };
        }

        //Now >= selected date, and TE is running
        private static object[] TestCase_MustIncludeRunningEntry()
        {
            var timeEntryList = new List<Toggl.TogglTimeEntryView>()
            {
                new Toggl.TogglTimeEntryView()
                    {GUID = "1", Started = _selectedDateUnix + 0, Ended = _selectedDateUnix + 150},
            };
            var scaleMode = 1;
            var runningEntry = new Toggl.TogglTimeEntryView() {GUID = "4", Started = _selectedDateUnix + 150};
            return new object[]
            {
                timeEntryList,
                new Toggl.TogglTimeEntryView() {GUID = "4", Started = _selectedDateUnix + 150}, 
                scaleMode,
                _selectedDate.AddSeconds(300),
                new List<(string Guid, double VerticalOffset, double Height, double HorizontalOffset, bool IsOverlapping)>()
                {
                    (timeEntryList[0].GUID, TimeToHeight(timeEntryList[0].Started - _selectedDateUnix, scaleMode),
                        TimeToHeight(timeEntryList[0].Ended - timeEntryList[0].Started, scaleMode), 0, false),
                    (runningEntry.GUID, TimeToHeight(runningEntry.Started - _selectedDateUnix, scaleMode),
                        TimeToHeight(_selectedDateUnix + 300 - runningEntry.Started, scaleMode), 0, false)
                }
            };
        }

        //Now < selected date, and TE is running
        private static object[] TestCase_MustNotIncludeRunningEntry()
        {
            var timeEntryList = new List<Toggl.TogglTimeEntryView>()
            {
                new Toggl.TogglTimeEntryView()
                    {GUID = "1", Started = _selectedDateUnix + 0, Ended = _selectedDateUnix + 150},
            };
            var scaleMode = 1;
            var runningEntry = new Toggl.TogglTimeEntryView() { GUID = "4", Started = _selectedDateUnix - 600 };
            return new object[]
            {
                timeEntryList,
                new Toggl.TogglTimeEntryView() {GUID = "4", Started = _selectedDateUnix + 150},
                scaleMode,
                _selectedDate.Subtract(TimeSpan.FromSeconds(300)),
                new List<(string Guid, double VerticalOffset, double Height, double HorizontalOffset, bool IsOverlapping)>()
                {
                    (timeEntryList[0].GUID, TimeToHeight(timeEntryList[0].Started - _selectedDateUnix, scaleMode),
                        TimeToHeight(timeEntryList[0].Ended - timeEntryList[0].Started, scaleMode), 0, false)
                }
            };
        }

        private static double TimeToHeight(ulong time, int mode) =>
            TimelineConstants.ScaleModes[mode] * (1d*time / 3600);
    }
}
