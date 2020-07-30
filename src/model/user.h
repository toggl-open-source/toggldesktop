// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_USER_H_
#define SRC_USER_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include <json/json.h>  // NOLINT

#include "model/base_model.h"
#include "related_data.h"
#include "types.h"
#include "model/workspace.h"

#include <Poco/LocalDateTime.h>
#include <Poco/Types.h>

namespace toggl {

class TOGGL_INTERNAL_EXPORT User : public BaseModel {
 public:
    User() : BaseModel() {}
    // Before undeleting, see how the copy constructor in BaseModel works
    User(const User &o) = delete;
    User &operator=(const User &o) = delete;
    ~User();

    Property<std::string> APIToken { "" };
    Property<std::string> Fullname { "" };
    Property<std::string> Email { "" };
    Property<std::string> TimeOfDayFormat { "" };
    Property<std::string> DurationFormat { "" };
    Property<std::string> OfflineData { "" };
    Property<Poco::UInt64> DefaultWID { 0 };
    Property<Poco::UInt64> DefaultPID { 0 };
    Property<Poco::UInt64> DefaultTID { 0 };
    // Unix timestamp of the user data; returned from API
    Property<Poco::Int64> Since { 0 };
    Property<bool> RecordTimeline { false };

    Property<bool> HasLoadedMore { false };
    Property<bool> CollapseEntries { false };

    Property<bool> IsNewUser { false };

    void SetAPIToken(const std::string &api_token);
    void SetFullname(const std::string &value);
    void SetEmail(const std::string &value);
    void SetTimeOfDayFormat(const std::string &value);
    void SetDurationFormat(const std::string &value);
    void SetOfflineData(const std::string &value);
    void SetDefaultWID(Poco::UInt64 value);
    void SetDefaultPID(Poco::UInt64 value);
    void SetDefaultTID(Poco::UInt64 value);
    // Unix timestamp of the user data; returned from API
    void SetSince(Poco::Int64 value);
    void SetRecordTimeline(bool value);
    void ConfirmLoadedMore();
    void SetCollapseEntries(bool value);

    // Derived data and modifiers
    bool HasValidSinceDate() const;

    error EnableOfflineLogin(
        const std::string &password);

    bool HasPremiumWorkspaces() const;
    bool CanAddProjects() const;

    bool CanSeeBillable(
        const Workspace *ws) const;

    void SetLastTEDate(const std::string &value);

    TimeEntry *RunningTimeEntry() const;
    bool IsTracking() const {
        return RunningTimeEntry() != nullptr;
    }

    TimeEntry *Start(
        const std::string &description,
        const std::string &duration,
        Poco::UInt64 task_id,
        Poco::UInt64 project_id,
        const std::string project_guid,
        const std::string tags,
        time_t started,
        time_t ended,
        bool stop_current_running);

    TimeEntry *Continue(
        const std::string &guid,
        bool manual_mode);

    void Stop(std::vector<TimeEntry *> *stopped = nullptr);

    // Discard time. Return a new time entry if
    // the discarded time was split into a new time entry
    TimeEntry *DiscardTimeAt(
        const std::string &guid,
        Poco::Int64 at,
        bool split_into_new_entry);

    Project *CreateProject(
        Poco::UInt64 workspace_id,
        Poco::UInt64 client_id,
        const std::string &client_guid,
        const std::string &client_name,
        const std::string &project_name,
        bool is_private,
        const std::string &project_color,
        bool billable);

    void AddProjectToList(Project *p);

    Client *CreateClient(
        Poco::UInt64 workspace_id,
        const std::string &client_name);

    void AddClientToList(Client *c);

    std::string DateDuration(TimeEntry *te) const;

    RelatedData related;

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;

    // Handle related model deletions
    void DeleteRelatedModelsWithWorkspace(Poco::UInt64 wid);
    void RemoveClientFromRelatedModels(Poco::UInt64 cid);
    void RemoveProjectFromRelatedModels(Poco::UInt64 pid);
    void RemoveTaskFromRelatedModels(Poco::UInt64 tid);

    error LoadUserUpdateFromJSONString(const std::string &json);

