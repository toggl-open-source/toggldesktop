using System;
using System.IO;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace TogglDesktopDLLInteropTest
{
[TestClass]
public class LibTest
{
    private static List<TogglDesktop.Toggl.TogglTimeEntryView> timeentries;

    private static TogglDesktop.Toggl.TogglTimeEntryView editorTimeEntry;
    private static string focusedFieldName;
    private static bool openTimeEntryEditor;
    private static bool openTimeEntryList;

    private static TogglDesktop.Toggl.TogglSettingsView settings;
    private static bool openSettings;

    private static TogglDesktop.Toggl.TogglTimeEntryView timerTimeEntry;
    private static bool running;

    private static bool openLogin;
    private static UInt64 userID;

    private static List<TogglDesktop.Toggl.TogglGenericView> clients;

    private static List<TogglDesktop.Toggl.TogglAutocompleteView> projectAutocomplete;

    [ClassInitialize]
    public static void ClassInit(TestContext context)
    {
        TogglDesktop.Toggl.Env = "test";

        Assert.IsTrue(TogglDesktop.Toggl.StartUI("0.0.0", null));

        Assert.AreEqual("test", TogglDesktop.Toggl.Env);

        TogglDesktop.Toggl.OnTimeEntryList +=Toggl_OnTimeEntryList;
        TogglDesktop.Toggl.OnTimeEntryEditor += Toggl_OnTimeEntryEditor;
        TogglDesktop.Toggl.OnSettings += Toggl_OnSettings;
        TogglDesktop.Toggl.OnRunningTimerState += Toggl_OnRunningTimerState;
        TogglDesktop.Toggl.OnStoppedTimerState += Toggl_OnStoppedTimerState;
        TogglDesktop.Toggl.OnLogin += Toggl_OnLogin;
        TogglDesktop.Toggl.OnProjectAutocomplete += Toggl_OnProjectAutocomplete;
        TogglDesktop.Toggl.OnClientSelect += Toggl_OnClientSelect;
    }

    static void Toggl_OnClientSelect(List<TogglDesktop.Toggl.TogglGenericView> list)
    {
        clients = list;
    }

    static void Toggl_OnProjectAutocomplete(List<TogglDesktop.Toggl.TogglAutocompleteView> list)
    {
        projectAutocomplete = list;
    }

    static void Toggl_OnLogin(bool open, ulong user_id)
    {
        openLogin = open;
        userID = user_id;
    }

    static void Toggl_OnStoppedTimerState()
    {
        running = false;
        timerTimeEntry = new TogglDesktop.Toggl.TogglTimeEntryView();
    }

    static void Toggl_OnRunningTimerState(TogglDesktop.Toggl.TogglTimeEntryView te)
    {
        running = true;
        timerTimeEntry = te;
    }

    static void Toggl_OnSettings(bool open, TogglDesktop.Toggl.TogglSettingsView s)
    {
        openSettings = true;
        settings = s;
    }

    static void Toggl_OnTimeEntryEditor(bool open, TogglDesktop.Toggl.TogglTimeEntryView te, string focused_field_name)
    {
        openTimeEntryEditor = open;
        editorTimeEntry = te;
        focusedFieldName = focused_field_name;
    }

    static void Toggl_OnTimeEntryList(bool open, List<TogglDesktop.Toggl.TogglTimeEntryView> list, bool showLoadMoreButton)
    {
        openTimeEntryList = open;
        timeentries = list;
    }

    [ClassCleanup]
    public static void ClassCleanup()
    {
        TogglDesktop.Toggl.Clear();
    }

    [TestInitialize]
    public void Initialize()
    {
        openTimeEntryList = false;
        timeentries = null;

        openTimeEntryEditor = false;
        focusedFieldName = null;
        editorTimeEntry = new TogglDesktop.Toggl.TogglTimeEntryView();

        openSettings = false;
        settings = new TogglDesktop.Toggl.TogglSettingsView();

        running = false;
        timerTimeEntry = new TogglDesktop.Toggl.TogglTimeEntryView();

        openLogin = false;
        userID = 0;

        clients = null;

        projectAutocomplete = null;

        string path = Path.Combine(Directory.GetCurrentDirectory(), "me.json");
        Assert.AreNotEqual("", path);
        Assert.IsNotNull(path);

        string json = File.ReadAllText(path);
        Assert.AreNotEqual("", json);
        Assert.IsNotNull(json);
        Assert.IsTrue(TogglDesktop.Toggl.SetLoggedInUser(json));
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
        string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";

        TogglDesktop.Toggl.ViewTimeEntryList();
        Assert.IsNotNull(timeentries);
        TogglDesktop.Toggl.TogglTimeEntryView te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNotNull(te);
        Assert.AreEqual(guid, te.GUID);
    }

    [TestMethod]
    public void TestEdit()
    {
        string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
        TogglDesktop.Toggl.Edit(guid, false, "description");
        Assert.IsTrue(openTimeEntryEditor);
        Assert.AreEqual(guid, editorTimeEntry.GUID);
        Assert.AreEqual("description", focusedFieldName);
    }

    [TestMethod]
    public void TestEditPreferences()
    {
        TogglDesktop.Toggl.EditPreferences();
        Assert.IsTrue(openSettings);
        Assert.IsNotNull(settings);
    }

    [TestMethod]
    public void TestContinue()
    {
        string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
        Assert.IsTrue(TogglDesktop.Toggl.Continue(guid));
        Assert.IsTrue(running);
        Assert.IsNotNull(timerTimeEntry);
        Assert.AreNotEqual(guid, timerTimeEntry.GUID);
    }

    [TestMethod]
    public void TestContinueLatest()
    {
        Assert.IsTrue(TogglDesktop.Toggl.ContinueLatest());
        Assert.IsTrue(running);
        Assert.IsNotNull(timerTimeEntry);
    }

    [TestMethod]
    public void TestDeleteTimeEntry()
    {
        TogglDesktop.Toggl.ViewTimeEntryList();

        string guid = "6c97dc31-582e-7662-1d6f-5e9d623b1685";

        TogglDesktop.Toggl.TogglTimeEntryView te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNotNull(te);

        Assert.IsTrue(TogglDesktop.Toggl.DeleteTimeEntry(guid));

        Assert.IsNotNull(timeentries);

        te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNull(te);
    }

    [TestMethod]
    public void TestSetTimeEntryDuration()
    {
        string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
        Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryDuration(guid, "1 hour"));
        TogglDesktop.Toggl.TogglTimeEntryView te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNotNull(te);
        Assert.AreEqual("1:00:00", te.Duration);
    }

    [TestMethod]
    public void TestSetTimeEntryProject()
    {
        string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
        uint task_id = 0;
        uint project_id = 2567324;
        string project_guid = null;
        Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryProject(guid, task_id, project_id, project_guid));
        TogglDesktop.Toggl.TogglTimeEntryView te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNotNull(te);
        Assert.AreEqual(project_id, te.PID);
    }

    [TestMethod]
    public void TestSetTimeEntryStart()
    {
        string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
        Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryStart(guid, "12:34"));
        TogglDesktop.Toggl.TogglTimeEntryView te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNotNull(te);
        Assert.AreEqual("12:34", te.StartTimeString);
    }

    [TestMethod]
    public void TestSetTimeEntryDate()
    {
        DateTime now = DateTime.Now;
        string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
        Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryDate(guid, now));
        TogglDesktop.Toggl.TogglTimeEntryView te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNotNull(te);
        DateTime started = TogglDesktop.Toggl.DateTimeFromUnix(te.Started);
        Assert.AreEqual(now.Year, started.Year);
        Assert.AreEqual(now.Month, started.Month);
        Assert.AreEqual(now.Day, started.Day);
    }

    [TestMethod]
    public void TestSetTimeEntryEnd()
    {
        string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
        Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryEnd(guid, "23:45"));
        TogglDesktop.Toggl.TogglTimeEntryView te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNotNull(te);
        Assert.AreEqual("23:45", te.EndTimeString);
    }

    [TestMethod]
    public void TestSetTimeEntryTags()
    {
        string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
        List<string> tags = new List<string>() { "John", "Anna", "Monica"
        };
        Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryTags(guid, tags));
        TogglDesktop.Toggl.TogglTimeEntryView te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNotNull(te);
        string[] assignedTags = te.Tags.Split('\t');
        Assert.AreEqual(tags.Count, assignedTags.Length);
    }

    [TestMethod]
    public void TestSetTimeEntryBillable()
    {
        string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";

        Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryBillable(guid, true));
        TogglDesktop.Toggl.TogglTimeEntryView te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNotNull(te);
        Assert.IsTrue(te.Billable);

        Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryBillable(guid, false));
        te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNotNull(te);
        Assert.IsFalse(te.Billable);
    }

    [TestMethod]
    public void TestSetTimeEntryDescription()
    {
        string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
        Assert.IsTrue(TogglDesktop.Toggl.SetTimeEntryDescription(guid, "blah"));
        TogglDesktop.Toggl.TogglTimeEntryView te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNotNull(te);
        Assert.AreEqual("blah", te.Description);
    }

    [TestMethod]
    public void TestStop()
    {
        string guid = TogglDesktop.Toggl.Start("doing stuff", "", 0, 0, "", "");
        Assert.IsNotNull(guid);
        Assert.AreNotEqual("", guid);
        Assert.AreEqual(guid, timerTimeEntry.GUID);
        Assert.IsTrue(running);

        Assert.IsTrue(TogglDesktop.Toggl.Stop());
        Assert.IsFalse(running);
        Assert.AreNotEqual(guid, timerTimeEntry.GUID);
    }

    [TestMethod]
    public void TestStart()
    {
        string guid = TogglDesktop.Toggl.Start("doing stuff", "", 0, 0, "", "");
        Assert.IsNotNull(guid);
        Assert.AreNotEqual("", guid);
        Assert.IsTrue(running);
        Assert.AreEqual(guid, timerTimeEntry.GUID);
    }

    [TestMethod]
    public void TestDiscardAt()
    {
        string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
        ulong at = (ulong)TogglDesktop.Toggl.UnixFromDateTime(DateTime.Now);

        int count = timeentries.Count;

        bool split = false;
        Assert.IsTrue(TogglDesktop.Toggl.DiscardTimeAt(guid, at - 1, split));
        Assert.AreEqual(count, timeentries.Count);

        split = true;
        Assert.IsTrue(TogglDesktop.Toggl.DiscardTimeAt(guid, at, split));
        Assert.AreEqual(count + 1, timeentries.Count);
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
        Assert.IsTrue(openLogin);
        Assert.AreEqual((UInt64)0, userID);
    }

    [TestMethod]
    public void TestClearCache()
    {
        Assert.IsTrue(TogglDesktop.Toggl.ClearCache());
        Assert.IsTrue(openLogin);
        Assert.AreEqual((UInt64)0, userID);
    }

    [TestMethod]
    public void TestAddProject()
    {
        int count = projectAutocomplete.Count;

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
                              is_private,
                              null);
        Assert.IsNotNull(project_guid);
        Assert.AreNotEqual("", project_guid);

        TogglDesktop.Toggl.TogglTimeEntryView te = timeentries.Find(x => guid == x.GUID);
        Assert.IsNotNull(te);
        Assert.AreEqual(project_name, te.ProjectLabel);

        Assert.IsNotNull(projectAutocomplete);
        Assert.AreEqual(count + 1, projectAutocomplete.Count);
    }

    [TestMethod]
    public void TestCreateClient()
    {
        ulong workspace_id = 123456789;
        string name = "new test client";
        string guid = TogglDesktop.Toggl.CreateClient(workspace_id, name);
        Assert.IsNotNull(guid);
        Assert.AreNotEqual("", guid);
        Assert.IsNotNull(clients);
        TogglDesktop.Toggl.TogglGenericView c = clients.Find(x => name == x.Name);
        Assert.IsNotNull(c);
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
    public void TestFormatDurationInSecondsHHMMSS()
    {
        string formatted = TogglDesktop.Toggl.FormatDurationInSecondsHHMMSS(123);
        Assert.AreEqual("02:03 min", formatted);
    }
}
}
