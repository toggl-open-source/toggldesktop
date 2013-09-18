// Copyright 2013 Tanel Lebedev

#include "./toggl_api_client.h"

#include <string>
#include <sstream>

#include "Poco/Stopwatch.h"
#include "Poco/Bugcheck.h"
#include "Poco/Exception.h"
#include "Poco/InflatingStream.h"
#include "Poco/DeflatingStream.h"
#include "Poco/Logger.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/DateTimeParser.h"
#include "Poco/DateTime.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/HTTPSClientSession.h"

#include "./libjson.h"

namespace kopsik {

    // const std::string TOGGL_SERVER_URL("https://www.toggl.com");
    const std::string TOGGL_SERVER_URL("http://localhost:8080");

// Start a time entry, mark it as dirty and add to user time entry collection.
// Do not save here, dirtyness will be handled outside of this module.
TimeEntry *User::Start() {
    Stop();
    TimeEntry *te = new TimeEntry();
    te->UID = this->ID;
    te->Start = time(0);
    te->DurationInSeconds = -time(0);
    te->WID = this->DefaultWID;
    te->Dirty = true;
    te->UIModifiedAt = time(0);
    TimeEntries.push_back(te);
    return te;
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
        te->DurationInSeconds = time(0) + te->DurationInSeconds;
        te->Dirty = true;
        te->UIModifiedAt = time(0);
        te = RunningTimeEntry();
    }
    return result;
}

TimeEntry *User::RunningTimeEntry() {
    for (std::vector<TimeEntry *>::const_iterator it =
            this->TimeEntries.begin();
            it != this->TimeEntries.end(); it++) {
        if ((*it)->DurationInSeconds < 0) {
            return *it;
        }
    }
    return 0;
}

bool TimeEntry::NeedsPush() {
    return (UIModifiedAt > 0) || !ID;
}

