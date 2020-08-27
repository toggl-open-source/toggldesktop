// Copyright 2014 Toggl Desktop developers.

#include "./timeentryview.h"

#include <QDateTime>

TimeEntryView::TimeEntryView(QObject *parent) : QObject(parent) {
}

TimeEntryView *TimeEntryView::importOne(TogglTimeEntryView *view) {
    TimeEntryView *result = new TimeEntryView();
    result->DurationInSeconds = view->DurationInSeconds;
    result->ProjectAndTaskLabel = toQString(view->ProjectAndTaskLabel);
    result->Description = toQString(view->Description);
    result->ProjectLabel = toQString(view->ProjectLabel);
    result->TaskLabel = toQString(view->TaskLabel);
    result->ClientLabel = toQString(view->ClientLabel);
    result->WID = view->WID;
    result->PID = view->PID;
    result->TID = view->TID;
    result->Duration = toQString(view->Duration);
    result->Color = toQString(view->Color);
    result->GUID = toQString(view->GUID);
    result->Billable = view->Billable;
    result->Tags = toQString(view->Tags);
    result->Started = view->Started;
    result->Ended = view->Ended;
    result->StartTimeString = toQString(view->StartTimeString);
    result->EndTimeString = toQString(view->EndTimeString);
    result->UpdatedAt = view->UpdatedAt;
    result->DateHeader = toQString(view->DateHeader);
    result->DateDuration = toQString(view->DateDuration);
    result->IsHeader = view->IsHeader;
    result->CanSeeBillable = view->CanSeeBillable;
    result->CanAddProjects = view->CanAddProjects;
    result->DefaultWID = view->DefaultWID;
    result->WorkspaceName = toQString(view->WorkspaceName);
    result->Error = toQString(view->Error);
    result->Unsynced = view->Unsynced;
    // Grouped entries mode
    result->Group = view->Group;
    result->GroupOpen = view->GroupOpen;
    result->GroupName = toQString(view->GroupName);
    result->GroupDuration = toQString(view->GroupDuration);
    result->GroupItemCount = view->GroupItemCount;

    return result;
}

QVector<TimeEntryView *> TimeEntryView::importAll(
    TogglTimeEntryView *first) {
    QVector<TimeEntryView *> result;
    TogglTimeEntryView *view = first;
    while (view) {
        result.push_back(importOne(view));
        view = static_cast<TogglTimeEntryView *>(view->Next);
    }
    return result;
}

const QString TimeEntryView::lastUpdate() {
    return QString("Last update ") +
           QDateTime::fromTime_t(static_cast<uint>(UpdatedAt)).toString();
}

bool TimeEntryView::confirmlessDelete() {
    if (DurationInSeconds < 0) {
        int64_t actual_duration = DurationInSeconds + time(nullptr);
        return actual_duration < 15;
    } else {
        return DurationInSeconds < 15;
    }
}
