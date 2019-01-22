// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_SETTINGSVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_SETTINGSVIEW_H_

#include <QObject>
#include <QTime>

#include "./toggl_api.h"

class SettingsView : public QObject {
    Q_OBJECT

 public:
    explicit SettingsView(QObject *parent = 0);

    static SettingsView *importOne(TogglSettingsView *view) {
        SettingsView *result = new SettingsView();
        result->AutodetectProxy = view->AutodetectProxy;
        result->UseProxy = view->UseProxy;
        result->ProxyHost = QString(view->ProxyHost);
        result->ProxyPort = view->ProxyPort;
        result->ProxyUsername = QString(view->ProxyUsername);
        result->ProxyPassword = QString(view->ProxyPassword);
        result->UseIdleDetection = view->UseIdleDetection;
        result->MenubarTimer = view->MenubarTimer;
        result->DockIcon = view->DockIcon;
        result->OnTop = view->OnTop;
        result->Reminder = view->Reminder;
        result->RecordTimeline = view->RecordTimeline;
        result->IdleMinutes = view->IdleMinutes;
        result->ReminderMinutes = view->ReminderMinutes;
        result->ManualMode = view->ManualMode;
        result->FocusOnShortcut = view->FocusOnShortcut;
        result->Pomodoro = view->Pomodoro;
        result->PomodoroMinutes = view->PomodoroMinutes;
        result->PomodoroBreak = view->PomodoroBreak;
        result->PomodoroBreakMinutes = view->PomodoroBreakMinutes;
        result->RemindOnMonday = view->RemindMon;
        result->RemindOnTuesday = view->RemindTue;
        result->RemindOnWednesday = view->RemindWed;
        result->RemindOnThursday = view->RemindThu;
        result->RemindOnFriday = view->RemindFri;
        result->RemindOnSaturday = view->RemindSat;
        result->RemindOnSunday = view->RemindSun;
        result->RemindStartTime = QTime::fromString(view->RemindStarts, "HH:mm");
        result->RemindEndTime = QTime::fromString(view->RemindEnds, "HH:mm");
        result->StopEntryOnShutdownSleep = view->StopEntryOnShutdownSleep;
        return result;
    }

    bool UseProxy;
    QString ProxyHost;
    uint64_t ProxyPort;
    QString ProxyUsername;
    QString ProxyPassword;
    bool UseIdleDetection;
    bool MenubarTimer;
    bool DockIcon;
    bool OnTop;
    bool Reminder;
    bool RecordTimeline;
    uint64_t IdleMinutes;
    uint64_t ReminderMinutes;
    bool ManualMode;
    bool AutodetectProxy;
    bool FocusOnShortcut;
    bool Pomodoro;
    uint64_t PomodoroMinutes;
    bool PomodoroBreak;
    uint64_t PomodoroBreakMinutes;
    bool RemindOnMonday;
    bool RemindOnTuesday;
    bool RemindOnWednesday;
    bool RemindOnThursday;
    bool RemindOnFriday;
    bool RemindOnSaturday;
    bool RemindOnSunday;
    QTime RemindStartTime;
    QTime RemindEndTime;
    bool StopEntryOnShutdownSleep;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_SETTINGSVIEW_H_
