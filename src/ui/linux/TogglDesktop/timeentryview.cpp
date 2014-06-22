#include "timeentryview.h"

#include <QDateTime>

TimeEntryView::TimeEntryView(QObject *parent) :
    QObject(parent)
{
}

TimeEntryView *TimeEntryView::importOne(KopsikTimeEntryViewItem *view) {
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
    result->CanSeeBillable = view->CanSeeBillable;
    result->CanAddProjects = view->CanAddProjects;
    result->DefaultWID = view->DefaultWID;
    return result;
}

QVector<TimeEntryView *> TimeEntryView::importAll(KopsikTimeEntryViewItem *first) {
    QVector<TimeEntryView *> result;
    KopsikTimeEntryViewItem *view = first;
    while (view) {
        result.push_back(importOne(view));
        view = static_cast<KopsikTimeEntryViewItem *>(view->Next);
    }
    return result;
}

const QString TimeEntryView::timeOverview()
{
    QString result = DateHeader + " from " + StartTimeString;
    if (DurationInSeconds >= 0)
    {
        result = result + " to " + EndTimeString;
    }
    return result;
}

const QString TimeEntryView::lastUpdate()
{
    return QString("Last update ") + QDateTime::fromTime_t(UpdatedAt).toString();
}
