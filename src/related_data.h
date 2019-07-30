// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_RELATED_DATA_H_
#define SRC_RELATED_DATA_H_

#include <vector>
#include <set>
#include <string>
#include <map>
#include <functional>

#include "./timeline_event.h"
#include "./types.h"

#include "Poco/Mutex.h"
#include <functional>
#include <thread>
#include <mutex>

namespace toggl {

class AutotrackerRule;
class Client;
class ObmAction;
class ObmExperiment;
class Project;
class Tag;
class Task;
class TimeEntry;
class Workspace;

namespace view {
class TimeEntry;
};

template<typename T>
T *modelByID(const Poco::UInt64 id, const std::set<T *> &list);

template <typename T>
T *modelByGUID(const guid GUID, const std::set<T *> &list);

template <typename T>
class locked : public std::unique_lock<std::recursive_mutex> {
public:
    locked()
        : std::unique_lock<std::recursive_mutex>()
        , data_(nullptr)
    {}
    locked(mutex_type &mutex, T *data)
        : std::unique_lock<std::recursive_mutex>(mutex)
        , data_(data)
    {}
    T* data() { return data_; }
    T *operator->() { return data_; }
    T &operator*() { return *data_; }
    operator bool() const {
        return owns_lock() && data_;
    }
private:
    T *data_;
};


class RelatedData {
    std::set<Workspace *> workspaces_;
    mutable std::recursive_mutex workspaces_m_;
    std::set<Client *> clients_;
    mutable std::recursive_mutex clients_m_;
    std::set<Project *> projects_;
    mutable std::recursive_mutex projects_m_;
    std::set<Task *> tasks_;
    mutable std::recursive_mutex tasks_m_;
    std::set<Tag *> tags_;
    mutable std::recursive_mutex tags_m_;
    std::set<TimeEntry *> timeEntries_;
    mutable std::recursive_mutex timeEntries_m_;
    std::set<AutotrackerRule *> autotrackerRules_;
    mutable std::recursive_mutex autotrackerRules_m_;
    std::set<TimelineEvent *> timelineEvents_;
    mutable std::recursive_mutex timelineEvents_m_;
    std::set<ObmAction *> obmActions_;
    mutable std::recursive_mutex obmActions_m_;
    std::set<ObmExperiment *> obmExperiments_;
    mutable std::recursive_mutex obmExperiments_m_;
public:
    locked<std::set<Workspace*>> Workspaces() { return { workspaces_m_, &workspaces_ }; }
    locked<std::set<Client*>> Clients() { return { clients_m_, &clients_}; }
    locked<std::set<Project*>> Projects() { return { projects_m_, &projects_}; }
    locked<std::set<Task*>> Tasks() { return { tasks_m_, &tasks_}; }
    locked<std::set<Tag*>> Tags() { return { tags_m_, &tags_ }; }
    locked<std::set<TimeEntry*>> TimeEntries() { return { timeEntries_m_, &timeEntries_ }; }
    locked<std::set<AutotrackerRule*>> AutotrackerRules() { return { autotrackerRules_m_, &autotrackerRules_ }; }
    locked<std::set<TimelineEvent*>> TimelineEvents() { return { timelineEvents_m_, &timelineEvents_ }; }
    locked<std::set<ObmAction*>> ObmActions() { return { obmActions_m_, &obmActions_ }; }
    locked<std::set<ObmExperiment*>> ObmExperiments() { return { obmExperiments_m_, &obmExperiments_ }; }
    locked<const std::set<Workspace*>> Workspaces() const { return { workspaces_m_, &workspaces_ }; }
    locked<const std::set<Client*>> Clients() const { return { clients_m_, &clients_}; }
    locked<const std::set<Project*>> Projects() const { return { projects_m_, &projects_}; }
    locked<const std::set<Task*>> Tasks() const { return { tasks_m_, &tasks_}; }
    locked<const std::set<Tag*>> Tags() const { return { tags_m_, &tags_ }; }
    locked<const std::set<TimeEntry*>> TimeEntries() const { return { timeEntries_m_, &timeEntries_ }; }
    locked<const std::set<AutotrackerRule*>> AutotrackerRules() const { return { autotrackerRules_m_, &autotrackerRules_ }; }
    locked<const std::set<TimelineEvent*>> TimelineEvents() const { return { timelineEvents_m_, &timelineEvents_ }; }
    locked<const std::set<ObmAction*>> ObmActions() const { return { obmActions_m_, &obmActions_ }; }
    locked<const std::set<ObmExperiment*>> ObmExperiments() const { return { obmExperiments_m_, &obmExperiments_ }; }

    template<typename T> void clearList(std::set<T *> &list);

    void clearWorkspaces();
    void clearClients();
    void clearProjects();
    void clearTasks();
    void clearTags();
    void clearTimeEntries();
    void clearAutotrackerRules();
    void clearTimelineEvents();
    void clearObmActions();
    void clearObmExperiments();

