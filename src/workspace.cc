// Copyright 2014 Toggl Desktop developers.

#include "../src/workspace.h"

#include <sstream>
#include <cstring>

namespace toggl {

std::string Workspace::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " name=" << name_;
    return ss.str();
}

void Workspace::SetName(const std::string value) {
    if (name_ != value) {
        name_ = value;
        SetDirty();
    }
}

void Workspace::SetPremium(const bool value) {
    if (premium_ != value) {
        premium_ = value;
        SetDirty();
    }
}

void Workspace::SetOnlyAdminsMayCreateProjects(const bool value) {
    if (only_admins_may_create_projects_ != value) {
        only_admins_may_create_projects_ = value;
        SetDirty();
    }
}

void Workspace::SetAdmin(const bool value) {
    if (admin_ != value) {
        admin_ = value;
        SetDirty();
    }
}

void Workspace::LoadFromJSON(Json::Value n) {
    SetID(n["id"].asUInt64());
    SetName(n["name"].asString());
    SetPremium(n["premium"].asBool());
    SetOnlyAdminsMayCreateProjects(
        n["only_admins_may_create_projects"].asBool());
    SetAdmin(n["admin"].asBool());
}

}   // namespace toggl
