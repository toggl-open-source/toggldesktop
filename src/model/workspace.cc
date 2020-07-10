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

void Workspace::SetName(const std::string &value) {
    if (Name.Set(value))
        SetDirty();
}

void Workspace::SetPremium(bool value) {
    if (Premium.Set(value))
        SetDirty();
}

void Workspace::SetOnlyAdminsMayCreateProjects(bool value) {
    if (OnlyAdminsMayCreateProjects.Set(value))
        SetDirty();
}

void Workspace::SetAdmin(bool value) {
    if (Admin.Set(value))
        SetDirty();
}

void Workspace::SetProjectsBillableByDefault(bool value) {
    if (ProjectsBillableByDefault.Set(value))
        SetDirty();
}

void Workspace::SetBusiness(bool value) {
    if (Business.Set(value))
        SetDirty();
}

void Workspace::SetLockedTime(time_t value) {
    if (LockedTime.Set(value))
        SetDirty();
}

void Workspace::LoadFromJSON(const Json::Value &n) {
    SetID(n["id"].asUInt64());
    SetName(n["name"].asString());
    SetPremium(n["premium"].asBool());
    SetOnlyAdminsMayCreateProjects(
        n["only_admins_may_create_projects"].asBool());
    SetAdmin(n["admin"].asBool());
    SetProjectsBillableByDefault(n["projects_billable_by_default"].asBool());

    auto profile = n["profile"].asUInt64();
    SetBusiness(profile > 13);
}

void Workspace::LoadSettingsFromJson(Json::Value n) {
    auto lockDateString = n["report_locked_at"].asString();
    if (!lockDateString.empty()) {
        auto time = Formatter::Parse8601(lockDateString);
        SetLockedTime(time);
    }
}

std::string Workspace::ModelName() const {
    return kModelWorkspace;
}

std::string Workspace::ModelURL() const {
    return "/api/v9/workspaces";
}


}   // namespace toggl
