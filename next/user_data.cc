
#include "user_data.h"

#include "model/client.h"
#include "model/project.h"
#include "model/time_entry.h"
#include "model/timeline_event.h"

#include <Poco/UTF8String.h>
#include <Poco/Logger.h>
#include <Poco/Crypto/CipherFactory.h>
#include <Poco/RandomStream.h>
#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>
#include <Poco/Crypto/Cipher.h>
#include <Poco/SHA1Engine.h>
#include <Poco/DigestStream.h>
#include <Poco/Crypto/CipherKey.h>

namespace toggl {

template<class T>
void UserData::deleteZombies(locked<std::vector<T>> &list, const std::set<Poco::UInt64> &alive) {
    for (auto it = list->begin(); it != list->end(); ++it) {
        BaseModel *model = *it;
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
void UserData::deleteRelatedModelsWithWorkspace(const Poco::UInt64 wid, locked<std::vector<T *>> &list) {
    for (auto it = list->begin(); it != list->end(); it++) {
        T *model = *it;
        if (model->WID() == wid) {
            model->MarkAsDeletedOnServer();
        }
    }
}

template <typename T>
void UserData::removeProjectFromRelatedModels(const Poco::UInt64 pid, locked<std::vector<T *>> &list) {
    for (auto it = list->begin(); it != list->end(); it++) {
        T *model = *it;
        if (model->PID() == pid) {
            model->SetPID(0);
        }
    }
}


Project *UserData::CreateProject(
    const Poco::UInt64 workspace_id,
    const Poco::UInt64 client_id,
    const std::string client_guid,
    const std::string client_name,
    const std::string project_name,
    const bool is_private,
    const std::string project_color,
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

void UserData::AddProjectToList(Project *p) {
    bool WIDMatch = false;
    bool CIDMatch = false;

    // We should push the project to correct alphabetical position
    // (since we try to avoid sorting the large list)
    auto projects = Projects();
    for (auto it = projects->begin(); it != projects->end(); it++) {
        Project *pr = *it;
        if (p->WID() == pr->WID()) {
            WIDMatch = true;
            if ((p->CID() == 0 && p->ClientGUID().empty()) && pr->CID() == 0) {
                // Handle adding project without client
                CIDMatch = true;
                if (Poco::UTF8::icompare(p->Name(), pr->Name()) < 0) {
                    projects->insert(it, p);
                    return;
                }
            } else if (Poco::UTF8::icompare(p->ClientName(), pr->ClientName()) == 0) {
                // Handle adding project with client
                CIDMatch = true;
                if (Poco::UTF8::icompare(p->FullName(), pr->FullName()) < 0) {
                    projects->insert(it,p);
                    return;
                }
            } else if (CIDMatch) {
                // in case new project is last in client list
                projects->insert(it,p);
                return;
            } else if ((p->CID() != 0 || !p->ClientGUID().empty()) && pr->CID() != 0) {
                if (Poco::UTF8::icompare(p->FullName(), pr->FullName()) < 0) {
                    projects->insert(it,p);
                    return;
                }
            }
        } else if (WIDMatch) {
            //In case new project is last in workspace list
            projects->insert(it,p);
            return;
        }
    }

    // if projects vector is empty or project should be added to the end
    projects->push_back(p);
}

Client *UserData::CreateClient(
    const Poco::UInt64 workspace_id,
    const std::string client_name) {
    Client *c = new Client();
    c->SetWID(workspace_id);
    c->SetName(client_name);
    c->SetUID(ID());

    AddClientToList(c);

    return c;
}

void UserData::AddClientToList(Client *c) {
    bool foundMatch = false;

    // We should push the project to correct alphabetical position
    // (since we try to avoid sorting the large list)
    auto clients = Clients();
    for (auto it = clients->begin(); it != clients->end(); it++) {
        Client *cl = *it;
        if (c->WID() == cl->WID()) {
            foundMatch = true;
            if (Poco::UTF8::icompare(c->Name(), cl->Name()) < 0) {
                clients->insert(it,c);
                return;
            }
        } else if (foundMatch) {
            clients->insert(it,c);
            return;
        }
    }

    // if clients vector is empty or client should be added to the end
    clients->push_back(c);
}

// Start a time entry, mark it as dirty and add to user time entry collection.
// Do not save here, dirtyness will be handled outside of this module.
locked<TimeEntry> UserData::Start(
    const std::string description,
    const std::string duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string project_guid,
    const std::string tags) {

    Stop();

    time_t now = time(nullptr);

    std::stringstream ss;
    ss << "UserData::Start now=" << now;

    auto te = TimeEntries.create();
    // OVERHAUL TODO
    //te->SetCreatedWith(HTTPSClient::Config.UserAgent());
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
    locked<Project> p;
    if (te->PID()) {
        p = Projects.findByID(te->PID());
    } else if (!te->ProjectGUID().empty()) {
        p = Projects.findByGUID(te->ProjectGUID());
    }
    if (p) {
        te->SetWID(p->WID());
        te->SetBillable(p->Billable());
    }

    // Try to set workspace ID from task
    if (!te->WID() && te->TID()) {
        auto t = Tasks.findByID(te->TID());
        if (t) {
            te->SetWID(t->WID());
        }
    }

    EnsureWID(te);

    te->SetUIModified();

    return te;
}

locked<TimeEntry> UserData::Continue(
    const std::string GUID,
    const bool manual_mode) {

    auto existing = TimeEntries.findByGUID(GUID);
    if (!existing) {
        logger().warning("Time entry not found: " + GUID);
        return {};
    }

    if (existing->DeletedAt()) {
        logger().warning(kCannotContinueDeletedTimeEntry);
        return {};
    }

    Stop();

    time_t now = time(nullptr);

    auto result = TimeEntries.create();
    // OVERHAUL TODO
    //result->SetCreatedWith(HTTPSClient::Config.UserAgent());
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

    // OVERHAUL TODO
    //result->SetCreatedWith(HTTPSClient::Config.UserAgent());

    return result;
}

std::string UserData::DateDuration(TimeEntry * const te) const {
    Poco::Int64 date_duration(0);
    std::string date_header = Formatter::FormatDateHeader(te->Start());
    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
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


bool UserData::HasPremiumWorkspaces() const {
    auto workspaces = Workspaces();
    for (auto it = workspaces->begin(); it != workspaces->end(); it++) {
        Workspace *model = *it;
        if (model->Premium()) {
            return true;
        }
    }
    return false;
}

bool UserData::CanAddProjects() const {
    auto workspaces = Workspaces();
    for (auto it = workspaces->begin(); it != workspaces->end(); it++) {
        Workspace *model = *it;
        if (model->OnlyAdminsMayCreateProjects()) {
            return false;
        }
    }
    return true;
}

error UserData::SetAPITokenFromOfflineData(const std::string password) {
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
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

bool UserData::isTimeEntryLocked(TimeEntry *te) {
    auto ws = Workspaces.findByID(te->WID());
    return isTimeLockedInWorkspace(te->Start(), ws);
}

bool UserData::canChangeStartTimeTo(TimeEntry *te, time_t t) {
    auto ws = Workspaces.findByID(te->WID());
    return !isTimeLockedInWorkspace(t, ws);
}

bool UserData::canChangeProjectTo(TimeEntry *te, Project *p) {
    auto ws = Workspaces.findByID(p->WID());
    return !isTimeLockedInWorkspace(te->Start(), ws);
}

bool UserData::isTimeLockedInWorkspace(time_t t, locked<Workspace> &ws) {
    if (!ws)
        return false;
    if (!ws->Business())
        return false;
    if (ws->Admin())
        return false;
    auto lockedTime = ws->LockedTime();
    if (lockedTime == 0)
        return false;
    return t < lockedTime;
}


// Stop a time entry, mark it as dirty.
// Note that there may be multiple TE-s running. If there are,
// all of them are stopped (multi-tracking is not supported by Toggl).
void UserData::Stop(std::vector<TimeEntry *> *stopped) {
    auto te = RunningTimeEntry();
    while (te) {
        if (stopped) {
            // OVERHAUL TODO
            //stopped->push_back(te);
        }
        te->StopTracking();
        te = RunningTimeEntry();
    }
}

locked<TimeEntry> UserData::DiscardTimeAt(
        const std::string guid,
        const Poco::Int64 at,
        const bool split_into_new_entry) {

    if (!(at > 0)) {
        logger().error("Cannot discard without valid timestamp");
        return {};
    }

    std::stringstream ss;
    ss << "User is discarding time entry " << guid << " at " << at;
    logger().debug(ss.str());

    auto te = TimeEntries.findByGUID(guid);
    if (te) {
        te->DiscardAt(at);
    }

    if (te && split_into_new_entry) {
        auto split = TimeEntries.create();
        // OVERHAUL TODO
        //split->SetCreatedWith(HTTPSClient::Config.UserAgent());
        split->SetUID(ID());
        split->SetStart(at);
        split->SetDurationInSeconds(-at);
        split->SetUIModified();
        split->SetWID(te->WID());
        return split;
    }

    return {};
}

locked<TimeEntry> UserData::RunningTimeEntry() {
    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
        if ((*it)->DurationInSeconds() < 0) {
            return TimeEntries.make_locked(*it);
        }
    }
    return {};
}

locked<const TimeEntry> UserData::RunningTimeEntry() const {
    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
        if ((*it)->DurationInSeconds() < 0) {
            return TimeEntries.make_locked(*it);
        }
    }
    return {};
}



void UserData::DeleteRelatedModelsWithWorkspace(const Poco::UInt64 wid) {
    {
        auto clients = Clients();
        deleteRelatedModelsWithWorkspace(wid, clients);
    }
    {
        auto projects = Projects();
        deleteRelatedModelsWithWorkspace(wid, projects);
    }
    {
        auto tasks = Tasks();
        deleteRelatedModelsWithWorkspace(wid, tasks);
    }
    {
        auto timeEntries = TimeEntries();
        deleteRelatedModelsWithWorkspace(wid, timeEntries);
    }
    {
        auto tags = Tags();
        deleteRelatedModelsWithWorkspace(wid, tags);
    }
}

void UserData::RemoveClientFromRelatedModels(const Poco::UInt64 cid) {
    auto projects = Projects();
    for (auto it = projects->begin(); it != projects->end(); it++) {
        Project *model = *it;
        if (model->CID() == cid) {
            model->SetCID(0);
        }
    }
}

void UserData::RemoveProjectFromRelatedModels(const Poco::UInt64 pid) {
    {
        auto tasks = Tasks();
        removeProjectFromRelatedModels(pid, tasks);
    }
    {
        auto timeEntries = TimeEntries();
        removeProjectFromRelatedModels(pid, timeEntries);
    }
}

void UserData::loadUserTagFromJSON(Json::Value data, std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }

    auto model = Tags.findByID(id);

    if (!model) {
        model = Tags.findByGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = Tags.create();
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
}

void UserData::loadUserTaskFromJSON(Json::Value data, std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }

    auto model = Tasks.findByID(id);

    // Tasks have no GUID

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = Tasks.create();
    }

    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
}

void UserData::loadUserWorkspaceFromJSON(Json::Value data, std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }
    auto model = Workspaces.findByID(id);

    // Workspaces have no GUID

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = Workspaces.create();
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
}


error UserData::LoadTimeEntriesFromJSONString(const std::string& json) {
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

    auto timeEntries = TimeEntries();
    deleteZombies(timeEntries, alive);

    return noError;
}


void UserData::loadObmExperimentFromJson(Json::Value const &obm) {
    Poco::UInt64 nr = obm["nr"].asUInt64();
    if (!nr) {
        return;
    }
    locked<ObmExperiment> model;
    auto obmExperiments = ObmExperiments();
    for (auto it = obmExperiments->begin(); it != obmExperiments->end(); it++) {
        ObmExperiment *existing = *it;
        if (existing->Nr() == nr) {
            model = ObmExperiments.make_locked(existing);
            break;
        }
    }
    if (!model) {
        model = ObmExperiments.create();
        model->SetUID(ID());
        model->SetNr(nr);
    }
    model->SetIncluded(obm["included"].asBool());
    model->SetActions(obm["actions"].asString());
}


void UserData::loadUserAndRelatedDataFromJSON(Json::Value data, const bool &including_related_data) {

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
            auto workspaces = Workspaces();
            deleteZombies(workspaces, alive);
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
            auto clients = Clients();
            deleteZombies(clients, alive);
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
            auto projects = Projects();
            deleteZombies(projects, alive);
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
            auto tasks = Tasks();
            deleteZombies(tasks, alive);
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
            auto tags = Tags();
            deleteZombies(tags, alive);
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
            auto timeEntries = TimeEntries();
            deleteZombies(timeEntries, alive);
        }
    }
}

