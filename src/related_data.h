// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_RELATED_DATA_H_
#define SRC_RELATED_DATA_H_

#include <vector>
#include <set>
#include <string>
#include <map>
#include <functional>
#include <mutex>

#include "./timeline_event.h"
#include "./types.h"

#include "Poco/Mutex.h"
#include <functional>

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
T *modelByID(const Poco::UInt64 id, std::vector<T *> const *list);

template <typename T>
T *modelByGUID(const guid GUID, std::vector<T *> const *list);

template <typename T> using protected_vector = std::pair<std::unique_lock<std::mutex>, std::vector<T>*>;
template <typename T> using protected_const_vector = std::pair<std::unique_lock<std::mutex>, const std::vector<T>*>;

class RelatedData {
 public:
    // ========================= BEWARE ============================
    // when using these methods, the result HAS TO be stored:
    // USE THIS: auto WSs = GetWorkspaces(); WS.second->...
    // NOT THIS: GetWorkspaces().second;
    // the latter will unlock the mutex immediately
    //
    // If not unlocked explicitely (by calling WSs.first.unlock()),
    // it will get unlocked when the surrounding scope ends
    protected_vector<Workspace *> GetWorkspaces();
    protected_vector<Client *> GetClients();
    protected_vector<Project *> GetProjects();
    protected_vector<Task *> GetTasks();
    protected_vector<Tag *> GetTags();
    protected_vector<TimeEntry *> GetTimeEntries();
    protected_vector<AutotrackerRule *> GetAutotrackerRules();
    protected_vector<TimelineEvent *> GetTimelineEvents();
    protected_vector<ObmAction *> GetObmActions();
    protected_vector<ObmExperiment *> GetObmExperiments();

    protected_const_vector<Workspace *> GetWorkspaces() const;
    protected_const_vector<Client *> GetClients() const;
    protected_const_vector<Project *> GetProjects() const;
    protected_const_vector<Task *> GetTasks() const;
    protected_const_vector<Tag *> GetTags() const ;
    protected_const_vector<TimeEntry *> GetTimeEntries() const;
    protected_const_vector<AutotrackerRule *> GetAutotrackerRules() const;
    protected_const_vector<TimelineEvent *> GetTimelineEvents() const;
    protected_const_vector<ObmAction *> GetObmActions() const;
    protected_const_vector<ObmExperiment *> GetObmExperiments() const;

    void Clear();

    Task *TaskByID(const Poco::UInt64 id) const;
    Client *ClientByID(const Poco::UInt64 id) const;
    Project *ProjectByID(const Poco::UInt64 id) const;
    Tag *TagByID(const Poco::UInt64 id) const;
    Workspace *WorkspaceByID(const Poco::UInt64 id) const;
    TimeEntry *TimeEntryByID(const Poco::UInt64 id) const;

    void TagList(
        std::vector<std::string> *result,
        const Poco::UInt64 wid) const;
    void WorkspaceList(std::vector<Workspace *> *) const;
    void ClientList(std::vector<Client *> *) const;

    TimeEntry *TimeEntryByGUID(const guid GUID) const;
    Tag *TagByGUID(const guid GUID) const;
    Project *ProjectByGUID(const guid GUID) const;
    Client *ClientByGUID(const guid GUID) const;
    TimelineEvent *TimelineEventByGUID(const guid GUID) const;

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

    Client *clientByProject(Project *p) const;

    void pushBackTimeEntry(TimeEntry  *timeEntry);

    void forEachTimeEntries(std::function<void(TimeEntry *)> f);

 private:
    Poco::Mutex timeEntries_m_;

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

    std::vector<Workspace *> _Workspaces;
    mutable std::mutex _WorkspacesMutex;
    std::vector<Client *> _Clients;
    mutable std::mutex _ClientsMutex;
    std::vector<Project *> _Projects;
    mutable std::mutex _ProjectsMutex;
    std::vector<Task *> _Tasks;
    mutable std::mutex _TasksMutex;
    std::vector<Tag *> _Tags;
    mutable std::mutex _TagsMutex;
    std::vector<TimeEntry *> _TimeEntries;
    mutable std::mutex _TimeEntriesMutex;
    std::vector<AutotrackerRule *> _AutotrackerRules;
    mutable std::mutex _AutotrackerRulesMutex;
    std::vector<TimelineEvent *> _TimelineEvents;
    mutable std::mutex _TimelineEventsMutex;
    std::vector<ObmAction *> _ObmActions;
    mutable std::mutex _ObmActionsMutex;
    std::vector<ObmExperiment *> _ObmExperiments;
    mutable std::mutex _ObmExperimentsMutex;
};

template<typename T>
void clearList(std::vector<T *> *list);

}  // namespace toggl

#endif  // SRC_RELATED_DATA_H_
