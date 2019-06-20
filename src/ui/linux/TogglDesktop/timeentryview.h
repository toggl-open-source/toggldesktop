// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYVIEW_H_

#include <QObject>
#include <QVector>

#include "./toggl_api.h"
#include "./gui.h"

class TimeEntryView : public QObject, public toggl::view::TimeEntry {
    Q_OBJECT

 public:
    explicit TimeEntryView(QObject *parent = nullptr, const toggl::view::TimeEntry* view = nullptr);

    static TimeEntryView *importOne(const TogglTimeEntryView *view);
    static QVector<TimeEntryView *> importAll(const TogglTimeEntryView *first);

    bool confirmlessDelete();
    const QString lastUpdate();
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYVIEW_H_
