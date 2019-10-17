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
    DurationInSecondsSet(view->DurationInSeconds);
    DescriptionSet(view->Description);
    ProjectAndTaskLabelSet(view->ProjectAndTaskLabel);
    ProjectLabelSet(view->ProjectLabel);
    TaskLabelSet(view->TaskLabel);
    ClientLabelSet(view->ClientLabel);
    WIDSet(view->WID);
    PIDSet(view->PID);
    TIDSet(view->TID);
    DurationSet(view->Duration);
    ColorSet(view->Color);
    GUIDSet(view->GUID);
    BillableSet(view->Billable);
    TagsSet(view->Tags);
    StartedSet(view->Started);
    EndedSet(view->Ended);
    StartTimeStringSet(view->StartTimeString);
    EndTimeStringSet(view->EndTimeString);
    UpdatedAtSet(view->UpdatedAt);
    DateHeaderSet(QString("<tr><td>%1</td><td align=right>%2</td></tr>").arg(view->DateHeader).arg(view->DateDuration));
    DateDurationSet(view->DateDuration);
    IsHeaderSet(view->IsHeader);
    CanAddProjectsSet(view->CanAddProjects);
    CanSeeBillableSet(view->CanSeeBillable);
    DefaultWIDSet(view->DefaultWID);
    WorkspaceNameSet(view->WorkspaceName);
    ErrorSet(view->Error);
    UnsyncedSet(view->Unsynced);
    GroupSet(view->Group);
    GroupOpenSet(view->GroupOpen);
    GroupNameSet(view->GroupName);
    GroupDurationSet(view->GroupDuration);
    GroupItemCountSet(view->GroupItemCount);
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
        if (!newOrder.contains(uid(libraryView)))
            newOrder.append(uid(libraryView));
        // and also build the set of the GUIDs now for easier comparison
        newGuids.insert(uid(libraryView));
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

    // first we remove all items with guids that are gone in the update
    for (auto i : removed) {
        remove(i);
    }

    // now pass through the list again to update the items we already have
    libraryView = first;
    while (libraryView) {
        if (updated.contains(uid(libraryView))) {
            storage_[uid(libraryView)]->update(libraryView);
            // and also build the order of GUIDs we already had (before)
            reordered.append(libraryView);
            updated.remove(uid(libraryView));
        }
        libraryView = static_cast<TogglTimeEntryView*>(libraryView->Next);
    }

    // and then we take the new order of the current items and reorder them according to the new order
    for (int i = 0; i < reordered.count(); i++) {
        if (uid(reordered[i]) != guidOrder_[i]) {
            move(uid(reordered[i]), i);
        }
    }

    // and finally, we insert newly added items into the reordered set of old items that stayed in the storage
    for (int i = 0; i < newOrder.count(); i++) {
        if (i >= guidOrder_.count() || guidOrder_[i] != newOrder[i]) {
            beginInsertRows(QModelIndex(), i, i);
            QString guid = newOrder[i];
            guidOrder_.insert(i, guid);
            auto v = TimeEntryView::importOne(flattened[i]);
            v->moveToThread(uiThread);
            storage_.insert(guid, v);
            endInsertRows();
        }
    }
}

QVariant TimeEntryViewStorage::data(const QModelIndex &index, int role) const {
    if (index.isValid() && role == Qt::UserRole) {
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

QString TimeEntryViewStorage::uid(TogglTimeEntryView *view) {
    if (view->Group)
        return QString("Group %1").arg(view->GUID);
    return view->GUID;
}

QString TimeEntryViewStorage::uid(TimeEntryView *view) {
    if (view->Group)
        return QString("Group %1").arg(view->GUID);
    return view->GUID;
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
