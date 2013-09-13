// Copyright 2013 Tanel Lebedev

#include <string>
#include <sstream>

#include "./toggl_api_client.h"

#include "Poco/Stopwatch.h"
#include "Poco/Bugcheck.h"
#include "Poco/Exception.h"
#include "Poco/InflatingStream.h"
#include "Poco/Logger.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/HTTPSClientSession.h"

#include "libjson.h" // NOLINT

namespace kopsik {

error User::Fetch() {
    poco_assert(!APIToken.empty());

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
    try {
        const Poco::URI uri("https://www.toggl.com");
        const Poco::Net::Context::Ptr context(new Poco::Net::Context(
            Poco::Net::Context::CLIENT_USE, "", "", "",
            Poco::Net::Context::VERIFY_NONE, 9, false,
            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"));
        Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(),
            context);
        session.setKeepAlive(false);

        Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,
            "/api/v8/me?with_related_data=true",
            Poco::Net::HTTPMessage::HTTP_1_1);
        req.setKeepAlive(false);
        req.set("Accept-Encoding", "gzip");
        req.set("Content-Encoding", "gzip");
        req.setChunkedTransferEncoding(true);

        logger.debug("Sending request..");

        Poco::Net::HTTPBasicCredentials cred(APIToken, "api_token");
        cred.authenticate(req);
        session.sendRequest(req) << std::flush;

        std::stringstream request_string;
        req.write(request_string);
        logger.debug(request_string.str());

        logger.debug("Request sent. Receiving response..");

        Poco::Net::HTTPResponse response;
        std::istream& is = session.receiveResponse(response);

        Poco::InflatingInputStream inflater(is,
            Poco::InflatingStreamBuf::STREAM_GZIP);
        std::stringstream ss;
        ss << inflater.rdbuf();
        const std::string &json = ss.str();

        std::stringstream response_string;
        response_string << "Response status: " << response.getStatus()
            << ", reason: " << response.getReason()
            << ", Content type: " << response.getContentType()
            << ", Content-Encoding: " << response.get("Content-Encoding");
        logger.debug(response_string.str());

        if ((response.getStatus() != 202) && (response.getStatus() != 200)) {
            return json;
        }

        error err = this->Load(json);
        if (err != noError) {
            return err;
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }

    stopwatch.stop();
    std::stringstream ss;
    ss << "User with related data JSON fetched and parsed in "
        << stopwatch.elapsed() / 1000 << " ms";
    logger.debug(ss.str());

    return noError;
};

error User::Load(const std::string &json) {
    poco_assert(!json.empty());

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    JSONNODE *root = json_parse(json.c_str());
    JSONNODE_ITERATOR current_node = json_begin(root);
    JSONNODE_ITERATOR last_node = json_end(root);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "since") == 0) {
            this->Since = json_as_int(*current_node);
            std::stringstream s;
            s << "User data as of: " << this->Since;
            Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
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
    Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
    logger.debug(ss.str());

    return noError;
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

error User::Save(Database *db) {
    /*

        if (strcmp(node_name, "id") == 0) {
            this->ID = json_as_int(*current_node);
        } else if (strcmp(node_name, "default_wid") == 0) {
            this->DefaultWID = json_as_int(*current_node);
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
    }
    */
    return noError;
}

error User::loadProjects(JSONNODE *list) {
    poco_assert(list);

    this->Projects.clear();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        Project model;
        error err = model.Load(*current_node);
        if (err != noError) {
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

    this->Tasks.clear();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        Task model;
        error err = model.Load(*current_node);
        if (err != noError) {
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

    this->Workspaces.clear();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        Workspace model;
        error err = model.Load(*current_node);
        if (err != noError) {
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

    this->Tags.clear();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        Tag model;
        error err = model.Load(*current_node);
        if (err != noError) {
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

    this->Clients.clear();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        Client model;
        error err = model.Load(*current_node);
        if (err != noError) {
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

    this->TimeEntries.clear();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        TimeEntry model;
        error err = model.Load(*current_node);
        if (err != noError) {
            return err;
        }
        this->TimeEntries.push_back(model);
        ++current_node;
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
    " uimodifiedat=" << UIModifiedAt;
    return ss.str();
}


error Workspace::Load(JSONNODE *data) {
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
        }
        ++current_node;
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


error TimeEntry::Load(JSONNODE *data) {
    poco_assert(data);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "id") == 0) {
            this->ID = json_as_int(*current_node);
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
            this->Start = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "stop") == 0) {
            this->Stop = std::string(json_as_string(*current_node));
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
