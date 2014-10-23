// Copyright 2014 Toggl Desktop developers.

#include "./json.h"

#include <sstream>
#include <cstring>

#include "Poco/NumberParser.h"

namespace toggl {

namespace json {

Poco::UInt64 ID(JSONNODE * const data) {
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

error UserID(const std::string json_data_string, Poco::UInt64 *result) {
    *result = 0;
    Json::Value root;
    Json::Reader reader;
    bool ok = reader.parse(json_data_string, root);
    if (!ok) {
        return error("error parsing UserID JSON");
    }
    if (!Poco::NumberParser::tryParseUnsigned64(root["data"]["id"].asString(),
            *result)) {
        return error("error parsing UserID");
    }
    return noError;
}

error LoginToken(const std::string json_data_string, std::string *result) {
    *result = "";
    Json::Value root;
    Json::Reader reader;
    bool ok = reader.parse(json_data_string, root);
    if (!ok) {
        return error("error parsing UserID JSON");
    }
    *result = root["login_token"].asString();
    return noError;
}

guid GUID(JSONNODE * const data) {
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

bool IsValid(const std::string json) {
    Json::Value root;
    Json::Reader reader;
    return reader.parse(json, root);
}

error LoadTags(
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

Poco::UInt64 UIModifiedAt(
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

error LoadTimeEntryTags(
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

}   // namespace json

}   // namespace toggl
