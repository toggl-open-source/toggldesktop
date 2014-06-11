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

std::string LoginTokenFromJSONDataString(const std::string json_data_string) {
    std::string result("");

    JSONNODE *root = json_parse(json_data_string.c_str());

    JSONNODE_ITERATOR current_node = json_begin(root);
    JSONNODE_ITERATOR last_node = json_end(root);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "login_token") == 0) {
            result = std::string(json_as_string(*current_node));
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
