// Copyright 2014 Toggl Desktop developers.

#include "../src/project.h"

#include <sstream>
#include <ctime>

#include "Poco/UTF8String.h"
#include "Poco/NumberParser.h"
#include "Poco/UTF8String.h"

#include "./formatter.h"

namespace toggl {

static const char *known_colors[] = {
    "#06aaf5", "#c56bff", "#ea468d", "#fb8b14", "#c7741c",
    "#4bc800", "#04bb9b", "#e19a86", "#3750b5", "#a01aa5",
    "#f1c33f", "#205500", "#890000", "#e20505", "#000000"
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
        << " name=" << name_
        << " wid=" << wid_
        << " cid=" << cid_
        << " client_guid=" << client_guid_
        << " active=" << active_
        << " public=" << private_
        << " color=" << color_
        << " billable=" << billable_
        << " client_guid=" << client_guid_;
    return ss.str();
}

std::string Project::FullName() const {
    std::stringstream ss;
    ss << client_name_
       << name_;
    return ss.str();
}

void Project::SetClientGUID(const std::string &value) {
    if (client_guid_ != value) {
        client_guid_ = value;
        SetDirty();
    }
}

void Project::SetActive(const bool value) {
    if (active_ != value) {
        active_ = value;
        SetDirty();
    }
}

void Project::SetPrivate(const bool value) {
    if (private_ != value) {
        private_ = value;
        SetDirty();
    }
}

void Project::SetName(const std::string &value) {
    if (name_ != value) {
        name_ = value;
        SetDirty();
    }
}

void Project::SetBillable(const bool value) {
    if (billable_ != value) {
        billable_ = value;
        SetDirty();
    }
}

void Project::SetColor(const std::string &value) {
    if (color_ != value) {
        color_ = Poco::UTF8::toLower(value);
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

void Project::SetWID(const Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        SetDirty();
    }
}

void Project::SetCID(const Poco::UInt64 value) {
    if (cid_ != value) {
        cid_ = value;
        SetDirty();
    }
}

void Project::SetClientName(const std::string &value) {
    if (client_name_ != value) {
        client_name_ = value;
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
    SetWID(data["wid"].asUInt64());
    SetCID(data["cid"].asUInt64());
    SetActive(data["active"].asBool());
    SetBillable(data["billable"].asBool());
}

Json::Value Project::SaveToJSON() const {
    Json::Value n;
    if (ID()) {
        n["id"] = Json::UInt64(ID());
    }
    n["name"] = Formatter::EscapeJSONString(Name());
    n["wid"] = Json::UInt64(WID());
    if (CID()) {
        n["cid"] = Json::UInt64(CID());
    } else {
        n["cid"] = Json::nullValue;
    }
    // There is no way to set it in UI and free ws gets error when it's sent
    // n["billable"] = Billable();
    n["is_private"] = IsPrivate();
    n["color"] = Poco::UTF8::toLower(Color());
    n["active"] = Active();

    return n;
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
    if (!IsPrivate() && onlyAdminsCanChangeProjectVisibility(err)) {
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
