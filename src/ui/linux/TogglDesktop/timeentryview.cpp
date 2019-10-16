// Copyright 2014 Toggl Desktop developers.

#include "./timeentryview.h"

#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <QMetaObject>

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
    result->ProjectAndTaskLabel = view->ProjectAndTaskLabel;
    result->Description = QString(view->Description);
    result->ProjectLabel = QString(view->ProjectLabel);
    result->TaskLabel = QString(view->TaskLabel);
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
    result->DateHeader = QString("<tr><td>%1</td><td align=right>%2</td></tr>").arg(view->DateHeader).arg(view->DateDuration);
    result->DateDuration = QString(view->DateDuration);
    result->IsHeader = view->IsHeader;
    result->CanSeeBillable = view->CanSeeBillable;
    result->CanAddProjects = view->CanAddProjects;
    result->DefaultWID = view->DefaultWID;
    result->WorkspaceName = QString(view->WorkspaceName);
    result->Error = QString(view->Error);
    result->Unsynced = view->Unsynced;
    // Grouped entries mode
    result->Group = view->Group;
    result->GroupOpen = view->GroupOpen;
    result->GroupName = QString(view->GroupName);
    result->GroupDuration = QString(view->GroupDuration);
    result->GroupItemCount = view->GroupItemCount;

    return result;
}

/*
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
*/

