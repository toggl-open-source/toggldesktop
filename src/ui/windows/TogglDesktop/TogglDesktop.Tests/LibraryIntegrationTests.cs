using System;
using System.Collections.Generic;
using System.Linq;
using Xunit;

namespace TogglDesktop.Tests
{
    public class LibraryIntegrationTests : IClassFixture<LibraryFixture>
    {
        private readonly LibraryFixture _state;
        private readonly string _firstTimeEntryGuid;

        public LibraryIntegrationTests(LibraryFixture libraryFixture)
        {
            _state = libraryFixture;
            Assert.True(Toggl.SetLoggedInUser(_state.MeJson));
            _firstTimeEntryGuid = _state.TimeEntries[0].GUID;
        }

        [Fact]
        public void ShowAppShouldNotCrash()
        {
            Toggl.ShowApp();
        }

        [Fact]
        public void LoginWithFakeUserShouldFail()
        {
            Assert.False(Toggl.Login("john@doe.com", "foobar", 1));
        }

        [Fact]
        public void LoginWithFakeGoogleTokenShouldFail()
        {
            Assert.False(Toggl.GoogleLogin("faketoken"));
        }

        [Fact]
        public void ForgotPasswordShouldNotCrash()
        {
            Toggl.PasswordForgot();
        }

        [Fact]
        public void OpenInBrowserShouldNotCrash()
        {
            Toggl.OpenInBrowser();
        }

        [Fact]
        public void SendFeedbackShouldNotFail()
        {
            Assert.True(Toggl.SendFeedback("topic", "details", "filename"));
        }

        [Fact]
        public void ViewTimeEntryListShouldContainKnownTimeEntry()
        {
            Toggl.ViewTimeEntryList();
            Assert.Contains(_state.TimeEntries, te => te.ID == (ulong) 89818605);
        }

        [Fact]
        public void EditShouldTriggerOnTimeEntryEditor()
        {
            var assertionExecuted = false;

            Toggl.OnTimeEntryEditor += Assertion;
            Toggl.Edit(_firstTimeEntryGuid, false, "description");
            Toggl.OnTimeEntryEditor -= Assertion;

            void Assertion(bool open, Toggl.TogglTimeEntryView timeEntry, string focusedFieldName)
            {
                Assert.True(open);
                Assert.Equal(timeEntry.GUID, _firstTimeEntryGuid);
                Assert.Equal("description", focusedFieldName);
                assertionExecuted = true;
            }
            Assert.True(assertionExecuted);
        }

        [Fact]
        public void EditPreferencesShouldOpenSettings()
        {
            var assertionExecuted = false;

            Toggl.OnSettings += Assertion;
            Toggl.EditPreferences();
            Toggl.OnSettings -= Assertion;

            void Assertion(bool open, Toggl.TogglSettingsView settings)
            {
                Assert.True(open);
                assertionExecuted = true;
            }
            Assert.True(assertionExecuted);
        }

        [Fact]
        public void StartShouldStartTimeEntry()
        {
            var guid = Toggl.Start("doing stuff", "", 0, 0, "", "");
            Assert.NotNull(guid);
            Assert.NotEqual(string.Empty, guid);
            Assert.True(_state.IsRunning);
            Assert.Equal(_state.RunningEntry.GUID, guid);
        }

        [Fact]
        public void StopShouldStopTimeEntry()
        {
            var guid = Toggl.Start("doing stuff", "", 0, 0, "", "");
            Assert.NotNull(guid);
            Assert.NotEqual(string.Empty, guid);
            Assert.True(_state.IsRunning);
            Assert.Equal(_state.RunningEntry.GUID, guid);

            Assert.True(Toggl.Stop());
            Assert.False(_state.IsRunning);
            Assert.NotEqual(_state.RunningEntry.GUID, guid);
        }

        [Fact]
        public void ContinueShouldStartTimeEntry()
        {
            var guid = _state.TimeEntries[0].GUID;
            var description = _state.TimeEntries[0].Description;
            Toggl.Stop();
            Assert.False(_state.IsRunning);

            Toggl.Continue(guid);

            Assert.True(_state.IsRunning);
            Assert.Equal(description, _state.RunningEntry.Description);
        }

        [Fact]
        public void ContinueLatestShouldStartTimeEntry()
        {
            Assert.True(Toggl.Stop());
            Assert.False(_state.IsRunning);

            Toggl.ContinueLatest();

            Assert.True(_state.IsRunning);
        }

