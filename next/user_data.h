#ifndef SRC_USER_DATA_H_
#define SRC_USER_DATA_H_

#include "model/autotracker.h"
#include "model/client.h"
#include "model/obm_action.h"
#include "model/project.h"
#include "model/settings.h"
#include "model/tag.h"
#include "model/task.h"
#include "model/time_entry.h"
#include "model/timeline_event.h"
#include "model/user.h"
#include "model/workspace.h"

#include "views.h"

namespace toggl {

namespace view {
    class TimeEntry;
    class Autocomplete;
    class Generic;
    class Settings;
    class AutotrackerRule;
    class TimelineEvent;
    class Country;
}

class UserData : public User {
public:
    ProtectedModel<Workspace> Workspaces;
    ProtectedModel<Client> Clients;
    ProtectedModel<Project> Projects;
    ProtectedModel<Task> Tasks;
    ProtectedModel<Tag> Tags;
    ProtectedModel<TimeEntry> TimeEntries;
    ProtectedModel<AutotrackerRule> AutotrackerRules;
    ProtectedModel<TimelineEvent> TimelineEvents;
    ProtectedModel<ObmAction> ObmActions;
    ProtectedModel<ObmExperiment> ObmExperiments;

    bool CanSeeBillable(
        const Workspace *ws) const;

    //RelatedData related;

    locked<TimeEntry> RunningTimeEntry();
    locked<const TimeEntry> RunningTimeEntry() const;
    bool IsTracking() const {
        return RunningTimeEntry();
    }

    locked<TimeEntry> Start(
        const std::string description,
        const std::string duration,
        const Poco::UInt64 task_id,
        const Poco::UInt64 project_id,
        const std::string project_guid,
        const std::string tags);

    locked<TimeEntry> Continue(
        const std::string GUID,
        const bool manual_mode);

    void Stop(std::vector<TimeEntry *> *stopped = nullptr);

    // Discard time. Return a new time entry if
    // the discarded time was split into a new time entry
    locked<TimeEntry> DiscardTimeAt(
        const std::string guid,
        const Poco::Int64 at,
        const bool split_into_new_entry);

    Project *CreateProject(
        const Poco::UInt64 workspace_id,
        const Poco::UInt64 client_id,
        const std::string client_guid,
        const std::string client_name,
        const std::string project_name,
        const bool is_private,
        const std::string project_color,
        const bool billable);

    void AddProjectToList(Project *p);

    Client *CreateClient(
        const Poco::UInt64 workspace_id,
        const std::string client_name);

    void AddClientToList(Client *c);

    std::string DateDuration(TimeEntry *te) const;

    void MarkTimelineBatchAsUploaded(
        const std::vector<TimelineEvent> &events);
    void CompressTimeline();
    std::vector<TimelineEvent> CompressedTimeline() const;

    error UpdateJSON(
        std::vector<TimeEntry *> * const,
        std::string *result) const;


    std::string dirtyObjectsJSON(std::vector<TimeEntry *> * const) const;

    void processResponseArray(
        std::vector<BatchUpdateResult> * const results,
        std::vector<TimeEntry *> *dirty,
        std::vector<error> *errors);


    template<class T>
    void deleteZombies(locked<std::vector<T>> &list, const std::set<Poco::UInt64> &alive);

    template <typename T>
    void deleteRelatedModelsWithWorkspace(const Poco::UInt64 wid, locked<std::vector<T *>> &list);

    template <typename T>
    void removeProjectFromRelatedModels(const Poco::UInt64 pid, locked<std::vector<T *>> &list);

    void DeleteRelatedModelsWithWorkspace(const Poco::UInt64 wid);

    void RemoveClientFromRelatedModels(const Poco::UInt64 cid);

    void RemoveProjectFromRelatedModels(const Poco::UInt64 pid);

    void RemoveTaskFromRelatedModels(const Poco::UInt64 tid);

    void loadUserTagFromJSON(Json::Value data, std::set<Poco::UInt64> *alive);

    void loadUserTaskFromJSON(Json::Value data, std::set<Poco::UInt64> *alive);

    error LoadTimeEntriesFromJSONString(const std::string& json);

    void loadObmExperimentFromJson(Json::Value const &obm);

    void loadUserAndRelatedDataFromJSON(Json::Value data, const bool &including_related_data);

    void loadUserClientFromSyncJSON(Json::Value data, std::set<Poco::UInt64> *alive);

    void loadUserClientFromJSON(Json::Value data, std::set<Poco::UInt64> *alive);

    void loadUserProjectFromSyncJSON(Json::Value data, std::set<Poco::UInt64> *alive);

    void loadUserProjectFromJSON(Json::Value data, std::set<Poco::UInt64> *alive);

    void loadUserTimeEntryFromJSON(Json::Value data, std::set<Poco::UInt64> *alive);

    void loadUserWorkspaceFromJSON(Json::Value data, std::set<Poco::UInt64> *alive);

