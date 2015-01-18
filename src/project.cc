// Copyright 2014 Toggl Desktop developers.

#include "../src/project.h"

#include <sstream>
#include <ctime>

#include "Poco/UTF8String.h"
#include "Poco/NumberParser.h"

#include "./formatter.h"

namespace toggl {

const char *known_colors[] = {
    "#4dc3ff", "#bc85e6", "#df7baa", "#f68d38", "#b27636",
    "#8ab734", "#14a88e", "#268bb5", "#6668b4", "#a4506c",
    "#67412c", "#3c6526", "#094558", "#bc2d07", "#999999"
};

template<typename T, size_t N> T *end(T (&ra)[N]) {
    return ra + N;
}

std::vector<std::string> Project::color_codes(known_colors, end(known_colors));

std::string Project::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " name=" << name_
        << " wid=" << wid_
        << " cid=" << cid_
        << " guid=" << GUID()
        << " active=" << active_
        << " public=" << private_
        << " color=" << color_
        << " billable=" << billable_
        << " client_guid=" << client_guid_;
    return ss.str();
}

void Project::SetClientGUID(const std::string value) {
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

void Project::SetName(const std::string value) {
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

void Project::SetColor(const std::string value) {
    if (color_ != value) {
        color_ = value;
        SetDirty();
    }
}

std::string Project::ColorCode() const {
    int index(0);
    if (!Poco::NumberParser::tryParse(Color(), index)) {
        return color_codes.back();
    }
    return color_codes[index % color_codes.size()];
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

void Project::LoadFromJSON(Json::Value data) {
    if (data.isMember("guid")) {
        SetGUID(data["guid"].asString());
    }

    SetID(data["id"].asUInt64());
    SetName(data["name"].asString());
    SetWID(data["wid"].asUInt64());
    SetCID(data["cid"].asUInt64());
    SetColor(data["color"].asString());
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
    n["guid"] = GUID();
    n["cid"] = Json::UInt64(CID());
    n["billable"] = Billable();
    n["is_private"] = IsPrivate();
    n["ui_modified_at"] = Json::UInt64(UIModifiedAt());

    return n;
}

bool Project::DuplicateResource(const toggl::error err) const {
    return (std::string::npos !=
            std::string(err).find("Name has already been taken"));
}

bool Project::ResourceCannotBeCreated(const toggl::error err) const {
    return (std::string::npos != std::string(err).find(
        "User cannot add or edit projects in workspace"));
}

bool Project::clientIsInAnotherWorkspace(const toggl::error err) const {
    return (std::string::npos != std::string(err).find(
        "client is in another workspace"));
}

bool Project::ResolveError(const toggl::error err) {
    if (userCannotAccessWorkspace(err)) {
        SetWID(0);
        return true;
    }
    if (clientIsInAnotherWorkspace(err)) {
        SetCID(0);
        return true;
    }
    return false;
}

}   // namespace toggl
