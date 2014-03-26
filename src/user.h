// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_USER_H_
#define SRC_USER_H_

#include <string>
#include <vector>
#include <set>
#include <map>

#include "libjson.h" // NOLINT

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
#include "Poco/Logger.h"

namespace kopsik {

class User : public BaseModel {
 public:
    User(
        const std::string app_name,
        const std::string app_version) :
    BasicAuthUsername(""),
    BasicAuthPassword(""),
    api_token_(""),
    default_wid_(0),
    since_(0),
    fullname_(""),
    app_name_(app_name),
    app_version_(app_version),
    email_(""),
    record_timeline_(false) {}
    ~User();

    error FullSync(HTTPSClient *https_client);
    error PartialSync(HTTPSClient *https_client);
    error Login(
        HTTPSClient *https_client,
        const std::string &email,
        const std::string &password);

    std::string String() const;

    void ClearWorkspaces();
    void ClearClients();
    void ClearProjects();
    void ClearTasks();
    void ClearTags();
    void ClearTimeEntries();

    bool HasPremiumWorkspaces() const;
    bool CanAddProjects() const;

    Workspace *GetWorkspaceByID(const Poco::UInt64 id) const;
    Client *GetClientByID(const Poco::UInt64 id) const;
    Client *GetClientByGUID(const guid GUID) const;
    Project *GetProjectByID(const Poco::UInt64 id) const;
    Project *GetProjectByGUID(const guid GUID) const;
    Project *GetProjectByName(const std::string name) const;
    Task *GetTaskByID(const Poco::UInt64 id) const;
    Tag *GetTagByID(const Poco::UInt64 id) const;
    Tag *GetTagByGUID(const guid GUID) const;
    TimeEntry *GetTimeEntryByID(const Poco::UInt64 id) const;
    TimeEntry *GetTimeEntryByGUID(const guid GUID) const;

    void CollectPushableTimeEntries(
        std::vector<TimeEntry *> *result,
        std::map<std::string, BaseModel *> *models = 0) const;
    void CollectPushableProjects(
        std::vector<Project *> *result,
        std::map<std::string, BaseModel *> *models = 0) const;

    TimeEntry *RunningTimeEntry() const;
    TimeEntry *Start(
        const std::string description,
        const std::string duration,
        const Poco::UInt64 task_id,
        const Poco::UInt64 project_id);
    kopsik::error Continue(
        const std::string GUID,
        TimeEntry **);
    TimeEntry *Latest() const;
    std::vector<TimeEntry *> Stop();

    TimeEntry *StopAt(const Poco::Int64 at);

    Project *AddProject(
        const Poco::UInt64 workspace_id,
        const Poco::UInt64 client_id,
        const std::string project_name);

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

    // Following 2 fields are not saved into database:
    // They are only used to log user in.
    std::string BasicAuthUsername;
    std::string BasicAuthPassword;

    RelatedData related;

    std::string ModelName() const {
        return "user";
    }
    std::string ModelURL() const {
        return "/api/v8/me";
    }

 private:
    error pull(
        HTTPSClient *https_client,
        const bool full_sync,
        const bool with_related_data);
    error push(
        HTTPSClient *https_client);

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
    std::string app_name_;
    std::string app_version_;
    std::string email_;
    bool record_timeline_;
    bool store_start_and_stop_time_;
};

}  // namespace kopsik

#endif  // SRC_USER_H_
