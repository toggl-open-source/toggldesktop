// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_USER_H_
#define SRC_USER_H_

#include <string>
#include <vector>
#include <set>
#include <map>

#include <json/json.h>  // NOLINT

#include "./types.h"
#include "./https_client.h"
#include "./workspace.h"
#include "./client.h"
#include "./project.h"
#include "./task.h"
#include "./time_entry.h"
#include "./tag.h"
#include "./related_data.h"
#include "./batch_update_result.h"
#include "./base_model.h"

#include "Poco/Types.h"

namespace toggl {

class User : public BaseModel {
 public:
    User() :
    api_token_(""),
    default_wid_(0),
    since_(0),
    fullname_(""),
    email_(""),
    record_timeline_(false),
    timeofday_format_("") {}

    ~User();

    error PullAllUserData(HTTPSClient *https_client);
    error PullChanges(HTTPSClient *https_client);
    error PushChanges(HTTPSClient *https_client);

    static error Me(
        HTTPSClient *https_client,
        const std::string email,
        const std::string password,
        std::string *user_data);

    std::string String() const;

    bool HasPremiumWorkspaces() const;
    bool CanAddProjects() const;

    void SetLastTEDate(const std::string value);

    void CollectPushableTimeEntries(
        std::vector<TimeEntry *> *result,
        std::map<std::string, BaseModel *> *models = 0) const;

    void CollectPushableProjects(
        std::vector<Project *> *result,
        std::map<std::string, BaseModel *> *models = 0) const;

    TimeEntry *RunningTimeEntry() const;
    bool IsTracking() const {
        return RunningTimeEntry() != 0;
    }

    bool HasTrackedTimeToday() const;

    void Start(
        const std::string description,
        const std::string duration,
        const Poco::UInt64 task_id,
        const Poco::UInt64 project_id);

    toggl::error Continue(
        const std::string GUID);

    std::vector<TimeEntry *> Stop();

    TimeEntry *DiscardTimeAt(
        const std::string guid,
        const Poco::Int64 at);

    Project *AddProject(
        const Poco::UInt64 workspace_id,
        const Poco::UInt64 client_id,
        const std::string project_name,
        const bool is_private);

    std::string DateDuration(TimeEntry *te) const;

    std::string APIToken() const {
        return api_token_;
    }
    void SetAPIToken(std::string api_token);

    Poco::UInt64 DefaultWID() const {
        return default_wid_;
    }
    void SetDefaultWID(Poco::UInt64 value);

    // Unix timestamp of the user data; returned from API
    Poco::UInt64 Since() const {
        return since_;
    }
    void SetSince(const Poco::UInt64 value);

    std::string Fullname() const {
        return fullname_;
    }
    void SetFullname(std::string value);

    std::string TimeOfDayFormat() const {
        return timeofday_format_;
    }
    void SetTimeOfDayFormat(std::string value);

    std::string Email() const {
        return email_;
    }
    void SetEmail(const std::string value);

    bool RecordTimeline() const {
        return record_timeline_;
    }
    void SetRecordTimeline(const bool value);

    void ActiveProjects(std::vector<Project *> *list) const;

    bool StoreStartAndStopTime() const {
        return store_start_and_stop_time_;
    }
    void SetStoreStartAndStopTime(const bool value);

    RelatedData related;

    std::string ModelName() const {
        return "user";
    }
    std::string ModelURL() const {
        return "/api/v8/me";
    }

    // Handle related model deletions
    void DeleteRelatedModelsWithWorkspace(const Poco::UInt64 wid);
    void RemoveClientFromRelatedModels(const Poco::UInt64 cid);
    void RemoveProjectFromRelatedModels(const Poco::UInt64 pid);
    void RemoveTaskFromRelatedModels(const Poco::UInt64 tid);

    error LoadUserUpdateFromJSONString(const std::string json);

    error LoadUserAndRelatedDataFromJSONString(
        const std::string &json);

    static error UserID(
        const std::string json_data_string,
        Poco::UInt64 *result);

    static error LoginToken(
        const std::string json_data_string,
        std::string *result);

 private:
    std::string updateJSON(
        std::vector<Project *> * const,
        std::vector<TimeEntry *> * const);

    void loadUserTagFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = 0);

    void loadUserAndRelatedDataFromJSON(Json::Value node);

    void loadUserProjectsFromJSON(
        Json::Value list);

    void loadUserTagsFromJSON(
        Json::Value list);

    void loadUserClientsFromJSON(
        Json::Value list);

    void loadUserTasksFromJSON(
        Json::Value list);

    void loadUserTimeEntriesFromJSON(
        Json::Value list);

    void loadUserWorkspacesFromJSON(
        Json::Value list);

    void loadUserUpdateFromJSON(
        Json::Value list);

    void loadUserProjectFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = 0);

    void loadUserWorkspaceFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = 0);

    void loadUserClientFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = 0);

    void loadUserTaskFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = 0);

    void loadUserTimeEntryFromJSON(
        Json::Value data,
        std::set<Poco::UInt64> *alive = 0);

    std::string dirtyObjectsJSON(std::vector<TimeEntry *> * const) const;

    void processResponseArray(
        std::vector<BatchUpdateResult> * const results,
        std::vector<TimeEntry *> *dirty,
        std::vector<error> *errors);

    error collectErrors(std::vector<error> *errors) const;

    error requestJSON(
        const std::string method,
        const std::string relative_url,
        const std::string json,
        const bool authenticate_with_api_token,
        std::string *response_body);
    void parseResponseArray(
        const std::string response_body,
        std::vector<BatchUpdateResult> *responses);

    void ensureWID(TimeEntry *te) const;

    std::string api_token_;
    Poco::UInt64 default_wid_;
    // Unix timestamp of the user data; returned from API
    Poco::UInt64 since_;
    std::string fullname_;
    std::string email_;
    bool record_timeline_;
    bool store_start_and_stop_time_;
    std::string timeofday_format_;
};

}  // namespace toggl

#endif  // SRC_USER_H_
