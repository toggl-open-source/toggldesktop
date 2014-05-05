// Copyright 2014 Toggl Desktop developers.

#include "./client.h"

#include <sstream>
#include <cstring>

namespace kopsik {

std::string Client::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " name=" << name_
        << " wid=" << wid_
        << " guid=" << GUID();
    return ss.str();
}

void Client::SetName(const std::string value) {
    if (name_ != value) {
        name_ = value;
        dirty_ = true;
    }
}

void Client::SetWID(const Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        dirty_ = true;
    }
}

bool CompareClientByName(Client *a, Client *b) {
    return (strcmp(a->Name().c_str(), b->Name().c_str()) < 0);
}

void Client::LoadFromJSONNode(JSONNODE * const data) {
    poco_check_ptr(data);

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

}   // namespace kopsik
