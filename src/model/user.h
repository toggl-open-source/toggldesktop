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
#include "model/alpha_features.h"

#include <Poco/LocalDateTime.h>
#include <Poco/Types.h>

namespace toggl {

// TODO ERRORS a lot of these should not actually be user-facing
enum UserErrors {
    ERROR_OFFLINE_LOGIN_MISSING_EMAIL = ErrorBase::FIRST_AVAILABLE_ENUM,
    ERROR_OFFLINE_LOGIN_MISSING_PASSWORD,
    ERROR_OFFLINE_LOGIN_MISSING_TOKEN,
    ERROR_OFFLINE_LOGIN_FAILED,
    ERROR_OFFLINE_DECRYPT_MISSING_EMAIL,
    ERROR_OFFLINE_DECRYPT_MISSING_PASSWORD,
    ERROR_OFFLINE_DECRYPT_EMPTY_STRING,

    ERROR_USER_LOST_ACCESS_TO_WORKSPACE
};
inline static const std::map<int, std::string> UserErrorMessages {
    // why the hell are these two different?
    { ERROR_OFFLINE_LOGIN_MISSING_EMAIL, "cannot enable offline login without an e-mail" },
    { ERROR_OFFLINE_LOGIN_MISSING_PASSWORD, "cannot enable offline login without a password" },
    { ERROR_OFFLINE_LOGIN_MISSING_TOKEN, "cannot enable offline login without an API token" },
    { ERROR_OFFLINE_LOGIN_FAILED, "offline login encryption failed" },
    { ERROR_OFFLINE_DECRYPT_MISSING_EMAIL, "cannot decrypt offline data without an e-mail" },
    { ERROR_OFFLINE_DECRYPT_MISSING_PASSWORD, "cannot decrypt offline data without a password" },
    { ERROR_OFFLINE_DECRYPT_EMPTY_STRING, "cannot decrypt empty string" },
    { ERROR_USER_LOST_ACCESS_TO_WORKSPACE, "You no longer have access to your last workspace" }
};

class UserError : public EnumBasedError<UserErrors, UserErrorMessages> {
public:
    using Parent = EnumBasedError<UserErrors, UserErrorMessages>;
    using Parent::Parent;
    UserError &operator=(const UserError &o) = default;
    std::string Class() const override { return "UserError"; }
};


class TOGGL_INTERNAL_EXPORT User : public BaseModel {
 public:
    User() : BaseModel(),
        AlphaFeatureSettings(nullptr) {}
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
    Property<Poco::UInt8> BeginningOfWeek { 1 };
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
    void SetBeginningOfWeek(Poco::UInt8 value);
    void SetRecordTimeline(bool value);
    void ConfirmLoadedMore();
    void SetCollapseEntries(bool value);

    // Derived data and modifiers
    bool HasValidSinceDate() const;

    Error EnableOfflineLogin(
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

    Error LoadUserUpdateFromJSONString(const std::string &json);

    Error LoadUserAndRelatedDataFromJSONString(const std::string &json,
        bool including_related_data, bool syncServer);

    void LoadUserAndRelatedDataFromJSON(const Json::Value &root,
        bool including_related_data,
        bool syncServer);

    Error LoadWorkspacesFromJSONString(const std::string & json);

    Error LoadTimeEntriesFromJSONString(const std::string &json);

    Error SetAPITokenFromOfflineData(const std::string &password);

    void MarkTimelineBatchAsUploaded(
        const std::vector<const TimelineEvent*> &events);
    void CompressTimeline();

    std::vector<const TimelineEvent *> CompressedTimelineForUI(const Poco::LocalDateTime *date) const;
    std::vector<const TimelineEvent *> CompressedTimelineForUpload(const Poco::LocalDateTime *date = nullptr) const;

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

    static Error UserID(
        const std::string &json_data_string,
        Poco::UInt64 *result);

    static Error LoginToken(
        const std::string &json_data_string,
        std::string *result);

    static Error GenerateOfflineLogin(
        const std::string &email,
        const std::string &password,
        std::string *result);

    AlphaFeatures* AlphaFeatureSettings;
    void LoadAlphaFeaturesFromJSON(const Json::Value& data);

 private:
    void loadUserTagFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = nullptr);

    Error loadUserFromJSON(
        const Json::Value &node);

    Error loadRelatedDataFromJSON(const Json::Value &node,
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
