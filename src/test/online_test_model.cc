#include "online_test_model.h"

#include <iterator>
#include <iostream>
#include <sstream>

namespace test {

test::Country::Country(const TogglCountryView *view)
    : Model(view ? view->Name : "INVALID", view ? view->ID : 0)
{

}

test::Settings::Settings(const TogglSettingsView *view)
    : Model("")
{
    if (view) {
        useProxy_ = view->UseProxy;
        proxyHost_ = view->ProxyHost;
        proxyPort_ = view->ProxyPort;
        proxyUsername_ = view->ProxyUsername;
        proxyPassword_ = view->ProxyPassword;
        useIdleDetection_ = view->UseIdleDetection;
        menubarTimer_ = view->MenubarTimer;
        menubarProject_ = view->MenubarProject;
        dockIcon_ = view->DockIcon;
        onTop_ = view->OnTop;
        reminder_ = view->Reminder;
        recordTimeline_ = view->RecordTimeline;
        idleMinutes_ = view->IdleMinutes;
        focusOnShortcut_ = view->FocusOnShortcut;
        reminderMinutes_ = view->ReminderMinutes;
        manualMode_ = view->ManualMode;
        autodetectProxy_ = view->AutodetectProxy;
        remindMon_ = view->RemindMon;
        remindTue_ = view->RemindTue;
        remindWed_ = view->RemindWed;
        remindThu_ = view->RemindThu;
        remindFri_ = view->RemindFri;
        remindSat_ = view->RemindSat;
        remindSun_ = view->RemindSun;
        remindStarts_ = view->RemindStarts;
        remindEnds_ = view->RemindEnds;
        autotrack_ = view->Autotrack;
        openEditorOnShortcut_ = view->OpenEditorOnShortcut;
        pomodoro_ = view->Pomodoro;
        pomodoroBreak_ = view->PomodoroBreak;
        pomodoroMinutes_ = view->PomodoroMinutes;
        pomodoroBreakMinutes_ = view->PomodoroBreakMinutes;
        stopEntryOnShutdownSleep_ = view->StopEntryOnShutdownSleep;
    }
}

bool Settings::isNull() const {
    return !(useProxy_ || !proxyHost_.empty() || proxyPort_ || !proxyUsername_.empty() || !proxyPassword_.empty() || useIdleDetection_ || menubarTimer_ || menubarProject_ || dockIcon_ || onTop_ || reminder_ || recordTimeline_ || idleMinutes_ || focusOnShortcut_ || reminderMinutes_ || manualMode_ || autodetectProxy_ || remindMon_ || remindTue_ || remindWed_ || remindThu_ || remindFri_ || remindSat_ || remindSun_ || !remindStarts_.empty() || !remindEnds_.empty() || autotrack_ || openEditorOnShortcut_ || pomodoro_ || pomodoroBreak_ || pomodoroMinutes_ || pomodoroBreakMinutes_ || stopEntryOnShutdownSleep_);
}

test::HelpArticle::HelpArticle(const TogglHelpArticleView *view)
    : Model(view ? view->Name : "")
{

}

test::Autocomplete::Autocomplete(const TogglAutocompleteView *view)
    : Model(view ? view->Text : "")
{

}

test::Tag::Tag(const TogglGenericView *view)
    : Model(view ? view->Name : "", view ? view->ID : 0)
{

}

test::Client::Client(const TogglGenericView *view)
    : Model(view ? view->Name : "", view ? view->ID : 0)
{

}

test::TimeEntry::TimeEntry(const TogglTimeEntryView *view)
    : Model(view ? view->Description : "", view ? view->ID : 0)
{
    if (view) {
        if (view->GUID)
            guid_ = view->GUID;
        started_ = view->Started;
        if (view->StartTimeString)
            startedString_ = view->StartTimeString;
        ended_ = view->Ended;
        if (view->EndTimeString)
            endedString_ = view->EndTimeString;

        if (view->Tags) {
            std::string intags { view->Tags };
            std::stringstream ss(intags);
            std::istream_iterator<std::string> begin(ss);
            std::istream_iterator<std::string> end;
            tags_ = std::list<std::string>(begin, end);
        }

        projectLabel_ = view->ProjectLabel;
        clientLabel_ = view->ClientLabel;
    }
}

test::Workspace::Workspace(const TogglGenericView *view)
    : Model(view ? view->Name : "", view ? view->ID : 0)
{

}

} // namespace test
