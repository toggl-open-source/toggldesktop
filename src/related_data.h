// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_RELATED_DATA_H_
#define SRC_RELATED_DATA_H_

#include <vector>
#include <set>
#include <string>
#include <map>
#include <functional>

#include "workspace.h"
#include "client.h"
#include "project.h"
#include "task.h"
#include "tag.h"
#include "time_entry.h"
#include "autotracker.h"
#include "obm_action.h"
#include "timeline_event.h"

#include "types.h"
#include "util/memory.h"

#include <Poco/Mutex.h>
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

class TOGGL_INTERNAL_EXPORT RelatedData {
 public:
    RelatedData() {

    }

    ProtectedContainer<Workspace> Workspaces { this };
    ProtectedContainer<Client> Clients { this };
    ProtectedContainer<Project> Projects { this };
    ProtectedContainer<Task> Tasks { this };
    ProtectedContainer<Tag> Tags { this };
    ProtectedContainer<TimeEntry> TimeEntries { this };
    ProtectedContainer<AutotrackerRule> AutotrackerRules { this };
    ProtectedContainer<TimelineEvent> TimelineEvents { this };
    ProtectedContainer<ObmAction> ObmActions { this };
    ProtectedContainer<ObmExperiment> ObmExperiments { this };

    void Clear();

    void TagList(std::vector<std::string> *result, const Poco::UInt64 wid) const;
    void WorkspaceList(std::vector<Workspace *> *) const;
    void ClientList(std::vector<Client *> *) const;

    Poco::Int64 NumberOfUnsyncedTimeEntries() const;

    // Find the time entry that was stopped most recently
    locked<TimeEntry> LatestTimeEntry();

    // Collect visible timeline events
    std::vector<TimelineEvent *> VisibleTimelineEvents() const;

    // Collect visible time entries
    std::vector<TimeEntry *> VisibleTimeEntries() const;

    Poco::Int64 TotalDurationForDate(locked<TimeEntry> &match);

    // avoid duplicates
    bool HasMatchingAutotrackerRule(const std::string &lowercase_term) const;

    error DeleteAutotrackerRule(const Poco::Int64 local_id);

    void TimeEntryAutocompleteItems(std::vector<view::Autocomplete> *) const;
    void MinitimerAutocompleteItems(std::vector<view::Autocomplete> *) const;
    void ProjectAutocompleteItems(std::vector<view::Autocomplete> *) const;

    void ProjectLabelAndColorCode(locked<TimeEntry> &te, view::TimeEntry *view) const;

    locked<AutotrackerRule> FindAutotrackerRule(locked<TimelineEvent> &event);

    locked<Client> clientByProject(locked<Project> &p);
    locked<const Client> clientByProject(locked<const Project> &p) const;

    void forEachTimeEntries(std::function<void(TimeEntry *)> f);

 private:
    Poco::Mutex timeEntries_m_;

    void timeEntryAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<view::Autocomplete> *list,
        std::map<std::string, std::vector<view::Autocomplete> > *items) const;

    void taskAutocompleteItems(std::set<std::string> *unique_names,
                               std::map<Poco::UInt64, std::string> *ws_names,
                               std::vector<view::Autocomplete> *list,
                               std::map<Poco::UInt64, std::vector<view::Autocomplete> > *items) const;

    void projectAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<view::Autocomplete> *list,
        std::map<std::string, std::vector<view::Autocomplete> > *items,
        std::map<Poco::UInt64, std::vector<view::Autocomplete> > *task_items) const;

    void workspaceAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<view::Autocomplete> *list) const;

    void mergeGroupedAutocompleteItems(
        std::vector<view::Autocomplete> *result,
        std::map<std::string, std::vector<view::Autocomplete> > *items) const;
};

template<typename T>
void clearList(std::vector<T *> *list);

}  // namespace toggl

#endif  // SRC_RELATED_DATA_H_
