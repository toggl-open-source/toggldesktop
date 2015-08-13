using System;
using System.IO;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace TogglDesktopDLLInteropTest
{
    [TestClass]
    public class LibTest
    {
        [TestInitialize]
        public void Initialize()
        {
            Assert.IsTrue(TogglDesktop.Toggl.StartUI("0.0.0"));
            string path = Path.Combine(Directory.GetCurrentDirectory(), "me.json");
            Assert.AreNotEqual("", path);
            Assert.IsNotNull(path);
            string json = File.ReadAllText(path);
            Assert.AreNotEqual("", json);
            Assert.IsNotNull(json);
            Assert.IsTrue(TogglDesktop.Toggl.SetLoggedInUser(json));
        }

        [TestCleanup]
        public void Cleanup()
        {
            TogglDesktop.Toggl.Clear();
        }

        [TestMethod]
        public void TestShowApp()
        {
            TogglDesktop.Toggl.ShowApp();
        }

        [TestMethod]
        public void TestSignup()
        {
            // Signing up with fake data in test mode should fail
            Assert.IsFalse(TogglDesktop.Toggl.Signup("john@doe.com", "foobar"));
        }

        [TestMethod]
        public void TestLogin()
        {
            // Logging in with a fake user should fail
            Assert.IsFalse(TogglDesktop.Toggl.Login("john@doe.com", "foobar"));
        }

        [TestMethod]
        public void TestGoogleLogin()
        {
            // Logging in with a fake token should fail
            Assert.IsFalse(TogglDesktop.Toggl.GoogleLogin("faketoken"));
        }

        [TestMethod]
        public void TestPasswordForgot()
        {
            TogglDesktop.Toggl.PasswordForgot();
        }

        [TestMethod]
        public void TestOpenInBrowser()
        {
            TogglDesktop.Toggl.OpenInBrowser();
        }

        [TestMethod]
        public void TestSendFeedback()
        {
            Assert.IsTrue(TogglDesktop.Toggl.SendFeedback("topic", "details", "filename"));
        }

        [TestMethod]
        public void TestViewTimeEntryList()
        {
            TogglDesktop.Toggl.ViewTimeEntryList();
            // FIXME: that TE list was rendered
        }

        [TestMethod]
        public void TestEdit()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f"; 
            TogglDesktop.Toggl.Edit(guid, false, "description");
            // FIXME: check that TE editor was opened
        }

        [TestMethod]
        public void TestEditPreferences()
        {
            TogglDesktop.Toggl.EditPreferences();
            // FIXME: check that prefs were rendered
        }

        [TestMethod]
        public void TestContinue()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f"; 
            Assert.IsTrue(TogglDesktop.Toggl.Continue(guid));
            // FIXME: check that TE "07fba193-91c4-0ec8-2894-820df0548a8f" is running now
        }

        [TestMethod]
        public void TestContinueLatest()
        {
            Assert.IsTrue(TogglDesktop.Toggl.ContinueLatest());
            // FIXME: check that TE is really running
        }

        [TestMethod]
        public void TestDeleteTimeEntry()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
            Assert.IsTrue(TogglDesktop.Toggl.DeleteTimeEntry(guid));
            // FIXME: that TE is really gone
        }

        [TestMethod]
        public void TestSetTimeEntryDuration()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
            Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryDuration(guid, "1 hour"));
            // FIXME: check duration
        }

        [TestMethod]
        public void TestSetTimeEntryProject()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
            uint task_id = 0;
            uint project_id = 2567324;
            string project_guid = null;
            Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryProject(guid, task_id, project_id, project_guid));
            // FIXME: check project
        }

        [TestMethod]
        public void TestSetTimeEntryStart()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
            Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryStart(guid, "12:34"));
            // FIXME: check start
        }

        [TestMethod]
        public void TestSetTimeEntryDate()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
            Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryDate(guid, DateTime.Now));
            // FIXME: check date
        }

        [TestMethod]
        public void TestSetTimeEntryEnd()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
            Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryEnd(guid, "23:45"));
            // FIXME: check end
        }

        [TestMethod]
        public void TestSetTimeEntryTags()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
            List<string> tags = new List<string>() { "John", "Anna", "Monica" };
            Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryTags(guid, tags));
            // FIXME: check tags
        }

        [TestMethod]
        public void TestSetTimeEntryBillable()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
            Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryBillable(guid, true));
            // FIXME: check billable flag
        }

        [TestMethod]
        public void TestSetTimeEntryDescription()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
            Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryDescription(guid, "blah"));
            // FIXME: check description
        }

        [TestMethod]
        public void TestStop()
        {
            string guid = TogglDesktop.Toggl.Start("doing stuff", "", 0, 0, "", "");
            Assert.IsNotNull(guid);
            Assert.AreNotEqual("", guid);
            Assert.IsTrue(TogglDesktop.Toggl.Stop());
        }

        [TestMethod]
        public void TestStart()
        {
            string guid = TogglDesktop.Toggl.Start("doing stuff", "", 0, 0, "", null);
            Assert.IsNotNull(guid);
            Assert.AreNotEqual("", guid);
            // FIXME: check that timer got updated
        }

        [TestMethod]
        public void TestDiscardAt()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
            bool split = false;
            ulong at = (ulong)TogglDesktop.Toggl.UnixFromDateTime(DateTime.Now);
            Assert.IsTrue(TogglDesktop.Toggl.DiscardTimeAt(guid, at-1, split));
            split = true;
            Assert.IsTrue(TogglDesktop.Toggl.DiscardTimeAt(guid, at, split));
            // FIXME: check that TE was actually split 2 times
        }

        [TestMethod]
        public void TestSetWindowSettings()
        {
            Assert.IsTrue(TogglDesktop.Toggl.SetWindowSettings(1, 2, 3, 4));
            long x = 0, y = 0, h = 0, w = 0;
            Assert.IsTrue(TogglDesktop.Toggl.WindowSettings(ref x, ref y, ref h, ref w));
        }

        [TestMethod]
        public void TestSetSettings()
        {
            TogglDesktop.Toggl.TogglSettingsView settings = new TogglDesktop.Toggl.TogglSettingsView();
            Assert.IsTrue(TogglDesktop.Toggl.SetSettings(settings));
            // FIXME: twiddle with settings flags and see if thy actually were re-rendered via callbacks
        }

        [TestMethod]
        public void TestIsTimelineRecordingEnabled()
        {
            Assert.IsTrue(TogglDesktop.Toggl.IsTimelineRecordingEnabled());
        }

        [TestMethod]
        public void TestLogout()
        {
            Assert.IsTrue(TogglDesktop.Toggl.Logout());
            // FIXME: check that login view was displayed
        }

        [TestMethod]
        public void TestClearCache()
        {
            Assert.IsTrue(TogglDesktop.Toggl.ClearCache());
            // FIXME: check that user is gone now
        }

        [TestMethod]
        public void TestAddProject()
        {
            string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
            ulong workspace_id = 123456789;
            ulong client_id = 0;
            string client_guid = null;
            string project_name = "testing project adding";
            bool is_private = false;
            string project_guid = TogglDesktop.Toggl.AddProject(guid,
                workspace_id,
                client_id,
                client_guid,
                project_name,
                is_private);
            Assert.IsNotNull(project_guid);
            Assert.AreNotEqual("", project_guid);
            // FIXME: check that TE has the project attached now
            // FIXME: check that the project exists now
        }

        [TestMethod]
        public void TestCreateClient()
        {
            ulong workspace_id = 123456789;
            string guid = TogglDesktop.Toggl.CreateClient(workspace_id, "new test client");
            Assert.IsNotNull(guid);
            Assert.AreNotEqual("", guid);
            // FIXME: check that the client exists now
        }

        [TestMethod]
        public void TestSetUpdateChannel()
        {
            Assert.IsFalse(TogglDesktop.Toggl.SetUpdateChannel("nonexisting channel"));
            Assert.IsTrue(TogglDesktop.Toggl.SetUpdateChannel("stable"));
            string channel = TogglDesktop.Toggl.UpdateChannel();
        }

        [TestMethod]
        public void TestUserEmail()
        {
            Assert.AreEqual("johnsmith@toggl.com", TogglDesktop.Toggl.UserEmail());
        }

        [TestMethod]
        public void TestSync()
        {
            TogglDesktop.Toggl.Sync();
        }

        [TestMethod]
        public void TestSetSleep()
        {
            TogglDesktop.Toggl.SetSleep();
        }

        [TestMethod]
        public void TestSetWake()
        {
            TogglDesktop.Toggl.SetWake();
        }

        [TestMethod]
        public void TestSetIdleSeconds()
        {
            TogglDesktop.Toggl.SetIdleSeconds(123);
        }

        [TestMethod]
        public void Test()
        {
            string formatted = TogglDesktop.Toggl.FormatDurationInSecondsHHMMSS(123);
            Assert.AreEqual("02:03", formatted);
        }
    }
}
