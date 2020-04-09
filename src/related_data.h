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
#include "util/memory.h"
#include "util/logger.h"

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

    template <typename T>
    error LoadModelFromJSON(ProtectedContainer<T> &into, const Json::Value &data, Poco::UInt64 uid, std::set<Poco::UInt64> *alive = nullptr) {
        std::cerr << "WORKING ON " << into.ModelName() << " and now the JSON is:" << std::endl;
        Json::StyledWriter w;
        std::cerr << w.write(data) << std::endl << std::endl << std::flush;
        auto id = data["id"].asUInt64();
        if (!id) {
            logger().error("Backend is sending invalid data: ignoring update without an ID");
            // TODO maybe return something?
            return noError;
        }
        locked<T> model = into.byID(id);
        if (!data["server_deleted_at"].asString().empty()) {
            if (model)
                model->MarkAsDeletedOnServer();
            return noError;
        }
        if (alive)
            alive->insert(id);
        if (!model) {
            model = into.create(data);
            model->SetUID(uid);
        }
        else {
            model->SetUID(uid);
            model->LoadFromJSON(data);
        }
        return noError;
    }
    template <typename T>
    error LoadModelFromJSON(ProtectedContainer<T> &into, const std::string &data, Poco::UInt64 uid, std::set<Poco::UInt64> *alive = nullptr) {
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(data, root)) {
            return error("Failed to parse JSON for model " + into.ModelName());
        }
        if (root.size() == 0)
            return kMissingModelData;
        if (root.isArray()) {
            for (auto i : root) {
                auto err = LoadModelFromJSON<T>(into, i, uid, alive);
                if (err != noError)
                    return err;
            }
        }
        else {
            return LoadModelFromJSON<T>(into, root, uid, alive);
        }
        return noError;
    }

    template<class T>
    void deleteZombies(ProtectedContainer<T> &list, const std::set<Poco::UInt64> &alive) {
        for (auto model : list) {
            if (!model->ID()) {
                // If model has no server-assigned ID, it's not even
                // pushed to server. So actually we don't know if it's
                // a zombie or not. Ignore:
                continue;
            }
            if (alive.end() == alive.find(model->ID())) {
                model->MarkAsDeletedOnServer();
            }
        }
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

    locked<const Client> clientByProject(locked<const Project> &p) const;

 private:
    Poco::Mutex timeEntries_m_;

    Logger logger() const {
        return { "related_data" };
    }

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
