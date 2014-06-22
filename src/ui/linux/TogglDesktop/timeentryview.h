#ifndef TIMEENTRYVIEW_H
#define TIMEENTRYVIEW_H

#include <QObject>
#include <QVector>

#include "kopsik_api.h"

class TimeEntryView : public QObject
{
    Q_OBJECT
public:
    explicit TimeEntryView(QObject *parent = 0);

    static TimeEntryView *importOne(KopsikTimeEntryViewItem *view);
    static QVector<TimeEntryView *> importAll(KopsikTimeEntryViewItem *first);

    const QString timeOverview();
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
    bool DurOnly;
    bool IsHeader;
    bool CanAddProjects;
    bool CanSeeBillable;
    uint64_t DefaultWID;

signals:

public slots:

};

#endif // TIMEENTRYVIEW_H