error User::Push() {
    Poco::Stopwatch stopwatch;
    stopwatch.start();

    // Collect dirty objects
    std::vector<TimeEntry *>dirty;
    for (std::vector<TimeEntry *>::const_iterator it =
            this->TimeEntries.begin();
            it != this->TimeEntries.end(); it++) {
        TimeEntry *te = *it;
        if (te->NeedsPush()) {
            dirty.push_back(te);
        }
    }

    // Convert the dirty objcets to batch updates JSON
    JSONNODE *c = json_new(JSON_ARRAY);
    for (std::vector<TimeEntry *>::const_iterator it =
            dirty.begin();
            it != dirty.end(); it++) {
        TimeEntry *te = *it;
        JSONNODE *n = te->JSON();
        json_set_name(n, "time_entry");

        JSONNODE *body = json_new(JSON_NODE);
        json_set_name(body, "body");
        json_push_back(body, n);

        JSONNODE *update = json_new(JSON_NODE);
        json_push_back(update, json_new_a("method", "POST"));
        json_push_back(update, json_new_a("relative_url",
            "/api/v8/time_entries"));
        json_push_back(update, body);

        json_push_back(c, update);
    }
    json_char *jc = json_write_formatted(c);
    std::string json(jc);
    json_free(jc);
    json_delete(c);

    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
    logger.debug(json);

    std::string response_body("");
    error err = requestJSON(Poco::Net::HTTPRequest::HTTP_POST,
        "/api/v8/batch_updates",
        json,
        &response_body);
    if (err != noError) {
        return err;
    }

    std::vector<BatchUpdateResult> results;
    parseResponseArray(response_body, &results);
    // Iterate through response array, parse response bodies.
    // Collect errors into a vector.
    std::vector<error> errors;
    int response_index = 0;
    for (std::vector<BatchUpdateResult>::const_iterator it = results.begin();
            it != results.end();
            it++) {
        BatchUpdateResult result = *it;

        std::stringstream ss;
        ss << "batch update result status " << result.StatusCode
            << ", body " << result.Body;
        logger.error(ss.str());

        JSONNODE *n = json_parse(result.Body.c_str());
        JSONNODE_ITERATOR i = json_begin(n);
        JSONNODE_ITERATOR e = json_end(n);
        while (i != e) {
            json_char *node_name = json_name(*i);
            if (strcmp(node_name, "data") == 0) {
                TimeEntry *te = dirty[response_index];
                poco_assert(te);
                error err = te->Load(*i);
                if (err != noError) {
                    errors.push_back(err);
                }
            }
            ++i;
        }
        json_delete(n);

        response_index++;
    }

    // Collect errors
    if (!errors.empty()) {
        for (std::vector<error>::const_iterator it = errors.begin();
                it != errors.end();
                it++) {
            error err = *it;
            logger.error(err);
        }

        std::stringstream ss;
        std::copy(errors.begin(), errors.end(),
            std::ostream_iterator<std::string>(ss, ". "));
        return error(ss.str());
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
    this->StatusCode = 0;
    this->Body = "";
    JSONNODE_ITERATOR i = json_begin(n);
    JSONNODE_ITERATOR e = json_end(n);
    while (i != e) {
        json_char *node_name = json_name(*i);
        if (strcmp(node_name, "status") == 0) {
            this->StatusCode = json_as_int(*i);
        } else if (strcmp(node_name, "body") == 0) {
            this->Body = std::string(json_as_string(*i));
        }
        ++i;
    }
}

error User::requestJSON(std::string method, std::string relative_url,
        std::string json, std::string *response_body) {
    poco_assert(!method.empty());
    poco_assert(!relative_url.empty());
    poco_assert(response_body);
    *response_body = "";
    try {
        const Poco::URI uri(TOGGL_SERVER_URL);

        const Poco::Net::Context::Ptr context(new Poco::Net::Context(
            Poco::Net::Context::CLIENT_USE, "", "", "",
            Poco::Net::Context::VERIFY_NONE, 9, false,
            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"));

        Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(),
            context);
        session.setKeepAlive(false);

        std::istringstream requestStream(json);
        Poco::DeflatingInputStream gzipRequest(requestStream,
            Poco::DeflatingStreamBuf::STREAM_GZIP);
        Poco::DeflatingStreamBuf *pBuff = gzipRequest.rdbuf();

        Poco::Int64 size = pBuff->pubseekoff(0, std::ios::end, std::ios::in);
        pBuff->pubseekpos(0, std::ios::in);

        Poco::Net::HTTPRequest req(method,
            relative_url, Poco::Net::HTTPMessage::HTTP_1_1);
        req.setKeepAlive(false);
        req.setContentType("application/json");
        req.setContentLength(size);
        req.set("Content-Encoding", "gzip");
        req.set("Accept-Encoding", "gzip");
        req.setChunkedTransferEncoding(true);

        Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
        logger.debug("Sending request..");

        Poco::Net::HTTPBasicCredentials cred(APIToken, "api_token");
        cred.authenticate(req);
        session.sendRequest(req) << pBuff << std::flush;

        // Log out request contents
        std::stringstream request_string;
        req.write(request_string);
        logger.debug(request_string.str());

        logger.debug("Request sent. Receiving response..");

        // Receive response
        Poco::Net::HTTPResponse response;
        std::istream& is = session.receiveResponse(response);

        // Inflate
        Poco::InflatingInputStream inflater(is,
            Poco::InflatingStreamBuf::STREAM_GZIP);
        std::stringstream ss;
        ss << inflater.rdbuf();
        *response_body = ss.str();

        // Log out response contents
        std::stringstream response_string;
        response_string << "Response status: " << response.getStatus()
            << ", reason: " << response.getReason()
            << ", Content type: " << response.getContentType()
            << ", Content-Encoding: " << response.get("Content-Encoding");
        logger.debug(response_string.str());
        logger.debug(*response_body);

        if (!isStatusOK(response.getStatus())) {
            // FIXME: backoff
            return "Data push failed with error: " + *response_body;
        }

        // FIXME: reset backoff
    } catch(const Poco::Exception& exc) {
        // FIXME: backoff
        return exc.displayText();
    } catch(const std::exception& ex) {
        // FIXME: backoff
        return ex.what();
    } catch(const std::string& ex) {
        // FIXME: backoff
        return ex;
    }
    return noError;
}

bool User::isStatusOK(int status) {
    return status >= 200 && status < 300;
}

// FIXME: move code into a GET method
error User::Pull() {
    Poco::Stopwatch stopwatch;
    stopwatch.start();

    std::string response_body("");
    error err = requestJSON(Poco::Net::HTTPRequest::HTTP_GET,
        "/api/v8/me?with_related_data=true",
        "",
        &response_body);
    if (err != noError) {
        return err;
    }

    err = this->Load(response_body);
    if (err != noError) {
        return err;
    }

    stopwatch.stop();
    std::stringstream ss;
    ss << "User with related data JSON fetched and parsed in "
        << stopwatch.elapsed() / 1000 << " ms";
    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
    logger.debug(ss.str());

    return noError;
};

error User::Load(const std::string &json) {
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
            this->Since = json_as_int(*current_node);
            std::stringstream s;
            s << "User data as of: " << this->Since;
            logger.debug(s.str());
        } else if (strcmp(node_name, "data") == 0) {
            error err = this->Load(*current_node);
            if (err != noError) {
                return err;
            }
        }
        ++current_node;
    }
    json_delete(root);

    stopwatch.stop();
    std::stringstream ss;
    ss << json.length() << " bytes of JSON parsed in " <<
        stopwatch.elapsed() / 1000 << " ms";
    logger.debug(ss.str());

    return noError;
}

