// Copyright 2014 Toggl Desktop developers.

#include "model/tag.h"

#include <sstream>

#include "const.h"

namespace toggl {

std::string Tag::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " name=" << Name()
        << " wid=" << WID()
        << " guid=" << GUID();
    return ss.str();
}

void Tag::SetWID(Poco::UInt64 value) {
    if (WID() != value) {
        WID.Set(value);
        SetDirty();
    }
}

void Tag::SetName(const std::string &value) {
    if (Name() != value) {
        Name.Set(value);
        SetDirty();
    }
}

void Tag::LoadFromJSON(const Json::Value &data) {
    SetID(data["id"].asUInt64());
    SetName(data["name"].asString());
    if (data.isMember("wid"))
        SetWID(data["wid"].asUInt64());
    else
        SetWID(data["workspace_id"].asUInt64());
}

std::string Tag::ModelName() const {
    return kModelTag;
}

std::string Tag::ModelURL() const {
    return "/api/v9/tags";
}

}   // namespace toggl
