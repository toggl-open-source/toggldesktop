// Copyright 2014 Toggl Desktop developers.

#include "./client.h"

#include <sstream>
#include <cstring>

namespace toggl {

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

void Client::LoadFromJSON(Json::Value data) {
    std::string guid = data["guid"].asString();
    if (!guid.empty()) {
        SetGUID(guid);
    }

    SetID(data["id"].asUInt64());
    SetName(data["name"].asString());
    SetWID(data["wid"].asUInt64());
}

}   // namespace toggl