    error LoadUserAndRelatedDataFromJSONString(const std::string &json,
        bool including_related_data, bool syncServer);

    void LoadUserAndRelatedDataFromJSON(const Json::Value &root,
        bool including_related_data,
        bool syncServer);

    error LoadWorkspacesFromJSONString(const std::string & json);

    error LoadTimeEntriesFromJSONString(const std::string &json);

    error SetAPITokenFromOfflineData(const std::string &password);

    void MarkTimelineBatchAsUploaded(
        const std::vector<const TimelineEvent*> &events);
    void CompressTimeline();

    std::vector<const TimelineEvent *> CompressedTimelineForUI(const Poco::LocalDateTime *date) const;
    std::vector<const TimelineEvent *> CompressedTimelineForUpload(const Poco::LocalDateTime *date = nullptr) const;

    error UpdateJSON(
        std::vector<TimeEntry *> * const,
        std::string *result) const;

    void LoadObmExperiments(Json::Value const &obm);

    // excludeCollapseTimeEntries should be removed when getting rid of Context::pullUserPreferences
    bool LoadUserPreferencesFromJSON(
        const Json::Value &data,
        bool excludeCollapseTimeEntries);

    template <class T>
    bool SetModelID(
        Poco::UInt64 id,
        T* model);

    template<typename T>
    void EnsureWID(T *model) const {
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
        std::vector<Workspace *>::const_iterator it =
            related.Workspaces.begin();
        if (it != related.Workspaces.end()) {
            Workspace *ws = *it;
            model->SetWID(ws->ID());
        }
    }

    static error UserID(
        const std::string &json_data_string,
        Poco::UInt64 *result);

    static error LoginToken(
        const std::string &json_data_string,
        std::string *result);

    static error GenerateOfflineLogin(
        const std::string &email,
        const std::string &password,
        std::string *result);

 private:
    void loadUserTagFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr);

    error loadUserFromJSON(
        const Json::Value &node);

    error loadRelatedDataFromJSON(const Json::Value &node,
        bool including_related_data,
        bool syncServer);

    void loadUserUpdateFromJSON(
        Json::Value list);

    void loadUserProjectFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr,
        bool syncServer = false);

    void loadUserProjectFromSyncJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr,
        bool syncServer = false);

    void loadUserWorkspaceFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr);

    void loadUserClientFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr,
        bool syncServer = false);

    void loadUserClientFromSyncJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr,
        bool syncServer = false);

    void loadUserTaskFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr);

    void loadUserTimeEntryFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr,
        bool syncServer = false);

    std::string dirtyObjectsJSON(std::vector<TimeEntry *> * const) const;

    std::string generateKey(const std::string &password);

    void loadObmExperimentFromJson(Json::Value const &obm);

    std::vector<const TimelineEvent *> CompressedTimeline(
        const Poco::LocalDateTime *date = nullptr, bool is_for_upload = true) const;

    Poco::Mutex loadTimeEntries_m_;
};

template<class T>
bool User::SetModelID(Poco::UInt64 id, T *model) {
    poco_check_ptr(model);

    {
        Poco::Mutex::ScopedLock lock(loadTimeEntries_m_);
        auto otherModel = related.ModelByID<T>(id);
        if (otherModel && otherModel != model) {
            // this means that somehow we already have a time entry with the ID
            // that was just returned from a response to time entry creation request
            logger().error("There is already a newer version of this ", model->ModelName(), " with ID ", id);

            // clearing the GUID to make sure there's no GUID conflict
            model->SetGUID("");

            // deleting the duplicate entry
            // this entry has no ID so the corresponding server entry will not be deleted
            model->Delete();
            return false;
        }
        model->SetID(id);
        return true;
    }
}

template<class T>
void deleteZombies(
    const std::vector<T> &list,
    const std::set<Poco::UInt64> &alive);

template <typename T>
void deleteRelatedModelsWithWorkspace(Poco::UInt64 wid,
                                      std::vector<T *> *list);

template <typename T>
void removeProjectFromRelatedModels(Poco::UInt64 pid,
                                    std::vector<T *> *list);

}  // namespace toggl

#endif  // SRC_USER_H_
