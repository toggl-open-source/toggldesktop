// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYVIEW_H_

#include <QObject>
#include <QVector>

#include "./toggl_api.h"

class TimeEntryView : public QObject {
    Q_OBJECT

 public:
    explicit TimeEntryView(QObject *parent = 0);

    static TimeEntryView *importOne(TogglTimeEntryView *view);
    static QVector<TimeEntryView *> importAll(TogglTimeEntryView *first);

    const QString lastUpdate();

    int64_t DurationInSeconds;
    QString Description;
    QString ProjectAndTaskLabel;
    QString ProjectLabel;
    QString ClientLabel;
    uint64_t WID;
    uint64_t PID;
    uint64_t TID;
    QString Duration;
    QString Color;
    QString GUID;
    bool Billable;
    QString Tags;
    uint64_t Started;
    uint64_t Ended;
    QString StartTimeString;
    QString EndTimeString;
    uint64_t UpdatedAt;
    QString DateHeader;
    QString DateDuration;
    bool IsHeader;
    bool CanAddProjects;
    bool CanSeeBillable;
    uint64_t DefaultWID;
    QString WorkspaceName;
    QString Error;
    bool ConfirmlessDelete;
    bool Unsynced;
    // Group mode items
    bool Group;
    bool GroupOpen;
    QString GroupName;
    QString GroupDuration;
    u_int64_t GroupItemCount;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYVIEW_H_
