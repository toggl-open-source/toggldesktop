// Copyright 2013 Tanel Lebedev

#include "./toggl_api_client.h"

#include <set>
#include <string>
#include <cstring>
#include <sstream>

#include <iostream> // NOLINT

#include "Poco/Stopwatch.h"
#include "Poco/Exception.h"
#include "Poco/Logger.h"
#include "Poco/DateTimeParser.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Timespan.h"
#include "Poco/NumberParser.h"
#include "Poco/String.h"
#include "Poco/RegularExpression.h"
#include "Poco/StringTokenizer.h"

#include "./https_client.h"
#include "./version.h"

#include "./libjson.h"

namespace kopsik {

const char *known_colors[] = {
    "#4dc3ff", "#bc85e6", "#df7baa", "#f68d38", "#b27636",
    "#8ab734", "#14a88e", "#268bb5", "#6668b4", "#a4506c",
    "#67412c", "#3c6526", "#094558", "#bc2d07", "#999999"
};
template<typename T, size_t N> T *end(T (&ra)[N]) {
    return ra + N;
}
std::vector<std::string> Project::color_codes(known_colors, end(known_colors));

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
TimeEntry *User::Start(const std::string description,
    const Poco::UInt64 time_entry_id,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id) {
  Stop();
  TimeEntry *te = new TimeEntry();
  te->SetDescription(description);
  te->SetUID(ID());
  te->SetPID(project_id);
  te->SetTID(task_id);
  te->SetStart(time(0));
  te->SetDurationInSeconds(-time(0));
  te->SetUIModifiedAt(time(0));
  te->SetCreatedWith(kopsik::UserAgent(app_name_, app_version_));

  if (time_entry_id) {
    TimeEntry *original = GetTimeEntryByID(time_entry_id);
    if (original) {
        te->SetDescription(original->Description());
        te->SetPID(original->PID());
        te->SetTID(original->TID());
        te->SetTags(original->Tags());
    }
  }

  // Try to set workspace ID from project
  if (te->PID()) {
    Project *p = GetProjectByID(te->PID());
    if (p) {
        te->SetWID(p->WID());
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

  related.TimeEntries.push_back(te);
  return te;
}

TimeEntry *User::Continue(const std::string GUID) {
    Stop();
    TimeEntry *existing = GetTimeEntryByGUID(GUID);
    poco_assert(existing);
    TimeEntry *te = new TimeEntry();
    te->SetDescription(existing->Description());
    te->SetUID(ID());
    te->SetStart(time(0));
    te->SetDurationInSeconds(-time(0));
    te->SetWID(existing->WID());
    te->SetPID(existing->PID());
    te->SetTID(existing->TID());
    te->SetUIModifiedAt(time(0));
    te->SetCreatedWith(kopsik::UserAgent(app_name_, app_version_));
    related.TimeEntries.push_back(te);
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

std::string User::ProjectNameIncludingClient(Project *p) {
    poco_assert(p);
    std::stringstream ss;
    ss << p->Name();
    if (p->CID()) {
        kopsik::Client *c = GetClientByID(p->CID());
        if (c) {
            if (!p->Name().empty()) {
                ss << ". ";
            }
            ss << c->Name();
        }
    }
    return ss.str();
}

void User::SetFullname(std::string value) {
  if (fullname_ != value) {
    fullname_ = value;
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
//  poco_assert(te->Start() == at);

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

bool TimeEntry::NeedsPush() {
    return NeedsPOST() || NeedsPUT() || NeedsDELETE();
}

void TimeEntry::StopAt(const Poco::Int64 at) {
    poco_assert(at);
    SetDurationInSeconds(at + DurationInSeconds());
    poco_assert(DurationInSeconds() > 0);
    SetStop(at);
    SetUIModifiedAt(time(0));
}

bool TimeEntry::NeedsPOST() {
    // No server side ID yet, meaning it's not POSTed yet
    return !id_ && !(deleted_at_ > 0);
}

bool TimeEntry::NeedsPUT() {
    // User has modified model via UI, needs a PUT
    return ui_modified_at_ > 0 && !(deleted_at_ > 0);
}

bool TimeEntry::NeedsDELETE() {
    // TE is deleted, needs a DELETE on server side
    return id_ && (deleted_at_ > 0);
}

void User::CollectPushableObjects(std::vector<TimeEntry *> *result) {
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

// Convert the dirty objects to batch updates JSON.
std::string User::dirtyObjectsJSON(std::vector<TimeEntry *> *dirty) {
    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
    JSONNODE *c = json_new(JSON_ARRAY);
    for (std::vector<TimeEntry *>::const_iterator it =
            dirty->begin();
            it != dirty->end(); it++) {
        TimeEntry *te = *it;
        JSONNODE *n = te->JSON();
        json_set_name(n, "time_entry");

        JSONNODE *body = json_new(JSON_NODE);
        json_set_name(body, "body");
        json_push_back(body, n);

        JSONNODE *update = json_new(JSON_NODE);
        if (te->NeedsDELETE()) {
            std::stringstream url;
            url << "/api/v8/time_entries/" << te->ID();
            json_push_back(update, json_new_a("method", "DELETE"));
            json_push_back(update, json_new_a("relative_url",
                url.str().c_str()));
            std::stringstream ss;
            ss << "Time entry " << te->String() << " needs a DELETE";
            logger.debug(ss.str());

        } else if (te->NeedsPOST()) {
            json_push_back(update, json_new_a("method", "POST"));
            json_push_back(update, json_new_a("relative_url",
                "/api/v8/time_entries"));
            std::stringstream ss;
            ss << "Time entry " << te->String() << " needs a POST";
            logger.debug(ss.str());

        } else if (te->NeedsPUT()) {
            std::stringstream url;
            url << "/api/v8/time_entries/" << te->ID();
            json_push_back(update, json_new_a("method", "PUT"));
            json_push_back(update, json_new_a("relative_url",
                url.str().c_str()));
            std::stringstream ss;
            ss << "Time entry " << te->String() << " needs a PUT";
            logger.debug(ss.str());
        }
        json_push_back(update, json_new_a("GUID", te->GUID().c_str()));
        json_push_back(update, body);

        json_push_back(c, update);
    }
    json_char *jc = json_write_formatted(c);
    std::string json(jc);
    json_free(jc);
    json_delete(c);
    return json;
}

// Iterate through response array, parse response bodies.
// Collect errors into a vector.
void User::processResponseArray(std::vector<BatchUpdateResult> *results,
        std::vector<TimeEntry *> *dirty,
        std::vector<error> *errors) {
    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
    for (std::vector<BatchUpdateResult>::const_iterator it = results->begin();
            it != results->end();
            it++) {
        BatchUpdateResult result = *it;

        std::stringstream ss;
        ss  << "batch update result GUID: " << result.GUID
            << ", StatusCode: " << result.StatusCode
            << ", ContentType: " << result.ContentType
            << ", Body: " << result.Body;
        logger.debug(ss.str());

        if (result.StatusCode != 404)  {
            if ((result.StatusCode < 200) || (result.StatusCode >= 300)) {
                if ("null" == result.Body) {
                    std::stringstream ss;
                    ss  << "Request failed with status code "
                        << result.StatusCode;
                    errors->push_back(ss.str());
                } else {
                    errors->push_back(result.Body);
                }
                continue;
            }

            poco_assert(!result.GUID.empty());
            poco_assert(json_is_valid(result.Body.c_str()));
        }

        TimeEntry *te = 0;
        for (std::vector<TimeEntry *>::const_iterator it =
                dirty->begin(); it != dirty->end(); it++) {
            if ((*it)->GUID() == result.GUID) {
                te = *it;
                break;
            }
        }
        poco_assert(te);

        // If TE was deleted, the body won't contain useful data.
        if (("DELETE" == result.Method) || (404 == result.StatusCode)) {
            te->MarkAsDeletedOnServer();
            continue;
        }

        JSONNODE *n = json_parse(result.Body.c_str());
        JSONNODE_ITERATOR i = json_begin(n);
        JSONNODE_ITERATOR e = json_end(n);
        while (i != e) {
            json_char *node_name = json_name(*i);
            if (strcmp(node_name, "data") == 0) {
                te->LoadFromJSONNode(*i);
            }
            ++i;
        }
        json_delete(n);
    }
}

error User::collectErrors(std::vector<error> *errors) {
    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
    std::stringstream ss;
    ss << "Errors encountered while syncing data: ";
    for (std::vector<error>::const_iterator it = errors->begin();
            it != errors->end();
            it++) {
        error err = *it;
        if (!err.empty()) {
            if (err[err.size() - 1] == '\n') {
                err[err.size() - 1] = '.';
            }
        }
        if (it != errors->begin()) {
            ss << " ";
        }
        ss << err;
        logger.error(err);
    }
    return error(ss.str());
}

error User::Push(HTTPSClient *https_client) {
    Poco::Stopwatch stopwatch;
    stopwatch.start();

    std::vector<TimeEntry *>dirty;
    CollectPushableObjects(&dirty);

    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
    if (dirty.empty()) {
        logger.debug("Nothing to push.");
        return noError;
    }

    {
        std::stringstream ss;
        ss << dirty.size() << " model(s) need a push";
        logger.debug(ss.str());
    }

    std::string json = dirtyObjectsJSON(&dirty);

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
    parseResponseArray(response_body, &results);

    std::vector<error> errors;
    processResponseArray(&results, &dirty, &errors);

    if (!errors.empty()) {
        return collectErrors(&errors);
    }

    stopwatch.stop();
    std::stringstream ss;
    ss << "Changes data JSON pushed and responses parsed in "
        << stopwatch.elapsed() / 1000 << " ms";
    logger.debug(ss.str());

    return noError;
}

void User::parseResponseArray(std::string response_body,
        std::vector<BatchUpdateResult> *responses) {
    poco_assert(responses);
    poco_assert(!response_body.empty());
    JSONNODE *response_array = json_parse(response_body.c_str());
    JSONNODE_ITERATOR i = json_begin(response_array);
    JSONNODE_ITERATOR e = json_end(response_array);
    while (i != e) {
        BatchUpdateResult result;
        result.parseResponseJSON(*i);
        responses->push_back(result);
        ++i;
    }
    json_delete(response_array);
}

void BatchUpdateResult::parseResponseJSON(JSONNODE *n) {
    poco_assert(n);
    StatusCode = 0;
    Body = "";
    GUID = "";
    ContentType = "";
    JSONNODE_ITERATOR i = json_begin(n);
    JSONNODE_ITERATOR e = json_end(n);
    while (i != e) {
        json_char *node_name = json_name(*i);
        if (strcmp(node_name, "status") == 0) {
            StatusCode = json_as_int(*i);
        } else if (strcmp(node_name, "body") == 0) {
            Body = std::string(json_as_string(*i));
        } else if (strcmp(node_name, "guid") == 0) {
            GUID = std::string(json_as_string(*i));
        } else if (strcmp(node_name, "content_type") == 0) {
            ContentType = std::string(json_as_string(*i));
        } else if (strcmp(node_name, "method") == 0) {
            Method = std::string(json_as_string(*i));
        }
        ++i;
    }
}

error User::Login(HTTPSClient *https_client,
    const std::string &email, const std::string &password) {
  BasicAuthUsername = email;
  BasicAuthPassword = password;
  return pull(https_client, true, false);
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

error User::pull(HTTPSClient *https_client,
    const bool full_sync,
    const bool with_related_data) {
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

  LoadFromJSONString(response_body, full_sync, with_related_data);

  stopwatch.stop();
  std::stringstream ss;
  ss << "User with related data JSON fetched and parsed in "
    << stopwatch.elapsed() / 1000 << " ms";
  Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
  logger.debug(ss.str());

  return noError;
};

void User::LoadFromJSONString(const std::string &json,
        const bool full_sync,
        const bool with_related_data) {
    poco_assert(!json.empty());

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");

    JSONNODE *root = json_parse(json.c_str());
    JSONNODE_ITERATOR current_node = json_begin(root);
    JSONNODE_ITERATOR last_node = json_end(root);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "since") == 0) {
            SetSince(json_as_int(*current_node));
            std::stringstream s;
            s << "User data as of: " << Since();
            logger.debug(s.str());
        } else if (strcmp(node_name, "data") == 0) {
            LoadDataFromJSONNode(*current_node, full_sync, with_related_data);
        }
        ++current_node;
    }
    json_delete(root);

    stopwatch.stop();
    std::stringstream ss;
    ss << json.length() << " bytes of JSON parsed in " <<
        stopwatch.elapsed() / 1000 << " ms";
    logger.debug(ss.str());
}

std::string User::String() {
    std::stringstream ss;
    ss  << "ID=" << id_
        << " default_wid=" << default_wid_
        << " local_id=" << local_id_
        << " api_token=" << api_token_
        << " since=" << since_
        << " record_timeline=" << record_timeline_;
    return ss.str();
}

void User::LoadDataFromJSONNode(JSONNODE *data,
        const bool full_sync,
        const bool with_related_data) {
    poco_assert(data);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "id") == 0) {
            SetID(json_as_int(*current_node));
        } else if (strcmp(node_name, "default_wid") == 0) {
            SetDefaultWID(json_as_int(*current_node));
        } else if (strcmp(node_name, "api_token") == 0) {
            SetAPIToken(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "email") == 0) {
            SetEmail(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "fullname") == 0) {
            SetFullname(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "record_timeline") == 0) {
            SetRecordTimeline(json_as_bool(*current_node));
        } else if (with_related_data) {
            if (strcmp(node_name, "projects") == 0) {
                loadProjectsFromJSONNode(*current_node, full_sync);
            } else if (strcmp(node_name, "tags") == 0) {
                loadTagsFromJSONNode(*current_node, full_sync);
            } else if (strcmp(node_name, "tasks") == 0) {
                loadTasksFromJSONNode(*current_node, full_sync);
            } else if (strcmp(node_name, "time_entries") == 0) {
                loadTimeEntriesFromJSONNode(*current_node, full_sync);
            } else if (strcmp(node_name, "workspaces") == 0) {
                loadWorkspacesFromJSONNode(*current_node, full_sync);
            } else if (strcmp(node_name, "clients") == 0) {
                loadClientsFromJSONNode(*current_node, full_sync);
            }
        }
        ++current_node;
    }
}

void User::LoadUpdateFromJSONString(const std::string json) {
    poco_assert(!json.empty());
    JSONNODE *root = json_parse(json.c_str());
    loadUpdateFromJSONNode(root);
    json_delete(root);
}

void User::loadUpdateFromJSONNode(JSONNODE *node) {
    poco_assert(node);

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
    poco_assert(data);

    std::stringstream ss;
    ss << "Update parsed into action=" << action
        << ", model=" + model;
    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
    logger.debug(ss.str());

    if ("workspace" == model) {
        loadWorkspaceFromJSONNode(data, 0);
    } else if ("client" == model) {
        loadClientFromJSONNode(data, 0);
    } else if ("project" == model) {
        loadProjectFromJSONNode(data, 0);
    } else if ("task" == model) {
        loadTaskFromJSONNode(data, 0);
    } else if ("time_entry" == model) {
        loadTimeEntryFromJSONNode(data, 0);
    } else if ("tag" == model) {
        loadTagFromJSONNode(data, 0);
    }
}

void User::loadProjectFromJSONNode(JSONNODE *data,
        std::set<Poco::UInt64> *alive) {
  Poco::UInt64 id = getIDFromJSONNode(data);
  Project *model = GetProjectByID(id);
  if (!model) {
    if (isDeletedAtServer(data)) {
      return;
    }
    model = new Project();
    related.Projects.push_back(model);
  }
  if (alive) {
    alive->insert(id);
  }
  model->SetUID(ID());
  model->LoadFromJSONNode(data);
}

void User::loadProjectsFromJSONNode(JSONNODE *list, const bool full_sync) {
    poco_assert(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadProjectFromJSONNode(*current_node, &alive);
        ++current_node;
    }

    if (!full_sync) {
        return;
    }

    for (std::vector<Project *>::const_iterator it =
            related.Projects.begin();
            it != related.Projects.end();
            it++) {
        Project *model = *it;
        if (alive.end() == alive.find(model->ID())) {
            model->MarkAsDeletedOnServer();
        }
    }
}

std::string Project::String() {
    std::stringstream ss;
    ss << "ID=" << id_ << " name=" << name_ << " wid=" << wid_ <<
        " guid=" << guid_ << " active=" << active_;
    return ss.str();
}

std::string Project::UppercaseName() {
    return Poco::toUpper(name_);
}

void Project::SetActive(const bool value) {
    if (active_ != value) {
        active_ = value;
        dirty_ = true;
    }
}

void Project::SetName(std::string value) {
    if (name_ != value) {
        name_ = value;
        dirty_ = true;
    }
}

void Project::SetColor(std::string value) {
    if (color_ != value) {
        color_ = value;
        dirty_ = true;
    }
}

std::string Project::ColorCode() {
    int index(0);
    if (!Poco::NumberParser::tryParse(Color(), index)) {
        return color_codes.back();
    }
    if (!index) {
        return color_codes.back();
    }
    return color_codes[index % color_codes.size()];
}

void Project::SetID(Poco::UInt64 value) {
    if (id_ != value) {
        id_ = value;
        dirty_ = true;
    }
}

void Project::SetGUID(std::string value) {
    if (guid_ != value) {
        guid_ = value;
        dirty_ = true;
    }
}

void Project::SetWID(Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        dirty_ = true;
    }
}

void Project::SetCID(Poco::UInt64 value) {
    if (cid_ != value) {
        cid_ = value;
        dirty_ = true;
    }
}

void Project::SetUID(Poco::UInt64 value) {
    if (uid_ != value) {
        uid_ = value;
        dirty_ = true;
    }
}

void User::loadTaskFromJSONNode(JSONNODE *data,
        std::set<Poco::UInt64> *alive) {
  Poco::UInt64 id = getIDFromJSONNode(data);
  Task *model = GetTaskByID(id);
  if (!model) {
    if (isDeletedAtServer(data)) {
      return;
    }
    model = new Task();
    related.Tasks.push_back(model);
  }
  if (alive) {
    alive->insert(id);
  }
  model->SetUID(ID());
  model->LoadFromJSONNode(data);
}

void User::loadTasksFromJSONNode(JSONNODE *list, const bool full_sync) {
    poco_assert(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadTaskFromJSONNode(*current_node, &alive);
        ++current_node;
    }

    if (!full_sync) {
      return;
    }

    for (std::vector<Task *>::const_iterator it =
       related.Tasks.begin();
       it != related.Tasks.end();
       it++) {
      Task *model = *it;
      if (alive.end() == alive.find(model->ID())) {
        model->MarkAsDeletedOnServer();
      }
    }
}

std::string Task::String() {
    std::stringstream ss;
    ss << "ID=" << id_ << " name=" << name_ << " wid=" << wid_ <<
        " pid=" << pid_;
    return ss.str();
}

void Task::SetID(Poco::UInt64 value) {
    if (id_ != value) {
        id_ = value;
        dirty_ = true;
    }
}

void Task::SetPID(Poco::UInt64 value) {
    if (pid_ != value) {
        pid_ = value;
        dirty_ = true;
    }
}

void Task::SetWID(Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        dirty_ = true;
    }
}

