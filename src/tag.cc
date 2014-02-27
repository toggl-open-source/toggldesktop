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

}   // namespace kopsik