void UserData::loadUserClientFromSyncJSON(Json::Value data, std::set<Poco::UInt64> *alive) {
    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }
    auto model = Clients.findByID(id);

    if (!model) {
        model = Clients.findByGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        auto c = new Client();
        c->SetUID(ID());
        c->LoadFromJSON(data);
        AddClientToList(c);
    }
    else {
        model->SetUID(ID());
        model->LoadFromJSON(data);
    }
    if (alive) {
        alive->insert(id);
    }
}

void UserData::loadUserClientFromJSON(Json::Value data, std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }
    auto model = Clients.findByID(id);

    if (!model) {
        model = Clients.findByGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = Clients.create();
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
}

void UserData::loadUserProjectFromSyncJSON(Json::Value data, std::set<Poco::UInt64> *alive) {
    bool addNew = false;
    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }

    auto model = Projects.findByID(id);

    if (!model) {
        model = Projects.findByGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        auto p = new Project();
        p->SetUID(ID());
        p->LoadFromJSON(data);

        auto c = clientByProject(p);
        if (c) {
            p->SetClientName(c->Name());
        }

        AddProjectToList(p);
    }
    else {
        model->SetUID(ID());
        model->LoadFromJSON(data);

        auto c = clientByProject(model);
        if (c) {
            model->SetClientName(c->Name());
        }

    }
    if (alive) {
        alive->insert(id);
    }
}

