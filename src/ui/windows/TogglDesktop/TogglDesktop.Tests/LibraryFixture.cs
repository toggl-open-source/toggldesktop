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

        private readonly IDisposable _runningTimerState;

        public LibraryFixture()
        {
            Toggl.Env = "test";
            Toggl.OnTimeEntryList += (open, list, button) => TimeEntries = list;
            _runningTimerState = Toggl.RunningTimeEntry.Subscribe( te =>
            {
                RunningEntry = te ?? default;
                IsRunning = te.HasValue;
            });
            Assert.True(Toggl.StartUI("0.0.0"));
            Toggl.ClearCache();
            Toggl.SetManualMode(false);
        }

        public void Dispose()
        {
            _runningTimerState.Dispose();
            Toggl.Clear();
        }
    }
}