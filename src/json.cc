// Copyright 2014 Toggl Desktop developers.

#include "./json.h"

#include <sstream>
#include <cstring>

#include "Poco/Logger.h"

namespace kopsik {

Poco::UInt64 IDFromJSONNode(JSONNODE * const data) {
    poco_check_ptr(data);

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

Poco::UInt64 UserIDFromJSONDataString(const std::string json_data_string) {
    Poco::UInt64 result(0);

    JSONNODE *root = json_parse(json_data_string.c_str());

    JSONNODE_ITERATOR current_node = json_begin(root);
    JSONNODE_ITERATOR last_node = json_end(root);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "data") == 0) {
            result = IDFromJSONNode(*current_node);
            break;
        }
        ++current_node;
    }

    json_delete(root);

    return result;
}

guid GUIDFromJSONNode(JSONNODE * const data) {
    poco_check_ptr(data);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "guid") == 0) {
            return std::string(json_as_string(*current_node));
        }
        ++current_node;
    }
    return "";
}

bool IsDeletedAtServer(JSONNODE * const data) {
    poco_check_ptr(data);

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

bool IsValidJSON(const std::string json) {
    return json_is_valid(json.c_str());
}

void LoadUserAndRelatedDataFromJSONString(
    User *model,
    const std::string &json) {

    poco_check_ptr(model);

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
            model->SetSince(json_as_int(*current_node));

            Poco::Logger &logger = Poco::Logger::get("json");
            std::stringstream s;
            s << "User data as of: " << model->Since();
            logger.debug(s.str());

        } else if (strcmp(node_name, "data") == 0) {
            LoadUserAndRelatedDataFromJSONNode(model, *current_node);
        }
        ++current_node;
    }
    json_delete(root);
}

void LoadUserAndRelatedDataFromJSONNode(
    User *model,
    JSONNODE * const data) {

    poco_check_ptr(model);
    poco_check_ptr(data);

    JSONNODE_ITERATOR n = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (n != last_node) {
        json_char *node_name = json_name(*n);
        if (strcmp(node_name, "id") == 0) {
            model->SetID(json_as_int(*n));
        } else if (strcmp(node_name, "default_wid") == 0) {
            model->SetDefaultWID(json_as_int(*n));
        } else if (strcmp(node_name, "api_token") == 0) {
            model->SetAPIToken(std::string(json_as_string(*n)));
        } else if (strcmp(node_name, "email") == 0) {
            model->SetEmail(std::string(json_as_string(*n)));
        } else if (strcmp(node_name, "fullname") == 0) {
            model->SetFullname(std::string(json_as_string(*n)));
        } else if (strcmp(node_name, "record_timeline") == 0) {
            model->SetRecordTimeline(json_as_bool(*n));
        } else if (strcmp(node_name, "store_start_and_stop_time") == 0) {
            model->SetStoreStartAndStopTime(json_as_bool(*n));
        } else if (strcmp(node_name, "timeofday_format") == 0) {
            model->SetTimeOfDayFormat(std::string(json_as_string(*n)));
        } else if (strcmp(node_name, "projects") == 0) {
            LoadUserProjectsFromJSONNode(model, *n);
        } else if (strcmp(node_name, "tags") == 0) {
            LoadUserTagsFromJSONNode(model, *n);
        } else if (strcmp(node_name, "tasks") == 0) {
            LoadUserTasksFromJSONNode(model, *n);
        } else if (strcmp(node_name, "time_entries") == 0) {
            LoadUserTimeEntriesFromJSONNode(model, *n);
        } else if (strcmp(node_name, "workspaces") == 0) {
            LoadUserWorkspacesFromJSONNode(model, *n);
        } else if (strcmp(node_name, "clients") == 0) {
            LoadUserClientsFromJSONNode(model, *n);
        }
        ++n;
    }
}

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

void LoadUserTagsFromJSONNode(
    User *model,
    JSONNODE * const list) {

    poco_check_ptr(model);
    poco_check_ptr(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadUserTagFromJSONNode(model, *current_node, &alive);
        ++current_node;
    }

    deleteZombies(model->related.Tags, alive);
}

void loadUserTagFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {

    poco_check_ptr(user);
    poco_check_ptr(data);
    // alive can be 0, dont assert/check it

    Poco::UInt64 id = IDFromJSONNode(data);
    Tag *model = user->TagByID(id);

    if (!model) {
        model = user->TagByGUID(GUIDFromJSONNode(data));
    }

    if (IsDeletedAtServer(data)) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = new Tag();
        user->related.Tags.push_back(model);
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(user->ID());
    model->LoadFromJSONNode(data);
}

void LoadUserTasksFromJSONNode(
    User *user,
    JSONNODE * const list) {

    poco_check_ptr(user);
    poco_check_ptr(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadUserTaskFromJSONNode(user, *current_node, &alive);
        ++current_node;
    }

    deleteZombies(user->related.Tasks, alive);
}

void loadUserTaskFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {

    poco_check_ptr(user);
    poco_check_ptr(data);
    // alive can be 0, dont assert/check it

    Poco::UInt64 id = IDFromJSONNode(data);
    Task *model = user->TaskByID(id);

    // Tasks have no GUID

    if (IsDeletedAtServer(data)) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = new Task();
        user->related.Tasks.push_back(model);
    }

    if (alive) {
        alive->insert(id);
    }
    model->SetUID(user->ID());
    model->LoadFromJSONNode(data);
}

void LoadUserUpdateFromJSONString(
    User *user,
    const std::string json) {

    if (json.empty()) {
        return;
    }

    poco_check_ptr(user);

    JSONNODE *root = json_parse(json.c_str());
    LoadUserUpdateFromJSONNode(user, root);
    json_delete(root);
}

void LoadUserUpdateFromJSONNode(
    User *user,
    JSONNODE * const node) {

    poco_check_ptr(user);
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
        loadUserWorkspaceFromJSONNode(user, data);
    } else if ("client" == model) {
        loadUserClientFromJSONNode(user, data);
    } else if ("project" == model) {
        loadUserProjectFromJSONNode(user, data);
    } else if ("task" == model) {
        loadUserTaskFromJSONNode(user, data);
    } else if ("time_entry" == model) {
        loadUserTimeEntryFromJSONNode(user, data);
    } else if ("tag" == model) {
        loadUserTagFromJSONNode(user, data);
    }
}

void loadUserWorkspaceFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {

    poco_check_ptr(user);
    poco_check_ptr(data);
    // alive can be 0, dont assert/check it

    Poco::UInt64 id = IDFromJSONNode(data);
    Workspace *model = user->WorkspaceByID(id);

    // Workspaces have no GUID

    if (IsDeletedAtServer(data)) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = new Workspace();
        user->related.Workspaces.push_back(model);
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(user->ID());
    model->LoadFromJSONNode(data);
}

error LoadTagsFromJSONNode(
    TimeEntry *te,
    JSONNODE * const list) {

    poco_check_ptr(te);
    poco_check_ptr(list);

    te->TagNames.clear();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        std::string tag = std::string(json_as_string(*current_node));
        if (!tag.empty()) {
            te->TagNames.push_back(tag);
        }
        ++current_node;
    }
    return noError;
}

void loadUserClientFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {

    poco_check_ptr(user);
    poco_check_ptr(data);
    // alive can be 0, dont assert/check it

    Poco::UInt64 id = IDFromJSONNode(data);
    Client *model = user->ClientByID(id);

    if (!model) {
        model = user->ClientByGUID(GUIDFromJSONNode(data));
    }

    if (IsDeletedAtServer(data)) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = new Client();
        user->related.Clients.push_back(model);
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(user->ID());
    model->LoadFromJSONNode(data);
}