void UserData::loadUserProjectFromJSON(Json::Value data, std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }

    auto model = Projects.findByID(id);

    if (!model) {
        model = Projects.findByGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = Projects.create();
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
}

void UserData::loadUserTimeEntryFromJSON(Json::Value data, std::set<Poco::UInt64> *alive) {

    // alive can be 0, dont assert/check it

    Poco::UInt64 id = data["id"].asUInt64();
    if (!id) {
        logger().error("Backend is sending invalid data: ignoring update without an ID");  // NOLINT
        return;
    }

    auto model = TimeEntries.findByID(id);

    if (!model) {
        model = TimeEntries.findByGUID(data["guid"].asString());
    }

    if (!data["server_deleted_at"].asString().empty()) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = TimeEntries.create();
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSON(data);
    model->EnsureGUID();
}

error UserData::EnableOfflineLogin(const std::string password) {
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
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}


bool UserData::CanSeeBillable(
    const Workspace *ws) const {
    if (!HasPremiumWorkspaces()) {
        return false;
    }
    if (ws && !ws->Premium()) {
        return false;
    }
    return true;
}

void UserData::MarkTimelineBatchAsUploaded(
    const std::vector<TimelineEvent> &events) {

    for (auto i = events.begin();
            i != events.end();
            ++i) {
        TimelineEvent event = *i;
        auto uploaded = TimelineEvents.findByGUID(event.GUID());
        if (!uploaded) {
            logger().error(
                "Could not find timeline event to mark it as uploaded: "
                + event.String());
            continue;
        }
        uploaded->SetUploaded(true);
    }
}