void Task::SetUID(Poco::UInt64 value) {
    if (uid_ != value) {
        uid_ = value;
        dirty_ = true;
    }
}

void Task::SetName(std::string value) {
    if (name_ != value) {
        name_ = value;
        dirty_ = true;
    }
}

void User::loadWorkspaceFromJSONNode(JSONNODE *data,
        std::set<Poco::UInt64> *alive) {
  Poco::UInt64 id = getIDFromJSONNode(data);
  Workspace *model = GetWorkspaceByID(id);
  if (!model) {
    if (isDeletedAtServer(data)) {
      return;
    }
    model = new Workspace();
    related.Workspaces.push_back(model);
  }
  if (alive) {
    alive->insert(id);
  }
  model->SetUID(ID());
  model->LoadFromJSONNode(data);
}

void User::loadWorkspacesFromJSONNode(JSONNODE *list, const bool full_sync) {
    poco_assert(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
      loadWorkspaceFromJSONNode(*current_node, &alive);
        ++current_node;
    }

    if (!full_sync) {
        return;
    }

    for (std::vector<Workspace *>::const_iterator it =
       related.Workspaces.begin();
       it != related.Workspaces.end();
       it++) {
      Workspace *model = *it;
      if (alive.end() == alive.find(model->ID())) {
        model->MarkAsDeletedOnServer();
      }
    }
}

