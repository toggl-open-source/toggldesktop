using System;
using System.Collections.Generic;
using System.IO;
using System.Reactive.Disposables;
using Xunit;

namespace TogglDesktop.Tests
{
    public class LibraryFixture : IDisposable
    {
        public List<Toggl.TogglTimeEntryView> TimeEntries { get; private set; }
        public Toggl.TogglTimeEntryView RunningEntry { get; private set; }
        public bool IsRunning { get; private set; }

        public string MeJson = File.ReadAllText("me.json");

        public LibraryFixture()
        {
            Toggl.Env = "test";
            Toggl.OnTimeEntryList += (open, list, button) => TimeEntries = list;
            Toggl.OnRunningTimerState += te =>
            {
                RunningEntry = te;
                IsRunning = true;
            };
            Toggl.OnStoppedTimerState += () =>
            {
                RunningEntry = default;
                IsRunning = false;
            };
            Assert.True(Toggl.StartUI("0.0.0", new ulong[0]));
            Toggl.ClearCache();
            Toggl.SetManualMode(false);
        }

        public void Dispose()
        {
            Toggl.Clear();
        }
    }
}