std::string User::String() {
    std::stringstream ss;
    ss << "ID=" << ID <<
        " default_wid=" << DefaultWID <<
        " local_id=" << LocalID <<
        " api_token=" << APIToken <<
        " since=" << Since;
    return ss.str();
}

error User::Load(JSONNODE *data) {
    poco_assert(data);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        error err = noError;
        if (strcmp(node_name, "id") == 0) {
            this->ID = json_as_int(*current_node);
        } else if (strcmp(node_name, "default_wid") == 0) {
            this->DefaultWID = json_as_int(*current_node);
        } else if (strcmp(node_name, "api_token") == 0) {
            this->APIToken = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "projects") == 0) {
            err = this->loadProjects(*current_node);
        } else if (strcmp(node_name, "tags") == 0) {
            err = this->loadTags(*current_node);
        } else if (strcmp(node_name, "tasks") == 0) {
            err = this->loadTasks(*current_node);
        } else if (strcmp(node_name, "time_entries") == 0) {
            err = this->loadTimeEntries(*current_node);
        } else if (strcmp(node_name, "workspaces") == 0) {
            err = this->loadWorkspaces(*current_node);
        } else if (strcmp(node_name, "clients") == 0) {
            err = this->loadClients(*current_node);
        }
        if (err != noError) {
            return err;
        }
        ++current_node;
    }
    return noError;
}

error User::loadProjects(JSONNODE *list) {
    poco_assert(list);

    ClearProjects();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        Project *model = new Project();
        model->UID = this->ID;
        error err = model->Load(*current_node);
        if (err != noError) {
            delete model;
            return err;
        }
        this->Projects.push_back(model);
        ++current_node;
    }
    return noError;
}

std::string Project::String() {
    std::stringstream ss;
    ss << "ID=" << ID << " name=" << Name << " wid=" << WID << " guid=" << GUID;
    return ss.str();
}

error User::loadTasks(JSONNODE *list) {
    poco_assert(list);

    ClearTasks();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        Task *model = new Task();
        model->UID = this->ID;
        error err = model->Load(*current_node);
        if (err != noError) {
            delete model;
            return err;
        }
        this->Tasks.push_back(model);
        ++current_node;
    }
    return noError;
}

std::string Task::String() {
    std::stringstream ss;
    ss << "ID=" << ID << " name=" << Name << " wid=" << WID << " pid=" << PID;
    return ss.str();
}

error User::loadWorkspaces(JSONNODE *list) {
    poco_assert(list);

    ClearWorkspaces();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        Workspace *model = new Workspace();
        model->UID = this->ID;
        error err = model->Load(*current_node);
        if (err != noError) {
            delete model;
            return err;
        }
        this->Workspaces.push_back(model);
        ++current_node;
    }
    return noError;
}

std::string Workspace::String() {
    std::stringstream ss;
    ss << "ID=" << ID << " name=" << Name;
    return ss.str();
}

error User::loadTags(JSONNODE *list) {
    poco_assert(list);

    ClearTags();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        Tag *model = new Tag();
        model->UID = this->ID;
        error err = model->Load(*current_node);
        if (err != noError) {
            delete model;
            return err;
        }
        this->Tags.push_back(model);
        ++current_node;
    }
    return noError;
}

std::string Tag::String() {
    std::stringstream ss;
    ss << "ID=" << ID << " name=" << Name << " wid=" << WID << " guid=" << GUID;
    return ss.str();
}

