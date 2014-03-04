// Copyright 2014 Toggl Desktop developers.

#include "./user.h"

#include <sstream>

#include "./version.h"
#include "./formatter.h"
#include "./json.h"

#include "Poco/Logger.h"
#include "Poco/Stopwatch.h"

namespace kopsik {

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
    const std::string project_name) {
  Project *p = new Project();
  p->SetWID(workspace_id);
  p->SetName(project_name);
  p->SetCID(client_id);
  p->SetUID(ID());
  p->SetActive(true);
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
  TimeEntry *te = new TimeEntry();
  te->SetDescription(description);
  te->SetUID(ID());
  te->SetPID(project_id);
  te->SetTID(task_id);
  te->SetStart(time(0));
  if (!duration.empty()) {
    te->SetDurationUserInput(duration);
  } else {
    te->SetDurationInSeconds(-time(0));
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

  // Set default wid
  if (!te->WID()) {
    te->SetWID(DefaultWID());
  }

  te->SetDurOnly(!StoreStartAndStopTime());
  te->SetUIModifiedAt(time(0));

  related.TimeEntries.push_back(te);
  return te;
}

TimeEntry *User::Continue(const std::string GUID) {
  Stop();
  TimeEntry *existing = GetTimeEntryByGUID(GUID);
  if (!existing) {
    return 0;
  }
  TimeEntry *te = 0;
  if (existing->DurOnly() && existing->IsToday()) {
    te = existing;
    te->SetDurationInSeconds(-time(0) + te->DurationInSeconds());
  } else {
    te = new TimeEntry();
    te->SetDescription(existing->Description());
    te->SetDurOnly(existing->DurOnly());
    te->SetWID(existing->WID());
    te->SetPID(existing->PID());
    te->SetTID(existing->TID());
    te->SetUID(ID());
    te->SetStart(time(0));
    te->SetCreatedWith(kopsik::UserAgent(app_name_, app_version_));
    te->SetDurationInSeconds(-time(0));
    te->SetBillable(existing->Billable());
    te->SetTags(existing->Tags());
    related.TimeEntries.push_back(te);
  }
  te->SetUIModifiedAt(time(0));
  return te;
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

void User::SetFullname(const std::string value) {
  if (fullname_ != value) {
    fullname_ = value;
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
        te->SetDurationInSeconds(time(0) + te->DurationInSeconds());
        te->SetUIModifiedAt(time(0));
        te = RunningTimeEntry();
    }
    return result;
}

TimeEntry *User::SplitAt(const Poco::Int64 at) {
  poco_assert(at > 0);

  std::stringstream ss;
  ss << "User is splitting running time entry at " << at;
  logger().debug(ss.str());

  TimeEntry *running = RunningTimeEntry();
  if (!running) {
    return 0;
  }
  running->StopAt(at);

  TimeEntry *te = new TimeEntry();
  te->SetDescription("");
  te->SetUID(ID());
  te->SetStart(at);
  te->SetDurationInSeconds(-at);
  te->SetWID(running->WID());
  te->SetPID(running->PID());
  te->SetTID(running->TID());
  te->SetUIModifiedAt(time(0));
  te->SetCreatedWith(kopsik::UserAgent(app_name_, app_version_));

  poco_assert(te->DurationInSeconds() < 0);

  related.TimeEntries.push_back(te);
  return te;
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

void User::ClearTasks() {
    for (std::vector<Task *>::const_iterator it =
            related.Tasks.begin();
            it != related.Tasks.end();
            it++) {
        delete *it;
    }
    related.Tasks.clear();
}

void User::ClearTags() {
    for (std::vector<Tag *>::const_iterator it =
            related.Tags.begin();
            it != related.Tags.end();
            it++) {
        delete *it;
    }
    related.Tags.clear();
}

void User::ClearClients() {
    for (std::vector<Client *>::const_iterator it =
            related.Clients.begin();
            it != related.Clients.end();
            it++) {
        delete *it;
    }
    related.Clients.clear();
}

void User::ClearTimeEntries() {
    for (std::vector<TimeEntry *>::const_iterator it =
            related.TimeEntries.begin();
            it != related.TimeEntries.end();
            it++) {
        delete *it;
    }
    related.TimeEntries.clear();
}

void User::ClearWorkspaces() {
    for (std::vector<Workspace *>::const_iterator it =
            related.Workspaces.begin();
            it != related.Workspaces.end();
            it++) {
        delete *it;
    }
    related.Workspaces.clear();
}

void User::ClearProjects() {
    for (std::vector<Project *>::const_iterator it =
            related.Projects.begin();
            it != related.Projects.end();
            it++) {
        delete *it;
    }
    related.Projects.clear();
}

Task *User::GetTaskByID(const Poco::UInt64 id) const {
    poco_assert(id > 0);
    for (std::vector<Task *>::const_iterator it = related.Tasks.begin();
            it != related.Tasks.end(); it++) {
        if ((*it)->ID() == id) {
            return *it;
        }
    }
    return 0;
}

Client *User::GetClientByID(const Poco::UInt64 id) const {
    poco_assert(id > 0);
    for (std::vector<Client *>::const_iterator it = related.Clients.begin();
            it != related.Clients.end(); it++) {
        if ((*it)->ID() == id) {
            return *it;
        }
    }
    return 0;
}

Project *User::GetProjectByID(const Poco::UInt64 id) const {
    poco_assert(id > 0);
    for (std::vector<Project *>::const_iterator it = related.Projects.begin();
            it != related.Projects.end(); it++) {
        if ((*it)->ID() == id) {
            return *it;
        }
    }
    return 0;
}

TimeEntry *User::GetTimeEntryByGUID(const guid GUID) const {
    if (GUID.empty()) {
      return 0;
    }
    for (std::vector<TimeEntry *>::const_iterator it =
            related.TimeEntries.begin();
            it != related.TimeEntries.end();
            it++) {
        if ((*it)->GUID() == GUID) {
            return *it;
        }
    }
    return 0;
}

Tag *User::GetTagByGUID(const guid GUID) const {
  if (GUID.empty()) {
    return 0;
  }
  for (std::vector<Tag *>::const_iterator it =
      related.Tags.begin();
      it != related.Tags.end();
      it++) {
    if ((*it)->GUID() == GUID) {
      return *it;
    }
  }
  return 0;
}

Tag *User::GetTagByID(const Poco::UInt64 id) const {
    poco_assert(id > 0);
    for (std::vector<Tag *>::const_iterator it = related.Tags.begin();
            it != related.Tags.end(); it++) {
        if ((*it)->ID() == id) {
            return *it;
        }
    }
    return 0;
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
    if (model->NeedsPush()) {
      result->push_back(model);
      if (models) {
        (*models)[model->GUID()] = model;
      }
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
    ParseResponseArray(response_body, &results);

    std::vector<error> errors;

    ProcessResponseArray(&results, &models, &errors);

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

    LoadUserFromJSONString(this, response_body, full_sync, with_related_data);

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
  for (std::vector<error>::const_iterator it = errors->begin();
      it != errors->end();
      it++) {
    error err = *it;
    if (!err.empty() && err[err.size() - 1] == '\n') {
      err[err.size() - 1] = '.';
    }
    if (it != errors->begin()) {
      ss << " ";
    }
    ss << err;
    logger().error(err);
  }
  return error(ss.str());
}

Workspace *User::GetWorkspaceByID(const Poco::UInt64 id) const {
  poco_assert(id > 0);
  for (std::vector<Workspace *>::const_iterator it =
      related.Workspaces.begin();
      it != related.Workspaces.end();
      it++) {
    if ((*it)->ID() == id) {
      return *it;
    }
  }
  return 0;
}

Project *User::GetProjectByGUID(const guid GUID) const {
    if (GUID.empty()) {
      return 0;
    }
    for (std::vector<Project *>::const_iterator it = related.Projects.begin();
            it != related.Projects.end(); it++) {
        if ((*it)->GUID() == GUID) {
            return *it;
        }
    }
    return 0;
}

Client *User::GetClientByGUID(const guid GUID) const {
    if (GUID.empty()) {
      return 0;
    }
    for (std::vector<Client *>::const_iterator it = related.Clients.begin();
         it != related.Clients.end(); it++) {
      if ((*it)->GUID() == GUID) {
        return *it;
      }
    }
    return 0;
}

TimeEntry *User::GetTimeEntryByID(const Poco::UInt64 id) const {
    poco_assert(id > 0);
    for (std::vector<TimeEntry *>::const_iterator it =
            related.TimeEntries.begin();
            it != related.TimeEntries.end();
            it++) {
        if ((*it)->ID() == id) {
            return *it;
        }
    }
    return 0;
}

void User::LoadFromJSONNode(JSONNODE * const) {
}

}   // namespace kopsik
