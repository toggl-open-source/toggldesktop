// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_WORKSPACE_H_
#define SRC_WORKSPACE_H_

#include <string>

#include <json/json.h>  // NOLINT

#include "./base_model.h"

namespace toggl {

class Workspace : public BaseModel {
 public:
    Workspace()
        : BaseModel()
    , name_("")
    , premium_(false)
    , only_admins_may_create_projects_(false)
    , admin_(false) {}

    std::string String() const;

    const std::string &Name() const {
        return name_;
    }
    void SetName(const std::string value);

    const bool &Premium() const {
        return premium_;
    }
    void SetPremium(const bool value);

    const bool &OnlyAdminsMayCreateProjects() const {
        return only_admins_may_create_projects_;
    }
    void SetOnlyAdminsMayCreateProjects(const bool);

    const bool &Admin() const {
        return admin_;
    }
    void SetAdmin(const bool);

    std::string ModelName() const {
        return "workspace";
    }

    std::string ModelURL() const {
        return "/api/v8/workspaces";
    }

    void LoadFromJSON(Json::Value value);

 private:
    std::string name_;
    bool premium_;
    bool only_admins_may_create_projects_;
    bool admin_;
};

bool CompareWorkspaceByName(Workspace *a, Workspace *b);

}  // namespace toggl

#endif  // SRC_WORKSPACE_H_
