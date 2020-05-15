// Copyright 2014 Toggl Desktop developers.

#include "model/workspace.h"

#include <sstream>
#include "util/formatter.h"

namespace toggl {

std::string Workspace::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " name=" << Name();
    return ss.str();
}

void Workspace::LoadFromJSON(Json::Value n) {
    ID.Set(n["id"].asUInt64());
    Name.Set(n["name"].asString());
    Premium.Set(n["premium"].asBool());
    OnlyAdminsMayCreateProjects.Set(n["only_admins_may_create_projects"].asBool());
    Admin.Set(n["admin"].asBool());
    ProjectsBillableByDefault.Set(n["projects_billable_by_default"].asBool());

    auto profile = n["profile"].asUInt64();
    Business.Set(profile > 13);
}

void Workspace::LoadSettingsFromJson(Json::Value n) {
    auto lockDateString = n["report_locked_at"].asString();
    if (!lockDateString.empty()) {
        auto time = Formatter::Parse8601(lockDateString);
        LockedTime.Set(time);
    }
}

std::string Workspace::ModelName() const {
    return kModelWorkspace;
}

std::string Workspace::ModelURL() const {
    return "/api/v9/workspaces";
}


}   // namespace toggl