void UserData::CompressTimeline() {
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
           << " number of events=" << TimelineEvents.size();

        logger().debug(ss.str());
    }

    auto timelineEvents = TimelineEvents();
    for (auto i = timelineEvents->begin(); i != timelineEvents->end(); ++i) {
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
           << timelineEvents->size()
           << " compressed into "
           << compressed.size()
           << " chunks";
        logger().debug(ss.str());
    }
}

std::vector<TimelineEvent> UserData::CompressedTimeline() const {
    std::vector<TimelineEvent> list;
    auto timelineEvents = TimelineEvents();
    for (auto i = timelineEvents->begin(); i != timelineEvents->end(); ++i) {
        TimelineEvent *event = *i;
        poco_check_ptr(event);
        if (event->VisibleToUser()) {
            // Make a copy of the timeline event
            list.push_back(*event);
        }
    }
    return list;
}

error UserData::UpdateJSON(
    std::vector<TimeEntry *> * const time_entries,
    std::string *result) const {

    poco_check_ptr(time_entries);

    Json::Value c;

    // Time entries go last
    for (auto it =
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


error UserData::LoadWorkspacesFromJSONString(const std::string& json) {
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
        // OVERHAUL TODO
        //loadUserWorkspaceFromJSON(root[i]);
    }

    return noError;
}

void UserData::LoadObmExperiments(Json::Value const &obm) {
    if (obm.isObject()) {
        loadObmExperimentFromJson(obm);
    } else if (obm.isArray()) {
        for (unsigned int i = 0; i < obm.size(); i++) {
            loadObmExperimentFromJson(obm[i]);
        }
    }
}


error UserData::LoadUserAndRelatedDataFromJSONString(const std::string &json, const bool &including_related_data) {

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



void UserData::loadUserUpdateFromJSON(Json::Value node) {

    Json::Value data = node["data"];
    std::string model = node["model"].asString();
    std::string action = node["action"].asString();

    Poco::UTF8::toLowerInPlace(action);

    std::stringstream ss;
    ss << "Update parsed into action=" << action
       << ", model=" + model;
    Poco::Logger &logger = Poco::Logger::get("json");
    logger.debug(ss.str());

    /*
     * OVERHAUL TODO
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
    */
}




error UserData::LoadUserUpdateFromJSONString(const std::string json) {

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

std::string UserData::generateKey(const std::string password) {
    Poco::SHA1Engine sha1;
    Poco::DigestOutputStream outstr(sha1);
    outstr << Email();
    outstr << password;
    outstr.flush();
    const Poco::DigestEngine::Digest &digest = sha1.digest();
    return Poco::DigestEngine::digestToHex(digest);
}

void UserData::Clear() {
    Workspaces()->clear();
    Clients()->clear();
    Projects()->clear();
    Tasks()->clear();
    Tags()->clear();
    TimeEntries()->clear();
    AutotrackerRules()->clear();
    TimelineEvents()->clear();
    ObmActions()->clear();
    ObmExperiments()->clear();
}

error UserData::DeleteAutotrackerRule(const Poco::Int64 local_id) {
    if (!local_id) {
        return error("cannot delete rule without an ID");
    }
    auto autotrackerRules = AutotrackerRules();
    for (auto it = autotrackerRules->begin(); it != autotrackerRules->end(); it++) {
        AutotrackerRule *rule = *it;
        // Autotracker settings are not saved to DB,
        // so the ID will be 0 always. But will have local ID
        if (rule->LocalID() == local_id) {
            rule->MarkAsDeletedOnServer();
            rule->Delete();
            break;
        }
    }
    return noError;
}

AutotrackerRule *UserData::FindAutotrackerRule(const TimelineEvent event) const {
    auto autotrackerRules = AutotrackerRules();
    for (auto it = autotrackerRules->begin(); it != autotrackerRules->end(); it++) {
        AutotrackerRule *rule = *it;
        if (rule->Matches(event)) {
            return rule;
        }
    }
    return nullptr;
}

bool UserData::HasMatchingAutotrackerRule(const std::string lowercase_term) const {
    auto autotrackerRules = AutotrackerRules();
    for (auto it = autotrackerRules->begin(); it != autotrackerRules->end(); it++) {
        AutotrackerRule *rule = *it;
        if (rule->Term() == lowercase_term) {
            return true;
        }
    }
    return false;
}

Poco::Int64 UserData::NumberOfUnsyncedTimeEntries() const {
    Poco::Int64 count(0);
    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
        TimeEntry *te = *it;
        if (te->NeedsPush()) {
            count++;
        }
    }

    return count;
}

std::vector<TimelineEvent *> UserData::VisibleTimelineEvents() const {
    std::vector<TimelineEvent *> result;
    auto timelineEvents = TimelineEvents();
    for (auto i = timelineEvents->begin(); i != timelineEvents->end(); ++i) {
        TimelineEvent *event = *i;
        if (event && event->VisibleToUser()) {
            result.push_back(event);
        }
    }
    return result;
}

std::vector<TimeEntry *> UserData::VisibleTimeEntries() const {
    std::vector<TimeEntry *> result;
    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
        TimeEntry *te = *it;
        if (te->GUID().empty()) {
            continue;
        }
        if (te->DeletedAt() > 0) {
            continue;
        }
        result.push_back(te);
    }
    return result;
}

Poco::Int64 UserData::TotalDurationForDate(const TimeEntry *match) const {
    std::string date_header = Formatter::FormatDateHeader(match->Start());
    Poco::Int64 duration(0);
    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
        TimeEntry *te = *it;
        if (te->GUID().empty()) {
            continue;
        }
        if (te->DeletedAt() > 0) {
            continue;
        }
        if (Formatter::FormatDateHeader(te->Start()) == date_header) {
            duration += Formatter::AbsDuration(te->Duration());
        }
    }
    return duration;
}

