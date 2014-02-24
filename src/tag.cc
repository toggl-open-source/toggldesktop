// Copyright 2014 Toggl Desktop developers.

#include "./tag.h"

#include <sstream>

namespace kopsik {

std::string Tag::String() {
    std::stringstream ss;
    ss  << "ID=" << id_
        << " local_id=" << local_id_
        << " name=" << name_
        << " wid=" << wid_
        << " guid=" << guid_;
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

}   // namespace kopsik
