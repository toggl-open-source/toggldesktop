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
    , admin_(false)
    , projects_billable_by_default_(false)
    , business_(false)
    , locked_time_(0) {}

    const std::string &Name() const {
        return name_;
    }
    void SetName(const std::string &value);

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

    const bool &ProjectsBillableByDefault() const {
        return projects_billable_by_default_;
    }
    void SetProjectsBillableByDefault(const bool);

    const bool &Business() const {
        return business_;
    }
    void SetBusiness(const bool value);

    const time_t &LockedTime() const {
        return locked_time_;
    }
    void SetLockedTime(const time_t value);

    // Override BaseModel
    std::string String() const;
    std::string ModelName() const;
    std::string ModelURL() const;
    void LoadFromJSON(Json::Value value);
    void LoadSettingsFromJson(Json::Value value);

 private:
    std::string name_;
    bool premium_;
    bool only_admins_may_create_projects_;
    bool admin_;
    bool projects_billable_by_default_;
    bool business_;
    time_t locked_time_;
};

}  // namespace toggl

#endif  // SRC_WORKSPACE_H_
