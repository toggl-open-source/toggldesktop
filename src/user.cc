// Copyright 2014 Toggl Desktop developers.

#include "./user.h"
#include <time.h>
#include <sstream>

#include "./formatter.h"
#include "./json.h"

#include "Poco/Logger.h"
#include "Poco/Stopwatch.h"

namespace kopsik {

template<class T>
void deleteZombies(
    std::vector<T> &list,
    std::set<Poco::UInt64> &alive) {
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

User::~User() {
    clearList(&related.Workspaces);
    clearList(&related.Clients);
    clearList(&related.Projects);
    clearList(&related.Tasks);
    clearList(&related.Tags);
    clearList(&related.TimeEntries);
}

void User::ActiveProjects(std::vector<Project *> *list) const {
    for (unsigned int i = 0; i < related.Projects.size(); i++) {
        kopsik::Project *p = related.Projects[i];
        if (p->Active()) {
            list->push_back(p);
        }
    }
}

Project *User::AddProject(
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

// Start a time entry, mark it as dirty and add to user time entry collection.
// Do not save here, dirtyness will be handled outside of this module.
void User::Start(
    const std::string description,
    const std::string duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id) {
    Stop();

    time_t now = time(0);

    TimeEntry *te = new TimeEntry();
    te->SetDescription(description);
    te->SetUID(ID());
    te->SetPID(project_id);
    te->SetTID(task_id);

    if (!duration.empty()) {
        int seconds = Formatter::ParseDurationString(duration);
        te->SetDurationInSeconds(seconds);
        if (last_date_ != 0) {
            now = Formatter::ParseLastDate(last_date_, now);
        }
        te->SetStop(now);
        te->SetStart(te->Stop() - te->DurationInSeconds());
    } else {
        te->SetDurationInSeconds(-now);
        // dont set Stop, TE is running
        te->SetStart(now);
    }
    te->SetCreatedWith(HTTPSClient::UserAgent());

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
}

void User::ensureWID(TimeEntry *te) const {
    // Set default wid
    if (!te->WID()) {
        te->SetWID(DefaultWID());
    }
}

kopsik::error User::Continue(
    const std::string GUID) {

    Stop();
    TimeEntry *existing = related.TimeEntryByGUID(GUID);
    if (!existing) {
        logger().warning("Time entry not found: " + GUID);
        return noError;
    }

    if (existing->DurOnly() && existing->IsToday()) {
        existing->SetDurationInSeconds(
            -time(0) + existing->DurationInSeconds());
        existing->SetUIModified();
        return kopsik::noError;
    }

    TimeEntry *result = new TimeEntry();
    result->SetDescription(existing->Description());
    result->SetDurOnly(existing->DurOnly());
    result->SetWID(existing->WID());
    result->SetPID(existing->PID());
    result->SetTID(existing->TID());
    result->SetUID(ID());
    result->SetStart(time(0));
    result->SetCreatedWith(HTTPSClient::UserAgent());
    result->SetDurationInSeconds(-time(0));
    result->SetBillable(existing->Billable());
    result->SetTags(existing->Tags());

    related.TimeEntries.push_back(result);

    return kopsik::noError;
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
    return Formatter::FormatDurationInSecondsHHMMSS(date_duration);
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
    if (timeofday_format_ != value) {
        timeofday_format_ = value;
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
    if (api_token_ != value) {
        api_token_ = value;
        SetDirty();
    }
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

void User::SetLastTEDate(const std::string value) {
    std::time_t t = Formatter::Parse8601(value);
    if (last_date_ != t) {
        last_date_ = t;
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
    const Poco::Int64 at) {
    poco_assert(at > 0);

    std::stringstream ss;
    ss << "User is discarding time entry " << guid << " at " << at;
    logger().debug(ss.str());

    TimeEntry *te = related.TimeEntryByGUID(guid);
    if (te) {
        te->DiscardAt(at);
    }
    return te;
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

bool User::HasTrackedTimeToday() const {
    for (std::vector<TimeEntry *>::const_iterator it =
        related.TimeEntries.begin();
            it != related.TimeEntries.end();
            it++) {
        if ((*it)->IsToday()) {
            return true;
        }
    }
    return false;
}

void User::CollectPushableTimeEntries(
    std::vector<TimeEntry *> *result,
    std::map<std::string, BaseModel *> *models) const {

    poco_check_ptr(result);

    for (std::vector<TimeEntry *>::const_iterator it =
        related.TimeEntries.begin();
            it != related.TimeEntries.end();
            it++) {
        TimeEntry *model = *it;
        if (!model->NeedsPush()) {
            continue;
        }
        ensureWID(model);
        result->push_back(model);
        if (models) {
            (*models)[model->GUID()] = model;
        }
    }
}

void User::CollectPushableProjects(
    std::vector<Project *> *result,
    std::map<std::string, BaseModel *> *models) const {

    poco_check_ptr(result);

    for (std::vector<Project *>::const_iterator it =
        related.Projects.begin();
            it != related.Projects.end();
            it++) {
        Project *model = *it;
        if (model->NeedsPush()) {
            result->push_back(model);
            if (models) {
                (*models)[model->GUID()] = model;
            }
        }
    }
}

error User::PullAllUserData(
    HTTPSClient *https_client) {
    try {
        Poco::Stopwatch stopwatch;
        stopwatch.start();

        std::string user_data_json("");
        error err = Me(https_client, APIToken(), "api_token", &user_data_json);
        if (err != noError) {
            return err;
        }

        LoadUserAndRelatedDataFromJSONString(user_data_json);

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

error User::PushChanges(HTTPSClient *https_client) {
    try {
        Poco::Stopwatch stopwatch;
        stopwatch.start();

        std::map<std::string, BaseModel *> models;

        std::vector<TimeEntry *> time_entries;
        CollectPushableTimeEntries(&time_entries, &models);

        std::vector<Project *> projects;
        CollectPushableProjects(&projects, &models);

        if (time_entries.empty() && projects.empty()) {
            return noError;
        }

        std::string json = kopsik::json::UpdateJSON(&projects, &time_entries);

        logger().debug(json);

        std::string response_body("");
        error err = https_client->PostJSON("/api/v8/batch_updates",
                                           json,
                                           APIToken(),
                                           "api_token",
                                           &response_body);
        if (err != noError) {
            return err;
        }

        std::vector<BatchUpdateResult> results;
        BatchUpdateResult::ParseResponseArray(response_body, &results);

        std::vector<error> errors;

        BatchUpdateResult::ProcessResponseArray(&results, &models, &errors);

        if (!errors.empty()) {
            return collectErrors(&errors);
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
    HTTPSClient *https_client,
    const std::string email,
    const std::string password,
    std::string *user_data_json) {
    try {
        poco_check_ptr(user_data_json);
        poco_check_ptr(https_client);

        std::stringstream relative_url;
        relative_url << "/api/v8/me"
                     << "?app_name=" << HTTPSClient::AppName
                     << "&with_related_data=true";

        return https_client->GetJSON(relative_url.str(),
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
    return noError;
}

error User::collectErrors(std::vector<error> * const errors) const {
    std::stringstream ss;
    ss << "Errors encountered while syncing data: ";
    std::set<error> unique;
    for (std::vector<error>::const_iterator it = errors->begin();
            it != errors->end();
            it++) {
        error err = *it;
        // skip error if not unique
        if (unique.end() != unique.find(err)) {
            continue;
        }
        if (!err.empty() && err[err.size() - 1] == '\n') {
            err[err.size() - 1] = '.';
        }
        if (it != errors->begin()) {
            ss << " ";
        }
        ss << err;
        unique.insert(err);
        logger().error(err);
    }
    return error(ss.str());
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

void User::loadUserTagFromJSONNode(
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {

    poco_check_ptr(data);
    // alive can be 0, dont assert/check it

    Poco::UInt64 id = kopsik::json::ID(data);
    Tag *model = related.TagByID(id);

    if (!model) {
        model = related.TagByGUID(kopsik::json::GUID(data));
    }

    if (kopsik::json::IsDeletedAtServer(data)) {
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
    model->LoadFromJSONNode(data);
}

void User::loadUserTagsFromJSONNode(
    JSONNODE * const list) {

    poco_check_ptr(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadUserTagFromJSONNode(*current_node, &alive);
        ++current_node;
    }

    deleteZombies(related.Tags, alive);
}

void User::loadUserTasksFromJSONNode(
    JSONNODE * const list) {

    poco_check_ptr(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadUserTaskFromJSONNode(*current_node, &alive);
        ++current_node;
    }

    deleteZombies(related.Tasks, alive);
}

void User::loadUserTaskFromJSONNode(
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {

    poco_check_ptr(data);
    // alive can be 0, dont assert/check it

    Poco::UInt64 id = kopsik::json::ID(data);
    Task *model = related.TaskByID(id);

    // Tasks have no GUID

    if (kopsik::json::IsDeletedAtServer(data)) {
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
    model->LoadFromJSONNode(data);
}

void User::LoadUserUpdateFromJSONString(
    const std::string json) {

    if (json.empty()) {
        return;
    }

    JSONNODE *root = json_parse(json.c_str());
    loadUserUpdateFromJSONNode(root);
    json_delete(root);
}

void User::loadUserUpdateFromJSONNode(
    JSONNODE * const node) {

    poco_check_ptr(node);

    JSONNODE *data = 0;
    std::string model("");
    std::string action("");

    JSONNODE_ITERATOR i = json_begin(node);
    JSONNODE_ITERATOR e = json_end(node);
    while (i != e) {
        json_char *node_name = json_name(*i);
        if (strcmp(node_name, "data") == 0) {
            data = *i;
        } else if (strcmp(node_name, "model") == 0) {
            model = std::string(json_as_string(*i));
        } else if (strcmp(node_name, "action") == 0) {
            action = std::string(json_as_string(*i));
            Poco::toLowerInPlace(action);
        }
        ++i;
    }
    poco_check_ptr(data);

    std::stringstream ss;
    ss << "Update parsed into action=" << action
       << ", model=" + model;
    Poco::Logger &logger = Poco::Logger::get("json");
    logger.debug(ss.str());

    if ("workspace" == model) {
        loadUserWorkspaceFromJSONNode(data);
    } else if ("client" == model) {
        loadUserClientFromJSONNode(data);
    } else if ("project" == model) {
        loadUserProjectFromJSONNode(data);
    } else if ("task" == model) {
        loadUserTaskFromJSONNode(data);
    } else if ("time_entry" == model) {
        loadUserTimeEntryFromJSONNode(data);
    } else if ("tag" == model) {
        loadUserTagFromJSONNode(data);
    } else if ("user" == model) {
        loadUserAndRelatedDataFromJSONNode(data);
    }
}

void User::loadUserWorkspaceFromJSONNode(
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {

    poco_check_ptr(data);
    // alive can be 0, dont assert/check it

    Poco::UInt64 id = kopsik::json::ID(data);
    Workspace *model = related.WorkspaceByID(id);

    // Workspaces have no GUID

    if (kopsik::json::IsDeletedAtServer(data)) {
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
    model->LoadFromJSONNode(data);
}

void User::LoadUserAndRelatedDataFromJSONString(
    const std::string &json) {

    if (json.empty()) {
        Poco::Logger &logger = Poco::Logger::get("json");
        logger.warning("cannot load empty JSON");
        return;
    }

    JSONNODE *root = json_parse(json.c_str());
    JSONNODE_ITERATOR current_node = json_begin(root);
    JSONNODE_ITERATOR last_node = json_end(root);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "since") == 0) {
            SetSince(json_as_int(*current_node));

            Poco::Logger &logger = Poco::Logger::get("json");
            std::stringstream s;
            s << "User data as of: " << Since();
            logger.debug(s.str());

        } else if (strcmp(node_name, "data") == 0) {
            loadUserAndRelatedDataFromJSONNode(*current_node);
        }
        ++current_node;
    }
    json_delete(root);
}

void User::loadUserAndRelatedDataFromJSONNode(
    JSONNODE * const data) {

    poco_check_ptr(data);

    JSONNODE_ITERATOR n = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (n != last_node) {
        json_char *node_name = json_name(*n);
        if (strcmp(node_name, "id") == 0) {
            SetID(json_as_int(*n));
        } else if (strcmp(node_name, "default_wid") == 0) {
            SetDefaultWID(json_as_int(*n));
        } else if (strcmp(node_name, "api_token") == 0) {
            SetAPIToken(std::string(json_as_string(*n)));
        } else if (strcmp(node_name, "email") == 0) {
            SetEmail(std::string(json_as_string(*n)));
        } else if (strcmp(node_name, "fullname") == 0) {
            SetFullname(std::string(json_as_string(*n)));
        } else if (strcmp(node_name, "record_timeline") == 0) {
            SetRecordTimeline(json_as_bool(*n) != 0);
        } else if (strcmp(node_name, "store_start_and_stop_time") == 0) {
            SetStoreStartAndStopTime(json_as_bool(*n) != 0);
        } else if (strcmp(node_name, "timeofday_format") == 0) {
            SetTimeOfDayFormat(std::string(json_as_string(*n)));
        } else if (strcmp(node_name, "projects") == 0) {
            loadUserProjectsFromJSONNode(*n);
        } else if (strcmp(node_name, "tags") == 0) {
            loadUserTagsFromJSONNode(*n);
        } else if (strcmp(node_name, "tasks") == 0) {
            loadUserTasksFromJSONNode(*n);
        } else if (strcmp(node_name, "time_entries") == 0) {
            loadUserTimeEntriesFromJSONNode(*n);
        } else if (strcmp(node_name, "workspaces") == 0) {
            loadUserWorkspacesFromJSONNode(*n);
        } else if (strcmp(node_name, "clients") == 0) {
            loadUserClientsFromJSONNode(*n);
        }
        ++n;
    }
}

void User::loadUserClientFromJSONNode(
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {

    poco_check_ptr(data);
    // alive can be 0, dont assert/check it

    Poco::UInt64 id = kopsik::json::ID(data);
    Client *model = related.ClientByID(id);

    if (!model) {
        model = related.ClientByGUID(kopsik::json::GUID(data));
    }

    if (kopsik::json::IsDeletedAtServer(data)) {
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
    model->LoadFromJSONNode(data);
}

void User::loadUserClientsFromJSONNode(
    JSONNODE * const list) {

    poco_check_ptr(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadUserClientFromJSONNode(*current_node, &alive);
        ++current_node;
    }

    deleteZombies(related.Clients, alive);
}

void User::loadUserProjectFromJSONNode(
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {

    poco_check_ptr(data);
    // alive can be 0, dont assert/check it

    Poco::UInt64 id = kopsik::json::ID(data);
    Project *model = related.ProjectByID(id);

    if (!model) {
        model = related.ProjectByGUID(kopsik::json::GUID(data));
    }

    if (kopsik::json::IsDeletedAtServer(data)) {
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
    model->LoadFromJSONNode(data);
}

void User::loadUserProjectsFromJSONNode(
    JSONNODE * const list) {

    poco_check_ptr(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadUserProjectFromJSONNode(*current_node, &alive);
        ++current_node;
    }

    deleteZombies(related.Projects, alive);
}

void User::loadUserTimeEntryFromJSONNode(
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {

    poco_check_ptr(data);
    // alive can be 0, dont assert/check it

    Poco::UInt64 id = kopsik::json::ID(data);
    TimeEntry *model = related.TimeEntryByID(id);

    if (!model) {
        model = related.TimeEntryByGUID(kopsik::json::GUID(data));
    }

    if (kopsik::json::IsDeletedAtServer(data)) {
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
    model->LoadFromJSONNode(data);
    model->EnsureGUID();
}

void User::loadUserWorkspacesFromJSONNode(
    JSONNODE * const list) {

    poco_check_ptr(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadUserWorkspaceFromJSONNode(*current_node, &alive);
        ++current_node;
    }

    deleteZombies(related.Workspaces, alive);
}

void User::loadUserTimeEntriesFromJSONNode(
    JSONNODE * const list) {

    poco_check_ptr(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadUserTimeEntryFromJSONNode(*current_node, &alive);
        ++current_node;
    }

    deleteZombies(related.TimeEntries, alive);
}

}  // namespace kopsik
