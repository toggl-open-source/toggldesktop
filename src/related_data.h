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
class protected_set : public std::unique_lock<std::recursive_mutex> {
public:
    protected_set(mutex_type &mutex, std::set<T> *data)
        : std::unique_lock<std::recursive_mutex>(mutex)
        , data_(data)
    {}
    std::set<T> *operator->() { return data_; }
    std::set<T> &operator*() { return *data_; }
private:
    std::set<T> *data_;
};

template <typename T>
class const_protected_set : public std::unique_lock<std::recursive_mutex> {
public:
    const_protected_set(mutex_type &mutex, const std::set<T> *data)
        : std::unique_lock<std::recursive_mutex>(mutex)
        , data_(data)
    {}
    const std::set<T> *operator->() const { return data_; }
    const std::set<T> &operator*() const { return *data_; }
private:
    const std::set<T> *data_;
};

template <typename T>
class protected_variable : public std::unique_lock<std::recursive_mutex> {
public:
    protected_variable()
        : std::unique_lock<std::recursive_mutex>()
        , data_(nullptr)
    {}
    protected_variable(mutex_type &mutex, T *data)
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

template <typename T>
class const_protected_variable : public std::unique_lock<std::recursive_mutex> {
public:
    const_protected_variable()
        : std::unique_lock<std::recursive_mutex>()
        , data_(nullptr)
    {}
    const_protected_variable(mutex_type &mutex, const T *data)
        : std::unique_lock<std::recursive_mutex>(mutex)
        , data_(data)
    {}
    const T* data() const { return data_; }
    const T *operator->() const { return data_; }
    const T &operator*() const { return *data_; }
    operator bool() const {
        return owns_lock() && data_;
    }
private:
    const T *data_;
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
    protected_set<Workspace*> Workspaces() { return { workspaces_m_, &workspaces_ }; }
    protected_set<Client*> Clients() { return { clients_m_, &clients_}; }
    protected_set<Project*> Projects() { return { projects_m_, &projects_}; }
    protected_set<Task*> Tasks() { return { tasks_m_, &tasks_}; }
    protected_set<Tag*> Tags() { return { tags_m_, &tags_ }; }
    protected_set<TimeEntry*> TimeEntries() { return { timeEntries_m_, &timeEntries_ }; }
    protected_set<AutotrackerRule*> AutotrackerRules() { return { autotrackerRules_m_, &autotrackerRules_ }; }
    protected_set<TimelineEvent*> TimelineEvents() { return { timelineEvents_m_, &timelineEvents_ }; }
    protected_set<ObmAction*> ObmActions() { return { obmActions_m_, &obmActions_ }; }
    protected_set<ObmExperiment*> ObmExperiments() { return { obmExperiments_m_, &obmExperiments_ }; }
    const_protected_set<Workspace*> Workspaces() const { return { workspaces_m_, &workspaces_ }; }
    const_protected_set<Client*> Clients() const { return { clients_m_, &clients_}; }
    const_protected_set<Project*> Projects() const { return { projects_m_, &projects_}; }
    const_protected_set<Task*> Tasks() const { return { tasks_m_, &tasks_}; }
    const_protected_set<Tag*> Tags() const { return { tags_m_, &tags_ }; }
    const_protected_set<TimeEntry*> TimeEntries() const { return { timeEntries_m_, &timeEntries_ }; }
    const_protected_set<AutotrackerRule*> AutotrackerRules() const { return { autotrackerRules_m_, &autotrackerRules_ }; }
    const_protected_set<TimelineEvent*> TimelineEvents() const { return { timelineEvents_m_, &timelineEvents_ }; }
    const_protected_set<ObmAction*> ObmActions() const { return { obmActions_m_, &obmActions_ }; }
    const_protected_set<ObmExperiment*> ObmExperiments() const { return { obmExperiments_m_, &obmExperiments_ }; }

    void Clear();

    template <typename T> protected_variable<T> make_protected(T *data);

    protected_variable<Task> TaskByID(const Poco::UInt64 id);
    protected_variable<Client> ClientByID(const Poco::UInt64 id);
    protected_variable<Project> ProjectByID(const Poco::UInt64 id);
    protected_variable<Tag> TagByID(const Poco::UInt64 id);
    protected_variable<Workspace> WorkspaceByID(const Poco::UInt64 id);
    protected_variable<TimeEntry> TimeEntryByID(const Poco::UInt64 id);

    protected_variable<TimeEntry> TimeEntryByGUID(const guid GUID);
    protected_variable<Tag> TagByGUID(const guid GUID);
    protected_variable<Project> ProjectByGUID(const guid GUID);
    protected_variable<Client> ClientByGUID(const guid GUID);
    protected_variable<TimelineEvent> TimelineEventByGUID(const guid GUID);

    const_protected_variable<Task> TaskByID(const Poco::UInt64 id) const;
    const_protected_variable<Client> ClientByID(const Poco::UInt64 id) const;
    const_protected_variable<Project> ProjectByID(const Poco::UInt64 id) const;
    const_protected_variable<Tag> TagByID(const Poco::UInt64 id) const;
    const_protected_variable<Workspace> WorkspaceByID(const Poco::UInt64 id) const;
    const_protected_variable<TimeEntry> TimeEntryByID(const Poco::UInt64 id) const;

    const_protected_variable<TimeEntry> TimeEntryByGUID(const guid GUID) const;
    const_protected_variable<Tag> TagByGUID(const guid GUID) const;
    const_protected_variable<Project> ProjectByGUID(const guid GUID) const;
    const_protected_variable<Client> ClientByGUID(const guid GUID) const;
    const_protected_variable<TimelineEvent> TimelineEventByGUID(const guid GUID) const;

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

    AutotrackerRule *FindAutotrackerRule(const TimelineEvent event) const;

    protected_variable<Client> clientByProject(Project *p);
    const_protected_variable<Client> clientByProject(const Project *p) const;

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

template<typename T>
void clearList(std::set<T *> &list);

}  // namespace toggl

#endif  // SRC_RELATED_DATA_H_
