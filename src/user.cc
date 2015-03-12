// Copyright 2014 Toggl Desktop developers.

#include "../src/user.h"

#include <time.h>

#include <sstream>

#include "./client.h"
#include "./const.h"
#include "./formatter.h"
#include "./https_client.h"
#include "./project.h"
#include "./tag.h"
#include "./task.h"
#include "./time_entry.h"
#include "./workspace.h"

#include "Poco/Base64Decoder.h"
#include "Poco/Base64Encoder.h"
#include "Poco/Crypto/Cipher.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/CipherKey.h"
#include "Poco/Crypto/CryptoStream.h"
#include "Poco/DigestStream.h"
#include "Poco/Logger.h"
#include "Poco/Random.h"
#include "Poco/RandomStream.h"
#include "Poco/SHA1Engine.h"
#include "Poco/Stopwatch.h"
#include "Poco/UTF8String.h"

namespace toggl {

User::~User() {
    clearList(&related.Workspaces);
    clearList(&related.Clients);
    clearList(&related.Projects);
    clearList(&related.Tasks);
    clearList(&related.Tags);
    clearList(&related.TimeEntries);
}

Project *User::CreateProject(
    const Poco::UInt64 workspace_id,
    const Poco::UInt64 client_id,
    const std::string project_name,
    const bool is_private) {
    Project *p = new Project();
    p->SetWID(workspace_id);
    p->SetName(project_name);
    p->SetCID(client_id);
    p->SetUID(ID());
    p->SetActive(true);
    p->SetPrivate(is_private);
    related.Projects.push_back(p);
    return p;
}

Client *User::CreateClient(
    const Poco::UInt64 workspace_id,
    const std::string client_name) {
    Client *c = new Client();
    c->SetWID(workspace_id);
    c->SetName(client_name);
    related.Clients.push_back(c);
    return c;
}

// Start a time entry, mark it as dirty and add to user time entry collection.
// Do not save here, dirtyness will be handled outside of this module.
TimeEntry *User::Start(
    const std::string description,
    const std::string duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id) {
    Stop();

    time_t now = time(0);

    std::stringstream ss;
    ss << "User::Start now=" << now;

    TimeEntry *te = new TimeEntry();
    te->SetCreatedWith(HTTPSClient::Config.UserAgent());
    te->SetDescription(description);
    te->SetUID(ID());
    te->SetPID(project_id);
    te->SetTID(task_id);

    if (!duration.empty()) {
        int seconds = Formatter::ParseDurationString(duration);
        te->SetDurationInSeconds(seconds);
        te->SetStop(now);
        te->SetStart(te->Stop() - te->DurationInSeconds());
    } else {
        te->SetDurationInSeconds(-now);
        // dont set Stop, TE is running
        te->SetStart(now);
    }

    // Try to set workspace ID from project
    if (te->PID()) {
        Project *p = related.ProjectByID(te->PID());
        if (p) {
            te->SetWID(p->WID());
            te->SetBillable(p->Billable());
        }
    }

    // Try to set workspace ID from task
    if (!te->WID() && te->TID()) {
        Task *t = related.TaskByID(te->TID());
        if (t) {
            te->SetWID(t->WID());
        }
    }

    ensureWID(te);

    te->SetDurOnly(!StoreStartAndStopTime());
    te->SetUIModified();

    related.TimeEntries.push_back(te);

    return te;
}

template<typename T>
void User::ensureWID(T *model) const {
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

toggl::error User::Continue(
    const std::string GUID) {

    TimeEntry *existing = related.TimeEntryByGUID(GUID);
    if (!existing) {
        logger().warning("Time entry not found: " + GUID);
        return noError;
    }

    if (existing->DeletedAt()) {
        return error(kCannotContinueDeletedTimeEntry);
    }

    Stop();

    if (existing->DurOnly() && existing->IsToday()) {
        existing->SetDurationInSeconds(
            -time(0) + existing->DurationInSeconds());
        existing->SetUIModified();
        return toggl::noError;
    }

    TimeEntry *result = new TimeEntry();
    result->SetCreatedWith(HTTPSClient::Config.UserAgent());
    result->SetDescription(existing->Description());
    result->SetDurOnly(existing->DurOnly());
    result->SetWID(existing->WID());
    result->SetPID(existing->PID());
    result->SetTID(existing->TID());
    result->SetBillable(existing->Billable());
    result->SetTags(existing->Tags());
    result->SetUID(ID());
    result->SetStart(time(0));
    result->SetDurationInSeconds(-time(0));
    result->SetCreatedWith(HTTPSClient::Config.UserAgent());

    related.TimeEntries.push_back(result);

    return toggl::noError;
}

std::string User::DateDuration(TimeEntry * const te) const {
    Poco::Int64 date_duration(0);
    std::string date_header = te->DateHeaderString();
    for (std::vector<TimeEntry *>::const_iterator it =
        related.TimeEntries.begin();
            it != related.TimeEntries.end();
            it++) {
        TimeEntry *n = *it;
        if (n->DateHeaderString() == date_header) {
            Poco::Int64 duration = n->DurationInSeconds();
            if (duration > 0) {
                date_duration += duration;
            }
        }
    }
    return Formatter::FormatDurationForDateHeader(date_duration);
}

bool User::HasPremiumWorkspaces() const {
    for (std::vector<Workspace *>::const_iterator it =
        related.Workspaces.begin();
            it != related.Workspaces.end();
            it++) {
        Workspace *model = *it;
        if (model->Premium()) {
            return true;
        }
    }
    return false;
}

bool User::CanAddProjects() const {
    for (std::vector<Workspace *>::const_iterator it =
        related.Workspaces.begin();
            it != related.Workspaces.end();
            it++) {
        Workspace *model = *it;
        if (model->OnlyAdminsMayCreateProjects()) {
            return false;
        }
    }
    return true;
}

void User::SetFullname(const std::string value) {
    if (fullname_ != value) {
        fullname_ = value;
        SetDirty();
    }
}

void User::SetTimeOfDayFormat(const std::string value) {
    Formatter::TimeOfDayFormat = value;
    if (timeofday_format_ != value) {
        timeofday_format_ = value;
        SetDirty();
    }
}

void User::SetDurationFormat(const std::string value) {
    Formatter::DurationFormat = value;
    if (duration_format_ != value) {
        duration_format_ = value;
        SetDirty();
    }
}

void User::SetOfflineData(const std::string value) {
    if (offline_data_ != value) {
        offline_data_ = value;
        SetDirty();
    }
}

void User::SetStoreStartAndStopTime(const bool value) {
    if (store_start_and_stop_time_ != value) {
        store_start_and_stop_time_ = value;
        SetDirty();
    }
}

void User::SetRecordTimeline(const bool value) {
    if (record_timeline_ != value) {
        record_timeline_ = value;
        SetDirty();
    }
}

void User::SetEmail(const std::string value) {
    if (email_ != value) {
        email_ = value;
        SetDirty();
    }
}

void User::SetAPIToken(const std::string value) {
    // API token is not saved into DB, so no
    // no dirty checking needed for it.
    api_token_ = value;
}

void User::SetSince(const Poco::UInt64 value) {
    if (since_ != value) {
        since_ = value;
        SetDirty();
    }
}

void User::SetDefaultWID(const Poco::UInt64 value) {
    if (default_wid_ != value) {
        default_wid_ = value;
        SetDirty();
    }
}

// Stop a time entry, mark it as dirty.
// Note that there may be multiple TE-s running. If there are,
// all of them are stopped (multi-tracking is not supported by Toggl).
std::vector<TimeEntry *> User::Stop() {
    std::vector<TimeEntry *> result;
    TimeEntry *te = RunningTimeEntry();
    while (te) {
        result.push_back(te);
        te->StopTracking();
        te = RunningTimeEntry();
    }
    return result;
}

TimeEntry *User::DiscardTimeAt(
    const std::string guid,
    const Poco::Int64 at,
    const bool split_into_new_entry) {

    if (!(at > 0)) {
        logger().error("Cannot discard without valid timestamp");
        return 0;
    }

    std::stringstream ss;
    ss << "User is discarding time entry " << guid << " at " << at;
    logger().debug(ss.str());

    TimeEntry *te = related.TimeEntryByGUID(guid);
    if (te) {
        te->DiscardAt(at);
    }

    if (te && split_into_new_entry) {
        TimeEntry *split = new TimeEntry();
        split->SetCreatedWith(HTTPSClient::Config.UserAgent());
        split->SetDurOnly(te->DurOnly());
        split->SetUID(ID());
        split->SetStart(at);
        split->SetDurationInSeconds(-at);
        split->SetUIModified();
        related.TimeEntries.push_back(split);
        return split;
    }

    return 0;
}

TimeEntry *User::RunningTimeEntry() const {
    for (std::vector<TimeEntry *>::const_iterator it =
        related.TimeEntries.begin();
            it != related.TimeEntries.end();
            it++) {
        if ((*it)->DurationInSeconds() < 0) {
            return *it;
        }
    }
    return 0;
}

template<typename T>
void User::CollectPushableModels(
    const std::vector<T *> list,
    std::vector<T *> *result,
    std::map<std::string, BaseModel *> *models) const {

    poco_check_ptr(result);

    for (typename std::vector<T *>::const_iterator it =
        list.begin();
            it != list.end();
            it++) {
        T *model = *it;
        if (!model->NeedsPush()) {
            continue;
        }
        ensureWID(model);
        model->EnsureGUID();
        result->push_back(model);
        if (models && !model->GUID().empty()) {
            (*models)[model->GUID()] = model;
        }
    }
}

error User::PullAllUserData(
    TogglClient *https_client) {

    if (APIToken().empty()) {
        return error("cannot pull user data without API token");
    }

    try {
        Poco::Stopwatch stopwatch;
        stopwatch.start();

        std::string user_data_json("");
        error err = Me(
            https_client,
            APIToken(),
            "api_token",
            &user_data_json,
            Since());
        if (err != noError) {
            return err;
        }

        LoadUserAndRelatedDataFromJSONString(user_data_json, !Since());

        stopwatch.stop();
        std::stringstream ss;
        ss << "User with related data JSON fetched and parsed in "
           << stopwatch.elapsed() / 1000 << " ms";
        logger().debug(ss.str());
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error User::PushChanges(
    TogglClient *https_client,
    bool *had_something_to_push) {

    if (APIToken().empty()) {
        return error("cannot push changes without API token");
    }

    poco_check_ptr(had_something_to_push);

    *had_something_to_push = true;
    try {
        Poco::Stopwatch stopwatch;
        stopwatch.start();

        std::map<std::string, BaseModel *> models;

        std::vector<TimeEntry *> time_entries;
        std::vector<Project *> projects;
        std::vector<Client *> clients;

        CollectPushableModels(related.TimeEntries, &time_entries, &models);
        CollectPushableModels(related.Projects, &projects, &models);
        CollectPushableModels(related.Clients, &clients, &models);

        if (time_entries.empty() && projects.empty() && clients.empty()) {
            *had_something_to_push = false;
            return noError;
        }

        std::string json("");
        error err = updateJSON(&clients, &projects, &time_entries, &json);
        if (err != noError) {
            return err;
        }

        logger().debug(json);

        std::string response_body("");
        err = https_client->Post(kAPIURL,
                                 "/api/v8/batch_updates",
                                 json,
                                 APIToken(),
                                 "api_token",
                                 &response_body);
        if (err != noError) {
            return err;
        }

        std::vector<BatchUpdateResult> results;
        err = BatchUpdateResult::ParseResponseArray(response_body, &results);
        if (err != noError) {
            return err;
        }

        std::vector<error> errors;

        BatchUpdateResult::ProcessResponseArray(&results, &models, &errors);

        if (!errors.empty()) {
            return Formatter::CollectErrors(&errors);
        }

        stopwatch.stop();
        std::stringstream ss;
        ss << "Changes data JSON pushed and responses parsed in "
           << stopwatch.elapsed() / 1000 << " ms";
        logger().debug(ss.str());
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

std::string User::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " default_wid=" << default_wid_
        << " api_token=" << api_token_
        << " since=" << since_
        << " record_timeline=" << record_timeline_;
    return ss.str();
}

error User::Me(
    TogglClient *https_client,
    const std::string email,
    const std::string password,
    std::string *user_data_json,
    const Poco::UInt64 since) {

    if (email.empty()) {
        return "Empty email or API token";
    }

    if (password.empty()) {
        return "Empty password";
    }

    try {
        poco_check_ptr(user_data_json);
        poco_check_ptr(https_client);

        std::stringstream relative_url;
        relative_url << "/api/v8/me"
                     << "?app_name=" << TogglClient::Config.AppName
                     << "&with_related_data=true"
                     << "&since=" << since;

        return https_client->Get(kAPIURL,
                                 relative_url.str(),
                                 email,
                                 password,
                                 user_data_json);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
}

error User::Signup(
    TogglClient *https_client,
    const std::string email,
    const std::string password,
    std::string *user_data_json) {

    if (email.empty()) {
        return "Empty email";
    }

    if (password.empty()) {
        return "Empty password";
    }

    try {
        poco_check_ptr(user_data_json);
        poco_check_ptr(https_client);

        Json::Value user;
        user["email"] = email;
        user["password"] = password;

        Json::Value root;
        root["user"] = user;

        return https_client->Post(kAPIURL,
                                  "/api/v8/signups",
                                  Json::StyledWriter().write(root),
                                  "",
                                  "",
                                  user_data_json);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
}

void User::DeleteRelatedModelsWithWorkspace(const Poco::UInt64 wid) {
    deleteRelatedModelsWithWorkspace(wid, &related.Clients);
    deleteRelatedModelsWithWorkspace(wid, &related.Projects);
    deleteRelatedModelsWithWorkspace(wid, &related.Tasks);
    deleteRelatedModelsWithWorkspace(wid, &related.TimeEntries);
    deleteRelatedModelsWithWorkspace(wid, &related.Tags);
}

void User::RemoveClientFromRelatedModels(const Poco::UInt64 cid) {
    for (std::vector<Project *>::iterator it = related.Projects.begin();
            it != related.Projects.end(); it++) {
        Project *model = *it;
        if (model->CID() == cid) {
            model->SetCID(0);
        }
    }
}

void User::RemoveProjectFromRelatedModels(const Poco::UInt64 pid) {
    removeProjectFromRelatedModels(pid, &related.Tasks);
    removeProjectFromRelatedModels(pid, &related.TimeEntries);
}

void User::RemoveTaskFromRelatedModels(const Poco::UInt64 tid) {
    for (std::vector<TimeEntry *>::iterator it = related.TimeEntries.begin();
            it != related.TimeEntries.end(); it++) {
        TimeEntry *model = *it;
        if (model->TID() == tid) {
            model->SetTID(0);
        }
    }
}

void User::loadUserTagFromJSON(
    Json::Value data,
    std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }

    Tag *model = related.TagByID(id);

    if (!model) {
        model = related.TagByGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = new Tag();
        related.Tags.push_back(model);
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
}

void User::loadUserTaskFromJSON(
    Json::Value data,
    std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }

    Task *model = related.TaskByID(id);

    // Tasks have no GUID

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = new Task();
        related.Tasks.push_back(model);
    }

    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
}

error User::LoadUserUpdateFromJSONString(
    const std::string json) {

    if (json.empty()) {
        return noError;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json, root)) {
        return error("Failed to LoadUserUpdateFromJSONString");
    }

    loadUserUpdateFromJSON(root);

    return noError;
}

void User::loadUserUpdateFromJSON(
    Json::Value node) {

    Json::Value data = node["data"];
    std::string model = node["model"].asString();
    std::string action = node["action"].asString();

    Poco::UTF8::toLowerInPlace(action);

    std::stringstream ss;
    ss << "Update parsed into action=" << action
       << ", model=" + model;
    Poco::Logger &logger = Poco::Logger::get("json");
    logger.debug(ss.str());

    if ("workspace" == model) {
        loadUserWorkspaceFromJSON(data);
    } else if ("client" == model) {
        loadUserClientFromJSON(data);
    } else if ("project" == model) {
        loadUserProjectFromJSON(data);
    } else if ("task" == model) {
        loadUserTaskFromJSON(data);
    } else if ("time_entry" == model) {
        loadUserTimeEntryFromJSON(data);
    } else if ("tag" == model) {
        loadUserTagFromJSON(data);
    } else if ("user" == model) {
        loadUserAndRelatedDataFromJSON(data, false);
    }
}

void User::loadUserWorkspaceFromJSON(
    Json::Value data,
    std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }
    Workspace *model = related.WorkspaceByID(id);

    // Workspaces have no GUID

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = new Workspace();
        related.Workspaces.push_back(model);
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
}

error User::LoadUserAndRelatedDataFromJSONString(
    const std::string &json,
    const bool &including_related_data) {

    if (json.empty()) {
        Poco::Logger &logger = Poco::Logger::get("json");
        logger.warning("cannot load empty JSON");
        return noError;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json, root)) {
        return error("Failed to LoadUserAndRelatedDataFromJSONString");
    }

    SetSince(root["since"].asUInt64());

    Poco::Logger &logger = Poco::Logger::get("json");
    std::stringstream s;
    s << "User data as of: " << Since();
    logger.debug(s.str());

    loadUserAndRelatedDataFromJSON(root["data"], including_related_data);

    return noError;
}

void User::loadUserAndRelatedDataFromJSON(
    Json::Value data,
    const bool &including_related_data) {

    if (!data["id"].asUInt64()) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }

    SetID(data["id"].asUInt64());
    SetDefaultWID(data["default_wid"].asUInt64());
    SetAPIToken(data["api_token"].asString());
    SetEmail(data["email"].asString());
    SetFullname(data["fullname"].asString());
    SetRecordTimeline(data["record_timeline"].asBool());
    SetStoreStartAndStopTime(data["store_start_and_stop_time"].asBool());
    SetTimeOfDayFormat(data["timeofday_format"].asString());
    SetDurationFormat(data["duration_format"].asString());

    {
        std::set<Poco::UInt64> alive;

        if (data.isMember("projects")) {
            Json::Value list = data["projects"];

            for (unsigned int i = 0; i < list.size(); i++) {
                loadUserProjectFromJSON(list[i], &alive);
            }
        }

        if (including_related_data) {
            deleteZombies(related.Projects, alive);
        }
    }

    {
        std::set<Poco::UInt64> alive;

        if (data.isMember("tags")) {
            Json::Value list = data["tags"];

            for (unsigned int i = 0; i < list.size(); i++) {
                loadUserTagFromJSON(list[i], &alive);
            }
        }

        if (including_related_data) {
            deleteZombies(related.Tags, alive);
        }
    }

    {
        std::set<Poco::UInt64> alive;

        if (data.isMember("tasks")) {
            Json::Value list = data["tasks"];

            for (unsigned int i = 0; i < list.size(); i++) {
                loadUserTaskFromJSON(list[i], &alive);
            }
        }

        if (including_related_data) {
            deleteZombies(related.Tasks, alive);
        }
    }

    {
        std::set<Poco::UInt64> alive;

        if (data.isMember("time_entries")) {
            Json::Value list = data["time_entries"];

            for (unsigned int i = 0; i < list.size(); i++) {
                loadUserTimeEntryFromJSON(list[i], &alive);
            }
        }

        if (including_related_data) {
            deleteZombies(related.TimeEntries, alive);
        }
    }

    {
        std::set<Poco::UInt64> alive;

        if (data.isMember("workspaces")) {
            Json::Value list = data["workspaces"];

            for (unsigned int i = 0; i < list.size(); i++) {
                loadUserWorkspaceFromJSON(list[i], &alive);
            }
        }

        if (including_related_data) {
            deleteZombies(related.Workspaces, alive);
        }
    }

    {
        std::set<Poco::UInt64> alive;

        if (data.isMember("clients")) {
            Json::Value list = data["clients"];

            for (unsigned int i = 0; i < list.size(); i++) {
                loadUserClientFromJSON(list[i], &alive);
            }
        }

        if (including_related_data) {
            deleteZombies(related.Clients, alive);
        }
    }
}

void User::loadUserClientFromJSON(
    Json::Value data,
    std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }
    Client *model = related.ClientByID(id);