TimeEntry *UserData::LatestTimeEntry() const {
    TimeEntry *latest = nullptr;
    std::string pomodoro_decription("Pomodoro Break");
    std::string pomodoro_tag("pomodoro-break");

    // Find the time entry that was stopped most recently
    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
        TimeEntry *te = *it;

        if (te->GUID().empty()) {
            continue;
        }
        if (te->DurationInSeconds() < 0) {
            continue;
        }
        if (te->DeletedAt() > 0) {
            continue;
        }
        if (te->Description().compare(pomodoro_decription) == 0) {
            continue;
        }
        if (te->Tags().find(pomodoro_tag) == 0) {
            continue;
        }

        if (!latest || (te->Stop() > latest->Stop())) {
            latest = te;
        }
    }

    return latest;
}

void UserData::timeEntryAutocompleteItems(std::set<std::string> *unique_names, std::map<Poco::UInt64, std::string> *ws_names, std::vector<view::Autocomplete> *list, std::map<std::string, std::vector<view::Autocomplete> > *items) const {

    poco_check_ptr(list);

    auto timeEntries = TimeEntries();
    for (auto it = timeEntries->begin(); it != timeEntries->end(); it++) {
        TimeEntry *te = *it;

        if (te->DeletedAt() || te->IsMarkedAsDeletedOnServer()
                || te->Description().empty()) {
            continue;
        }

        locked<const Task> t;
        if (te->TID()) {
            t = Tasks.findByID(te->TID());
        }

        locked<const Project> p;
        if (t && t->PID()) {
            p = Projects.findByID(t->PID());
        } else if (te->PID()) {
            p = Projects.findByID(te->PID());
        }

        if (p && !p->Active()) {
            continue;
        }

        std::string project_task_label =
                Formatter::JoinTaskName(t, p);

        std::string description = te->Description();

        std::stringstream search_parts;
        search_parts << te->Description();
        if (!project_task_label.empty()) {
            search_parts << " - " << project_task_label;
        }

        std::string text = search_parts.str();
        if (text.empty()) {
            continue;
        }

        if (unique_names->find(text) != unique_names->end()) {
            continue;
        }
        unique_names->insert(text);

        view::Autocomplete autocomplete_item;
        autocomplete_item.Text = text;
        autocomplete_item.Description = description;
        autocomplete_item.ProjectAndTaskLabel = project_task_label;
        if (p) {
            autocomplete_item.ProjectColor = p->ColorCode();
            autocomplete_item.ProjectID = p->ID();
            autocomplete_item.ProjectGUID = p->GUID();
            autocomplete_item.ProjectLabel = p->Name();
            if (p->CID()) {
                autocomplete_item.ClientLabel = p->ClientName();
                autocomplete_item.ClientID = p->CID();
            }
        }

        if (t) {
            autocomplete_item.TaskID = t->ID();
            autocomplete_item.TaskLabel = t->Name();
        }
        autocomplete_item.WorkspaceID = te->WID();
        if (ws_names) {
            autocomplete_item.WorkspaceName = (*ws_names)[te->WID()];
        }
        autocomplete_item.Tags = te->Tags();
        autocomplete_item.Type = kAutocompleteItemTE;
        autocomplete_item.Billable = te->Billable();

        if (items && !autocomplete_item.WorkspaceName.empty()) {
            (*items)[autocomplete_item.WorkspaceName].push_back(autocomplete_item);
        } else {
            list->push_back(autocomplete_item);
        }
    }
}

