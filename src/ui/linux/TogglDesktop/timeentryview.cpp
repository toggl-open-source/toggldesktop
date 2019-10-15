// Copyright 2014 Toggl Desktop developers.

#include "./timeentryview.h"

#include <QDateTime>
#include <QDebug>

TimeEntryView::TimeEntryView(QObject *parent) : QObject(parent) {
}

bool TimeEntryView::operator==(const TimeEntryView &o) {
    return this->DurationInSeconds == o.DurationInSeconds &&
            this->Description == o.Description &&
            this->ProjectAndTaskLabel == o.ProjectAndTaskLabel &&
            this->ProjectLabel == o.ProjectLabel &&
            this->TaskLabel == o.TaskLabel &&
            this->ClientLabel == o.ClientLabel &&
            this->WID == o.WID &&
            this->PID == o.PID &&
            this->TID == o.TID &&
            this->Duration == o.Duration &&
            this->Color == o.Color &&
            this->GUID == o.GUID &&
            this->Billable == o.Billable &&
            this->Tags == o.Tags &&
            this->Started == o.Started &&
            this->Ended == o.Ended &&
            this->StartTimeString == o.StartTimeString &&
            this->EndTimeString == o.EndTimeString &&
            this->DateHeader == o.DateHeader &&
            this->DateDuration == o.DateDuration &&
            this->CanAddProjects == o.CanAddProjects &&
            this->CanSeeBillable == o.CanSeeBillable &&
            this->DefaultWID == o.DefaultWID &&
            this->WorkspaceName == o.WorkspaceName &&
            this->Error == o.Error &&
            this->Unsynced == o.Unsynced &&
            this->Group == o.Group &&
            this->GroupOpen == o.GroupOpen &&
            this->GroupName == o.GroupName &&
            this->GroupDuration == o.GroupDuration &&
            this->GroupItemCount == o.GroupItemCount;
}

bool TimeEntryView::operator!=(const TimeEntryView &o) {
    return ! (*this == o);
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
    result->DateHeader = QString("<tr><td>%1</td><td align=right>%2</td></tr>").arg(toQString(view->DateHeader)).arg(toQString(view->DateDuration));
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
