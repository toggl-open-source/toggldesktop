// Copyright 2014 Toggl Desktop developers.

#include "model/project.h"

#include <sstream>
#include <ctime>

#include <Poco/UTF8String.h>
#include <Poco/NumberParser.h>
#include <Poco/UTF8String.h>

#include "util/formatter.h"

namespace toggl {

static const char *known_colors[] = {
    "#0b83d9", "#9e5bd9", "#d94182", "#e36a00", "#bf7000",
    "#2da608", "#06a893", "#c9806b", "#465bb3", "#990099",
    "#c7af14", "#566614", "#d92b2b", "#525266"
};

template<typename T, size_t N> T *end(T (&ra)[N]) {
    return ra + N;
}

std::vector<std::string> Project::ColorCodes(known_colors, end(known_colors));

std::string Project::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " guid=" << GUID()
        << " name=" << Name()
        << " wid=" << WID()
        << " cid=" << CID()
        << " client_guid=" << ClientGUID()
        << " active=" << Active()
        << " public=" << Private()
        << " color=" << Color()
        << " billable=" << Billable();
    return ss.str();
}

std::string Project::FullName() const {
    std::stringstream ss;
    ss << ClientName()
       << Name();
    return ss.str();
}

void Project::SetClientGUID(const std::string &value) {
    if (ClientGUID() != value) {
        ClientGUID.Set(value);
        SetDirty();
    }
}

void Project::SetActive(bool value) {
    if (Active() != value) {
        Active.Set(value);
        SetDirty();
    }
}

void Project::SetPrivate(bool value) {
    if (Private() != value) {
        Private.Set(value);
        SetDirty();
    }
}

void Project::SetName(const std::string &value) {
    if (Name() != value) {
        Name.Set(value);
        SetDirty();
    }
}

void Project::SetBillable(bool value) {
    if (Billable() != value) {
        Billable.Set(value);
        SetDirty();
    }
}

void Project::SetColor(const std::string &value) {
    if (Color() != value) {
        Color.Set(Poco::UTF8::toLower(value));
        SetDirty();
    }
}

std::string Project::ColorCode() const {
    unsigned index(0);
    if (!Poco::NumberParser::tryParseUnsigned(Color(), index)) {
        return Color();
    }
    return ColorCodes[index % ColorCodes.size()];
}

error Project::SetColorCode(const std::string &color_code) {
    SetColor(color_code);
    return noError;
}

void Project::SetWID(Poco::UInt64 value) {
    if (WID() != value) {
        WID.Set(value);
        SetDirty();
    }
}

void Project::SetCID(Poco::UInt64 value) {
    if (CID() != value) {
        CID.Set(value);
        SetDirty();
    }
}

void Project::SetClientName(const std::string &value) {
    if (ClientName() != value) {
        ClientName.Set(value);
        SetDirty();
    }
}

void Project::LoadFromJSON(Json::Value data) {
    if (data.isMember("hex_color")) {
        SetColor(data["hex_color"].asString());
    } else {
        SetColor(data["color"].asString());
    }

    SetID(data["id"].asUInt64());
    SetName(data["name"].asString());
    if (data.isMember("wid"))
        SetWID(data["wid"].asUInt64());
    else
        SetWID(data["workspace_id"].asUInt64());
    if (data.isMember("cid"))
        SetCID(data["cid"].asUInt64());
    else
        SetCID(data["client_id"].asUInt64());
    SetActive(data["active"].asBool());
    SetBillable(data["billable"].asBool());
}

Json::Value Project::SaveToJSON(int apiVersion) const {
    Json::Value n;
    if (ID()) {
        n["id"] = Json::UInt64(ID());
    }
    n["name"] = Formatter::EscapeJSONString(Name());
    if (apiVersion == 8) {
        n["wid"] = Json::UInt64(WID());
        if (CID()) {
            n["cid"] = Json::UInt64(CID());
        } else {
            n["cid"] = Json::nullValue;
        }
    }
    else {
        n["workspace_id"] = Json::UInt64(WID());
        if (CID()) {
            n["client_id"] = Json::UInt64(CID());
        } else {
            n["client_id"] = Json::nullValue;
        }
    }
    // There is no way to set it in UI and free ws gets error when it's sent
    // n["billable"] = Billable();
    n["is_private"] = Private();
    n["color"] = Poco::UTF8::toLower(Color());
    n["active"] = Active();

    return n;
}

Json::Value Project::SyncMetadata() const {
    Json::Value result;
    if (NeedsPOST()) {
        result["client_assigned_id"] = std::to_string(-LocalID());
    }
    else if (NeedsPUT() || NeedsDELETE()) {
        if (ID() > 0)
            result["id"] = Json::Int64(ID());
        else // and this really shouldn't happen
            result["id"] = std::to_string(-LocalID());
        result["workspace_id"] = Json::Int64(WID());
    }
    return result;
}

Json::Value Project::SyncPayload() const {
    Json::Value result;
    if (NeedsPOST()) {
        result["id"] = Json::Int64(-LocalID());
        result["workspace_id"] = Json::Int64(WID());
    }
    if (NeedsPOST() || NeedsPUT()) {
        if (CID())
            result["client_id"] = Json::Int64(CID());
        else
            result["client_id"] = ClientGUID();
        result["name"] = Name();
        result["is_private"] = Private();
        result["active"] = Active();
        result["color"] = Color();
        result["billable"] = Billable();
    }
    return result;
}

bool Project::DuplicateResource(const toggl::error &err) const {
    return (std::string::npos !=
            std::string(err).find("Name has already been taken"));
}

bool Project::ResourceCannotBeCreated(const toggl::error &err) const {
    return (std::string::npos != std::string(err).find(
        "User cannot add or edit projects in workspace"));
}

bool Project::clientIsInAnotherWorkspace(const toggl::error &err) const {
    return (std::string::npos != std::string(err).find(
        "client is in another workspace")
            || (std::string::npos != std::string(err).find("Client with the ID")
                && std::string::npos != std::string(err).find("isn't present in workspace")));
}

bool Project::onlyAdminsCanChangeProjectVisibility(
    const toggl::error &err) const {
    return (std::string::npos != std::string(err).find(
        "Only admins can change project visibility"));
}

bool Project::ResolveError(const toggl::error &err) {
    if (userCannotAccessWorkspace(err)) {
        SetWID(0);
        return true;
    }
    if (clientIsInAnotherWorkspace(err)) {
        SetCID(0);
        return true;
    }
    if (!Private() && onlyAdminsCanChangeProjectVisibility(err)) {
        SetPrivate(true);
        return true;
    }
    if (err.find(kProjectNameAlready) != std::string::npos) {
        // remove duplicate from db
        MarkAsDeletedOnServer();
        return true;
    }
    return false;
}

std::string Project::ModelName() const {
    return kModelProject;
}

std::string Project::ModelURL() const {
    std::stringstream relative_url;
    relative_url << "/api/v9/workspaces/"
                 << WID() << "/projects";

    return relative_url.str();
}

}   // namespace toggl