void UserData::taskAutocompleteItems(std::set<std::string> *unique_names, std::map<Poco::UInt64, std::string> *ws_names, std::vector<view::Autocomplete> *list, std::map<Poco::UInt64, std::vector<view::Autocomplete> > *items) const {

    poco_check_ptr(list);

    auto tasks = Tasks();
    for (auto it = tasks->begin(); it != tasks->end(); it++) {
        auto t = Tasks.make_locked<const Task>(*it);

        if (!t) {
            continue;
        }

        if (t->IsMarkedAsDeletedOnServer()) {
            continue;
        }

        if (!t->Active()) {
            continue;
        }

        locked<const Project> p;
        if (t->PID()) {
            p = Projects.findByID(t->PID());
        }

        if (p && !p->Active()) {
            continue;
        }

        std::string text = Formatter::JoinTaskName(t, p);
        if (text.empty()) {
            continue;
        }

        if (unique_names->find(text) != unique_names->end()) {
            continue;
        }
        unique_names->insert(text);

        view::Autocomplete autocomplete_item;
        autocomplete_item.Text = t->Name();
        autocomplete_item.ProjectAndTaskLabel = text;
        autocomplete_item.TaskLabel = t->Name();
        autocomplete_item.TaskID = t->ID();

        if (p) {
            autocomplete_item.ProjectColor = p->ColorCode();
            autocomplete_item.ProjectID = p->ID();
            autocomplete_item.ProjectGUID = p->GUID();
            autocomplete_item.ProjectLabel = p->Name();
            autocomplete_item.Billable = p->Billable();
            if (p->CID()) {
                autocomplete_item.ClientLabel = p->ClientName();
                autocomplete_item.ClientID = p->CID();
            }
        }
        if (ws_names) {
            autocomplete_item.WorkspaceName = (*ws_names)[t->WID()];
        }
        autocomplete_item.WorkspaceID = t->WID();
        autocomplete_item.Type = kAutocompleteItemTask;

        if (items) {
            (*items)[autocomplete_item.ProjectID].push_back(autocomplete_item);
        } else {
            list->push_back(autocomplete_item);
        }
    }
}

