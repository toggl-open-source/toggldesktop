// Copyright 2014 Toggl Desktop developers.

#include "user.h"

#include <time.h>

#include <sstream>

#include "client.h"
#include "const.h"
#include "formatter.h"
#include "https_client.h"
#include "obm_action.h"
#include "project.h"
#include "tag.h"
#include "task.h"
#include "time_entry.h"
#include "timeline_event.h"
#include "urls.h"

#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>
#include <Poco/Crypto/Cipher.h>
#include <Poco/Crypto/CipherFactory.h>
#include <Poco/Crypto/CipherKey.h>
#include <Poco/Crypto/CryptoStream.h>
#include <Poco/DigestStream.h>
#include <Poco/Random.h>
#include <Poco/RandomStream.h>
#include <Poco/SHA1Engine.h>
#include <Poco/Stopwatch.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>
#include <Poco/UTF8String.h>

namespace toggl {

User::~User() {
    related.Clear();
}

locked<Project> User::CreateProject(
    const Poco::UInt64 workspace_id,
    const Poco::UInt64 client_id,
    const std::string &client_guid,
    const std::string &client_name,
    const std::string &project_name,
    const bool is_private,
    const std::string &project_color,
    const bool billable) {

    auto p = related.Projects.create();
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

    // FIXME sorting not implemented yet
    //AddProjectToList(p);

    return p;
}

#if 0
void User::AddProjectToList(Project *p) {
    bool WIDMatch = false;
    bool CIDMatch = false;

    // We should push the project to correct alphabetical position
    // (since we try to avoid sorting the large list)
    for (auto pr : related.Projects) {
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
#endif

locked<Client> User::CreateClient(
    const Poco::UInt64 workspace_id,
    const std::string &client_name) {
    auto c = related.Clients.create();
    c->SetWID(workspace_id);
    c->SetName(client_name);
    c->SetUID(ID());


    // FIXME sorting not implemented yet
    //AddClientToList(c);

    return c;
}

#if 0
void User::AddClientToList(Client *c) {
    bool foundMatch = false;

    // We should push the project to correct alphabetical position
    // (since we try to avoid sorting the large list)
    for (std::vector<Client *>::iterator it =
        related.Clients.begin();
            it != related.Clients.end(); ++it) {
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
#endif

// Start a time entry, mark it as dirty and add to user time entry collection.
// Do not save here, dirtyness will be handled outside of this module.
locked<TimeEntry> User::Start(
    const std::string &description,
    const std::string &duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string project_guid,
    const std::string tags,
    const time_t started,
    const time_t ended,
    const bool stop_current_running) {

    if (stop_current_running) {
        Stop();
    }

    time_t now = time(nullptr);

    std::stringstream ss;
    ss << "User::Start now=" << now;

    auto te = related.TimeEntries.create();
    te->SetCreatedWith(HTTPSClient::Config.UserAgent());
    te->SetDescription(description);
    te->SetUID(ID());
    te->SetPID(project_id);
    te->SetProjectGUID(project_guid);
    te->SetTID(task_id);
    te->SetTags(tags);

    if (started == 0 && ended == 0) {
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
    } else {
        int seconds = int(ended - started);
        te->SetDurationInSeconds(seconds);
        te->SetStop(ended);
        te->SetStart(started);
    }

    // Try to set workspace ID from project
    locked<Project> p;
    if (te->PID()) {
        p = related.Projects.byID(te->PID());
    } else if (!te->ProjectGUID().empty()) {
        p = related.Projects.byGUID(te->ProjectGUID());
    }
    if (p) {
        te->SetWID(p->WID());
        te->SetBillable(p->Billable());
    }

    // Try to set workspace ID from task
    if (!te->WID() && te->TID()) {
        locked<Task> t = related.Tasks.byID(te->TID());
        if (t) {
            te->SetWID(t->WID());
        }
    }

    EnsureWID(te);

    te->SetUIModified();

    return te;
}

locked<TimeEntry> User::Continue(
    const std::string &GUID,
    const bool manual_mode) {

    locked<TimeEntry> existing = related.TimeEntries.byGUID(GUID);
    if (!existing) {
        logger().warning("Time entry not found: ", GUID);
        return {};
    }

    if (existing->DeletedAt()) {
        logger().warning(kCannotContinueDeletedTimeEntry);
        return {};
    }

    Stop();

    time_t now = time(nullptr);

    auto result = related.TimeEntries.create();
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

    return result;
}

std::string User::DateDuration(TimeEntry * const te) const {
    Poco::Int64 date_duration(0);
    std::string date_header = Formatter::FormatDateHeader(te->Start());
    for (auto n : related.TimeEntries) {
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
    for (auto model : related.Workspaces) {
        if (model->Premium()) {
            return true;
        }
    }
    return false;
}

bool User::CanAddProjects() const {
    for (auto model : related.Workspaces) {
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
// Martin - commented out `stopped` because it was never used anyway
void User::Stop(/*std::vector<TimeEntry *> *stopped */) {
    auto te = RunningTimeEntry();
    while (te) {
        /*
        if (stopped) {
            stopped->push_back(te);
        }
        */
        te->StopTracking();
        te = RunningTimeEntry();
    }
}

locked<TimeEntry> User::DiscardTimeAt(
    const std::string &guid,
    const Poco::Int64 at,
    const bool split_into_new_entry) {

    if (!(at > 0)) {
        logger().error("Cannot discard without valid timestamp");
        return {};
    }

    logger().debug("User is discarding time entry ", guid, " at ", at);

    locked<TimeEntry> te = related.TimeEntries.byGUID(guid);
    if (te) {
        te->DiscardAt(at);
    }

    if (te && split_into_new_entry) {
        auto split = related.TimeEntries.create();
        split->SetCreatedWith(HTTPSClient::Config.UserAgent());
        split->SetUID(ID());
        split->SetStart(at);
        split->SetDurationInSeconds(-at);
        split->SetUIModified();
        split->SetWID(te->WID());
        return split;
    }

    return {};
}

locked<TimeEntry> User::RunningTimeEntry() {
    for (auto i : related.TimeEntries) {
        if (i->DurationInSeconds() < 0) {
            return i;
        }
    }
    return {};
}

locked<const TimeEntry> User::RunningTimeEntry() const {
    for (auto i : related.TimeEntries) {
        if (i->DurationInSeconds() < 0) {
            return i;
        }
    }
    return {};
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
    deleteRelatedModelsWithWorkspace(wid, related.Clients);
    deleteRelatedModelsWithWorkspace(wid, related.Projects);
    deleteRelatedModelsWithWorkspace(wid, related.Tasks);
    deleteRelatedModelsWithWorkspace(wid, related.TimeEntries);
    deleteRelatedModelsWithWorkspace(wid, related.Tags);
}

void User::RemoveClientFromRelatedModels(const Poco::UInt64 cid) {
    for (auto model : related.Projects) {
        if (model->CID() == cid) {
            model->SetCID(0);
        }
    }
}

void User::RemoveProjectFromRelatedModels(const Poco::UInt64 pid) {
    removeProjectFromRelatedModels(pid, related.Tasks);
    removeProjectFromRelatedModels(pid, related.TimeEntries);
}

void User::RemoveTaskFromRelatedModels(const Poco::UInt64 tid) {
    for (auto model : related.TimeEntries) {
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
        logger().error("Backend is sending invalid data: ignoring update without an ID");
        return;
    }

    auto model = related.Tags.byID(id);

    if (!model) {
        model = related.Tags.byGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = related.Tags.create();
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
        logger().error("Backend is sending invalid data: ignoring update without an ID");
        return;
    }

    auto model = related.Tasks.byID(id);

    // Tasks have no GUID

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = related.Tasks.create();
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

    Logger("json").debug("Update parsed into action=", action, ", model=", model);

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
        logger().error("Backend is sending invalid data: ignoring update without an ID");
        return;
    }
    locked<Workspace> model = related.Workspaces.byID(id);

    // Workspaces have no GUID

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = related.Workspaces.create();
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
        Logger("json").warning("cannot load empty JSON");
        return noError;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json, root)) {
        return error("Failed to LoadUserAndRelatedDataFromJSONString");
    }

    SetSince(root["since"].asInt64());
    Logger("json").debug("User data as of: ", Since());

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
    locked<ObmExperiment> model;
    for (auto existing : related.ObmExperiments) {
        if (existing->Nr() == nr) {
            model = std::move(existing);
            break;
        }
    }
    if (!model) {
        model = related.ObmExperiments.create();
        model->SetUID(ID());
        model->SetNr(nr);
    }
    model->SetIncluded(obm["included"].asBool());
    model->SetActions(obm["actions"].asString());
}

void User::loadUserAndRelatedDataFromJSON(
    Json::Value data,
    const bool &including_related_data) {

    if (!data["id"].asUInt64()) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");
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
        logger().error("Backend is sending invalid data: ignoring update without an ID");
        return;
    }
    auto model = related.Clients.byID(id);

    if (!model) {
        model = related.Clients.byGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = related.Clients.create();
    }
    if (alive) {
        alive->insert(id);
    }

    model->SetUID(ID());
    model->LoadFromJSON(data);

    // FIXME sorting not implemented yet
    //if (addNew) {
    //    AddClientToList(model);
    //}
}

void User::loadUserClientFromJSON(
    Json::Value data,
    std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");
        return;
    }
    auto model = related.Clients.byID(id);

    if (!model) {
        model = related.Clients.byGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = related.Clients.create();
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
        logger().error("Backend is sending invalid data: ignoring update without an ID");
        return;
    }

    auto model = related.Projects.byID(id);

    if (!model) {
        model = related.Projects.byGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = related.Projects.create();
        addNew = true;
    }
    if (alive) {
        alive->insert(id);
    }

    model->SetUID(ID());
    model->LoadFromJSON(data);

    auto c = related.clientByProject(model);
    if (c) {
        model->SetClientName(c->Name());
    }

    // FIXME sorting not implemented yet
    //if (addNew) {
    //    AddProjectToList(model);
    //}
}

void User::loadUserProjectFromJSON(
    Json::Value data,
    std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");
        return;
    }

    locked<Project> model = related.Projects.byID(id);

    if (!model) {
        model = related.Projects.byGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = related.Projects.create();
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
}

bool User::SetTimeEntryID(Poco::UInt64 id,
    locked<TimeEntry> &timeEntry) {
    auto otherTimeEntry = related.TimeEntries.byID(id);
    if (otherTimeEntry) {
        // this means that somehow we already have a time entry with the ID
        // that was just returned from a response to time entry creation request
        logger().error("There is already a newer version of this entry");

        // clearing the GUID to make sure there's no GUID conflict
        timeEntry->SetGUID("");

        // deleting the duplicate entry
        // this entry has no ID so the corresponding server entry will not be deleted
        timeEntry->Delete();
        return false;
    }
    timeEntry->SetID(id);
    return true;
}

void User::loadUserTimeEntryFromJSON(
    Json::Value data,
    std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");
        return;
    }

    locked<TimeEntry> model;

    model = related.TimeEntries.byID(id);

    if (!model) {
        model = related.TimeEntries.byGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = related.TimeEntries.create();
        model->SetID(id);
    }

    if (!model->ID()) {
        // case where model was matched by GUID
        model->SetID(id);
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
            it != time_entries->end(); ++it) {
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

bool User::CanSeeBillable(locked<Workspace> &ws) const {
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
        auto uploaded = related.TimelineEvents.byGUID(event.GUID());
        if (!uploaded) {
            logger().error("Could not find timeline event to mark it as uploaded: ", event.String());
            continue;
        }
        uploaded->SetUploaded(true);
    }
}

void User::CompressTimeline() {
    // Group events by app name into chunks
    std::map<std::string, locked<TimelineEvent>> compressed;

    // Older events will be deleted
    Poco::Int64 minimum_time = time(nullptr) - kTimelineSecondsToKeep;

    // Find the chunk start time of current time.
    // then process only events that are older that this chunk start time.
    // Else we will have no full chunks to compress.
    Poco::Int64 chunk_up_to =
        (time(nullptr) / kTimelineChunkSeconds) * kTimelineChunkSeconds;


    time_t start = time(nullptr);

    logger().debug("CompressTimeline ",
                   " user_id=", ID(),
                   " chunk_up_to=", chunk_up_to,
                   " number of events=", related.TimelineEvents.size());

    for (auto event : related.TimelineEvents) {
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

        locked<TimelineEvent> chunk;
        if (compressed.find(key) == compressed.end()) {
            // If chunk is not created yet,
            // turn the timeline event into chunk
            chunk = std::move(event);
            chunk->SetEndTime(chunk->Start() + duration);
            chunk->SetChunked(true);
        } else {
            // If chunk already exists, add duration
            // to that junk and delete the original event
            chunk = compressed[key].split();
            chunk->SetEndTime(chunk->EndTime() + duration);
            event->Delete();
        }
        compressed[key] = std::move(chunk);
    }

    logger().debug("CompressTimeline done in ", (time(nullptr) - start), " seconds, ",
                   related.TimelineEvents.size(), " compressed into ", compressed.size(), " chunks");
}

std::vector<TimelineEvent> User::CompressedTimelineForUI(const Poco::LocalDateTime *date) const {
    return CompressedTimeline(date, false);
}

std::vector<TimelineEvent> User::CompressedTimelineForUpload(const Poco::LocalDateTime *date) const {
    return CompressedTimeline(date, true);
}

std::vector<TimelineEvent> User::CompressedTimeline(const Poco::LocalDateTime *date, bool is_for_upload) const {
    std::vector<TimelineEvent> list;
    for (auto event : related.TimelineEvents) {
        // Skip if this event is deleted or uploaded
        if (event->DeletedAt() > 0) {
            continue;
        }

        if (is_for_upload && !event->VisibleToUser()) {
            continue;
        }

        if (date) {
            // Check if timeline event occured on the
            // required date:
            Poco::LocalDateTime event_date(
                Poco::Timestamp::fromEpochTime(event->Start()));
            if (event_date.year() != date->year() ||
                    event_date.month() != date->month() ||
                    event_date.day() != date->day()) {
                continue;
            }
        }
        // Make a copy of the timeline event
        // FIXME timeline
        //list.push_back(*event);
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
void deleteZombies(ProtectedContainer<T> &list,
    const std::set<Poco::UInt64> &alive) {
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

template <typename T>
void deleteRelatedModelsWithWorkspace(const Poco::UInt64 wid,
                                      ProtectedContainer<T> &list) {
    for (auto model : list) {
        if (model->WID() == wid) {
            model->MarkAsDeletedOnServer();
        }
    }
}

template <typename T>
void removeProjectFromRelatedModels(const Poco::UInt64 pid,
                                    ProtectedContainer<T> &list) {
    for (auto model : list) {
        if (model->PID() == pid) {
            model->SetPID(0);
        }
    }
}

}  // namespace toggl
