// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_RELATED_DATA_H_
#define SRC_RELATED_DATA_H_

#include <vector>
#include <set>
#include <string>
#include <map>
#include <functional>

#include "const.h"
#include "types.h"
#include "util/logger.h"
#include "util/memory.h"

#include <Poco/Mutex.h>
#include <Poco/UTF8String.h>
#include <json/json.h>

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
class User;

namespace view {
class TimeEntry;
class Autocomplete;
};

template<typename T>
T *modelByID(const Poco::UInt64 id, std::vector<T *> const *list);

template <typename T>
T *modelByGUID(const guid GUID, std::vector<T *> const *list);

extern bool CompareClients(const Client *l, const Client *r);
extern bool CompareProjects(const Project *l, const Project *r);
extern bool CompareTimeEntries(const TimeEntry *a, const TimeEntry *b);

class TOGGL_INTERNAL_EXPORT RelatedData {
 public:
    RelatedData();

    ProtectedModel<toggl::User> User;
    ProtectedContainer<Workspace> Workspaces;
    ProtectedContainer<Client> Clients;
    ProtectedContainer<Project> Projects;
    ProtectedContainer<Task> Tasks;
    ProtectedContainer<Tag> Tags;
    ProtectedContainer<TimeEntry> TimeEntries;
    ProtectedContainer<AutotrackerRule> AutotrackerRules;
    ProtectedContainer<TimelineEvent> TimelineEvents;
    ProtectedContainer<ObmAction> ObmActions;
    ProtectedContainer<ObmExperiment> ObmExperiments;

    void Clear();

    template<typename T>
    error LoadModelFromJSON(Poco::UInt64 userID, ProtectedContainer<T> container, const Json::Value &data, std::set<Poco::UInt64> *alive = nullptr) {
        // alive can be 0, dont assert/check it
        Poco::UInt64 id = data["id"].asUInt64();
        if (!id) {
            logger().error(kBackendIsSendingInvalidDataError);
            return kBackendIsSendingInvalidDataError;
        }

        auto model = container.byID(id);

        if (!model) {
            model = container.byGUID(data["guid"].asString());
        }

        if (!data["server_deleted_at"].asString().empty()) {
            if (model) {
                model->MarkAsDeletedOnServer();
            }
            return noError;
        }

        if (!model) {
            model = container.create();
        }
        if (alive) {
            alive->insert(id);
        }
        model->SetUID(userID);
        model->LoadFromJSON(data);

        return noError;
    }

    void TagList(std::vector<std::string> *result, const Poco::UInt64 wid) const;
    std::vector<locked<Workspace>> WorkspaceList();
    std::vector<locked<Client>> ClientList();

    Poco::Int64 NumberOfUnsyncedTimeEntries() const;

    // Find the time entry that was stopped most recently
    locked<TimeEntry> LatestTimeEntry();

    // Collect visible timeline events
    std::vector<locked<TimelineEvent>> VisibleTimelineEvents();

    // Collect visible time entries
    std::vector<locked<TimeEntry>> VisibleTimeEntries();

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

    Logger logger() const;

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
