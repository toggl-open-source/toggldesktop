#include <string>
#include <sstream>
#include <iostream>

#include "toggl_api_client.h"

#include "Poco/Stopwatch.h"
#include "Poco/Bugcheck.h"
#include "Poco/Exception.h"
#include "Poco/Logger.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/HTTPSClientSession.h"

#include <libjson.h>

namespace kopsik {

error User::Fetch() {
	Poco::Stopwatch stopwatch;
	stopwatch.start();
	poco_assert(!APIToken.empty());
	Poco::Logger &logger = Poco::Logger::get("toggl_api_client");
	try {
		const Poco::URI uri("https://www.toggl.com");
		const Poco::Net::Context::Ptr context(new Poco::Net::Context(
			Poco::Net::Context::CLIENT_USE, "", "", "",
			Poco::Net::Context::VERIFY_NONE, 9, false, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"));
		Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
		session.setKeepAlive(false);

		Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,
			"/api/v8/me?with_related_data=true", Poco::Net::HTTPMessage::HTTP_1_1);
		req.setKeepAlive(false);

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

		std::stringstream response_string;
		response_string << "Response received: " << response.getStatus() << " " << response.getReason();
		logger.debug(response_string.str());

		std::ostringstream body;
		Poco::StreamCopier::copyStream(is, body);

		if ((response.getStatus() != 202) && (response.getStatus() != 200)) {
			return body.str();
		}

		std::string json = body.str();
		logger.debug(json);

		error err = this->Load(json);
		if (err != noError) {
			return err;
		}

	} catch (const Poco::Exception& exc) {
		return exc.displayText();
	} catch (const std::exception& ex) {
		return ex.what();
	} catch (const std::string& ex) {
		return ex;
	}

	stopwatch.stop();
	std::stringstream ss;
	ss << "User fetched and parsed in " << stopwatch.elapsed() / 1000 << " ms";
	logger.debug(ss.str());

	return noError;
};

error User::Load(const std::string &json) {
	poco_assert(!json.empty());
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

error User::Save(Database &db) {
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
	std::cout << "Loading projects" << std::endl;
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
        std::cout << "ID=" << model.ID << " name=" << model.Name << " wid=" << model.WID << " guid=" << model.GUID << std::endl;
        ++current_node;
    }
    return noError;
}

error User::loadTasks(JSONNODE *list) {
	std::cout << "Loading tasks" << std::endl;
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
        std::cout << "ID=" << model.ID << " name=" << model.Name << " wid=" << model.WID << " pid=" << model.PID << std::endl;
        ++current_node;
    }
    return noError;
}

error User::loadWorkspaces(JSONNODE *list) {
	std::cout << "Loading workspaces" << std::endl;
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
        std::cout << "ID=" << model.ID << " name=" << model.Name << std::endl;
        ++current_node;
    }
    return noError;
}

error User::loadTags(JSONNODE *list) {
	std::cout << "Loading tags" << std::endl;
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
        std::cout << "ID=" << model.ID << " name=" << model.Name << " wid=" << model.WID << " guid=" << model.GUID << std::endl;
        ++current_node;
    }
    return noError;
}

error User::loadClients(JSONNODE *list) {
	std::cout << "Loading clients" << std::endl;
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
        std::cout << "ID=" << model.ID << " name=" << model.Name << " wid=" << model.WID << " guid=" << model.GUID << std::endl;
        ++current_node;
    }
    return noError;
}

error User::loadTimeEntries(JSONNODE *list) {
	std::cout << "Loading time entries" << std::endl;
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
        std::cout << "ID=" << model.ID << " description=" << model.Description << " wid=" << model.WID << " guid=" << model.GUID << std::endl <<
            " pid=" << model.PID << " tid=" << model.TID << " start=" << model.Start << " stop=" << model.Stop << std::endl << 
            " duration=" << model.DurationInSeconds << " billable=" << model.Billable << std::endl <<
            " duronly=" << model.DurOnly << " uimodifiedat=" << model.UIModifiedAt << std::endl;
        ++current_node;
    }
    return noError;
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

}