error User::loadClients(JSONNODE *list) {
    poco_assert(list);

    ClearClients();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        Client *model = new Client();
        model->UID = this->ID;
        error err = model->Load(*current_node);
        if (err != noError) {
            delete model;
            return err;
        }
        this->Clients.push_back(model);
        ++current_node;
    }
    return noError;
}

std::string Client::String() {
    std::stringstream ss;
    ss << "ID=" << ID << " name=" << Name << " wid=" << WID << " guid=" << GUID;
    return ss.str();
}

error User::loadTimeEntries(JSONNODE *list) {
    poco_assert(list);

    ClearTimeEntries();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        TimeEntry *model = new TimeEntry();
        model->UID = this->ID;
        error err = model->Load(*current_node);
        if (err != noError) {
            delete model;
            return err;
        }
        this->TimeEntries.push_back(model);
        ++current_node;
    }
    return noError;
}

error User::loadTimeEntries(Poco::Data::Statement *select) {
    poco_assert(select);
    Poco::Data::RecordSet rs(*select);
    while (!select->done()) {
        select->execute();
        bool more = rs.moveFirst();
        while (more) {
            TimeEntry *model = new TimeEntry();
            model->LocalID = rs[0].convert<Poco::Int64>();
            model->ID = rs[1].convert<Poco::UInt64>();
            model->UID = rs[2].convert<Poco::UInt64>();
            model->Description = rs[3].convert<std::string>();
            model->WID = rs[4].convert<Poco::UInt64>();
            model->GUID = rs[5].convert<std::string>();
            model->PID = rs[6].convert<Poco::UInt64>();
            model->TID = rs[7].convert<Poco::UInt64>();
            model->Billable = rs[8].convert<bool>();
            model->DurOnly = rs[9].convert<bool>();
            model->UIModifiedAt = rs[10].convert<Poco::UInt64>();
            model->Start = rs[11].convert<Poco::UInt64>();
            model->Stop = rs[12].convert<Poco::UInt64>();
            model->DurationInSeconds = rs[13].convert<Poco::Int64>();
            model->SetTags(rs[14].convert<std::string>());
            TimeEntries.push_back(model);
            more = rs.moveNext();
        }
    }
    return noError;
}

std::string TimeEntry::String() {
    std::stringstream ss;
    ss << "ID=" << ID <<
    " description=" << Description <<
    " wid=" << WID <<
    " guid=" << GUID <<
    " pid=" << PID <<
    " tid=" << TID <<
    " start=" << Start <<
    " stop=" << Stop <<
    " duration=" << DurationInSeconds <<
    " billable=" << Billable <<
    " duronly=" << DurOnly <<
    " ui_modified_at=" << UIModifiedAt;
    return ss.str();
}

JSONNODE *TimeEntry::JSON() {
    JSONNODE *n = json_new(JSON_NODE);
    json_set_name(n, "time_entry");
    if (ID > 0) {
        json_push_back(n, json_new_i("id", (json_int_t)ID));
    }
    json_push_back(n, json_new_a("description", Description.c_str()));
    json_push_back(n, json_new_i("wid", (json_int_t)WID));
    if (!GUID.empty()) {
        json_push_back(n, json_new_a("guid", GUID.c_str()));
    }
    if (PID > 0) {
        json_push_back(n, json_new_i("pid", (json_int_t)PID));
    }
    if (TID > 0) {
        json_push_back(n, json_new_i("tid", (json_int_t)TID));
    }
    json_push_back(n, json_new_a("start", StartString().c_str()));
    if (Stop > 0) {
        json_push_back(n, json_new_a("stop", StopString().c_str()));
    }
    json_push_back(n, json_new_i("duration", (json_int_t)DurationInSeconds));
    json_push_back(n, json_new_b("billable", Billable));
    json_push_back(n, json_new_b("duronly", DurOnly));
    json_push_back(n, json_new_i("ui_modified_at", (json_int_t)UIModifiedAt));

    return n;
}

// FIXME: use map instead?

Workspace *User::GetWorkspaceByID(const Poco::UInt64 id) {
    poco_assert(id > 0);
    for (std::vector<Workspace *>::const_iterator it = this->Workspaces.begin();
            it != this->Workspaces.end(); it++) {
        if ((*it)->ID == id) {
            return *it;
        }
    }
    return 0;
}

