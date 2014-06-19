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

    static TimeEntryView *importOne(KopsikTimeEntryViewItem *view) {
        TimeEntryView *result = new TimeEntryView();
        result->DurationInSeconds = view->DurationInSeconds;
        result->ProjectAndTaskLabel = view->ProjectAndTaskLabel;
        result->Description = QString(view->Description);
        result->ProjectLabel = QString(view->ProjectLabel);
        result->ClientLabel = QString(view->ClientLabel);
        result->WID = view->WID;
        result->PID = view->PID;
        result->TID = view->TID;
        result->Duration = QString(view->Duration);
        result->Color = QString(view->Color);
        result->GUID = QString(view->GUID);
        result->Billable = view->Billable;
        result->Tags = QString(view->Tags);
        result->Started = view->Started;
        result->Ended = view->Ended;
        result->StartTimeString = QString(view->StartTimeString);
        result->EndTimeString = QString(view->EndTimeString);
        result->UpdatedAt = view->UpdatedAt;
        result->DateHeader = QString(view->DateHeader);
        result->DateDuration = QString(view->DateDuration);
        result->DurOnly = view->DurOnly;
        result->IsHeader = view->IsHeader;
        return result;
    }

    static QVector<TimeEntryView *> importAll(KopsikTimeEntryViewItem *first) {
        QVector<TimeEntryView *> result;
        KopsikTimeEntryViewItem *view = first;
        while (view) {
            result.push_back(importOne(view));
            view = static_cast<KopsikTimeEntryViewItem *>(view->Next);
        }
        return result;
    }

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
signals:

public slots:

};

#endif // TIMEENTRYVIEW_H
