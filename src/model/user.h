// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_USER_H_
#define SRC_USER_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include <json/json.h>  // NOLINT

#include "model/base_model.h"
#include "batch_update_result.h"
#include "related_data.h"
#include "types.h"
#include "model/workspace.h"

#include <Poco/LocalDateTime.h>
#include <Poco/Types.h>

namespace toggl {

class TOGGL_INTERNAL_EXPORT User : public BaseModel {
 public:
    User() {}
    ~User();

    // API token is not saved into DB, so no
    // no dirty checking needed for it.
    virtual bool IsDirty() const override {
        return BaseModel::IsDirty() || IsAnyPropertyDirty(Fullname, Email, TimeOfDayFormat, DurationFormat, OfflineData, Since, DefaultWID, DefaultPID, DefaultTID, RecordTimeline, StoreStartAndStopTime, HasLoadedMore, CollapseEntries);
    }
    virtual void ClearDirty() override {
        BaseModel::ClearDirty();
        AllPropertiesClearDirty(Fullname, Email, TimeOfDayFormat, DurationFormat, OfflineData, Since, DefaultWID, DefaultPID, DefaultTID, RecordTimeline, StoreStartAndStopTime, HasLoadedMore, CollapseEntries);
    }
    /* TODO:
     * SetTimeOfDayFormat and SetDurationFormat used to set Formatter::TimeOfDayFormat = value
     *    straight away globally. This is probably not handled now
     */

    Property<std::string> APIToken { "" };
    Property<std::string> Fullname { "" };
    Property<std::string> Email { "" };
    Property<std::string> TimeOfDayFormat { "" };
    Property<std::string> DurationFormat { "" };
    Property<std::string> OfflineData { "" };
    // Unix timestamp of the user data; returned from API
    Property<Poco::Int64> Since { 0 };
    Property<Poco::UInt64> DefaultWID { 0 };
    Property<Poco::UInt64> DefaultPID { 0 };
    Property<Poco::UInt64> DefaultTID { 0 };
    Property<bool> RecordTimeline { false };
    Property<bool> StoreStartAndStopTime { true };
    Property<bool> HasLoadedMore { false };
    Property<bool> CollapseEntries { false };

    RelatedData related;

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
        const Poco::UInt64 task_id,
        const Poco::UInt64 project_id,
        const std::string project_guid,
        const std::string tags,
        const time_t started,
        const time_t ended,
        const bool stop_current_running);

    TimeEntry *Continue(
        const std::string &GUID,
        const bool manual_mode);

    void Stop(std::vector<TimeEntry *> *stopped = nullptr);

    // Discard time. Return a new time entry if
    // the discarded time was split into a new time entry
    TimeEntry *DiscardTimeAt(
        const std::string &guid,
        const Poco::Int64 at,
        const bool split_into_new_entry);

    Project *CreateProject(
        const Poco::UInt64 workspace_id,
        const Poco::UInt64 client_id,
        const std::string &client_guid,
        const std::string &client_name,
        const std::string &project_name,
        const bool is_private,
        const std::string &project_color,
        const bool billable);

    void AddProjectToList(Project *p);

    Client *CreateClient(
        const Poco::UInt64 workspace_id,
        const std::string &client_name);

    void AddClientToList(Client *c);

    std::string DateDuration(TimeEntry *te) const;

    bool HasValidSinceDate() const;

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;

    // Handle related model deletions
    void DeleteRelatedModelsWithWorkspace(const Poco::UInt64 wid);
    void RemoveClientFromRelatedModels(const Poco::UInt64 cid);
    void RemoveProjectFromRelatedModels(const Poco::UInt64 pid);
    void RemoveTaskFromRelatedModels(const Poco::UInt64 tid);

    error LoadUserUpdateFromJSONString(const std::string &json);

    error LoadUserAndRelatedDataFromJSONString(
        const std::string &json,
        const bool &including_related_data);

    error LoadWorkspacesFromJSONString(const std::string & json);

    error LoadTimeEntriesFromJSONString(const std::string &json);

    error SetAPITokenFromOfflineData(const std::string &password);

    void MarkTimelineBatchAsUploaded(
        const std::vector<TimelineEvent> &events);
    void CompressTimeline();

    std::vector<TimelineEvent> CompressedTimelineForUI(const Poco::LocalDateTime *date) const;
    std::vector<TimelineEvent> CompressedTimelineForUpload(const Poco::LocalDateTime *date = nullptr) const;

    error UpdateJSON(
        std::vector<TimeEntry *> * const,
        std::string *result) const;

    void LoadObmExperiments(Json::Value const &obm);

    bool LoadUserPreferencesFromJSON(
        Json::Value data);

    template<class T>
    bool SetModelID(
        Poco::UInt64 id,
        T *model);

    template<typename T>
    void EnsureWID(T *model) const {
        // Do nothing if TE already has WID assigned
        if (model->WID()) {
            return;
        }

        // Try to set default user WID
        if (DefaultWID()) {
            model->WID.Set(DefaultWID());
            return;
        }

        // Try to set first WID available
        std::vector<Workspace *>::const_iterator it =
            related.Workspaces.begin();
        if (it != related.Workspaces.end()) {
            Workspace *ws = *it;
            model->WID.Set(ws->ID());
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

    void loadUserAndRelatedDataFromJSON(
        Json::Value node,
        const bool &including_related_data);

    void loadUserUpdateFromJSON(
        Json::Value list);

    void loadUserProjectFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr);

    void loadUserProjectFromSyncJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr);

    void loadUserWorkspaceFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr);

    void loadUserClientFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr);

    void loadUserClientFromSyncJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr);

    void loadUserTaskFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr);

    void loadUserTimeEntryFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr);

    std::string dirtyObjectsJSON(std::vector<TimeEntry *> * const) const;

    void processResponseArray(
        std::vector<BatchUpdateResult> * const results,
        std::vector<TimeEntry *> *dirty,
        std::vector<error> *errors);

    error requestJSON(
        const std::string &method,
        const std::string &relative_url,
        const std::string &json,
        const bool authenticate_with_api_token,
        std::string *response_body);

    void parseResponseArray(
        const std::string &response_body,
        std::vector<BatchUpdateResult> *responses);

    std::string generateKey(const std::string &password);

    void loadObmExperimentFromJson(Json::Value const &obm);

    std::vector<TimelineEvent> CompressedTimeline(
        const Poco::LocalDateTime *date = nullptr, bool is_for_upload = true) const;


    Poco::Mutex loadTimeEntries_m_;
};

template<class T>
bool User::SetModelID(Poco::UInt64 id, T *model) {
    poco_check_ptr(model);

    {
        Poco::Mutex::ScopedLock lock(loadTimeEntries_m_);
        auto otherModel = related.ModelByID<T>(id);
        if (otherModel) {
            // this means that somehow we already have a time entry with the ID
            // that was just returned from a response to time entry creation request
            logger().error("There is already a newer version of this entry");

            // clearing the GUID to make sure there's no GUID conflict
            model->GUID.Set("");

            // deleting the duplicate entry
            // this entry has no ID so the corresponding server entry will not be deleted
            model->Delete();
            return false;
        }
        model->ID.Set(id);
        return true;
    }
}

template<class T>
void deleteZombies(
    const std::vector<T> &list,
    const std::set<Poco::UInt64> &alive);

template <typename T>
void deleteRelatedModelsWithWorkspace(const Poco::UInt64 wid,
                                      std::vector<T *> *list);

template <typename T>
void removeProjectFromRelatedModels(const Poco::UInt64 pid,
                                    std::vector<T *> *list);

}  // namespace toggl

#endif  // SRC_USER_H_