        [Fact]
        public void DeleteTimeEntryShouldDeleteTimeEntry()
        {
            Toggl.DeleteTimeEntry(_firstTimeEntryGuid);

            Assert.DoesNotContain(_state.TimeEntries, te => te.GUID == _firstTimeEntryGuid);
        }

        [Fact]
        public void SetTimeEntryDurationShouldChangeDuration()
        {
            Assert.True(Toggl.SetTimeEntryDuration(_firstTimeEntryGuid, "1 hour"));

            Assert.Equal(3600, GetTimeEntry(_firstTimeEntryGuid).DurationInSeconds);
        }

        [Fact]
        public void SetTimeEntryProjectShouldChangeProject()
        {
            const ulong projectId = 2567324ul;

            Assert.True(Toggl.SetTimeEntryProject(_firstTimeEntryGuid, 0, projectId, ""));

            Assert.Equal(GetTimeEntry(_firstTimeEntryGuid).PID, projectId);
        }

        [Fact]
        public void SetTimeEntryStartShouldChangeStartTime()
        {
            Assert.True(Toggl.SetTimeEntryStart(_firstTimeEntryGuid, "12:34"));

            Assert.Equal("12:34", GetTimeEntry(_firstTimeEntryGuid).StartTimeString);
        }

        [Fact]
        public void SetTimeEntryDateShouldChangeStartDate()
        {
            var now = DateTime.Now;
            Assert.True(Toggl.SetTimeEntryDate(_firstTimeEntryGuid, now));

            var started = Toggl.DateTimeFromUnix(GetTimeEntry(_firstTimeEntryGuid).Started);
            Assert.Equal(started.Date, now.Date);
        }

        [Fact]
        public void SetTimeEntryEndShouldChangeEndTime()
        {
            Assert.True(Toggl.SetTimeEntryEnd(_firstTimeEntryGuid, "23:45"));

            Assert.Equal("23:45", GetTimeEntry(_firstTimeEntryGuid).EndTimeString);
        }

        [Fact]
        public void SetTimeEntryTagsShouldChangeTags()
        {
            var tagsList = new List<string> {"John", "Anna", "Monica"};
            Assert.True(Toggl.SetTimeEntryTags(_firstTimeEntryGuid, tagsList));

            Assert.Equal(GetTimeEntry(_firstTimeEntryGuid).Tags, string.Join(Toggl.TagSeparator, tagsList));
        }

        [Fact]
        public void SetTimeEntryBillableShouldChangeBillable()
        {
            Assert.True(Toggl.SetTimeEntryBillable(_firstTimeEntryGuid, true));
            Assert.True(GetTimeEntry(_firstTimeEntryGuid).Billable);

            Assert.True(Toggl.SetTimeEntryBillable(_firstTimeEntryGuid, false));
            Assert.False(GetTimeEntry(_firstTimeEntryGuid).Billable);
        }

        [Fact]
        public void SetTimeEntryDescriptionShouldChangeDescription()
        {
            Assert.True(Toggl.SetTimeEntryDescription(_firstTimeEntryGuid, "new description"));
            Assert.Equal("new description", GetTimeEntry(_firstTimeEntryGuid).Description);
        }

        [Theory]
        [InlineData(true)] // add as new entry
        [InlineData(false)] // discard
        public void DiscardAtShouldWorkCorrectly(bool split)
        {
            var guid = Toggl.Start("description", "", 0, 0, "", "");
            Assert.True(Toggl.SetTimeEntryDuration(guid, "01:00"));
            var entry = _state.RunningEntry;
            var at = entry.Started;
            var count = _state.TimeEntries.Count;

            Assert.True(Toggl.DiscardTimeAt(guid, (long) (at + 20), split));
            Assert.Equal(count + 1, _state.TimeEntries.Count);
            Assert.Equal(split, _state.IsRunning);
        }

        [Theory]
        [InlineData(1, 2, 3, 4)]
        [InlineData(0, 0, 768, 1024)]
        [InlineData(0, 0, 1200, 1900)]
        [InlineData(-1900, -1200, 1200, 1900)]
        public void SetWindowSettingsShouldSaveWindowSettings(long x, long y, long h, long w)
        {
            Toggl.SetWindowSettings(x, y, h, w);

            long x1 = 0, y1 = 0, h1 = 0, w1 = 0;
            Toggl.WindowSettings(ref x1, ref y1, ref h1, ref w1);
            Assert.Equal(new[]{x, y, h, w}, new[]{x1, y1, h1, w1});
        }