std::string Workspace::String() {
    std::stringstream ss;
    ss << "ID=" << id_ << " name=" << name_;
    return ss.str();
}

void Workspace::SetUID(Poco::UInt64 value) {
    if (uid_ != value) {
        uid_ = value;
        dirty_ = true;
    }
}

void Workspace::SetID(Poco::UInt64 value) {
    if (id_ != value) {
        id_ = value;
        dirty_ = true;
    }
}

void Workspace::SetName(std::string value) {
    if (name_ != value) {
        name_ = value;
        dirty_ = true;
    }
}

void User::loadTagFromJSONNode(JSONNODE *data,
        std::set<Poco::UInt64> *alive) {
  Poco::UInt64 id = getIDFromJSONNode(data);
  Tag *model = GetTagByID(id);
  if (!model) {
    if (isDeletedAtServer(data)) {
      return;
    }
    model = new Tag();
    related.Tags.push_back(model);
  }
  if (alive) {
    alive->insert(id);
  }
  model->SetUID(ID());
  model->LoadFromJSONNode(data);
}

void User::loadTagsFromJSONNode(JSONNODE *list, const bool full_sync) {
    poco_assert(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadTagFromJSONNode(*current_node, &alive);
        ++current_node;
    }

    if (!full_sync) {
      return;
    }

    for (std::vector<Tag *>::const_iterator it =
       related.Tags.begin();
       it != related.Tags.end();
       it++) {
      Tag *model = *it;
      if (alive.end() == alive.find(model->ID())) {
        model->MarkAsDeletedOnServer();
      }
    }
}

