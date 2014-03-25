// Copyright 2014 Toggl Desktop developers.

#include "./tag.h"

#include <sstream>

namespace kopsik {

std::string Tag::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " name=" << name_
        << " wid=" << wid_
        << " guid=" << GUID();
    return ss.str();
}

void Tag::SetWID(const Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        SetDirty();
    }
}

void Tag::SetName(const std::string value) {
    if (name_ != value) {
        name_ = value;
        SetDirty();
    }
}

void Tag::LoadFromJSONNode(JSONNODE * const data) {
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

}   // namespace kopsik
