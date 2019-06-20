// Copyright 2014 Toggl Desktop developers.

#include "./timeentryview.h"

#include <QDateTime>

TimeEntryView::TimeEntryView(QObject *parent) : QObject(parent) {
}

TimeEntryView *TimeEntryView::importOne(const TogglTimeEntryView *view) {
    TimeEntryView *result = new TimeEntryView();

    result->DurationInSeconds = TogglTimeEntryView_DurationInSeconds(view);
    result->ProjectAndTaskLabel = TogglTimeEntryView_ProjectAndTaskLabel(view);
    result->Description = TogglTimeEntryView_Description(view);
    result->ProjectLabel = TogglTimeEntryView_ProjectLabel(view);
    result->TaskLabel = TogglTimeEntryView_TaskLabel(view);
    result->ClientLabel = TogglTimeEntryView_ClientLabel(view);
    result->WID = TogglTimeEntryView_WID(view);
    result->PID = TogglTimeEntryView_PID(view);
    result->TID = TogglTimeEntryView_TID(view);
    result->Duration = TogglTimeEntryView_Duration(view);
    result->Color = TogglTimeEntryView_Color(view);
    result->GUID = TogglTimeEntryView_GUID(view);
    result->Billable = TogglTimeEntryView_Billable(view);
    result->Tags = TogglTimeEntryView_Tags(view);
    result->Started = TogglTimeEntryView_Started(view);
    result->Ended = TogglTimeEntryView_Ended(view);
    result->StartTimeString = TogglTimeEntryView_StartTimeString(view);
    result->EndTimeString = TogglTimeEntryView_EndTimeString(view);
    result->UpdatedAt = TogglTimeEntryView_UpdatedAt(view);
    result->DateHeader = TogglTimeEntryView_DateHeader(view);
    result->DateDuration = TogglTimeEntryView_DateDuration(view);
    result->IsHeader = TogglTimeEntryView_IsHeader(view);
    result->CanSeeBillable = TogglTimeEntryView_CanSeeBillable(view);
    result->CanAddProjects = TogglTimeEntryView_CanAddProjects(view);
    result->DefaultWID = TogglTimeEntryView_DefaultWID(view);
    result->WorkspaceName = TogglTimeEntryView_WorkspaceName(view);
    result->Error = TogglTimeEntryView_Error(view);
    result->Unsynced = TogglTimeEntryView_Unsynced(view);
    // Grouped entries mode
    result->Group = TogglTimeEntryView_Group(view);
    result->GroupOpen = TogglTimeEntryView_GroupOpen(view);
    result->GroupName = TogglTimeEntryView_GroupName(view);
    result->GroupDuration = TogglTimeEntryView_GroupDuration(view);
    result->GroupItemCount = TogglTimeEntryView_GroupItemCount(view);

    return result;
}

QVector<TimeEntryView *> TimeEntryView::importAll(
    const TogglTimeEntryView *first) {
    QVector<TimeEntryView *> result;
    const TogglTimeEntryView *view = reinterpret_cast<const TogglTimeEntryView*>(first);
    while (view) {
        result.push_back(importOne(view));
        view = TogglTimeEntryView_Next(view);
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
