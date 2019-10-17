// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYVIEW_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYVIEW_H_

#include <QObject>
#include <QVector>
#include <QAbstractListModel>

#include "./toggl_api.h"
#include "./common.h"

class TimeEntryView;

class TimeEntryViewStorage : public QAbstractListModel {
    Q_OBJECT
public:
    TimeEntryViewStorage(QObject *parent = nullptr);

    void importList(TogglTimeEntryView *first);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    QString uid(TogglTimeEntryView *view);
    QString uid(TimeEntryView *view);

    void remove(const QString &guid);
    void move(const QString &guid, int to);

    QList<QString> guidOrder_;
    QMap<QString, TimeEntryView*> storage_;

    QThread *uiThread;
};

class TimeEntryView : public QObject {
    Q_OBJECT

 public:
    explicit TimeEntryView(QObject *parent = 0);

    bool operator==(const TimeEntryView &o);
    bool operator!=(const TimeEntryView &o);

    static TimeEntryView *importOne(TogglTimeEntryView *view);
    //static QVector<TimeEntryView *> importAll(TogglTimeEntryView *first);
    void update(const TogglTimeEntryView *view);

    bool confirmlessDelete();
    Q_INVOKABLE const QString lastUpdate();

    mutable QMutex propertyMutex_;

    PROPERTY(int64_t, DurationInSeconds)
    PROPERTY(QString, Description)
    PROPERTY(QString, ProjectAndTaskLabel)
    PROPERTY(QString, ProjectLabel)
    PROPERTY(QString, TaskLabel)
    PROPERTY(QString, ClientLabel)
    PROPERTY(uint64_t, WID)
    PROPERTY(uint64_t, PID)
    PROPERTY(uint64_t, TID)
    PROPERTY(QString, Duration)
    PROPERTY(QString, Color)
    PROPERTY(QString, GUID)
    PROPERTY(bool, Billable)
    PROPERTY(QString, Tags)
    PROPERTY(uint64_t, Started)
    PROPERTY(uint64_t, Ended)
    PROPERTY(QString, StartTimeString)
    PROPERTY(QString, EndTimeString)
    PROPERTY(uint64_t, UpdatedAt)
    PROPERTY(QString, DateHeader)
    PROPERTY(QString, DateDuration)
    PROPERTY(bool, IsHeader)
    PROPERTY(bool, CanAddProjects)
    PROPERTY(bool, CanSeeBillable)
    PROPERTY(uint64_t, DefaultWID)
    PROPERTY(QString, WorkspaceName)
    PROPERTY(QString, Error)
    PROPERTY(bool, Unsynced)
    // Group mode items
    PROPERTY(bool, Group)
    PROPERTY(bool, GroupOpen)
    PROPERTY(QString, GroupName)
    PROPERTY(QString, GroupDuration)
    PROPERTY(quint64, GroupItemCount)
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYVIEW_H_
