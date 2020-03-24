// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_WORKSPACE_H_
#define SRC_WORKSPACE_H_

#include <string>

#include <json/json.h>  // NOLINT

#include "base_model.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT Workspace : public BaseModel {
    Workspace(ProtectedBase *container)
        : BaseModel(container)
    {}
    Workspace(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : BaseModel(container, rs)
    {
        for (size_t i = 0; i < query.ColumnCount(); i++) {
            bool result = query.columns_[i].load(this, rs, query.Offset() + i);
        }
        ClearDirty();
    }
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

    inline static const std::string modelName { kModelWorkspace };
    inline static const Query query {
        Query::Table{"workspaces"},
        Query::Columns{
            Query::Bind("name", &Workspace::name_, Query::Binding::REQUIRED),
            Query::Bind("premium", &Workspace::premium_, Query::Binding::REQUIRED),
            Query::Bind("only_admins_may_create_projects", &Workspace::only_admins_may_create_projects_, Query::Binding::REQUIRED),
            Query::Bind("admin", &Workspace::admin_, Query::Binding::REQUIRED),
            Query::Bind("projects_billable_by_default", &Workspace::projects_billable_by_default_, Query::Binding::REQUIRED),
            Query::Bind("is_business", &Workspace::business_, Query::Binding::REQUIRED),
            Query::Bind("locked_time", &Workspace::locked_time_, Query::Binding::REQUIRED),
        },
        Query::Join{},
        Query::OrderBy{"name"},
        &BaseModel::query
    };
};

}  // namespace toggl

#endif  // SRC_WORKSPACE_H_
