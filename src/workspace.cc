// Copyright 2014 Toggl Desktop developers.

#include "./workspace.h"

#include <sstream>
#include <cstring>

namespace kopsik {

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

bool CompareWorkspaceByName(Workspace *a, Workspace *b) {
    return (strcmp(a->Name().c_str(), b->Name().c_str()) < 0);
}

void Workspace::LoadFromJSONNode(JSONNODE * const n) {
    poco_check_ptr(n);

    Poco::UInt64 id(0);
    std::string name("");
    bool premium(false);
    bool only_admins_may_create_projects(false);
    bool admin(false);

    JSONNODE_ITERATOR i = json_begin(n);
    JSONNODE_ITERATOR e = json_end(n);
    while (i != e) {
        json_char *node_name = json_name(*i);
        if (strcmp(node_name, "id") == 0) {
            id = json_as_int(*i);
        } else if (strcmp(node_name, "name") == 0) {
            name = std::string(json_as_string(*i));
        } else if (strcmp(node_name, "premium") == 0) {
            premium = json_as_bool(*i) != 0;
        } else if (strcmp(node_name, "only_admins_may_create_projects") == 0) {
            only_admins_may_create_projects = json_as_bool(*i) != 0;
        } else if (strcmp(node_name, "admin") == 0) {
            admin = json_as_bool(*i) != 0;
        }
        ++i;
    }

    SetID(id);
    SetName(name);
    SetPremium(premium);
    SetOnlyAdminsMayCreateProjects(only_admins_may_create_projects);
    SetAdmin(admin);
}

}   // namespace kopsik
