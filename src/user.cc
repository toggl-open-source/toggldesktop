// Copyright 2014 Toggl Desktop developers.

#include "./user.h"

#include <sstream>

#include "./version.h"
#include "./formatter.h"
#include "./json.h"

#include "Poco/Logger.h"
#include "Poco/StopWatch.h"

namespace kopsik {

bool compareTimeEntriesByStart(TimeEntry *a, TimeEntry *b) {
  return a->Start() > b->Start();
}

void User::SortTimeEntriesByStart() {
  std::sort(related.TimeEntries.begin(), related.TimeEntries.end(),
    compareTimeEntriesByStart);
}

void User::ActiveProjects(std::vector<Project *> *list) {
  for (unsigned int i = 0; i < related.Projects.size(); i++) {
    kopsik::Project *p = related.Projects[i];
    if (p->Active()) {
      list->push_back(p);
    }
  }
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
    te->SetDurationString(duration);
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

TimeEntry *User::Latest() {
    if (related.TimeEntries.empty()) {
        return 0;
    }
    return related.TimeEntries[0];
}

std::string User::DateDuration(TimeEntry *te) {
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

bool User::HasPremiumWorkspaces() {
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

void User::SetFullname(std::string value) {
  if (fullname_ != value) {
    fullname_ = value;
    dirty_ = true;
  }
}

void User::SetStoreStartAndStopTime(const bool value) {
  if (store_start_and_stop_time_ != value) {
    store_start_and_stop_time_ = value;
    dirty_ = true;
  }
}

void User::SetRecordTimeline(const bool value) {
    if (record_timeline_ != value) {
        record_timeline_ = value;
        dirty_ = true;
    }
}

void User::SetEmail(const std::string value) {
  if (email_ != value) {
    email_ = value;
    dirty_ = true;
  }
}

void User::SetAPIToken(std::string value) {
    if (api_token_ != value) {
        api_token_ = value;
        dirty_ = true;
    }
}

void User::SetSince(Poco::UInt64 value) {
    if (since_ != value) {
        since_ = value;
        dirty_ = true;
    }
}

void User::SetID(Poco::UInt64 value) {
    if (id_ != value) {
        id_ = value;
        dirty_ = true;
    }
}

void User::SetDefaultWID(Poco::UInt64 value) {
    if (default_wid_ != value) {
        default_wid_ = value;
        dirty_ = true;
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

  Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
  logger.debug(ss.str());

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
  Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
  logger.debug(ss.str());

  TimeEntry *running = RunningTimeEntry();
  if (running) {
    running->StopAt(at);
  }
  return running;
}

TimeEntry *User::RunningTimeEntry() {
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

Task *User::GetTaskByID(const Poco::UInt64 id) {
    poco_assert(id > 0);
    for (std::vector<Task *>::const_iterator it = related.Tasks.begin();
            it != related.Tasks.end(); it++) {
        if ((*it)->ID() == id) {
            return *it;
        }
    }
    return 0;
}

Client *User::GetClientByID(const Poco::UInt64 id) {
    poco_assert(id > 0);
    for (std::vector<Client *>::const_iterator it = related.Clients.begin();
            it != related.Clients.end(); it++) {
        if ((*it)->ID() == id) {
            return *it;
        }
    }
    return 0;
}

Project *User::GetProjectByID(const Poco::UInt64 id) {
    poco_assert(id > 0);
    for (std::vector<Project *>::const_iterator it = related.Projects.begin();
            it != related.Projects.end(); it++) {
        if ((*it)->ID() == id) {
            return *it;
        }
    }
    return 0;
}

TimeEntry *User::GetTimeEntryByGUID(const guid GUID) {
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

Tag *User::GetTagByGUID(const guid GUID) {
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

Tag *User::GetTagByID(const Poco::UInt64 id) {
    poco_assert(id > 0);
    for (std::vector<Tag *>::const_iterator it = related.Tags.begin();
            it != related.Tags.end(); it++) {
        if ((*it)->ID() == id) {
            return *it;
        }
    }
    return 0;
}

void User::CollectPushableTimeEntries(std::vector<TimeEntry *> *result) {
    poco_assert(result);
    for (std::vector<TimeEntry *>::const_iterator it =
            related.TimeEntries.begin();
            it != related.TimeEntries.end();
            it++) {
        TimeEntry *te = *it;
        if (te->NeedsPush()) {
            result->push_back(te);
        }
    }
}

error User::Sync(HTTPSClient *https_client,
        const bool full_sync,
        const bool with_related_data) {
    BasicAuthUsername = APIToken();
    BasicAuthPassword = "api_token";
    error err = pull(https_client, full_sync, with_related_data);
    if (err != noError) {
        return err;
    }
    return Push(https_client);
}

error User::Push(HTTPSClient *https_client) {
  try {
    Poco::Stopwatch stopwatch;
    stopwatch.start();

    std::vector<TimeEntry *> pushable;
    CollectPushableTimeEntries(&pushable);

    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
    if (pushable.empty()) {
        logger.trace("Nothing to push.");
        return noError;
    }

    {
        std::stringstream ss;
        ss << pushable.size() << " time entries need a push";
        logger.debug(ss.str());
    }

    std::string json = DirtyUserObjectsJSON(this, &pushable);

    logger.debug(json);

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
    ProcessResponseArray(&results, &pushable, &errors);

    if (!errors.empty()) {
        return collectErrors(&errors);
    }

    stopwatch.stop();
    std::stringstream ss;
    ss << "Changes data JSON pushed and responses parsed in "
        << stopwatch.elapsed() / 1000 << " ms";
    logger.debug(ss.str());
  } catch(const Poco::Exception& exc) {
    return exc.displayText();
  } catch(const std::exception& ex) {
    return ex.what();
  } catch(const std::string& ex) {
    return ex;
  }
  return noError;
}

std::string User::String() {
  std::stringstream ss;
  ss  << "ID=" << id_
      << " local_id=" << local_id_
      << " default_wid=" << default_wid_
      << " api_token=" << api_token_
      << " since=" << since_
      << " record_timeline=" << record_timeline_;
  return ss.str();
}

error User::Login(HTTPSClient *https_client,
    const std::string &email, const std::string &password) {
  BasicAuthUsername = email;
  BasicAuthPassword = password;
  return pull(https_client, false, false);
}

error User::pull(HTTPSClient *https_client,
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
    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
    logger.debug(ss.str());
  } catch(const Poco::Exception& exc) {
    return exc.displayText();
  } catch(const std::exception& ex) {
    return ex.what();
  } catch(const std::string& ex) {
    return ex;
  }
  return noError;
};

error User::collectErrors(std::vector<error> *errors) {
  Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
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
    logger.error(err);
  }
  return error(ss.str());
}

Workspace *User::GetWorkspaceByID(const Poco::UInt64 id) {
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

Project *User::GetProjectByGUID(const guid GUID) {
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

Client *User::GetClientByGUID(const guid GUID) {
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

TimeEntry *User::GetTimeEntryByID(const Poco::UInt64 id) {
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

}   // namespace kopsik
