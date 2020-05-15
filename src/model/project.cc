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
        << " billable=" << Billable()
        << " client_name=" << ClientName();
    return ss.str();
}

std::string Project::FullName() const {
    std::stringstream ss;
    ss << ClientName()
       << Name();
    return ss.str();
}

std::string Project::ColorCode() const {
    unsigned index(0);
    if (!Poco::NumberParser::tryParseUnsigned(Color(), index)) {
        return Color();
    }
    return ColorCodes[index % ColorCodes.size()];
}

error Project::SetColorCode(const std::string &color_code) {
    Color.Set(color_code);
    return noError;
}

void Project::LoadFromJSON(Json::Value data) {
    if (data.isMember("hex_color")) {
        Color.Set(data["hex_color"].asString());
    } else {
        Color.Set(data["color"].asString());
    }

    ID.Set(data["id"].asUInt64());
    Name.Set(data["name"].asString());
    WID.Set(data["wid"].asUInt64());
    CID.Set(data["cid"].asUInt64());
    Active.Set(data["active"].asBool());
    Billable.Set(data["billable"].asBool());
}

Json::Value Project::SaveToJSON(int apiVersion) const {
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
    n["is_private"] = Private();
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
        WID.Set(0);
        return true;
    }
    if (clientIsInAnotherWorkspace(err)) {
        CID.Set(0);
        return true;
    }
    if (!Private() && onlyAdminsCanChangeProjectVisibility(err)) {
        Private.Set(true);
        return true;
    }
    if (err.find(kProjectNameAlready) != std::string::npos) {
        // remove duplicate from db
        IsMarkedAsDeletedOnServer.Set(true);
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
