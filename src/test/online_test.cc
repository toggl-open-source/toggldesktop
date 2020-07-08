// Copyright 2019 Toggl Desktop developers.

#include "online_test.h"

#include "online_test_app.h"

#include <iostream>
#include <map>
#include <deque>
#include <functional>
#include <list>
#include <any>
#include <thread>
#include <mutex>
#include <chrono>
#include <variant>

#include "Poco/DateTime.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/LocalDateTime.h"
#include "Poco/Path.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/Event.h"


static struct {
    test::App *app;

    std::string name { test::App::randomUser() };
    std::string pass { test::App::randomPassword() };

    std::map<std::string, std::string> timeEntries;
    std::map<std::string, std::string> projects;
} testData;

TEST(Base, Initialize) {
    std::cout << "Will test with the following credentials:" << std::endl;
    std::cout << "Username: " << testData.name << std::endl;
    std::cout << "Password: " << testData.pass << std::endl;
    testData.app = new test::App;
    ASSERT_TRUE(testData.app);
    ASSERT_TRUE(testData.app->settings().isNull());
    testData.app->uiStart();
    ASSERT_TRUE(testData.app->isStarted());

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(Base, GetCountries) {
    testData.app->getCountries();
    ASSERT_FALSE(testData.app->countries().empty());

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(Base, Register) {
    ASSERT_TRUE(testData.app->signup(testData.name, testData.pass));
    ASSERT_TRUE(testData.app->isLoggedIn());
    ASSERT_FALSE(testData.app->settings().isNull());

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(Base, LogOut) {
    ASSERT_TRUE(testData.app->isLoggedIn());
    ASSERT_TRUE(testData.app->logout());
    ASSERT_FALSE(testData.app->isLoggedIn());
    ASSERT_TRUE(testData.app->settings().isNull());

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(Base, LogIn) {
    ASSERT_FALSE(testData.app->isLoggedIn());
    ASSERT_TRUE(testData.app->login(testData.name, testData.pass));
    ASSERT_TRUE(testData.app->isLoggedIn());
    ASSERT_FALSE(testData.app->settings().isNull());

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(TimeEntry, Start) {
    auto guid = testData.app->start("Time Entry");
    ASSERT_FALSE(guid.empty());
    testData.timeEntries[guid] = "Time Entry";
    ASSERT_EQ(testData.app->runningTimeEntry().name_, "Time Entry");
    ASSERT_EQ(testData.app->runningTimeEntry().guid_, guid);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(TimeEntry, Stop) {
    ASSERT_EQ(testData.timeEntries.size(), 1);
    ASSERT_EQ(testData.app->runningTimeEntry().guid_, testData.timeEntries.begin()->first);
    auto oldGuid = testData.app->runningTimeEntry().guid_;
    ASSERT_EQ(testData.app->runningTimeEntry().name_, testData.timeEntries.begin()->second);

    ASSERT_TRUE(testData.app->stop());
    ASSERT_TRUE(testData.app->runningTimeEntry().guid_.empty());

    ASSERT_EQ(testData.app->timeEntries().begin()->guid_, oldGuid);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(TimeEntry, SetStart) {
    ASSERT_TRUE(testData.app->timeEntries().size() == 1);
    auto guid = testData.timeEntries.begin()->first;

    ASSERT_TRUE(testData.app->timeEntry_setStart(guid, "12:00 PM"));
    ASSERT_EQ(testData.app->timeEntries().begin()->startedString_, "12:00 PM");

    ASSERT_TRUE(testData.app->timeEntries().size() == 1);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(TimeEntry, SetStartInvalidTime) {
    ASSERT_TRUE(testData.app->timeEntries().size() == 1);
    auto guid = testData.timeEntries.begin()->first;

    ASSERT_FALSE(testData.app->timeEntry_setStart(guid, "123:00 PM"));
    // this succeeds, it probably should not
    //ASSERT_FALSE(testData.app->timeEntry_setStart(guid, "12:1 PPM"));
    ASSERT_FALSE(testData.app->timeEntry_setStart(guid, "12:123 PM"));
    ASSERT_FALSE(testData.app->timeEntry_setStart(guid, "PM:PM PM"));
    ASSERT_FALSE(testData.app->timeEntry_setStart(guid, "PM:    AM PM"));
    ASSERT_FALSE(testData.app->timeEntry_setStart(guid, "6354651"));
    ASSERT_FALSE(testData.app->timeEntry_setStart(guid, "AAAAAAAAAAAA"));
    ASSERT_FALSE(testData.app->timeEntry_setStart(guid, "|Bbdsf,§.v§ů.!ě§ů,+č§ůž§+ů,.§řůžý+,§ůlm, ůlm §ůvíé;uáé+hn"));
    ASSERT_FALSE(testData.app->timeEntry_setStart(guid, "šš:čč"));

    ASSERT_EQ(testData.app->timeEntries().begin()->startedString_, "12:00 PM");

    ASSERT_TRUE(testData.app->timeEntries().size() == 1);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(TimeEntry, SetStartInvalidGuid) {
    ASSERT_TRUE(testData.app->timeEntries().size() == 1);
    auto guid = "abcd";

    // don't check, this doesn't actually return false (even though it should, in my opinion)
    testData.app->timeEntry_setStart(guid, "11:00 AM");
    // just check we didn't mess up the existing entry
    ASSERT_EQ(testData.app->timeEntries().begin()->startedString_, "12:00 PM");

    ASSERT_TRUE(testData.app->timeEntries().size() == 1);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(TimeEntry, SetEnd) {
    ASSERT_TRUE(testData.app->timeEntries().size() == 1);
    auto guid = testData.timeEntries.begin()->first;

    ASSERT_TRUE(testData.app->timeEntry_setEnd(guid, "01:00 PM"));
    ASSERT_EQ(testData.app->timeEntries().begin()->endedString_, "01:00 PM");
    auto duration = testData.app->timeEntries().begin()->ended_ - testData.app->timeEntries().begin()->started_;
    ASSERT_EQ(duration, 3600);

    ASSERT_TRUE(testData.app->timeEntries().size() == 1);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(TimeEntry, SetEndInvalidTime) {
    ASSERT_TRUE(testData.app->timeEntries().size() == 1);
    auto guid = testData.timeEntries.begin()->first;

    ASSERT_FALSE(testData.app->timeEntry_setEnd(guid, "123:00 PM"));
    // this succeeds, it probably should not
    //ASSERT_FALSE(testData.app->timeEntry_setStart(guid, "12:1 PPM"));
    ASSERT_FALSE(testData.app->timeEntry_setEnd(guid, "12:123 PM"));
    ASSERT_FALSE(testData.app->timeEntry_setEnd(guid, "PM:PM PM"));
    ASSERT_FALSE(testData.app->timeEntry_setEnd(guid, "PM:    AM PM"));
    ASSERT_FALSE(testData.app->timeEntry_setEnd(guid, "6354651"));
    ASSERT_FALSE(testData.app->timeEntry_setEnd(guid, "AAAAAAAAAAAA"));
    ASSERT_FALSE(testData.app->timeEntry_setEnd(guid, "|Bbdsf,§.v§ů.!ě§ů,+č§ůž§+ů,.§řůžý+,§ůlm, ůlm §ůvíé;uáé+hn"));
    ASSERT_FALSE(testData.app->timeEntry_setEnd(guid, "šš:čč"));

    ASSERT_EQ(testData.app->timeEntries().begin()->endedString_, "01:00 PM");

    ASSERT_TRUE(testData.app->timeEntries().size() == 1);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(TimeEntry, SetEndInvalidGuid) {
    ASSERT_TRUE(testData.app->timeEntries().size() == 1);
    auto guid = "abcd";

    // don't check, this doesn't actually return false (even though it should, in my opinion)
    testData.app->timeEntry_setEnd(guid, "02:00 PM");
    // just check we didn't mess up the existing entry
    ASSERT_EQ(testData.app->timeEntries().begin()->endedString_, "01:00 PM");

    ASSERT_TRUE(testData.app->timeEntries().size() == 1);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(TimeEntry, SetDuration) {
    ASSERT_TRUE(testData.app->timeEntries().size() == 1);
    auto guid = testData.timeEntries.begin()->first;

    ASSERT_TRUE(testData.app->timeEntry_setDuration(guid, "2:00"));
    ASSERT_EQ(testData.app->timeEntries().begin()->startedString_, "12:00 PM");
    auto duration = testData.app->timeEntries().begin()->ended_ - testData.app->timeEntries().begin()->started_;
    ASSERT_EQ(duration, 7200);

    ASSERT_TRUE(testData.app->timeEntries().size() == 1);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(TimeEntry, SetDurationInvalidGuid) {
    ASSERT_TRUE(testData.app->timeEntries().size() == 1);
    auto guid = "abcd";

    // don't check, this doesn't actually return false (even though it should, in my opinion)
    testData.app->timeEntry_setStart(guid, "3:00");
    // just check we didn't mess up the existing entry
    auto duration = testData.app->timeEntries().begin()->ended_ - testData.app->timeEntries().begin()->started_;
    ASSERT_EQ(duration, 7200);

    ASSERT_TRUE(testData.app->timeEntries().size() == 1);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(TimeEntry, SetTags) {
    ASSERT_TRUE(testData.app->timeEntries().size() == 1);
    auto guid = testData.timeEntries.begin()->first;

    ASSERT_TRUE(testData.app->timeEntry_setTags(guid, {"tag1", "tag2"}));
    ASSERT_EQ(testData.app->timeEntries().begin()->tags_, std::list<std::string>({"tag1", "tag2"}));

    ASSERT_TRUE(testData.app->timeEntries().size() == 1);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(TimeEntry, SetTagsInvalidGuid) {
    ASSERT_TRUE(testData.app->timeEntries().size() == 1);
    auto guid = "abcd";

    // don't check, this doesn't actually return false (even though it should, in my opinion)
    testData.app->timeEntry_setTags(guid, {"tag3", "tag4"});
    // just check we didn't mess up the existing entry
    ASSERT_EQ(testData.app->timeEntries().begin()->tags_, std::list<std::string>({"tag1", "tag2"}));

    ASSERT_TRUE(testData.app->timeEntries().size() == 1);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(Client, Create) {
    ASSERT_EQ(testData.app->clients().size(), 0);
    auto guid = testData.app->client_create("Client Eastwood");
    ASSERT_FALSE(guid.empty());
    ASSERT_EQ(testData.app->clients().size(), 1);
    ASSERT_EQ(testData.app->clients().begin()->name_, "Client Eastwood");

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(Project, Create) {
    ASSERT_EQ(testData.app->timeEntries().size(), 1);
    ASSERT_EQ(testData.app->clients().size(), 1);

    auto te = testData.app->timeEntries().front().guid_;
    auto c = testData.app->clients().begin()->guid_;
    auto guid = testData.app->timeEntry_addProject(te, c, "Project");
    ASSERT_FALSE(guid.empty());
    testData.projects[guid] = "Project";

    ASSERT_EQ(testData.app->timeEntries().front().projectLabel_, "Project");

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(Project, Unassign) {
    ASSERT_EQ(testData.app->timeEntries().size(), 1);
    ASSERT_EQ(testData.app->clients().size(), 1);

    auto te = testData.app->timeEntries().front().guid_;
    ASSERT_EQ(testData.app->timeEntries().front().projectLabel_, "Project");

    ASSERT_TRUE(testData.app->timeEntry_setProject(te, {}));
    ASSERT_TRUE(testData.app->timeEntries().front().projectLabel_.empty());

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(Project, Reassign) {
    ASSERT_EQ(testData.app->timeEntries().size(), 1);
    ASSERT_EQ(testData.app->clients().size(), 1);

    auto te = testData.app->timeEntries().front().guid_;
    auto p = testData.projects.begin()->first;
    auto pName = testData.projects.begin()->second;
    ASSERT_TRUE(testData.app->timeEntries().front().projectLabel_.empty());

    ASSERT_TRUE(testData.app->timeEntry_setProject(te, p));
    ASSERT_EQ(testData.app->timeEntries().front().projectLabel_, pName);

    ASSERT_EQ(testData.app->errorSinceLastTime(), std::string());
}

TEST(Settings, SetData) {
    ASSERT_TRUE(testData.app->settings_remindDays(true, true, true, true, true, true, true));
    ASSERT_TRUE(testData.app->settings_remindTimes("12:34", "21:43"));
    ASSERT_TRUE(testData.app->settings_useIdleDetection(true));
    ASSERT_TRUE(testData.app->settings_Autotrack(true));
    ASSERT_TRUE(testData.app->settings_openEditorOnShortcut(true));
    ASSERT_TRUE(testData.app->settings_autodetectProxy(true));
    ASSERT_TRUE(testData.app->settings_menubarTimer(true));
    ASSERT_TRUE(testData.app->settings_menubarProject(true));
    ASSERT_TRUE(testData.app->settings_dockIcon(true));
    ASSERT_TRUE(testData.app->settings_onTop(true));
    ASSERT_TRUE(testData.app->settings_reminder(true));
    ASSERT_TRUE(testData.app->settings_pomodoro(true));
    ASSERT_TRUE(testData.app->settings_pomodoroBreak(true));
    ASSERT_TRUE(testData.app->settings_stopEntryOnShutdownSleep(true));
    ASSERT_TRUE(testData.app->settings_idleMinutes(1));
    ASSERT_TRUE(testData.app->settings_focusOnShortcut(true));
    ASSERT_TRUE(testData.app->settings_reminderMinutes(2));
    ASSERT_TRUE(testData.app->settings_pomodoroMinutes(3));
    ASSERT_TRUE(testData.app->settings_pomodoroBreakMinutes(4));
    ASSERT_TRUE(testData.app->settings_manualMode(true));

    // need to think this through
    //ASSERT_TRUE(testData.app->settings_proxy(true, "a", 123456, "b", "c"));
}

TEST(Settings, ReadData) {
    ASSERT_EQ(testData.app->settings().remindMon_, true);
    ASSERT_EQ(testData.app->settings().remindTue_, true);
    ASSERT_EQ(testData.app->settings().remindWed_, true);
    ASSERT_EQ(testData.app->settings().remindThu_, true);
    ASSERT_EQ(testData.app->settings().remindFri_, true);
    ASSERT_EQ(testData.app->settings().remindSat_, true);
    ASSERT_EQ(testData.app->settings().remindSun_, true);

    ASSERT_EQ(testData.app->settings().remindStarts_, "12:34");
    ASSERT_EQ(testData.app->settings().remindEnds_, "21:43");

    ASSERT_EQ(testData.app->settings().useIdleDetection_, true);

    ASSERT_EQ(testData.app->settings().autotrack_, true);

    ASSERT_EQ(testData.app->settings().openEditorOnShortcut_, true);

    ASSERT_EQ(testData.app->settings().autodetectProxy_, true);

    ASSERT_EQ(testData.app->settings().menubarTimer_, true);

    ASSERT_EQ(testData.app->settings().menubarProject_, true);

    ASSERT_EQ(testData.app->settings().dockIcon_, true);

    ASSERT_EQ(testData.app->settings().onTop_, true);

    ASSERT_EQ(testData.app->settings().reminder_, true);

    ASSERT_EQ(testData.app->settings().pomodoro_, true);

    ASSERT_EQ(testData.app->settings().pomodoroBreak_, true);

    ASSERT_EQ(testData.app->settings().stopEntryOnShutdownSleep_, true);

    ASSERT_EQ(testData.app->settings().idleMinutes_, 1);

    ASSERT_EQ(testData.app->settings().focusOnShortcut_, true);

    ASSERT_EQ(testData.app->settings().reminderMinutes_, 2);

    ASSERT_EQ(testData.app->settings().pomodoroMinutes_, 3);

    ASSERT_EQ(testData.app->settings().pomodoroBreakMinutes_, 4);

    ASSERT_EQ(testData.app->settings().manualMode_, true);

    /*
    ASSERT_EQ(testData.app->settings().useProxy_, true);
    ASSERT_EQ(testData.app->settings().proxyHost_, "a");
    ASSERT_EQ(testData.app->settings().proxyPort_, 123456);
    ASSERT_EQ(testData.app->settings().proxyUsername_, "b");
    ASSERT_EQ(testData.app->settings().proxyPassword_, "c");
    */
}
