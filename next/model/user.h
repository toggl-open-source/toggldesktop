// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_USER_H_
#define SRC_USER_H_

#include "base_model.h"
#include "batch_update_result.h"
#include "types.h"

#include <map>
#include <set>
#include <string>
#include <vector>

#include <json/json.h>  // NOLINT

#include <Poco/Types.h>

namespace toggl {

class User : public BaseModel {
 public:
    User()
        : api_token_("")
    , default_wid_(0)
    , since_(0)
    , fullname_("")
    , email_("")
    , record_timeline_(false)
    , store_start_and_stop_time_(true)
    , timeofday_format_("")
    , duration_format_("")
    , offline_data_("")
    , default_pid_(0)
    , default_tid_(0)
    , has_loaded_more_(false)
    , collapse_entries_(false) {}

    ~User();

    error EnableOfflineLogin(
        const std::string password);

    bool HasPremiumWorkspaces() const;
    bool CanAddProjects() const;

    void SetLastTEDate(const std::string value);

    const std::string &APIToken() const {
        return api_token_;
    }
    void SetAPIToken(std::string api_token);

    const Poco::UInt64 &DefaultWID() const {
        return default_wid_;
    }
    void SetDefaultWID(Poco::UInt64 value);

    // Unix timestamp of the user data; returned from API
    const Poco::Int64 &Since() const {
        return since_;
    }
    void SetSince(const Poco::Int64 value);

    bool HasValidSinceDate() const;

    const std::string &Fullname() const {
        return fullname_;
    }
    void SetFullname(std::string value);

    const std::string &TimeOfDayFormat() const {
        return timeofday_format_;
    }
    void SetTimeOfDayFormat(std::string value);

    const std::string &Email() const {
        return email_;
    }
    void SetEmail(const std::string value);

    const bool &RecordTimeline() const {
        return record_timeline_;
    }
    void SetRecordTimeline(const bool value);

    const std::string &DurationFormat() const {
        return duration_format_;
    }
    void SetDurationFormat(const std::string);

    const bool &StoreStartAndStopTime() const {
        return store_start_and_stop_time_;
    }
    void SetStoreStartAndStopTime(const bool value);

    const std::string& OfflineData() const {
        return offline_data_;
    }
    void SetOfflineData(const std::string);

    const Poco::UInt64& DefaultPID() const {
        return default_pid_;
    }
    void SetDefaultPID(const Poco::UInt64);

    const Poco::UInt64& DefaultTID() const {
        return default_tid_;
    }
    void SetDefaultTID(const Poco::UInt64);

    const bool &CollapseEntries() const {
        return collapse_entries_;
    }
    void SetCollapseEntries(const bool value);

    // Override BaseModel
    std::string String() const;
    std::string ModelName() const;
    std::string ModelURL() const;

    // Handle related model deletions
    void DeleteRelatedModelsWithWorkspace(const Poco::UInt64 wid);
    void RemoveClientFromRelatedModels(const Poco::UInt64 cid);
    void RemoveProjectFromRelatedModels(const Poco::UInt64 pid);
    void RemoveTaskFromRelatedModels(const Poco::UInt64 tid);

    error LoadUserUpdateFromJSONString(const std::string json);

    error LoadUserAndRelatedDataFromJSONString(
        const std::string &json,
        const bool &including_related_data);

    error SetAPITokenFromOfflineData(const std::string password);


    void LoadObmExperiments(Json::Value const &obm);

    bool LoadUserPreferencesFromJSON(
        Json::Value data);

    static error UserID(
        const std::string json_data_string,
        Poco::UInt64 *result);

    static error LoginToken(
        const std::string json_data_string,
        std::string *result);

    static error GenerateOfflineLogin(
        const std::string email,
        const std::string password,
        std::string *result);

    bool HasLoadedMore() {
        return has_loaded_more_;
    }

    void ConfirmLoadedMore() {
        has_loaded_more_ = true;
    }

 private:

    error requestJSON(
        const std::string method,
        const std::string relative_url,
        const std::string json,
        const bool authenticate_with_api_token,
        std::string *response_body);

    void parseResponseArray(
        const std::string response_body,
        std::vector<BatchUpdateResult> *responses);

    std::string generateKey(const std::string password);

    std::string api_token_;
    Poco::UInt64 default_wid_;
    // Unix timestamp of the user data; returned from API
    Poco::Int64 since_;
    std::string fullname_;
    std::string email_;
    bool record_timeline_;
    bool store_start_and_stop_time_;
    std::string timeofday_format_;
    std::string duration_format_;
    std::string offline_data_;
    Poco::UInt64 default_pid_;
    Poco::UInt64 default_tid_;

    bool has_loaded_more_;
    bool collapse_entries_;
};

}  // namespace toggl

#endif  // SRC_USER_H_