std::string Tag::String() {
    std::stringstream ss;
    ss << "ID=" << id_ << " name=" << name_ << " wid=" << wid_ <<
        " guid=" << guid_;
    return ss.str();
}

void Tag::SetUID(Poco::UInt64 value) {
    if (uid_ != value) {
        uid_ = value;
        dirty_ = true;
    }
}

void Tag::SetID(Poco::UInt64 value) {
    if (id_ != value) {
        id_ = value;
        dirty_ = true;
    }
}

void Tag::SetWID(Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        dirty_ = true;
    }
}

void Tag::SetName(std::string value) {
    if (name_ != value) {
        name_ = value;
        dirty_ = true;
    }
}

void Tag::SetGUID(std::string value) {
    if (guid_ != value) {
        guid_ = value;
        dirty_ = true;
    }
}

void User::loadClientFromJSONNode(JSONNODE *data,
        std::set<Poco::UInt64> *alive) {
  Poco::UInt64 id = getIDFromJSONNode(data);
  Client *model = GetClientByID(id);
  if (!model) {
    if (isDeletedAtServer(data)) {
      return;
    }
    model = new Client();
    related.Clients.push_back(model);
  }
  if (alive) {
    alive->insert(id);
  }
  model->SetUID(ID());
  model->LoadFromJSONNode(data);
}

