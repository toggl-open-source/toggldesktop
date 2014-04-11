// Copyright 2014 Toggl Desktop developers.

#include "./user.h"
#include <time.h>
#include <sstream>

#include "./version.h"
#include "./formatter.h"
#include "./json.h"

#include "Poco/Logger.h"
#include "Poco/Stopwatch.h"

namespace kopsik {

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
TimeEntry *User::Start(
    const std::string description,
    const std::string duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id) {
    Stop();

    time_t now = time(0);

    TimeEntry *te = new TimeEntry();
    te->SetDescription(Formatter::EscapeTabsAndLineBreaks(description, true));
    te->SetUID(ID());
    te->SetPID(project_id);
    te->SetTID(task_id);

    if (!duration.empty()) {
        int seconds = Formatter::ParseDurationString(duration);
        te->SetDurationInSeconds(seconds);
        if (last_date_) {
            // FIXME: use year, month and day from last_date_
        }
        te->SetStop(now);
        te->SetStart(te->Stop() - te->DurationInSeconds());
    } else {
        te->SetDurationInSeconds(-now);
        // dont set Stop, TE is running
        te->SetStart(now);
    }
    te->SetCreatedWith(kopsik::UserAgent(app_name_, app_version_));

    // Try to set workspace ID from project
    if (te->PID()) {
        Project *p = GetProjectByID(te->PID());
        if (p) {
            te->SetWID(p->WID());
            te->SetBillable(p->Billable());
        }
    }

    // Try to set workspace ID from task
    if (!te->WID() && te->TID()) {
        Task *t = GetTaskByID(te->TID());
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

void User::ensureWID(TimeEntry *te) const {
    // Set default wid
    if (!te->WID()) {
        te->SetWID(DefaultWID());
    }
}

kopsik::error User::Continue(
    const std::string GUID,
    TimeEntry **result) {

    poco_assert(result);

    Stop();
    TimeEntry *existing = GetTimeEntryByGUID(GUID);
    if (!existing) {
        logger().warning("Time entry not found: " + GUID);
        return noError;
    }

    *result = 0;
    if (existing->DurOnly() && existing->IsToday()) {
        *result = existing;
        (*result)->SetDurationInSeconds(
            -time(0) + (*result)->DurationInSeconds());
    } else {
        *result = new TimeEntry();
        (*result)->SetDescription(existing->Description());
        (*result)->SetDurOnly(existing->DurOnly());
        (*result)->SetWID(existing->WID());
        (*result)->SetPID(existing->PID());
        (*result)->SetTID(existing->TID());
        (*result)->SetUID(ID());
        (*result)->SetStart(time(0));
        (*result)->SetCreatedWith(kopsik::UserAgent(app_name_, app_version_));
        (*result)->SetDurationInSeconds(-time(0));
        (*result)->SetBillable(existing->Billable());
        (*result)->SetTags(existing->Tags());
        related.TimeEntries.push_back((*result));
    }
    (*result)->SetUIModified();
    return kopsik::noError;
}

TimeEntry *User::Latest() const {
    if (related.TimeEntries.empty()) {
        return 0;
    }
    std::vector<TimeEntry *> list(related.TimeEntries);
    std::sort(list.begin(), list.end(), CompareTimeEntriesByStart);
    return list[0];
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

// Stop a time entry, mark it as dirty.
// Note that there may be multiple TE-s running. If there are,
// all of them are stopped (multi-tracking is not supported by Toggl).
// Do not save here, dirtyness will be handled outside of this module.
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

TimeEntry *User::StopAt(const Poco::Int64 at) {
    poco_assert(at > 0);

    std::stringstream ss;
    ss << "User is stopping running time entry at " << at;
    logger().debug(ss.str());

    TimeEntry *running = RunningTimeEntry();
    if (running) {
        running->StopAt(at);
    }
    return running;
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

template<typename T>
T *getModelByID(const Poco::UInt64 id, std::vector<T *> *list) {
    poco_assert(id > 0);
    typedef typename std::vector<T *>::const_iterator iterator;
    for (iterator it = list->begin(); it != list->end(); it++) {
        T *model = *it;
        if (model->ID() == id) {
            return model;
        }
    }
    return 0;
}

Task *User::GetTaskByID(const Poco::UInt64 id) {
    return getModelByID<Task>(id, &related.Tasks);
}

Client *User::GetClientByID(const Poco::UInt64 id) {
    return getModelByID(id, &related.Clients);
}

Project *User::GetProjectByID(const Poco::UInt64 id) {
    return getModelByID(id, &related.Projects);
}

template <typename T>
T *getModelByGUID(const guid GUID, std::vector<T *> *list) {
    if (GUID.empty()) {
        return 0;
    }
    typedef typename std::vector<T *>::const_iterator iterator;
    for (iterator it = list->begin(); it != list->end(); it++) {
        T *model = *it;
        if (model->GUID() == GUID) {
            return model;
        }
    }
    return 0;
}


TimeEntry *User::GetTimeEntryByGUID(const guid GUID) {
    return getModelByGUID(GUID, &related.TimeEntries);
}

Tag *User::GetTagByGUID(const guid GUID) {
    return getModelByGUID(GUID, &related.Tags);
}

Tag *User::GetTagByID(const Poco::UInt64 id) {
    return getModelByID(id, &related.Tags);
}

void User::CollectPushableTimeEntries(
    std::vector<TimeEntry *> *result,
    std::map<std::string, BaseModel *> *models) const {
    poco_assert(result);
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
    poco_assert(result);
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

error User::FullSync(
    HTTPSClient *https_client) {
    BasicAuthUsername = APIToken();
    BasicAuthPassword = "api_token";
    error err = pull(https_client, true, true);
    if (err != noError) {
        return err;
    }
    return push(https_client);
}

error User::PartialSync(
    HTTPSClient *https_client) {
    BasicAuthUsername = APIToken();
    BasicAuthPassword = "api_token";
    // FIXME: if last sync was a while ago, fetch data using "since" parameter
    return push(https_client);
}

error User::push(HTTPSClient *https_client) {
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

        std::string json = UpdateJSON(&projects, &time_entries);

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

error User::Login(
    HTTPSClient *https_client,
    const std::string &email,
    const std::string &password) {
    BasicAuthUsername = email;
    BasicAuthPassword = password;
    return pull(https_client, true, true);
}

error User::pull(
    HTTPSClient *https_client,
    const bool full_sync,
    const bool with_related_data) {
    try {
        Poco::Stopwatch stopwatch;
        stopwatch.start();

        std::stringstream relative_url;
        relative_url << "/api/v8/me?app_name=kopsik";

        if (with_related_data) {
            relative_url << "&with_related_data=true";
        } else {
            relative_url << "&with_related_data=false";
        }

        if (!full_sync) {
            relative_url << "&since=" << since_;
        }

        std::string response_body("");

        error err = https_client->GetJSON(relative_url.str(),
                                          BasicAuthUsername,
                                          BasicAuthPassword,
                                          &response_body);
        if (err != noError) {
            return err;
        }

        LoadUserFromJSONString(this,
                               response_body,
                               full_sync,
                               with_related_data);

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
};

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

Workspace *User::GetWorkspaceByID(const Poco::UInt64 id) {
    return getModelByID(id, &related.Workspaces);
}

Project *User::GetProjectByGUID(const guid GUID) {
    return getModelByGUID(GUID, &related.Projects);
}

Client *User::GetClientByGUID(const guid GUID) {
    return getModelByGUID(GUID, &related.Clients);
}

TimeEntry *User::GetTimeEntryByID(const Poco::UInt64 id) {
    return getModelByID(id, &related.TimeEntries);
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

}  // namespace kopsik