    if (!model) {
        model = related.ClientByGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = new Client();
        related.Clients.push_back(model);
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
}

void User::loadUserProjectFromJSON(
    Json::Value data,
    std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }

    Project *model = related.ProjectByID(id);

    if (!model) {
        model = related.ProjectByGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = new Project();
        related.Projects.push_back(model);
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
}

void User::loadUserTimeEntryFromJSON(
    Json::Value data,
    std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }

    TimeEntry *model = related.TimeEntryByID(id);

    if (!model) {
        model = related.TimeEntryByGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = new TimeEntry();
        related.TimeEntries.push_back(model);
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
    model->EnsureGUID();
}

error User::UserID(
    const std::string json_data_string,
    Poco::UInt64 *result) {
    *result = 0;
    Json::Value root;
    Json::Reader reader;
    bool ok = reader.parse(json_data_string, root);
    if (!ok) {
        return error("error parsing UserID JSON");
    }
    *result = root["data"]["id"].asUInt64();
    return noError;
}

error User::LoginToken(
    const std::string json_data_string,
    std::string *result) {
    *result = "";
    Json::Value root;
    Json::Reader reader;
    bool ok = reader.parse(json_data_string, root);
    if (!ok) {
        return error("error parsing UserID JSON");
    }
    *result = root["login_token"].asString();
    return noError;
}

error User::updateJSON(
    std::vector<Client *> * const clients,
    std::vector<Project *> * const projects,
    std::vector<TimeEntry *> * const time_entries,
    std::string *result) const {

    poco_check_ptr(clients);
    poco_check_ptr(projects);
    poco_check_ptr(time_entries);

    Json::Value c;

    // First, clients, because projects depend on clients
    for (std::vector<Client *>::const_iterator it =
        clients->begin();
            it != clients->end(); it++) {
        Json::Value update;
        error err = (*it)->BatchUpdateJSON(&update);
        if (err != noError) {
            return err;
        }
        c.append(update);
    }

    // First, projects, because time entries depend on projects
    for (std::vector<Project *>::const_iterator it =
        projects->begin();
            it != projects->end(); it++) {
        Json::Value update;
        error err = (*it)->BatchUpdateJSON(&update);
        if (err != noError) {
            return err;
        }
        c.append(update);
    }

    // Time entries go last
    for (std::vector<TimeEntry *>::const_iterator it =
        time_entries->begin();
            it != time_entries->end(); it++) {
        Json::Value update;
        error err = (*it)->BatchUpdateJSON(&update);
        if (err != noError) {
            return err;
        }
        c.append(update);
    }

    Json::StyledWriter writer;
    *result = writer.write(c);

    return noError;
}

std::string User::generateKey(const std::string password) {
    Poco::SHA1Engine sha1;
    Poco::DigestOutputStream outstr(sha1);
    outstr << Email();
    outstr << password;
    outstr.flush();
    const Poco::DigestEngine::Digest &digest = sha1.digest();
    return Poco::DigestEngine::digestToHex(digest);
}

error User::SetAPITokenFromOfflineData(const std::string password) {
    if (Email().empty()) {
        return error("cannot decrypt offline data without an e-mail");
    }
    if (password.empty()) {
        return error("cannot decrypt offline data without a password");
    }
    if (OfflineData().empty()) {
        return error("cannot decrypt empty string");
    }
    try {
        Poco::Crypto::CipherFactory& factory =
            Poco::Crypto::CipherFactory::defaultFactory();

        std::string key = generateKey(password);

        Json::Value data;
        Json::Reader reader;
        if (!reader.parse(OfflineData(), data)) {
            return error("failed to parse offline data");
        }

        std::istringstream istr(data["salt"].asString());
        Poco::Base64Decoder decoder(istr);
        std::string salt("");
        decoder >> salt;

        Poco::Crypto::CipherKey ckey("aes-256-cbc", key, salt);
        Poco::Crypto::Cipher* pCipher = factory.createCipher(ckey);

        std::string decrypted = pCipher->decryptString(
            data["encrypted"].asString(),
            Poco::Crypto::Cipher::ENC_BASE64);

        delete pCipher;
        pCipher = 0;

        SetAPIToken(decrypted);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error User::EnableOfflineLogin(
    const std::string password) {
    if (Email().empty()) {
        return error("cannot enable offline login without an e-mail");
    }
    if (password.empty()) {
        return error("cannot enable offline login without a password");
    }
    if (APIToken().empty()) {
        return error("cannot enable offline login without an API token");
    }
    try {
        Poco::Crypto::CipherFactory& factory =
            Poco::Crypto::CipherFactory::defaultFactory();

        std::string key = generateKey(password);

        std::string salt("");
        Poco::RandomInputStream ri;
        ri >> salt;

        Poco::Crypto::CipherKey ckey("aes-256-cbc", key, salt);

        Poco::Crypto::Cipher* pCipher = factory.createCipher(ckey);

        std::ostringstream str;
        Poco::Base64Encoder enc(str);
        enc << salt;
        enc.close();

        Json::Value data;
        data["salt"] = str.str();
        data["encrypted"] = pCipher->encryptString(
            APIToken(),
            Poco::Crypto::Cipher::ENC_BASE64);
        std::string json = Json::FastWriter().write(data);

        delete pCipher;
        pCipher = 0;

        SetOfflineData(json);

        std::string token = APIToken();
        error err = SetAPITokenFromOfflineData(password);
        if (err != noError) {
            return err;
        }
        if (token != APIToken()) {
            return error("offline login encryption failed");
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

template<class T>
void deleteZombies(
    const std::vector<T> &list,
    const std::set<Poco::UInt64> &alive) {
    for (size_t i = 0; i < list.size(); ++i) {
        BaseModel *model = list[i];
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

template<typename T>
void clearList(std::vector<T *> *list) {
    for (size_t i = 0; i < list->size(); i++) {
        T *value = (*list)[i];
        delete value;
    }
    list->clear();
}

template <typename T>
void deleteRelatedModelsWithWorkspace(const Poco::UInt64 wid,
                                      std::vector<T *> *list) {
    typedef typename std::vector<T *>::iterator iterator;
    for (iterator it = list->begin(); it != list->end(); it++) {
        T *model = *it;
        if (model->WID() == wid) {
            model->MarkAsDeletedOnServer();
        }
    }
}

template <typename T>
void removeProjectFromRelatedModels(const Poco::UInt64 pid,
                                    std::vector<T *> *list) {
    typedef typename std::vector<T *>::iterator iterator;
    for (iterator it = list->begin(); it != list->end(); it++) {
        T *model = *it;
        if (model->PID() == pid) {
            model->SetPID(0);
        }
    }
}

}  // namespace toggl