void User::loadClientsFromJSONNode(JSONNODE *list, const bool full_sync) {
    poco_assert(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadClientFromJSONNode(*current_node, &alive);
        ++current_node;
    }

    if (!full_sync) {
        return;
    }

    for (std::vector<Client *>::const_iterator it =
       related.Clients.begin();
       it != related.Clients.end();
       it++) {
      Client *model = *it;
      if (alive.end() == alive.find(model->ID())) {
        model->MarkAsDeletedOnServer();
      }
    }
}

std::string Client::String() {
    std::stringstream ss;
    ss << "ID=" << id_ << " name=" << name_ << " wid=" << wid_ <<
        " guid=" << guid_;
    return ss.str();
}

void Client::SetID(Poco::UInt64 value) {
    if (id_ != value) {
        id_ = value;
        dirty_ = true;
    }
}

void Client::SetName(std::string value) {
    if (name_ != value) {
        name_ = value;
        dirty_ = true;
    }
}

void Client::SetUID(Poco::UInt64 value) {
    if (uid_ != value) {
        uid_ = value;
        dirty_ = true;
    }
}

void Client::SetGUID(std::string value) {
    if (guid_ != value) {
        guid_ = value;
        dirty_ = true;
    }
}

void Client::SetWID(Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        dirty_ = true;
    }
}

Poco::UInt64 getIDFromJSONNode(JSONNODE *data) {
    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "id") == 0) {
            return json_as_int(*current_node);
        }
        ++current_node;
    }
    poco_assert(false);
    return 0;
}

bool isDeletedAtServer(JSONNODE *data) {
    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "server_deleted_at") == 0) {
            return true;
        }
        ++current_node;
    }
    return false;
}

