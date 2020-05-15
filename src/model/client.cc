// Copyright 2014 Toggl Desktop developers.

#include "model/client.h"

#include <sstream>
#include <cstring>

#include "util/formatter.h"

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
        << " name=" << Name()
        << " wid=" << WID()
        << " guid=" << GUID();
    return ss.str();
}

void Client::LoadFromJSON(Json::Value data) {
    ID.Set(data["id"].asUInt64());
    Name.Set(data["name"].asString());
    WID.Set(data["wid"].asUInt64());
}

Json::Value Client::SaveToJSON(int apiVersion) const {
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
        Name.Set(Name() + " 1");
        return true;
    }
    if (err.find(kClientNameAlreadyExists) != std::string::npos) {
        // remove duplicate from db
        IsMarkedAsDeletedOnServer.Set(true);
        return true;
    }
    return false;
}

bool Client::nameHasAlreadyBeenTaken(const error &err) {
    return (std::string::npos != std::string(err).find(
        "Name has already been taken"));
}

bool Client::ResourceCannotBeCreated(const toggl::error &err) const {
    return (std::string::npos != std::string(err).find(
        "cannot add or edit clients in workspace"));
}

}   // namespace toggl