Poco::UInt64 UIModifiedAtFromJSONNode(
    JSONNODE * const data) {

    poco_check_ptr(data);

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

void LoadUserClientsFromJSONNode(
    User *user,
    JSONNODE * const list) {

    poco_check_ptr(user);
    poco_check_ptr(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadUserClientFromJSONNode(user, *current_node, &alive);
        ++current_node;
    }

    deleteZombies(user->related.Clients, alive);
}

void loadUserProjectFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {

    poco_check_ptr(user);
    poco_check_ptr(data);
    // alive can be 0, dont assert/check it

    Poco::UInt64 id = IDFromJSONNode(data);
    Project *model = user->ProjectByID(id);

    if (!model) {
        model = user->ProjectByGUID(GUIDFromJSONNode(data));
    }

    if (IsDeletedAtServer(data)) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = new Project();
        user->related.Projects.push_back(model);
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(user->ID());
    model->LoadFromJSONNode(data);
}

void LoadUserProjectsFromJSONNode(
    User *user,
    JSONNODE * const list) {

    poco_check_ptr(user);
    poco_check_ptr(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadUserProjectFromJSONNode(user, *current_node, &alive);
        ++current_node;
    }

    deleteZombies(user->related.Projects, alive);
}

error LoadTimeEntryTagsFromJSONNode(
    TimeEntry *te,
    JSONNODE * const list) {

    poco_check_ptr(te);
    poco_check_ptr(list);

    te->TagNames.clear();

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        std::string tag = std::string(json_as_string(*current_node));
        if (!tag.empty()) {
            te->TagNames.push_back(tag);
        }
        ++current_node;
    }
    return noError;
}

void loadUserTimeEntryFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {

    poco_check_ptr(user);
    poco_check_ptr(data);
    // alive can be 0, dont assert/check it

    Poco::UInt64 id = IDFromJSONNode(data);
    TimeEntry *model = user->TimeEntryByID(id);

    if (!model) {
        model = user->TimeEntryByGUID(GUIDFromJSONNode(data));
    }

    if (IsDeletedAtServer(data)) {
        if (model) {
            model->MarkAsDeletedOnServer();
        }
        return;
    }

    if (!model) {
        model = new TimeEntry();
        user->related.TimeEntries.push_back(model);
    }
    if (alive) {
        alive->insert(id);
    }
    model->SetUID(user->ID());
    model->LoadFromJSONNode(data);
    model->EnsureGUID();
}

void LoadUserWorkspacesFromJSONNode(
    User *user,
    JSONNODE * const list) {

    poco_check_ptr(user);
    poco_check_ptr(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadUserWorkspaceFromJSONNode(user, *current_node, &alive);
        ++current_node;
    }

    deleteZombies(user->related.Workspaces, alive);
}

void LoadUserTimeEntriesFromJSONNode(
    User *user,
    JSONNODE * const list) {

    poco_check_ptr(user);
    poco_check_ptr(list);

    std::set<Poco::UInt64> alive;

    JSONNODE_ITERATOR current_node = json_begin(list);
    JSONNODE_ITERATOR last_node = json_end(list);
    while (current_node != last_node) {
        loadUserTimeEntryFromJSONNode(user, *current_node, &alive);
        ++current_node;
    }

    deleteZombies(user->related.TimeEntries, alive);
}

std::string UpdateJSON(
    std::vector<Project *> * const projects,
    std::vector<TimeEntry *> * const time_entries) {

    poco_check_ptr(projects);
    poco_check_ptr(time_entries);

    JSONNODE *c = json_new(JSON_ARRAY);

    // First, projects, because time entries depend on projects
    for (std::vector<Project *>::const_iterator it =
        projects->begin();
            it != projects->end(); it++) {
        json_push_back(c, (*it)->BatchUpdateJSON());
    }

    // Time entries go last
    for (std::vector<TimeEntry *>::const_iterator it =
        time_entries->begin();
            it != time_entries->end(); it++) {
        json_push_back(c, (*it)->BatchUpdateJSON());
    }

    json_char *jc = json_write_formatted(c);
    std::string json(jc);
    json_free(jc);
    json_delete(c);
    return json;
}

}   // namespace kopsik
