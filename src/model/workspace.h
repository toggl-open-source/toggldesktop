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

    virtual bool IsDirty() const override {
        return BaseModel::IsDirty() || IsAnyPropertyDirty(Name, LockedTime, Premium, OnlyAdminsMayCreateProjects, Admin, ProjectsBillableByDefault, Business);
    }
    virtual void ClearDirty() override {
        BaseModel::ClearDirty();
        AllPropertiesClearDirty(Name, LockedTime, Premium, OnlyAdminsMayCreateProjects, Admin, ProjectsBillableByDefault, Business);
    }

    Property<std::string> Name { "" };
    Property<time_t> LockedTime { 0 };
    Property<bool> Premium { false };
    Property<bool> OnlyAdminsMayCreateProjects { false };
    Property<bool> Admin { false };
    Property<bool> ProjectsBillableByDefault { false };
    Property<bool> Business { false };

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    void LoadFromJSON(Json::Value value) override;

    void LoadSettingsFromJson(Json::Value value);
};

}  // namespace toggl

#endif  // SRC_WORKSPACE_H_
