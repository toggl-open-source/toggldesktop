using System;
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
            // FIXME: check result
            TogglDesktop.Toggl.Signup("john@doe.com", "foobar");
        }

        [TestMethod]
        public void TestLogin()
        {
            // FIXME: check result
            TogglDesktop.Toggl.Login("john@doe.com", "foobar");
        }

        [TestMethod]
        public void TestGoogleLogin()
        {
            // FIXME: check result
            TogglDesktop.Toggl.GoogleLogin("faketoken");
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
            // FIXME: check result
            TogglDesktop.Toggl.SendFeedback("topic", "details", "filename");
        }

        [TestMethod]
        public void TestViewTimeEntryList()
        {
            TogglDesktop.Toggl.ViewTimeEntryList();
        }

        [TestMethod]
        public void TestEdit()
        {
            string guid = "fakeguid";
            TogglDesktop.Toggl.Edit(guid, false, "description");
        }

        [TestMethod]
        public void TestEditPreferences()
        {
            TogglDesktop.Toggl.EditPreferences();
        }

        [TestMethod]
        public void TestContinue()
        {
            string guid = "fakeguid";
            // FIXME: check result
            TogglDesktop.Toggl.Continue(guid);
        }

        [TestMethod]
        public void TestContinueLatest()
        {
            // FIXME: check result
            TogglDesktop.Toggl.ContinueLatest();
        }

        [TestMethod]
        public void TestDeleteTimeEntry()
        {
            string guid = "fakeguid";
            TogglDesktop.Toggl.DeleteTimeEntry(guid);
        }

        [TestMethod]
        public void TestSetTimeEntryDuration()
        {
            // FIXME: check result
            string guid = "fakeguid";
            TogglDesktop.Toggl.SetTimeEntryDuration(guid, "1 hour");
        }

        [TestMethod]
        public void TestSetTimeEntryProject()
        {
            string guid = "fakeguid";
            uint task_id = 1;
            uint project_id = 2;
            string project_guid = "fakeprojectguid";
            // FIXME: check result
            TogglDesktop.Toggl.SetTimeEntryProject(guid, task_id, project_id, project_guid);
        }

        [TestMethod]
        public void TestSetTimeEntryStart()
        {
            string guid = "fakeguid";
            // FIXME: check result
            TogglDesktop.Toggl.SetTimeEntryStart(guid, "12:34");
        }

        [TestMethod]
        public void TestSetTimeEntryDate()
        {
            string guid = "fakeguid";
            // FIXME: check result
            TogglDesktop.Toggl.SetTimeEntryDate(guid, DateTime.Now);
        }

        [TestMethod]
        public void TestSetTimeEntryEnd()
        {
            string guid = "fakeguid";
            // FIXME: check result
            TogglDesktop.Toggl.SetTimeEntryEnd(guid, "23:45");
        }

        [TestMethod]
        public void TestSetTimeEntryTags()
        {
            string guid = "fakeguid";
            List<string> tags = new List<string>() {"John", "Anna", "Monica"};
            // FIXME: check result
            TogglDesktop.Toggl.SetTimeEntryTags(guid, tags);
        }

        [TestMethod]
        public void TestSetTimeEntryBillable()
        {
            string guid = "fakeguid";
            // FIXME: check result
            TogglDesktop.Toggl.SetTimeEntryBillable(guid, true);
        }

        [TestMethod]
        public void TestSetTimeEntryDescription()
        {
            string guid = "fakeguid";
            // FIXME: check result
            TogglDesktop.Toggl.SetTimeEntryDescription(guid, "blah");
        }

        [TestMethod]
        public void TestStop()
        {
            string guid = TogglDesktop.Toggl.Start("doing stuff", "", 0, 0, "", "");
            // FIXME: check result
            TogglDesktop.Toggl.Stop();
        }

    }
}