        [Theory]
        [InlineData(false, false)]
        [InlineData(false, true)]
        [InlineData(true, false)]
        [InlineData(true, true)]
        public void SetSettingsShouldSaveSettings(bool autotrack, bool pomodoro)
        {
            Toggl.TogglSettingsView currentSettings = default;
            Toggl.OnSettings += ReadCurrentSettings;
            Toggl.EditPreferences();
            Toggl.OnSettings -= ReadCurrentSettings;
            void ReadCurrentSettings(bool open, Toggl.TogglSettingsView settings) => currentSettings = settings;

            var savedAutotrack = false;
            var savedPomodoro = false;

            Toggl.OnSettings += ReadSavedSettings;

            currentSettings.Autotrack = autotrack;
            currentSettings.Pomodoro = pomodoro;
            Toggl.SetSettings(currentSettings);

            void ReadSavedSettings(bool open, Toggl.TogglSettingsView settings)
            {
                savedAutotrack = settings.Autotrack;
                savedPomodoro = settings.Pomodoro;
            }

            Assert.Equal(autotrack, savedAutotrack);
            Assert.Equal(pomodoro, savedPomodoro);

            Toggl.OnSettings -= ReadSavedSettings;
        }

        [Fact]
        public void LogoutShouldLogTheUserOut()
        {
            var assertionExecuted = false;
            Toggl.OnLogin += Assertion;

            Assert.True(Toggl.Logout());

            void Assertion(bool open, ulong userId)
            {
                Assert.True(open);
                Assert.Equal(0ul, userId);
                assertionExecuted = true;
            }
            Assert.True(assertionExecuted);

            Toggl.OnLogin -= Assertion;
        }

        [Fact]
        public void ClearCacheShouldLogTheUserOut()
        {
            var assertionExecuted = false;
            Toggl.OnLogin += Assertion;

            Assert.True(Toggl.Logout());

            void Assertion(bool open, ulong userId)
            {
                Assert.True(open);
                Assert.Equal(0ul, userId);
                assertionExecuted = true;
            }
            Assert.True(assertionExecuted);

            Toggl.OnLogin -= Assertion;
        }

        [Fact]
        public void AddProjectShouldCreateNewProjectAndAssignItToTimeEntry()
        {
            var projectId = Toggl.AddProject(_firstTimeEntryGuid, 123456789, 0, null,
                "testing project adding", false, null);
            Assert.NotNull(projectId);
            Assert.NotEqual(string.Empty, projectId);

            Assert.Equal("testing project adding", GetTimeEntry(_firstTimeEntryGuid).ProjectLabel);
        }

        [Fact]
        public void CreateClientShouldCreateNewClientInTheCorrectWorkspace()
        {
            var assertionExecuted = false;
            Toggl.OnClientSelect += Assertion;

            var clientId = Toggl.CreateClient(123456789, "test creating client");
            Assert.NotNull(clientId);
            Assert.NotEqual(string.Empty, clientId);

            void Assertion(List<Toggl.TogglGenericView> clients)
            {
                Assert.Contains(clients, c => c.Name == "test creating client" && c.WID == 123456789);
                assertionExecuted = true;
            }
            Assert.True(assertionExecuted);

            Toggl.OnClientSelect -= Assertion;
        }

        [Theory]
        [InlineData("fake channel", false)]
        [InlineData("stable", true)]
        [InlineData("beta", true)]
        [InlineData("dev", true)]
        public void SetChannelShouldSetChannelIfItIsValid(string channelName, bool isValid)
        {
            Assert.Equal(isValid, Toggl.SetUpdateChannel(channelName));

            if (isValid)
            {
                Assert.Equal(channelName, Toggl.UpdateChannel());
            }
        }

        [Fact]
        public void UserEmailReturnsCorrectData()
        {
            Assert.Equal("johnsmith@toggl.com", Toggl.UserEmail());
        }

        [Fact]
        public void SyncDoesNotCrash()
        {
            Toggl.Sync();
        }

        [Fact]
        public void SetSleepDoesNotCrash()
        {
            Toggl.SetSleep();
        }

        [Fact]
        public void SetWakeDoesNotCrash()
        {
            Toggl.SetWake();
        }

        [Fact]
        public void SetIdleSecondsDoesNotCrash()
        {
            Toggl.SetIdleSeconds(123);
        }
        private Toggl.TogglTimeEntryView GetTimeEntry(string guid) => _state.TimeEntries.First(te => te.GUID == guid);
    }
}