void TimeEntryView::update(const TogglTimeEntryView *view) {
    int64_t DurationInSeconds_ = view->DurationInSeconds;
    QMetaObject::invokeMethod(this, "DurationInSecondsSet", Qt::QueuedConnection, Q_ARG( const int64_t &, DurationInSeconds_ ));
    QString Description_ = view->Description;
    QMetaObject::invokeMethod(this, "DescriptionSet", Qt::QueuedConnection, Q_ARG( const QString &, Description_ ));
    QString ProjectAndTaskLabel_ = view->ProjectAndTaskLabel;
    QMetaObject::invokeMethod(this, "ProjectAndTaskLabelSet", Qt::QueuedConnection, Q_ARG( const QString &, ProjectAndTaskLabel_ ));
    QString ProjectLabel_ = view->ProjectLabel;
    QMetaObject::invokeMethod(this, "ProjectLabelSet", Qt::QueuedConnection, Q_ARG( const QString &, ProjectLabel_ ));
    QString TaskLabel_ = view->TaskLabel;
    QMetaObject::invokeMethod(this, "TaskLabelSet", Qt::QueuedConnection, Q_ARG( const QString &, TaskLabel_ ));
    QString ClientLabel_ = view->ClientLabel;
    QMetaObject::invokeMethod(this, "ClientLabelSet", Qt::QueuedConnection, Q_ARG( const QString &, ClientLabel_ ));
    uint64_t WID_ = view->WID;
    QMetaObject::invokeMethod(this, "WIDSet", Qt::QueuedConnection, Q_ARG( const uint64_t &, WID_ ));
    uint64_t PID_ = view->PID;
    QMetaObject::invokeMethod(this, "PIDSet", Qt::QueuedConnection, Q_ARG( const uint64_t &, PID_ ));
    uint64_t TID_ = view->TID;
    QMetaObject::invokeMethod(this, "TIDSet", Qt::QueuedConnection, Q_ARG( const uint64_t &, TID_ ));
    QString Duration_ = view->Duration;
    QMetaObject::invokeMethod(this, "DurationSet", Qt::QueuedConnection, Q_ARG( const QString &, Duration_ ));
    QString Color_ = view->Color;
    QMetaObject::invokeMethod(this, "ColorSet", Qt::QueuedConnection, Q_ARG( const QString &, Color_ ));
    QString GUID_ = view->GUID;
    QMetaObject::invokeMethod(this, "GUIDSet", Qt::QueuedConnection, Q_ARG( const QString &, GUID_ ));
    bool Billable_ = view->Billable;
    QMetaObject::invokeMethod(this, "BillableSet", Qt::QueuedConnection, Q_ARG( const bool &, Billable_ ));
    QString Tags_ = view->Tags;
    QMetaObject::invokeMethod(this, "TagsSet", Qt::QueuedConnection, Q_ARG( const QString &, Tags_ ));
    uint64_t Started_ = view->Started;
    QMetaObject::invokeMethod(this, "StartedSet", Qt::QueuedConnection, Q_ARG( const uint64_t &, Started_ ));
    uint64_t Ended_ = view->Ended;
    QMetaObject::invokeMethod(this, "EndedSet", Qt::QueuedConnection, Q_ARG( const uint64_t &, Ended_ ));
    QString StartTimeString_ = view->StartTimeString;
    QMetaObject::invokeMethod(this, "StartTimeStringSet", Qt::QueuedConnection, Q_ARG( const QString &, StartTimeString_ ));
    QString EndTimeString_ = view->EndTimeString;
    QMetaObject::invokeMethod(this, "EndTimeStringSet", Qt::QueuedConnection, Q_ARG( const QString &, EndTimeString_ ));
    uint64_t UpdatedAt_ = view->UpdatedAt;
    QMetaObject::invokeMethod(this, "UpdatedAtSet", Qt::QueuedConnection, Q_ARG( const uint64_t &, UpdatedAt_ ));
    QString DateHeader_ = view->DateHeader;
    QMetaObject::invokeMethod(this, "DateHeaderSet", Qt::QueuedConnection, Q_ARG( const QString &, DateHeader_ ));
    QString DateDuration_ = view->DateDuration;
    QMetaObject::invokeMethod(this, "DateDurationSet", Qt::QueuedConnection, Q_ARG( const QString &, DateDuration_ ));
    bool IsHeader_ = view->IsHeader;
    QMetaObject::invokeMethod(this, "IsHeaderSet", Qt::QueuedConnection, Q_ARG( const bool &, IsHeader_ ));
    bool CanAddProjects_ = view->CanAddProjects;
    QMetaObject::invokeMethod(this, "CanAddProjectsSet", Qt::QueuedConnection, Q_ARG( const bool &, CanAddProjects_ ));
    bool CanSeeBillable_ = view->CanSeeBillable;
    QMetaObject::invokeMethod(this, "CanSeeBillableSet", Qt::QueuedConnection, Q_ARG( const bool &, CanSeeBillable_ ));
    uint64_t DefaultWID_ = view->DefaultWID;
    QMetaObject::invokeMethod(this, "DefaultWIDSet", Qt::QueuedConnection, Q_ARG( const uint64_t &, DefaultWID_ ));
    QString WorkspaceName_ = view->WorkspaceName;
    QMetaObject::invokeMethod(this, "WorkspaceNameSet", Qt::QueuedConnection, Q_ARG( const QString &, WorkspaceName_ ));
    QString Error_ = view->Error;
    QMetaObject::invokeMethod(this, "ErrorSet", Qt::QueuedConnection, Q_ARG( const QString &, Error_ ));
    bool Unsynced_ = view->Unsynced;
    QMetaObject::invokeMethod(this, "UnsyncedSet", Qt::QueuedConnection, Q_ARG( const bool &, Unsynced_ ));
    bool Group_ = view->Group;
    QMetaObject::invokeMethod(this, "GroupSet", Qt::QueuedConnection, Q_ARG( const bool &, Group_ ));
    bool GroupOpen_ = view->GroupOpen;
    QMetaObject::invokeMethod(this, "GroupOpenSet", Qt::QueuedConnection, Q_ARG( const bool &, GroupOpen_ ));
    QString GroupName_ = view->GroupName;
    QMetaObject::invokeMethod(this, "GroupNameSet", Qt::QueuedConnection, Q_ARG( const QString &, GroupName_ ));
    QString GroupDuration_ = view->GroupDuration;
    QMetaObject::invokeMethod(this, "GroupDurationSet", Qt::QueuedConnection, Q_ARG( const QString &, GroupDuration_ ));
    quint64 GroupItemCount_ = view->GroupItemCount;
    QMetaObject::invokeMethod(this, "GroupItemCountSet", Qt::QueuedConnection, Q_ARG( const quint64 &, GroupItemCount_ ));
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

TimeEntryViewStorage::TimeEntryViewStorage(QObject *parent)
    : QAbstractListModel(parent)
    , uiThread(QThread::currentThread())
{
}

void TimeEntryViewStorage::importList(TogglTimeEntryView *first) {
    // this function is a tough (and ugly one)
    // first define the necessary data structures
    QSet<QString> added;
    QSet<QString> updated;
    QSet<QString> removed;
    QSet<QString> newGuids;
    QSet<QString> oldGuids;
    QList<QString> newOrder;
    QList<TogglTimeEntryView*> flattened;
    QList<TogglTimeEntryView*> reordered;

    // we take old GUIDs but make them a set so they're easier to work with (no need to know the order)
    oldGuids = guidOrder_.toSet();

    // now, let's pass through the new set of TEs
    TogglTimeEntryView *libraryView = first;
    while (libraryView) {
        // spew an error when there's no GUID (which would make our lives much harder)
        if (!libraryView->GUID || !*libraryView->GUID) {
            qCritical() << libraryView->Description << "Does not have a GUID!";
        }
        // build the order of the GUIDs in the new list
        if (!newOrder.contains(libraryView->GUID))
            newOrder.append(libraryView->GUID);
        // and also build the set of the GUIDs now for easier comparison
        newGuids.insert(libraryView->GUID);
        // and create an addressable list of the new items (library, not qt structures, though)
        flattened.append(libraryView);
        libraryView = static_cast<TogglTimeEntryView*>(libraryView->Next);
    }

    // now we compare the sets:
    // this gives us the newly added guids
    added = newGuids - oldGuids;
    // this gives us guids that stayed in place but we need to update them
    updated = newGuids;
    updated = updated.intersect(oldGuids);
    // and here we have guids that are gone
    removed = oldGuids - newGuids;

    qCritical() << "==================== NEW ROUND ====================";
    qCritical() << "======== Storage";
    qCritical() << storage_;
    qCritical() << "======== added";
    qCritical() << added;
    qCritical() << "======== updated";
    qCritical() << updated;
    qCritical() << "======== removed";
    qCritical() << removed;
    qCritical() << "======== newGuids";
    qCritical() << newGuids;
    qCritical() << "======== oldGuids";
    qCritical() << oldGuids;
    qCritical() << "======== newOrder";
    qCritical() << newOrder;
    qCritical() << "======== flattened";
    qCritical() << flattened;

    // first we remove all items with guids that are gone in the update
    for (auto i : removed) {
        qCritical() << "BAZBAZ";
        remove(i);
    }

    // now pass through the list again to update the items we already have
    libraryView = first;
    while (libraryView) {
        if (updated.contains(libraryView->GUID)) {
            qCritical() << "BAZ";
            qCritical() << "Current thread" << QThread::currentThread();
            qCritical() << "UI thread" << uiThread;
            storage_[libraryView->GUID]->update(libraryView);
            // and also build the order of GUIDs we already had (before)
            reordered.append(libraryView);
            updated.remove(libraryView->GUID);
        }
        libraryView = static_cast<TogglTimeEntryView*>(libraryView->Next);
    }

    qCritical() << "======== reordered";
    qCritical() << reordered;

    // and then we take the new order of the current items and reorder them according to the new order
    for (int i = 0; i < reordered.count(); i++) {
        if (reordered[i]->GUID != guidOrder_[i]) {
            qCritical() << "BAR";
            move(reordered[i]->GUID, i);
        }
    }

    // and finally, we insert newly added items into the reordered set of old items that stayed in the storage
    for (int i = 0; i < newOrder.count(); i++) {
        if (i >= guidOrder_.count() || guidOrder_[i] != newOrder[i]) {
            qCritical() << "FOO";
            beginInsertRows(QModelIndex(), i, i);
            QString guid = newOrder[i];
            guidOrder_.insert(i, guid);
            auto v = TimeEntryView::importOne(flattened[i]);
            qCritical() << "CREATED:" << v;
            qCritical() << "Current thread" << QThread::currentThread();
            qCritical() << "UI thread" << uiThread;
            v->moveToThread(uiThread);
            storage_.insert(guid, v);
            qCritical() << "Storage:" << storage_;
            endInsertRows();
            qCritical() << guidOrder_.count();
        }
    }
}

QVariant TimeEntryViewStorage::data(const QModelIndex &index, int role) const {
    if (index.isValid() && role == Qt::UserRole) {
        qCritical() << "============== REQUEST =============";
        qCritical() << "Looking for index" << index.row();
        qCritical() << "Storage:" << storage_;
        qCritical() << "Order:" << guidOrder_;
        qCritical() << "Going to return" << storage_[guidOrder_[index.row()]];
        return QVariant::fromValue(storage_[guidOrder_[index.row()]]);
    }
    return QVariant();
}

QHash<int, QByteArray> TimeEntryViewStorage::roleNames() const {
    return { { Qt::UserRole, "modelData" } };
}

int TimeEntryViewStorage::rowCount(const QModelIndex &parent) const {
    return guidOrder_.count();
}

void TimeEntryViewStorage::remove(const QString &guid) {
    int pos = guidOrder_.indexOf(guid);
    if (pos < 0) {
        qCritical() << "Could not remove" << guid << "because it's not present";
        return;
    }
    beginRemoveRows(QModelIndex(), pos, pos);
    guidOrder_.removeAt(pos);
    auto old = storage_[guid];
    storage_.remove(guid);
    old->deleteLater();
    endRemoveRows();
}

void TimeEntryViewStorage::move(const QString &guid, int to) {
    int pos = guidOrder_.indexOf(guid);
    if (pos < 0) {
        qCritical() << "Could not move" << guid << "because it's not present";
        return;
    }
    beginMoveRows(QModelIndex(), pos, pos, QModelIndex(), to + 1    );
    guidOrder_.move(pos, to);
    endMoveRows();
}
