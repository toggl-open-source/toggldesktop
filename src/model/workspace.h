// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_WORKSPACE_H_
#define SRC_WORKSPACE_H_

#include <string>

#include <json/json.h>  // NOLINT

#include "model/base_model.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT Workspace : public BaseModel {
 public:
    Workspace() : BaseModel() {}

    Property<std::string> Name { "" };
    Property<time_t> LockedTime { 0 };
    Property<bool> Premium { false };
    Property<bool> OnlyAdminsMayCreateProjects { false };
    Property<bool> Admin { false };
    Property<bool> ProjectsBillableByDefault { false };
    Property<bool> Business { false };

    void SetName(const std::string &value);
    void SetPremium(const bool value);
    void SetOnlyAdminsMayCreateProjects(const bool);
    void SetAdmin(const bool);
    void SetProjectsBillableByDefault(const bool);
    void SetBusiness(const bool value);
    void SetLockedTime(const time_t value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    void LoadFromJSON(Json::Value value) override;

    void LoadSettingsFromJson(Json::Value value);
};

}  // namespace toggl

#endif  // SRC_WORKSPACE_H_