Client *User::GetClientByID(const Poco::UInt64 id) {
    poco_assert(id > 0);
    for (std::vector<Client *>::const_iterator it = this->Clients.begin();
            it != this->Clients.end(); it++) {
        if ((*it)->ID == id) {
            return *it;
        }
    }
    return 0;
}

Project *User::GetProjectByID(const Poco::UInt64 id) {
    poco_assert(id > 0);
    for (std::vector<Project *>::const_iterator it = this->Projects.begin();
            it != this->Projects.end(); it++) {
        if ((*it)->ID == id) {
            return *it;
        }
    }
    return 0;
}

Task *User::GetTaskByID(const Poco::UInt64 id) {
    poco_assert(id > 0);
    for (std::vector<Task *>::const_iterator it = this->Tasks.begin();
            it != this->Tasks.end(); it++) {
        if ((*it)->ID == id) {
            return *it;
        }
    }
    return 0;
}

Tag *User::GetTagByID(const Poco::UInt64 id) {
    poco_assert(id > 0);
    for (std::vector<Tag *>::const_iterator it = this->Tags.begin();
            it != this->Tags.end(); it++) {
        if ((*it)->ID == id) {
            return *it;
        }
    }
    return 0;
}

TimeEntry *User::GetTimeEntryByID(const Poco::UInt64 id) {
    poco_assert(id > 0);
    for (std::vector<TimeEntry *>::const_iterator it =
            this->TimeEntries.begin(); it != this->TimeEntries.end(); it++) {
        if ((*it)->ID == id) {
            return *it;
        }
    }
    return 0;
}

void User::ClearWorkspaces() {
    for (std::vector<Workspace *>::const_iterator it = this->Workspaces.begin();
            it != this->Workspaces.end(); it++) {
        delete *it;
    }
    this->Workspaces.clear();
}

void User::ClearProjects() {
    for (std::vector<Project *>::const_iterator it = this->Projects.begin();
            it != this->Projects.end(); it++) {
        delete *it;
    }
    this->Projects.clear();
}

void User::ClearTasks() {
    for (std::vector<Task *>::const_iterator it = this->Tasks.begin();
            it != this->Tasks.end(); it++) {
        delete *it;
    }
    this->Tasks.clear();
}

void User::ClearTags() {
    for (std::vector<Tag *>::const_iterator it = this->Tags.begin();
            it != this->Tags.end(); it++) {
        delete *it;
    }
    this->Tags.clear();
}

void User::ClearClients() {
    for (std::vector<Client *>::const_iterator it = this->Clients.begin();
            it != this->Clients.end(); it++) {
        delete *it;
    }
    this->Clients.clear();
}

void User::ClearTimeEntries() {
    for (std::vector<TimeEntry *>::const_iterator it =
            this->TimeEntries.begin(); it != this->TimeEntries.end(); it++) {
        delete *it;
    }
    this->TimeEntries.clear();
}

error Workspace::Load(JSONNODE *n) {
    poco_assert(n);
    JSONNODE_ITERATOR i = json_begin(n);
    JSONNODE_ITERATOR e = json_end(n);
    while (i != e) {
        json_char *node_name = json_name(*i);
        error err = noError;
        if (strcmp(node_name, "id") == 0) {
            this->ID = json_as_int(*i);
        } else if (strcmp(node_name, "name") == 0) {
            this->Name = std::string(json_as_string(*i));
        }
        ++i;
    }
    return noError;
}

error Client::Load(JSONNODE *data) {
    poco_assert(data);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        error err = noError;
        if (strcmp(node_name, "id") == 0) {
            this->ID = json_as_int(*current_node);
        } else if (strcmp(node_name, "name") == 0) {
            this->Name = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "guid") == 0) {
            this->GUID = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "wid") == 0) {
        this->WID = json_as_int(*current_node);
        }
        ++current_node;
    }
    return noError;
}

error Project::Load(JSONNODE *data) {
    poco_assert(data);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        error err = noError;
        if (strcmp(node_name, "id") == 0) {
            this->ID = json_as_int(*current_node);
        } else if (strcmp(node_name, "name") == 0) {
            this->Name = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "guid") == 0) {
            this->GUID = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "wid") == 0) {
            this->WID = json_as_int(*current_node);
        } else if (strcmp(node_name, "cid") == 0) {
            this->CID = json_as_int(*current_node);
        }
        ++current_node;
    }
    return noError;
}

