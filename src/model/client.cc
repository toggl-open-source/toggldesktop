// Copyright 2014 Toggl Desktop developers.

#include "client.h"

#include "util/formatter.h"

#include <sstream>
#include <cstring>

namespace toggl {

std::string Client::ModelName() const {
    return kModelClient;
}

std::string Client::ModelURL() const {
    std::stringstream relative_url;
    relative_url << "/api/v9/workspaces/"
                 << WID() << "/clients";

    return relative_url.str();
}

std::string Client::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " name=" << name_
        << " wid=" << wid_
        << " guid=" << GUID();
    return ss.str();
}

void Client::SetName(const std::string &value) {
    if (name_ != value) {
        name_ = value;
        SetDirty();
    }
}

void Client::SetWID(const Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        SetDirty();
    }
}

void Client::LoadFromJSON(Json::Value data) {
    SetID(data["id"].asUInt64());
    SetName(data["name"].asString());
    SetWID(data["wid"].asUInt64());
}

Json::Value Client::SaveToJSON() const {
    Json::Value n;
    if (ID()) {
        n["id"] = Json::UInt64(ID());
    }
    n["name"] = Formatter::EscapeJSONString(Name());
    n["wid"] = Json::UInt64(WID());
    n["guid"] = GUID();
    n["ui_modified_at"] = Json::UInt64(UIModifiedAt());
    return n;
}

bool Client::ResolveError(const toggl::error &err) {
    if (nameHasAlreadyBeenTaken(err)) {
        SetName(Name() + " 1");
        return true;
    }
    if (err == error::kClientNameAlreadyExists) {
        // remove duplicate from db
        MarkAsDeletedOnServer();
        return true;
    }
    return false;
}

bool Client::nameHasAlreadyBeenTaken(const error &err) {
    return err == error::kNameAlreadyTaken;
}

bool Client::ResourceCannotBeCreated(const toggl::error &err) const {
    return err == error::kCannotModifyWorkspaceData;
}

}   // namespace toggl