void UserData::projectAutocompleteItems(std::set<std::string> *unique_names, std::map<Poco::UInt64, std::string> *ws_names, std::vector<view::Autocomplete> *list, std::map<std::string, std::vector<view::Autocomplete> > *items, std::map<Poco::UInt64, std::vector<view::Autocomplete> > *task_items) const {
    poco_check_ptr(list);

    auto projects = Projects();
    for (auto it = projects->begin(); it != projects->end(); it++) {
        auto p = Projects.make_locked<Project>(*it);

        if (!p->Active()) {
            continue;
        }

        auto c = clientByProject(p);

        auto nullTask = locked<const Task>();
        std::string text = Formatter::JoinTaskName(nullTask, p);
        if (text.empty()) {
            continue;
        }

        {
            std::stringstream ss;
            ss << p->WID() << "/" << text;
            std::string uniq_name = ss.str();
            if (unique_names->find(uniq_name) != unique_names->end()) {
                continue;
            }
            unique_names->insert(uniq_name);
        }

        view::Autocomplete autocomplete_item;
        autocomplete_item.Text = text;
        autocomplete_item.ProjectAndTaskLabel = text;
        autocomplete_item.ProjectLabel = p->Name();
        if (c) {
            autocomplete_item.ClientLabel = c->Name();
            autocomplete_item.ClientID = c->ID();
        }
        autocomplete_item.ProjectID = p->ID();
        autocomplete_item.ProjectGUID = p->GUID();
        autocomplete_item.ProjectColor = p->ColorCode();
        if (ws_names) {
            autocomplete_item.WorkspaceName = (*ws_names)[p->WID()];
        }
        autocomplete_item.WorkspaceID = p->WID();
        autocomplete_item.Type = kAutocompleteItemProject;

        if (items && !autocomplete_item.WorkspaceName.empty()) {
            (*items)[autocomplete_item.WorkspaceName].push_back(autocomplete_item);
            if (task_items) {
                for (std::vector<view::Autocomplete>::const_iterator it =
                     (*task_items)[autocomplete_item.ProjectID].begin();
                     it != (*task_items)[autocomplete_item.ProjectID].end(); it++) {
                    view::Autocomplete ac = *it;
                    (*items)[autocomplete_item.WorkspaceName].push_back(ac);
                }
            }
        } else {
            list->push_back(autocomplete_item);
            if (task_items) {
                for (std::vector<view::Autocomplete>::const_iterator it =
                     (*task_items)[autocomplete_item.ProjectID].begin();
                     it != (*task_items)[autocomplete_item.ProjectID].end(); it++) {
                    view::Autocomplete ac = *it;
                    list->push_back(ac);
                }
            }
        }
    }
}

void UserData::TimeEntryAutocompleteItems(std::vector<view::Autocomplete> *result) const {
    std::set<std::string> unique_names;
    std::map<Poco::UInt64, std::string> ws_names;
    std::map<std::string, std::vector<view::Autocomplete> > items;
    workspaceAutocompleteItems(&unique_names, &ws_names, result);
    timeEntryAutocompleteItems(&unique_names, &ws_names, result, &items);
    mergeGroupedAutocompleteItems(result, &items);
}

void UserData::MinitimerAutocompleteItems(std::vector<view::Autocomplete> *result) const {
    std::set<std::string> unique_names;
    std::map<Poco::UInt64, std::string> ws_names;
    std::map<std::string, std::vector<view::Autocomplete> > items;
    std::map<Poco::UInt64, std::vector<view::Autocomplete> > task_items;

    workspaceAutocompleteItems(&unique_names, &ws_names, result);
    timeEntryAutocompleteItems(&unique_names, &ws_names, result, &items);
    taskAutocompleteItems(&unique_names, &ws_names, result, &task_items);
    projectAutocompleteItems(&unique_names, &ws_names, result, &items, &task_items);

    mergeGroupedAutocompleteItems(result, &items);
}

void UserData::mergeGroupedAutocompleteItems(std::vector<view::Autocomplete> *result, std::map<std::string, std::vector<view::Autocomplete> > *items) const {
    // Join created workspace maps to a single vector
    Poco::UInt64 total_size = 0;
    for(std::map<std::string, std::vector<view::Autocomplete> >::iterator iter =
        items->begin(); iter != items->end(); ++iter)
    {
        total_size += iter->second.size();
    }

    result->reserve(total_size);

    for(std::map<std::string, std::vector<view::Autocomplete> >::iterator iter =
        items->begin(); iter != items->end(); ++iter)
    {
        result->insert(result->end(), iter->second.begin(), iter->second.end());
    }
}

void UserData::ProjectAutocompleteItems(std::vector<view::Autocomplete> *result) const {
    std::set<std::string> unique_names;
    std::map<Poco::UInt64, std::string> ws_names;
    std::map<Poco::UInt64, std::vector<view::Autocomplete> > task_items;
    workspaceAutocompleteItems(&unique_names, &ws_names, result);
    taskAutocompleteItems(&unique_names, &ws_names, result, &task_items);
    projectAutocompleteItems(&unique_names, &ws_names, result, nullptr, &task_items);
}