error Task::Load(JSONNODE *data) {
    poco_assert(data);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        error err = noError;
        if (strcmp(node_name, "id") == 0) {
            this->ID = json_as_int(*current_node);
        } else if (strcmp(node_name, "name") == 0) {
            this->Name = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "pid") == 0) {
            this->PID = json_as_int(*current_node);
        } else if (strcmp(node_name, "wid") == 0) {
            this->WID = json_as_int(*current_node);
        }
        ++current_node;
    }
    return noError;
}

error Tag::Load(JSONNODE *data) {
    poco_assert(data);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "id") == 0) {
            this->ID = json_as_int(*current_node);
        } else if (strcmp(node_name, "name") == 0) {
            this->Name = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "guid") == 0) {
            this->GUID = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "wid") == 0) {
            this->WID = json_as_int(*current_node);
        }
        ++current_node;
    }
    return noError;
}

std::time_t Parse8601(std::string iso_8601_formatted_date) {
    int tzd;
    Poco::DateTime dt;
    Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT,
        iso_8601_formatted_date, dt, tzd);
    Poco::Timestamp ts = dt.timestamp();
    return ts.epochTime();
}

std::string Format8601(std::time_t date) {
    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(date);
    return Poco::DateTimeFormatter::format(ts,
        Poco::DateTimeFormat::ISO8601_FORMAT);
}

std::string TimeEntry::StartString() {
     return Format8601(this->Start);
}

void TimeEntry::SetStartString(std::string value) {
    this->Start = Parse8601(value);
}

std::string TimeEntry::StopString() {
     return Format8601(this->Stop);
}

void TimeEntry::SetStopString(std::string value) {
    this->Stop = Parse8601(value);
}

std::string TimeEntry::Tags() {
    std::stringstream ss;
    std::copy(TagNames.begin(), TagNames.end(),
        std::ostream_iterator<std::string>(ss, "|"));
    return ss.str();
}

void TimeEntry::SetTags(std::string tags) {
    TagNames.clear();
    std::stringstream ss(tags);
    while (ss.good()) {
        std::string tag;
        getline(ss, tag, '|');
        TagNames.push_back(tag);
    }
}

error TimeEntry::Load(std::string json) {
    poco_assert(!json.empty());
    JSONNODE *root = json_parse(json.c_str());
    error err = Load(root);
    json_delete(root);
    return err;
}

error TimeEntry::Load(JSONNODE *data) {
    poco_assert(data);

    json_char *jc = json_write_formatted(data);
    std::string json(jc);
    json_free(jc);
    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
    logger.debug("Time entry is loading this JSON: " + json);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "id") == 0) {
            this->ID = json_as_int(*current_node);
            std::stringstream ss;
            ss << "Set time entry ID " << this->String() + " to " << this->ID;
            logger.debug(ss.str());
        } else if (strcmp(node_name, "description") == 0) {
            this->Description = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "guid") == 0) {
            this->GUID = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "wid") == 0) {
            this->WID = json_as_int(*current_node);
        } else if (strcmp(node_name, "pid") == 0) {
            this->PID = json_as_int(*current_node);
        } else if (strcmp(node_name, "tid") == 0) {
            this->TID = json_as_int(*current_node);
        } else if (strcmp(node_name, "start") == 0) {
            this->SetStartString(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "stop") == 0) {
            this->SetStopString(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "duration") == 0) {
            this->DurationInSeconds = json_as_int(*current_node);
        } else if (strcmp(node_name, "ui_modified_at") == 0) {
            this->UIModifiedAt = json_as_int(*current_node);
        } else if (strcmp(node_name, "billable") == 0) {
            this->Billable = json_as_bool(*current_node);
        } else if (strcmp(node_name, "duronly") == 0) {
            this->DurOnly = json_as_bool(*current_node);
        } else if (strcmp(node_name, "tags") == 0) {
            error err = this->loadTags(*current_node);
            if (err != noError) {
                return err;
            }
        }
        ++current_node;
    }

    Dirty = true;

    return noError;
}

error TimeEntry::loadTags(JSONNODE *list) {
    poco_assert(list);

    this->TagNames.clear();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        std::string tag = std::string(json_as_string(*current_node));
        if (!tag.empty()) {
            this->TagNames.push_back(tag);
        }
        ++current_node;
    }
    return noError;
}

}   // namespace kopsik
