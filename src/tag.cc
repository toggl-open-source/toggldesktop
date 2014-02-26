// Copyright 2014 Toggl Desktop developers.

#include "./tag.h"

#include <sstream>

namespace kopsik {

std::string Tag::String() const {
    std::stringstream ss;
    ss  << "ID=" << id_
        << " local_id=" << local_id_
        << " name=" << name_
        << " wid=" << wid_
        << " guid=" << guid_;
    return ss.str();
}

void Tag::SetUID(const Poco::UInt64 value) {
    if (uid_ != value) {
        uid_ = value;
        dirty_ = true;
    }
}

void Tag::SetID(const Poco::UInt64 value) {
    if (id_ != value) {
        id_ = value;
        dirty_ = true;
    }
}

void Tag::SetWID(const Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        dirty_ = true;
    }
}

void Tag::SetName(const std::string value) {
    if (name_ != value) {
        name_ = value;
        dirty_ = true;
    }
}

void Tag::SetGUID(const std::string value) {
    if (guid_ != value) {
        guid_ = value;
        dirty_ = true;
    }
}

}   // namespace kopsik
