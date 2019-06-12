// Copyright 2014 Toggl Desktop developers.

#include "../src/user.h"

#include <time.h>

#include <sstream>

#include "./client.h"
#include "./const.h"
#include "./formatter.h"
#include "./https_client.h"
#include "./obm_action.h"
#include "./project.h"
#include "./tag.h"
#include "./task.h"
#include "./time_entry.h"
#include "./timeline_event.h"
#include "./urls.h"

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
#include "Poco/Timestamp.h"
#include "Poco/Timespan.h"
#include "Poco/UTF8String.h"

namespace toggl {

User::~User() {
    related.Clear();
}

Project *User::CreateProject(
    const Poco::UInt64 workspace_id,
    const Poco::UInt64 client_id,
    const std::string &client_guid,
    const std::string &client_name,
    const std::string &project_name,
    const bool is_private,
    const std::string &project_color,
    const bool billable) {

    Project *p = new Project();
    p->SetWID(workspace_id);
    p->SetName(project_name);
    p->SetCID(client_id);
    p->SetClientGUID(client_guid);
    p->SetUID(ID());
    p->SetActive(true);
    p->SetPrivate(is_private);
    p->SetBillable(billable);
    p->SetClientName(client_name);
    if (!project_color.empty()) {
        p->SetColorCode(project_color);
    }

    AddProjectToList(p);

    return p;
}

void User::AddProjectToList(Project *p) {
    bool WIDMatch = false;
    bool CIDMatch = false;

    // We should push the project to correct alphabetical position
    // (since we try to avoid sorting the large list)
    for (std::vector<Project *>::iterator it =
        related.Projects.begin();
            it != related.Projects.end(); it++) {
        Project *pr = *it;
        if (p->WID() == pr->WID()) {
            WIDMatch = true;
            if ((p->CID() == 0 && p->ClientGUID().empty()) && pr->CID() == 0) {
                // Handle adding project without client
                CIDMatch = true;
                if (Poco::UTF8::icompare(p->Name(), pr->Name()) < 0) {
                    related.Projects.insert(it, p);
                    return;
                }
            } else if (Poco::UTF8::icompare(p->ClientName(), pr->ClientName()) == 0) {
                // Handle adding project with client
                CIDMatch = true;
                if (Poco::UTF8::icompare(p->FullName(), pr->FullName()) < 0) {
                    related.Projects.insert(it,p);
                    return;
                }
            } else if (CIDMatch) {
                // in case new project is last in client list
                related.Projects.insert(it,p);
                return;
            } else if ((p->CID() != 0 || !p->ClientGUID().empty()) && pr->CID() != 0) {
                if (Poco::UTF8::icompare(p->FullName(), pr->FullName()) < 0) {
                    related.Projects.insert(it,p);
                    return;
                }
            }
        } else if (WIDMatch) {
            //In case new project is last in workspace list
            related.Projects.insert(it,p);
            return;
        }
    }

    // if projects vector is empty or project should be added to the end
    related.Projects.push_back(p);
}

Client *User::CreateClient(
    const Poco::UInt64 workspace_id,
    const std::string &client_name) {
    Client *c = new Client();
    c->SetWID(workspace_id);
    c->SetName(client_name);
    c->SetUID(ID());

    AddClientToList(c);

    return c;
}

void User::AddClientToList(Client *c) {
    bool foundMatch = false;

    // We should push the project to correct alphabetical position
    // (since we try to avoid sorting the large list)
    for (std::vector<Client *>::iterator it =
        related.Clients.begin();
            it != related.Clients.end(); it++) {
        Client *cl = *it;
        if (c->WID() == cl->WID()) {
            foundMatch = true;
            if (Poco::UTF8::icompare(c->Name(), cl->Name()) < 0) {
                related.Clients.insert(it,c);
                return;
            }
        } else if (foundMatch) {
            related.Clients.insert(it,c);
            return;
        }
    }

    // if clients vector is empty or client should be added to the end
    related.Clients.push_back(c);
}

// Start a time entry, mark it as dirty and add to user time entry collection.
// Do not save here, dirtyness will be handled outside of this module.
TimeEntry *User::Start(
    const std::string &description,
    const std::string &duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string &project_guid,
    const std::string &tags) {

    Stop();

    time_t now = time(nullptr);

    std::stringstream ss;
    ss << "User::Start now=" << now;

    TimeEntry *te = new TimeEntry();
    te->SetCreatedWith(HTTPSClient::Config.UserAgent());
    te->SetDescription(description);
    te->SetUID(ID());
    te->SetPID(project_id);
    te->SetProjectGUID(project_guid);
    te->SetTID(task_id);
    te->SetTags(tags);

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
    Project *p = nullptr;
    if (te->PID()) {
        p = related.ProjectByID(te->PID());
    } else if (!te->ProjectGUID().empty()) {
        p = related.ProjectByGUID(te->ProjectGUID());
    }
    if (p) {
        te->SetWID(p->WID());
        te->SetBillable(p->Billable());
    }

    // Try to set workspace ID from task
    if (!te->WID() && te->TID()) {
        Task *t = related.TaskByID(te->TID());
        if (t) {
            te->SetWID(t->WID());
        }
    }

    EnsureWID(te);

    te->SetUIModified();

    related.pushBackTimeEntry(te);

    return te;
}

TimeEntry *User::Continue(
    const std::string &GUID,
    const bool manual_mode) {

    TimeEntry *existing = related.TimeEntryByGUID(GUID);
    if (!existing) {
        logger().warning("Time entry not found: " + GUID);
        return nullptr;
    }

    if (existing->DeletedAt()) {
        logger().warning(kCannotContinueDeletedTimeEntry);
        return nullptr;
    }

    Stop();

    time_t now = time(nullptr);

    TimeEntry *result = new TimeEntry();
    result->SetCreatedWith(HTTPSClient::Config.UserAgent());
    result->SetDescription(existing->Description());
    result->SetWID(existing->WID());
    result->SetPID(existing->PID());
    result->SetProjectGUID(existing->ProjectGUID());
    result->SetTID(existing->TID());
    result->SetBillable(existing->Billable());
    result->SetTags(existing->Tags());
    result->SetUID(ID());
    result->SetStart(now);

    if (!manual_mode) {
        result->SetDurationInSeconds(-now);
    }

    result->SetCreatedWith(HTTPSClient::Config.UserAgent());

    related.pushBackTimeEntry(result);

    return result;
}

std::string User::DateDuration(TimeEntry * const te) const {
    Poco::Int64 date_duration(0);
    std::string date_header = Formatter::FormatDateHeader(te->Start());
    for (std::vector<TimeEntry *>::const_iterator it =
        related.TimeEntries.begin();
            it != related.TimeEntries.end();
            it++) {
        TimeEntry *n = *it;
        if (Formatter::FormatDateHeader(n->Start()) == date_header) {
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

void User::SetFullname(const std::string &value) {
    if (fullname_ != value) {
        fullname_ = value;
        SetDirty();
    }
}

void User::SetTimeOfDayFormat(const std::string &value) {
    Formatter::TimeOfDayFormat = value;
    if (timeofday_format_ != value) {
        timeofday_format_ = value;
        SetDirty();
    }
}

void User::SetDurationFormat(const std::string &value) {
    Formatter::DurationFormat = value;
    if (duration_format_ != value) {
        duration_format_ = value;
        SetDirty();
    }
}

void User::SetOfflineData(const std::string &value) {
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

void User::SetEmail(const std::string &value) {
    if (email_ != value) {
        email_ = value;
        SetDirty();
    }
}

void User::SetAPIToken(const std::string &value) {
    // API token is not saved into DB, so no
    // no dirty checking needed for it.
    api_token_ = value;
}

void User::SetSince(const Poco::Int64 value) {
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

void User::SetDefaultPID(const Poco::UInt64 value) {
    if (default_pid_ != value) {
        default_pid_ = value;
        SetDirty();
    }
}

void User::SetDefaultTID(const Poco::UInt64 value) {
    if (default_tid_ != value) {
        default_tid_ = value;
        SetDirty();
    }
}

void User::SetCollapseEntries(const bool value) {
    if (collapse_entries_ != value) {
        collapse_entries_ = value;
        SetDirty();
    }
}

// Stop a time entry, mark it as dirty.
// Note that there may be multiple TE-s running. If there are,
// all of them are stopped (multi-tracking is not supported by Toggl).
void User::Stop(std::vector<TimeEntry *> *stopped) {
    TimeEntry *te = RunningTimeEntry();
    while (te) {
        if (stopped) {
            stopped->push_back(te);
        }
        te->StopTracking();
        te = RunningTimeEntry();
    }
}

TimeEntry *User::DiscardTimeAt(
    const std::string &guid,
    const Poco::Int64 at,
    const bool split_into_new_entry) {

    if (!(at > 0)) {
        logger().error("Cannot discard without valid timestamp");
        return nullptr;
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
        split->SetUID(ID());
        split->SetStart(at);
        split->SetDurationInSeconds(-at);
        split->SetUIModified();
        split->SetWID(te->WID());
        related.pushBackTimeEntry(split);
        return split;
    }

    return nullptr;
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
    return nullptr;
}

bool User::HasValidSinceDate() const {
    // has no value
    if (!Since()) {
        return false;
    }

    // too old
    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(time(nullptr))
                         - (60 * Poco::Timespan::DAYS);
    Poco::Int64 min_allowed = ts.epochTime();
    if (Since() < min_allowed) {
        return false;
    }

    return true;
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
    related.forEachTimeEntries([&](TimeEntry *model) {
        if (model->TID() == tid) {
            model->SetTID(0);
        }
    });
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
    const std::string &json) {

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

    if (kModelWorkspace == model) {
        loadUserWorkspaceFromJSON(data);
    } else if (kModelClient == model) {
        loadUserClientFromSyncJSON(data);
    } else if (kModelProject == model) {
        loadUserProjectFromSyncJSON(data);
    } else if (kModelTask == model) {
        loadUserTaskFromJSON(data);
    } else if (kModelTimeEntry == model) {
        loadUserTimeEntryFromJSON(data);
    } else if (kModelTag == model) {
        loadUserTagFromJSON(data);
    } else if (kModelUser == model) {
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

    SetSince(root["since"].asInt64());

    Poco::Logger &logger = Poco::Logger::get("json");
    std::stringstream s;
    s << "User data as of: " << Since();
    logger.debug(s.str());

    loadUserAndRelatedDataFromJSON(root["data"], including_related_data);

    return noError;
}

error User::LoadWorkspacesFromJSONString(const std::string & json) {
    if (json.empty()) {
        return noError;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json, root)) {
        return error("Failed to LoadWorkspacessFromJSONString");
    }

    if (root.size() == 0) {
        // Handle missing workspace issue.
        // If default wid is missing there are no workspaces
        return error(kMissingWS); // NOLINT
    }

    std::set<Poco::UInt64> alive;

    for (unsigned int i = 0; i < root.size(); i++) {
        loadUserWorkspaceFromJSON(root[i]);
    }

    return noError;
}

error User::LoadTimeEntriesFromJSONString(const std::string & json) {
    if (json.empty()) {
        return noError;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json, root)) {
        return error("Failed to LoadTimeEntriesFromJSONString");
    }

    std::set<Poco::UInt64> alive;

    for (unsigned int i = 0; i < root.size(); i++) {
        loadUserTimeEntryFromJSON(root[i], &alive);
    }

    deleteZombies(related.TimeEntries, alive);

    return noError;
}

void User::LoadObmExperiments(Json::Value const &obm) {
    if (obm.isObject()) {
        loadObmExperimentFromJson(obm);
    } else if (obm.isArray()) {
        for (unsigned int i = 0; i < obm.size(); i++) {
            loadObmExperimentFromJson(obm[i]);
        }
    }
}

void User::loadObmExperimentFromJson(Json::Value const &obm) {
    Poco::UInt64 nr = obm["nr"].asUInt64();
    if (!nr) {
        return;
    }
    ObmExperiment *model = nullptr;
    for (std::vector<ObmExperiment *>::const_iterator it =
        related.ObmExperiments.begin();
            it != related.ObmExperiments.end();
            it++) {
        ObmExperiment *existing = *it;
        if (existing->Nr() == nr) {
            model = existing;
            break;
        }
    }
    if (!model) {
        model = new ObmExperiment();
        model->SetUID(ID());
        model->SetNr(nr);
        related.ObmExperiments.push_back(model);
    }
    model->SetIncluded(obm["included"].asBool());
    model->SetActions(obm["actions"].asString());
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
                loadUserClientFromSyncJSON(list[i], &alive);
            }
        }

        if (including_related_data) {
            deleteZombies(related.Clients, alive);
        }
    }

    {
        std::set<Poco::UInt64> alive;

        if (data.isMember("projects")) {
            Json::Value list = data["projects"];

            for (unsigned int i = 0; i < list.size(); i++) {
                loadUserProjectFromSyncJSON(list[i], &alive);
            }
        }

        if (including_related_data) {
            deleteZombies(related.Projects, alive);
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
}

void User::loadUserClientFromSyncJSON(
    Json::Value data,
    std::set<Poco::UInt64> *alive) {
    bool addNew = false;
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
        addNew = true;
    }
    if (alive) {
        alive->insert(id);
    }

    model->SetUID(ID());
    model->LoadFromJSON(data);

    if (addNew) {
        AddClientToList(model);
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

void User::loadUserProjectFromSyncJSON(
    Json::Value data,
    std::set<Poco::UInt64> *alive) {
    bool addNew = false;
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
        addNew = true;
    }
    if (alive) {
        alive->insert(id);
    }

    model->SetUID(ID());
    model->LoadFromJSON(data);

    Client *c = related.clientByProject(model);
    if (c) {
        model->SetClientName(c->Name());
    }

    if (addNew) {
        AddProjectToList(model);
    }
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
        related.pushBackTimeEntry(model);
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
    model->EnsureGUID();
}

bool User::LoadUserPreferencesFromJSON(
    Json::Value data) {
    if (data.isMember("CollapseTimeEntries")
            && data["CollapseTimeEntries"].asBool() != CollapseEntries()) {
        SetCollapseEntries(data["CollapseTimeEntries"].asBool());
        return true;
    }
    return false;
}


error User::UserID(
    const std::string &json_data_string,
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
    const std::string &json_data_string,
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

error User::UpdateJSON(
    std::vector<TimeEntry *> * const time_entries,
    std::string *result) const {

    poco_check_ptr(time_entries);

    Json::Value c;

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

std::string User::generateKey(const std::string &password) {
    Poco::SHA1Engine sha1;
    Poco::DigestOutputStream outstr(sha1);
    outstr << Email();
    outstr << password;
    outstr.flush();
    const Poco::DigestEngine::Digest &digest = sha1.digest();
    return Poco::DigestEngine::digestToHex(digest);
}

error User::SetAPITokenFromOfflineData(const std::string &password) {
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
        pCipher = nullptr;

        SetAPIToken(decrypted);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error User::EnableOfflineLogin(
    const std::string &password) {
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
        pCipher = nullptr;

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
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

bool User::CanSeeBillable(
    const Workspace *ws) const {
    if (!HasPremiumWorkspaces()) {
        return false;
    }
    if (ws && !ws->Premium()) {
        return false;
    }
    return true;
}

void User::MarkTimelineBatchAsUploaded(
    const std::vector<TimelineEvent> &events) {

    for (std::vector<TimelineEvent>::const_iterator i = events.begin();
            i != events.end();
            ++i) {
        TimelineEvent event = *i;
        TimelineEvent *uploaded = related.TimelineEventByGUID(event.GUID());
        if (!uploaded) {
            logger().error(
                "Could not find timeline event to mark it as uploaded: "
                + event.String());
            continue;
        }
        uploaded->SetUploaded(true);
    }
}

void User::CompressTimeline() {
    // Group events by app name into chunks
    std::map<std::string, TimelineEvent *> compressed;

    // Older events will be deleted
    Poco::Int64 minimum_time = time(nullptr) - kTimelineSecondsToKeep;

    // Find the chunk start time of current time.
    // then process only events that are older that this chunk start time.
    // Else we will have no full chunks to compress.
    Poco::Int64 chunk_up_to =
        (time(nullptr) / kTimelineChunkSeconds) * kTimelineChunkSeconds;


    time_t start = time(nullptr);

    {
        std::stringstream ss;
        ss << "CompressTimeline "
           << " user_id=" << ID()
           << " chunk_up_to=" << chunk_up_to
           << " number of events=" << related.TimelineEvents.size();

        logger().debug(ss.str());
    }

    for (std::vector<TimelineEvent *>::iterator i =
        related.TimelineEvents.begin();
            i != related.TimelineEvents.end();
            ++i) {
        TimelineEvent *event = *i;

        poco_check_ptr(event);

        // Delete too old timeline events
        if (event->Start() < minimum_time) {
            event->Delete();
        }

        // Events that do not fit into chunk yet, ignore
        if (event->Start() >= chunk_up_to) {
            continue;
        }

        // Ignore deleted events
        if (event->DeletedAt()) {
            continue;
        }

        // Ignore chunked and already uploaded stuff
        if (event->Chunked() || event->Uploaded()) {
            continue;
        }

        // Calculate the start time of the chunk
        // that fits this timeline event
        time_t chunk_start_time =
            (event->Start() / kTimelineChunkSeconds)
            * kTimelineChunkSeconds;

        // Build dictionary key so that the chunk can be accessed later
        std::stringstream ss;
        ss << event->Filename();
        ss << "::";
        ss << event->Title();
        ss << "::";
        ss << event->Idle();
        ss << "::";
        ss << chunk_start_time;
        std::string key = ss.str();

        // Calculate positive value of timeline event duration
        time_t duration = event->Duration();
        if (duration < 0) {
            duration = 0;
        }

        poco_assert(!event->Uploaded());
        poco_assert(!event->Chunked());

        TimelineEvent *chunk = nullptr;
        if (compressed.find(key) == compressed.end()) {
            // If chunk is not created yet,
            // turn the timeline event into chunk
            chunk = event;
            chunk->SetEndTime(chunk->Start() + duration);
            chunk->SetChunked(true);
        } else {
            // If chunk already exists, add duration
            // to that junk and delete the original event
            chunk = compressed[key];
            chunk->SetEndTime(chunk->EndTime() + duration);
            event->Delete();
        }
        compressed[key] = chunk;
    }

    {
        std::stringstream ss;
        ss << "CompressTimeline done in " << (time(nullptr) - start)
           << " seconds, "
           << related.TimelineEvents.size()
           << " compressed into "
           << compressed.size()
           << " chunks";
        logger().debug(ss.str());
    }
}

std::vector<TimelineEvent> User::CompressedTimeline() const {
    std::vector<TimelineEvent> list;
    for (std::vector<TimelineEvent *>::const_iterator i =
        related.TimelineEvents.begin();
            i != related.TimelineEvents.end();
            ++i) {
        TimelineEvent *event = *i;
        poco_check_ptr(event);
        if (event->VisibleToUser()) {
            // Make a copy of the timeline event
            list.push_back(*event);
        }
    }
    return list;
}

std::string User::ModelName() const {
    return kModelUser;
}

std::string User::ModelURL() const {
    return "/api/v9/me";
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
