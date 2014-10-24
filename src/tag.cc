// Copyright 2014 Toggl Desktop developers.

#include "./tag.h"

#include <sstream>

namespace toggl {

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

void Tag::LoadFromJSON(Json::Value data) {
    if (data.isMember("guid")) {
        SetGUID(data["guid"].asString());
    }
    SetID(data["id"].asUInt64());
    SetName(data["name"].asString());
    SetWID(data["wid"].asUInt64());
}

}   // namespace toggl