void User::loadTimeEntriesFromJSONNode(JSONNODE *list, const bool full_sync) {
    poco_assert(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadTimeEntryFromJSONNode(*current_node, &alive);
        ++current_node;
    }

    if (!full_sync) {
        return;
    }

    for (std::vector<TimeEntry *>::const_iterator it =
       related.TimeEntries.begin();
       it != related.TimeEntries.end();
       it++) {
      TimeEntry *model = *it;
      if (alive.end() == alive.find(model->ID())) {
        model->MarkAsDeletedOnServer();
      }
    }
}

void User::loadTimeEntryFromJSONNode(JSONNODE *data,
        std::set<Poco::UInt64> *alive) {
    poco_assert(data);

    Poco::UInt64 id = getIDFromJSONNode(data);
    TimeEntry *model = GetTimeEntryByID(id);
    if (!model) {
        if (isDeletedAtServer(data)) {
            return;
        }
        model = new TimeEntry();
        related.TimeEntries.push_back(model);
    }
    if (alive) {
      alive->insert(id);
    }
    model->SetUID(ID());
    model->LoadFromJSONNode(data);
}

std::string TimeEntry::String() {
    std::stringstream ss;
    ss  << "ID=" << id_
        << " description=" << description_
        << " wid=" << wid_
        << " guid=" << guid_
        << " pid=" << pid_
        << " tid=" << tid_
        << " start=" << start_
        << " stop=" << stop_
        << " duration=" << duration_in_seconds_
        << " billable=" << billable_
        << " duronly=" << duronly_
        << " tags=" << Tags()
        << " created_with=" << CreatedWith()
        << " ui_modified_at=" << ui_modified_at_
        << " deleted_at=" << deleted_at_
        << " updated_at=" << updated_at_;
    return ss.str();
}

JSONNODE *TimeEntry::JSON() {
    JSONNODE *n = json_new(JSON_NODE);
    json_set_name(n, "time_entry");
    if (id_) {
        json_push_back(n, json_new_i("id", (json_int_t)id_));
    }
    json_push_back(n, json_new_a("description", description_.c_str()));
    json_push_back(n, json_new_i("wid", (json_int_t)wid_));
    json_push_back(n, json_new_a("guid", guid_.c_str()));
    json_push_back(n, json_new_i("pid", (json_int_t)pid_));
    json_push_back(n, json_new_i("tid", (json_int_t)tid_));
    json_push_back(n, json_new_a("start", StartString().c_str()));
    if (stop_) {
        json_push_back(n, json_new_a("stop", StopString().c_str()));
    }
    json_push_back(n, json_new_i("duration", (json_int_t)duration_in_seconds_));
    json_push_back(n, json_new_b("billable", billable_));
    json_push_back(n, json_new_b("duronly", duronly_));
    json_push_back(n, json_new_i("ui_modified_at",
        (json_int_t)ui_modified_at_));
    json_push_back(n, json_new_a("created_with", created_with_.c_str()));

    if (!TagNames.empty()) {
        JSONNODE *tag_nodes = json_new(JSON_ARRAY);
        json_set_name(tag_nodes, "tags");
        for (std::vector<std::string>::const_iterator it = TagNames.begin();
                it != TagNames.end();
                it++) {
            std::string tag_name = *it;
            json_push_back(tag_nodes, json_new_a(NULL, tag_name.c_str()));
        }
        json_push_back(n, tag_nodes);
    }

    return n;
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

Project *User::GetProjectByName(const std::string name) {
    for (std::vector<Project *>::const_iterator it = related.Projects.begin();
            it != related.Projects.end(); it++) {
        if ((*it)->Name() == name) {
            return *it;
        }
    }
    return 0;
}

Project *User::GetProjectByNameIncludingClient(
        const std::string name_with_client) {
    for (std::vector<Project *>::const_iterator it = related.Projects.begin();
            it != related.Projects.end(); it++) {
        Project *p = *it;
        if (ProjectNameIncludingClient(p) == name_with_client) {
            return p;
        }
    }
    return 0;
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

TimeEntry *User::GetTimeEntryByGUID(std::string GUID) {
    poco_assert(!GUID.empty());
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

void Workspace::LoadFromJSONNode(JSONNODE *n) {
    poco_assert(n);
    JSONNODE_ITERATOR i = json_begin(n);
    JSONNODE_ITERATOR e = json_end(n);
    while (i != e) {
        json_char *node_name = json_name(*i);
        if (strcmp(node_name, "id") == 0) {
            SetID(json_as_int(*i));
        } else if (strcmp(node_name, "name") == 0) {
            SetName(std::string(json_as_string(*i)));
        }
        ++i;
    }
}

void Client::LoadFromJSONNode(JSONNODE *data) {
    poco_assert(data);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "id") == 0) {
            SetID(json_as_int(*current_node));
        } else if (strcmp(node_name, "name") == 0) {
            SetName(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "guid") == 0) {
            SetGUID(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "wid") == 0) {
            SetWID(json_as_int(*current_node));
        }
        ++current_node;
    }
}