    error LoadWorkspacesFromJSONString(const std::string& json);

    error EnableOfflineLogin(const std::string password);

    void LoadObmExperiments(Json::Value const &obm);

    error LoadUserAndRelatedDataFromJSONString(const std::string &json, const bool &including_related_data);

    void loadUserUpdateFromJSON(Json::Value node);

    error LoadUserUpdateFromJSONString(const std::string json);

    std::string generateKey(const std::string password);


    template<typename T>
    void clearList(std::vector<T *> *list) {
        for (size_t i = 0; i < list->size(); i++) {
            T *value = (*list)[i];
            delete value;
        }
        list->clear();
    }

    void Clear();

    error DeleteAutotrackerRule(const Poco::Int64 local_id);

    AutotrackerRule *FindAutotrackerRule(const TimelineEvent event) const;

    bool HasMatchingAutotrackerRule(const std::string lowercase_term) const;

    Poco::Int64 NumberOfUnsyncedTimeEntries() const;

    std::vector<TimelineEvent *> VisibleTimelineEvents() const;

    std::vector<TimeEntry *> VisibleTimeEntries() const;

    Poco::Int64 TotalDurationForDate(const TimeEntry *match) const;

    TimeEntry *LatestTimeEntry() const;

    // Add time entries, in format:
    // Description - Task. Project. Client
    void timeEntryAutocompleteItems(std::set<std::string> *unique_names, std::map<Poco::UInt64, std::string> *ws_names, std::vector<view::Autocomplete> *list, std::map<std::string, std::vector<view::Autocomplete> > *items) const;

    // Add tasks, in format:
    // Task. Project. Client
    void taskAutocompleteItems(std::set<std::string> *unique_names, std::map<Poco::UInt64, std::string> *ws_names, std::vector<toggl::view::Autocomplete> *list, std::map<Poco::UInt64, std::vector<view::Autocomplete> > *items) const;

    // Add projects, in format:
    // Project. Client
    void projectAutocompleteItems(std::set<std::string> *unique_names, std::map<Poco::UInt64, std::string> *ws_names, std::vector<view::Autocomplete> *list, std::map<std::string, std::vector<view::Autocomplete> > *items, std::map<Poco::UInt64, std::vector<view::Autocomplete> > *task_items) const;

    void TimeEntryAutocompleteItems(std::vector<view::Autocomplete> *result) const;

    void MinitimerAutocompleteItems(std::vector<view::Autocomplete> *result) const;

    void mergeGroupedAutocompleteItems(std::vector<view::Autocomplete> *result, std::map<std::string, std::vector<view::Autocomplete> > *items) const;


    void ProjectAutocompleteItems(std::vector<view::Autocomplete> *result) const;

    void workspaceAutocompleteItems(std::set<std::string> *, std::map<Poco::UInt64, std::string> *ws_names, std::vector<view::Autocomplete> *) const;

    void TagList(std::vector<std::string> *tags, const Poco::UInt64 wid) const;

    void WorkspaceList(std::vector<Workspace *> *result) const;

    locked<std::vector<Client *> > ClientList();

    void ProjectLabelAndColorCode(TimeEntry * const te, view::TimeEntry *view) const;

    locked<const Client> clientByProject(const Project *p) const;
    locked<Client> clientByProject(locked<Project> &p);
    locked<const Client> clientByProject(locked<const Project> &p) const;

    bool HasPremiumWorkspaces() const;

    bool CanAddProjects() const;

    error SetAPITokenFromOfflineData(const std::string password);


    template <typename T>
    T *modelByGUID(const std::string &GUID, std::vector<T *> const *list) {
        if (GUID.empty()) {
            return nullptr;
        }
        typedef typename std::vector<T *>::const_iterator iterator;
        for (iterator it = list->begin(); it != list->end(); it++) {
            T *model = *it;
            if (model->GUID() == GUID) {
                return model;
            }
        }
        return nullptr;
    }

    template<typename T>
    T *modelByID(const Poco::UInt64 id, std::vector<T *> const *list) {
        if (!id) {
            return nullptr;
        }
        typedef typename std::vector<T *>::const_iterator iterator;
        for (iterator it = list->begin(); it != list->end(); it++) {
            T *model = *it;
            if (model->ID() == id) {
                return model;
            }
        }
        return nullptr;
    }

    template<typename T>
    void EnsureWID(locked<T> &model) const {
        // Do nothing if TE already has WID assigned
        if (model->WID()) {
            return;
        }

        // Try to set default user WID
        if (DefaultWID()) {
            model->SetWID(DefaultWID());
            return;
        }

        // Try to set first WID available
        auto workspaces = Workspaces();
        std::vector<Workspace *>::const_iterator it = workspaces->begin();
        if (it != workspaces->end()) {
            Workspace *ws = *it;
            model->SetWID(ws->ID());
        }
    }
};

} // namespace toggl

#endif  // SRC_USER_DATA_H_