    locked<Workspace> newWorkspace();
    locked<Client> newClient();
    locked<Project> newProject();
    locked<Task> newTask();
    locked<Tag> newTag();
    locked<TimeEntry> newTimeEntry();
    locked<AutotrackerRule> newAutotrackerRule();
    locked<TimelineEvent> newTimelineEvent();
    locked<ObmAction> newObmAction();
    locked<ObmExperiment> newObmExperiment();

    void Clear();

    template <typename T> locked<T> make_protected(T *data);
    template <typename T> locked<const T> make_protected(const T *data) const;

    locked<Task> TaskByID(const Poco::UInt64 id);
    locked<Client> ClientByID(const Poco::UInt64 id);
    locked<Project> ProjectByID(const Poco::UInt64 id);
    locked<Tag> TagByID(const Poco::UInt64 id);
    locked<Workspace> WorkspaceByID(const Poco::UInt64 id);
    locked<TimeEntry> TimeEntryByID(const Poco::UInt64 id);

    locked<TimeEntry> TimeEntryByGUID(const guid GUID);
    locked<Tag> TagByGUID(const guid GUID);
    locked<Project> ProjectByGUID(const guid GUID);
    locked<Client> ClientByGUID(const guid GUID);
    locked<TimelineEvent> TimelineEventByGUID(const guid GUID);

    locked<const Task> TaskByID(const Poco::UInt64 id) const;
    locked<const Client> ClientByID(const Poco::UInt64 id) const;
    locked<const Project> ProjectByID(const Poco::UInt64 id) const;
    locked<const Tag> TagByID(const Poco::UInt64 id) const;
    locked<const Workspace> WorkspaceByID(const Poco::UInt64 id) const;
    locked<const TimeEntry> TimeEntryByID(const Poco::UInt64 id) const;

    locked<const TimeEntry> TimeEntryByGUID(const guid GUID) const;
    locked<const Tag> TagByGUID(const guid GUID) const;
    locked<const Project> ProjectByGUID(const guid GUID) const;
    locked<const Client> ClientByGUID(const guid GUID) const;
    locked<const TimelineEvent> TimelineEventByGUID(const guid GUID) const;

    void TagList( std::vector<std::string> *result, const Poco::UInt64 wid) const;
    void WorkspaceList(std::vector<Workspace *> *) const;
    void ClientList(std::vector<Client *> *) const;

    Poco::Int64 NumberOfUnsyncedTimeEntries() const;

    // Find the time entry that was stopped most recently
    TimeEntry *LatestTimeEntry() const;

    // Collect visible timeline events
    std::vector<TimelineEvent *> VisibleTimelineEvents() const;

    // Collect visible time entries
    std::vector<TimeEntry *> VisibleTimeEntries() const;

    Poco::Int64 TotalDurationForDate(const TimeEntry *match) const;

    // avoid duplicates
    bool HasMatchingAutotrackerRule(const std::string lowercase_term) const;

    error DeleteAutotrackerRule(const Poco::Int64 local_id);

    void TimeEntryAutocompleteItems(std::vector<view::Autocomplete> *) const;
    void MinitimerAutocompleteItems(std::vector<view::Autocomplete> *) const;
    void ProjectAutocompleteItems(std::vector<view::Autocomplete> *) const;

    void ProjectLabelAndColorCode(
        TimeEntry * const te,
        view::TimeEntry *view) const;

    AutotrackerRule *FindAutotrackerRule(const TimelineEvent &event) const;

    locked<Client> clientByProject(Project *p);
    locked<const Client> clientByProject(const Project *p) const;

    void pushBackTimeEntry(TimeEntry  *timeEntry);

    void forEachTimeEntries(std::function<void(TimeEntry *)> f);

 private:
    void timeEntryAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<view::Autocomplete> *list,
        std::map<std::string, std::vector<view::Autocomplete> > *items) const;

    void taskAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<view::Autocomplete> *list,
        std::map<Poco::Int64, std::vector<view::Autocomplete> > *items) const;

    void projectAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<view::Autocomplete> *list,
        std::map<std::string, std::vector<view::Autocomplete> > *items,
        std::map<Poco::Int64, std::vector<view::Autocomplete> > *task_items) const;

    void workspaceAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<view::Autocomplete> *list) const;

    void mergeGroupedAutocompleteItems(
        std::vector<view::Autocomplete> *result,
        std::map<std::string, std::vector<view::Autocomplete> > *items) const;
};

template <> inline locked<const TimeEntry> RelatedData::make_protected(const TimeEntry *data) const {
    return { timeEntries_m_, data };
}
template <> inline locked<TimeEntry> RelatedData::make_protected(TimeEntry *data) {
    return { timeEntries_m_, data };
}

template <> inline locked<const ObmExperiment> RelatedData::make_protected(const ObmExperiment *data) const {
    return { obmExperiments_m_, data };
}
template <> inline locked<ObmExperiment> RelatedData::make_protected(ObmExperiment *data) {
    return { obmExperiments_m_, data };
}


template<typename T>
void clearList(std::set<T *> &list);

}  // namespace toggl

#endif  // SRC_RELATED_DATA_H_
