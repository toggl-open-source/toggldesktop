// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_WORKSPACE_H_
#define SRC_WORKSPACE_H_

#include <string>

#include <json/json.h>  // NOLINT

#include "base_model.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT Workspace : public BaseModel {
    inline static const std::string modelName{ kModelWorkspace };
    inline static const Query query{
        Query::Table{"workspaces"},
        Query::Columns {
            { "name", true },
            { "premium", true },
            { "only_admins_may_create_projects", true },
            { "admin", true },
            { "projects_billable_by_default", true },
            { "is_business", true },
            { "locked_time", true } 
        },
        Query::Join{},
        Query::OrderBy{"name"},
        &BaseModel::query
    };
    Workspace(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : BaseModel(container, rs)
    {
        size_t ptr{ query.Offset() };
        load(rs, query.IsRequired(ptr), ptr, name_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, premium_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, only_admins_may_create_projects_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, admin_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, projects_billable_by_default_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, business_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, locked_time_);
        ptr++;
        ClearDirty();
    }
    Workspace(ProtectedBase *container, const Json::Value &data)
        : BaseModel(container)
    {
        LoadFromJSON(data);
        EnsureGUID();
    }
    Workspace(ProtectedBase* container)
        : BaseModel(container)
    {}
public:
   friend class ProtectedBase;

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
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    void LoadFromJSON(Json::Value value) override;

    void LoadSettingsFromJson(Json::Value value);

 private:
    std::string name_ { "" };
    time_t locked_time_ { 0 };
    bool premium_ { false };
    bool only_admins_may_create_projects_ { false };
    bool admin_ { false };
    bool projects_billable_by_default_ { false };
    bool business_ { false };
};

}  // namespace toggl

#endif  // SRC_WORKSPACE_H_