void Project::LoadFromJSONNode(JSONNODE *data) {
    poco_assert(data);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "id") == 0) {
            SetID(json_as_int(*current_node));
        } else if (strcmp(node_name, "name") == 0) {
            SetName(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "guid") == 0) {
            SetGUID(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "wid") == 0) {
            SetWID(json_as_int(*current_node));
        } else if (strcmp(node_name, "cid") == 0) {
            SetCID(json_as_int(*current_node));
        } else if (strcmp(node_name, "color") == 0) {
            SetColor(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "active") == 0) {
            SetActive(json_as_bool(*current_node));
        }
        ++current_node;
    }
}

void Task::LoadFromJSONNode(JSONNODE *data) {
    poco_assert(data);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "id") == 0) {
            SetID(json_as_int(*current_node));
        } else if (strcmp(node_name, "name") == 0) {
            SetName(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "pid") == 0) {
            SetPID(json_as_int(*current_node));
        } else if (strcmp(node_name, "wid") == 0) {
            SetWID(json_as_int(*current_node));
        }
        ++current_node;
    }
}

void Tag::LoadFromJSONNode(JSONNODE *data) {
    poco_assert(data);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "id") == 0) {
            SetID(json_as_int(*current_node));
        } else if (strcmp(node_name, "name") == 0) {
            SetName(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "guid") == 0) {
            SetGUID(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "wid") == 0) {
            SetWID(json_as_int(*current_node));
        }
        ++current_node;
    }
}

std::string TimeEntry::StartString() {
     return Formatter::Format8601(start_);
}

void TimeEntry::SetStartString(std::string value) {
    SetStart(Formatter::Parse8601(value));
}

std::string TimeEntry::DateHeaderString() {
    return Formatter::FormatDateHeader(start_);
}

void TimeEntry::SetUpdatedAtString(std::string value) {
    SetUpdatedAt(Formatter::Parse8601(value));
}

std::string TimeEntry::UpdatedAtString() {
    return Formatter::Format8601(updated_at_);
}

std::string TimeEntry::DurationString() {
    return Formatter::FormatDurationInSecondsHHMMSS(duration_in_seconds_);
}

void TimeEntry::SetDurationString(std::string value) {
    // Parse duration in sconds HH:MM:SS
    Poco::RegularExpression re(":");
    Poco::StringTokenizer tokenizer(value, ":");
    std::cout << "token count=" << tokenizer.count() << std::endl;
    if (3 == tokenizer.count()) {
        int hours = 0;
        if (!Poco::NumberParser::tryParse(tokenizer[0], hours)) {
            return;
        }
        int minutes = 0;
        if (!Poco::NumberParser::tryParse(tokenizer[1], minutes)) {
            return;
        }
        int seconds = 0;
        if (!Poco::NumberParser::tryParse(tokenizer[2], seconds)) {
            return;
        }
        std::cout << "hours=" << hours << ", minutes=" << minutes
            << ", seconds=" << seconds << std::endl;
        Poco::Timespan span(0, hours, minutes, seconds, 0);
        std::cout << "span total seconds=" << span.totalSeconds() << std::endl;
        SetDurationInSeconds(span.totalSeconds());
    }
    // FIXME: parse duration string into duration in seconds
}

