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
#include "tbb/concurrent_vector.h"

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
T *modelByID(const Poco::UInt64 id, tbb::concurrent_vector<T *> const *list);

template <typename T>
T *modelByGUID(const guid GUID, tbb::concurrent_vector<T *> const *list);

class RelatedData {
 public:
    tbb::concurrent_vector<Workspace *> Workspaces;
    tbb::concurrent_vector<Client *> Clients;
    tbb::concurrent_vector<Project *> Projects;
    tbb::concurrent_vector<Task *> Tasks;
    tbb::concurrent_vector<Tag *> Tags;
    tbb::concurrent_vector<TimeEntry *> TimeEntries;
    tbb::concurrent_vector<AutotrackerRule *> AutotrackerRules;
    tbb::concurrent_vector<TimelineEvent *> TimelineEvents;
    tbb::concurrent_vector<ObmAction *> ObmActions;
    tbb::concurrent_vector<ObmExperiment *> ObmExperiments;

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

 private:
    Poco::Mutex lockMutex;

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

}  // namespace toggl

#endif  // SRC_RELATED_DATA_H_