void UserData::workspaceAutocompleteItems(std::set<std::string> *, std::map<Poco::UInt64, std::string> *ws_names, std::vector<view::Autocomplete> *) const {

    // remember workspaces that have projects
    std::set<Poco::UInt64> ws_ids_with_projects;

    auto projects = Projects();
    for (auto it = projects->begin(); it != projects->end(); it++) {
        Project *p = *it;

        if (p->Active()) {
            ws_ids_with_projects.insert(p->WID());
        }
    }

    auto workspaces = Workspaces();
    for (auto it = workspaces->begin(); it != workspaces->end(); it++) {
        Workspace *ws = *it;

        if (ws_ids_with_projects.find(ws->ID()) == ws_ids_with_projects.end()) {
            continue;
        }

        std::string ws_name = Poco::UTF8::toUpper(ws->Name());
        (*ws_names)[ws->ID()] = ws_name;
    }
}

void UserData::TagList(std::vector<std::string> *result, const Poco::UInt64 wid) const {
    poco_check_ptr(result);
    std::set<std::string> unique_names;

    auto tags = Tags();
    for (auto it = tags->begin(); it != tags->end(); it++) {
        Tag *tag = *it;
        if (wid && tag->WID() != wid) {
            continue;
        }
        if (unique_names.find(tag->Name()) != unique_names.end()) {
            continue;
        }
        unique_names.insert(tag->Name());
        result->push_back(tag->Name());
    }

    std::sort(result->rbegin(), result->rend());
}

void UserData::WorkspaceList(std::vector<Workspace *> *result) const {
    poco_check_ptr(result);

    auto workspaces = Workspaces();
    for (auto it = workspaces->begin(); it != workspaces->end(); it++) {
        Workspace *ws = *it;
        if (!ws->Admin() && ws->OnlyAdminsMayCreateProjects()) {
            continue;
        }
        result->push_back(ws);
    }
    std::sort(result->rbegin(), result->rend(), CompareWorkspaceByName);
}

locked<std::vector<Client*>> UserData::ClientList() {
    auto clients = Clients();
    static thread_local std::vector<Client*> result;
    result.clear();
    result = *clients;
    std::sort(result.rbegin(), result.rend(), CompareClientByName);
    return Clients.make_locked<std::vector<Client*>>(&result);
}

void UserData::ProjectLabelAndColorCode(TimeEntry * const te, view::TimeEntry *view) const {

    poco_check_ptr(te);
    poco_check_ptr(view);

    locked<const Workspace> ws;
    if (te->WID()) {
        ws = Workspaces.findByID(te->WID());
    }
    if (ws) {
        view->WorkspaceName = ws->Name();
    }

    locked<const Task> t;
    if (te->TID()) {
        t = Tasks.findByID(te->TID());
    }
    if (t) {
        view->TaskLabel = t->Name();
    }

    locked<const Project> p;
    if (t && t->PID()) {
        p = Projects.findByID(t->PID());
    }
    if (!p && te->PID()) {
        p = Projects.findByID(te->PID());
    }
    if (!p && !te->ProjectGUID().empty()) {
        p = Projects.findByGUID(te->ProjectGUID());
    }

    locked<const Client> c = clientByProject(p);

    view->ProjectAndTaskLabel = Formatter::JoinTaskName(t, p);

    if (p) {
        view->Color = p->ColorCode();
        view->ProjectLabel = p->Name();
    }

    if (c) {
        view->ClientLabel = c->Name();
    }
}

locked<const Client> UserData::clientByProject(const Project *p) const {
    locked<const Client> c;
    if (p && p->CID()) {
        c = Clients.findByID(p->CID());
    }
    if (!c && p && !p->ClientGUID().empty()) {
        c = Clients.findByGUID(p->ClientGUID());
    }
    return c;
}

locked<Client> UserData::clientByProject(locked<Project> &p) {
    locked<Client> c;
    if (p && p->CID()) {
        c = Clients.findByID(p->CID());
    }
    if (!c && p && !p->ClientGUID().empty()) {
        c = Clients.findByGUID(p->ClientGUID());
    }
    return c;
}

locked<const Client> UserData::clientByProject(locked<const Project> &p) const {
    locked<const Client> c;
    if (p && p->CID()) {
        c = Clients.findByID(p->CID());
    }
    if (!c && p && !p->ClientGUID().empty()) {
        c = Clients.findByGUID(p->ClientGUID());
    }
    return c;
}


}