void TimeEntry::SetCreatedWith(std::string value) {
    if (created_with_ != value) {
        created_with_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetDurOnly(bool value) {
    if (duronly_ != value) {
        duronly_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetBillable(bool value) {
    if (billable_ != value) {
        billable_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetDurationInSeconds(Poco::Int64 value) {
    if (duration_in_seconds_ != value) {
        duration_in_seconds_ = value;
        dirty_ = true;
    }
}

std::string TimeEntry::StopString() {
     return Formatter::Format8601(stop_);
}

void TimeEntry::SetStopString(std::string value) {
    SetStop(Formatter::Parse8601(value));
}

void TimeEntry::SetStop(Poco::UInt64 value) {
    if (stop_ != value) {
        stop_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetDeletedAt(Poco::UInt64 value) {
    if (deleted_at_ != value) {
        deleted_at_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetStart(Poco::UInt64 value) {
    if (start_ != value) {
        start_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetUpdatedAt(Poco::UInt64 value) {
    if (updated_at_ != value) {
        updated_at_ = value;
        dirty_ = true;
    }
}

std::string TimeEntry::Tags() {
    std::stringstream ss;
    for (std::vector<std::string>::const_iterator it =
            TagNames.begin();
            it != TagNames.end();
            it++) {
        if (it != TagNames.begin()) {
            ss << "|";
        }
        ss << *it;
    }
    return ss.str();
}

void TimeEntry::SetTags(std::string tags) {
    if (Tags() != tags) {
        TagNames.clear();
        if (!tags.empty()) {
          std::stringstream ss(tags);
          while (ss.good()) {
              std::string tag;
              getline(ss, tag, '|');
              TagNames.push_back(tag);
          }
        }
        dirty_ = true;
    }
}

Poco::UInt64 TimeEntry::getUIModifiedAtFromJSONNode(JSONNODE *data) {
    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "ui_modified_at") == 0) {
            return json_as_int(*current_node);
        }
        ++current_node;
    }
    return 0;
}

void TimeEntry::LoadFromJSONString(std::string json) {
    poco_assert(!json.empty());
    JSONNODE *root = json_parse(json.c_str());
    LoadFromJSONNode(root);
    json_delete(root);
}

void TimeEntry::LoadFromJSONNode(JSONNODE *data) {
    poco_assert(data);

    Poco::UInt64 ui_modified_at =
        getUIModifiedAtFromJSONNode(data);
    if (UIModifiedAt() > ui_modified_at) {
        Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
        std::stringstream ss;
        ss  << "Will not overwrite time entry "
            << String()
            << " with server data because we have a ui_modified_at";
        logger.debug(ss.str());
        return;
    }

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "id") == 0) {
            SetID(json_as_int(*current_node));
        } else if (strcmp(node_name, "description") == 0) {
            SetDescription(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "guid") == 0) {
            SetGUID(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "wid") == 0) {
            SetWID(json_as_int(*current_node));
        } else if (strcmp(node_name, "pid") == 0) {
            SetPID(json_as_int(*current_node));
        } else if (strcmp(node_name, "tid") == 0) {
            SetTID(json_as_int(*current_node));
        } else if (strcmp(node_name, "start") == 0) {
            SetStartString(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "stop") == 0) {
            SetStopString(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "duration") == 0) {
            SetDurationInSeconds(json_as_int(*current_node));
        } else if (strcmp(node_name, "billable") == 0) {
            SetBillable(json_as_bool(*current_node));
        } else if (strcmp(node_name, "duronly") == 0) {
            SetDurOnly(json_as_bool(*current_node));
        } else if (strcmp(node_name, "tags") == 0) {
            loadTagsFromJSONNode(*current_node);
        } else if (strcmp(node_name, "created_with") == 0) {
            SetCreatedWith(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "at") == 0) {
            SetUpdatedAtString(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "server_deleted_at") == 0) {
            MarkAsDeletedOnServer();
        }
        ++current_node;
    }

    SetUIModifiedAt(0);
}

void TimeEntry::SetWID(Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetID(Poco::UInt64 value) {
    if (id_ != value) {
        id_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetPID(Poco::UInt64 value) {
    if (pid_ != value) {
        pid_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetTID(Poco::UInt64 value) {
    if (tid_ != value) {
        tid_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetUID(Poco::UInt64 value) {
    if (uid_ != value) {
        uid_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetDescription(std::string value) {
    if (description_ != value) {
        description_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetGUID(std::string value) {
    if (guid_ != value) {
        guid_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetUIModifiedAt(Poco::UInt64 value) {
    if (ui_modified_at_ != value) {
        ui_modified_at_ = value;
        dirty_ = true;
    }
}

error TimeEntry::loadTagsFromJSONNode(JSONNODE *list) {
    poco_assert(list);

    TagNames.clear();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        std::string tag = std::string(json_as_string(*current_node));
        if (!tag.empty()) {
            TagNames.push_back(tag);
        }
        ++current_node;
    }
    return noError;
}

std::string Formatter::FormatDateHeader(std::time_t date) {
    poco_assert(date);

    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(date);
    Poco::DateTime datetime(ts);

    Poco::DateTime today;
    if (today.year() == datetime.year() &&
            today.month() == datetime.month() &&
            today.day() == datetime.day()) {
        return "TODAY";
    }

    Poco::DateTime yesterday = today -
        Poco::Timespan(24 * Poco::Timespan::HOURS);
    if (yesterday.year() == datetime.year() &&
            yesterday.month() == datetime.month() &&
            yesterday.day() == datetime.day()) {
        return "YESTERDAY";
    }

    std::string formatted =
        Poco::DateTimeFormatter::format(ts, "%w %d. %b");
    return Poco::toUpper(formatted);
}

std::string Formatter::FormatDurationInSeconds(const Poco::Int64 value,
        const std::string format) {
    Poco::Int64 duration = value;
    if (duration < 0) {
        duration += time(0);
    }
    Poco::Timespan span(duration * Poco::Timespan::SECONDS);
    return Poco::DateTimeFormatter::format(span, format);
}

std::string Formatter::FormatDurationInSecondsHHMMSS(const Poco::Int64 value) {
    return FormatDurationInSeconds(value, "%H:%M:%S");
}

std::string Formatter::FormatDurationInSecondsHHMM(const Poco::Int64 value,
        const bool is_blink) {
    if (is_blink) {
        return FormatDurationInSeconds(value, "%H %M");
    }
    return FormatDurationInSeconds(value, "%H:%M");
}

std::time_t Formatter::Parse8601(std::string iso_8601_formatted_date) {
    if ("null" == iso_8601_formatted_date) {
        return 0;
    }
    int tzd;
    Poco::DateTime dt;
    Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT,
        iso_8601_formatted_date, dt, tzd);
    dt.makeUTC(tzd);
    Poco::Timestamp ts = dt.timestamp();
    return ts.epochTime();
}

std::string Formatter::Format8601(std::time_t date) {
    if (!date) {
        return "null";
    }
    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(date);
    return Poco::DateTimeFormatter::format(ts,
        Poco::DateTimeFormat::ISO8601_FORMAT);
}

}   // namespace kopsik
