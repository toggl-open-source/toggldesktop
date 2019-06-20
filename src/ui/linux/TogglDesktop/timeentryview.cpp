// Copyright 2014 Toggl Desktop developers.

#include "./timeentryview.h"

#include <QDateTime>

TimeEntryView::TimeEntryView(QObject *parent, const toggl::view::TimeEntry* view)
    : QObject(parent)
    , toggl::view::TimeEntry(*view) {
}

TimeEntryView *TimeEntryView::importOne(const TogglTimeEntryView *view) {
    const toggl::view::TimeEntry *v = reinterpret_cast<const toggl::view::TimeEntry*>(view);
    return new TimeEntryView(nullptr, v);
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
