// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_PROJECT_H_
#define SRC_PROJECT_H_

#include <string>
#include <vector>

#include "base_model.h"
#include "types.h"

#include <Poco/Types.h>

namespace toggl {

class TOGGL_INTERNAL_EXPORT Project : public BaseModel {
    inline static const std::string modelName{ kModelProject };
    inline static const Query query{
        Query::Table{"projects"},
        Query::Columns {
            { "name", true },
            { "wid", true },
            { "color", false },
            { "cid", false },
            { "active", true },
            { "billable", true },
            { "client_guid", false },
            { "clients.name", false }
        },
        Query::Join{
            "LEFT JOIN clients ON projects.cid = clients.id",
            "LEFT JOIN workspaces ON projects.wid = workspaces.id"
        },
        Query::OrderBy{
            "workspaces.name COLLATE NOCASE ASC",
            "clients.name COLLATE NOCASE ASC",
            "projects.name COLLATE NOCASE ASC"
        },
        &BaseModel::query
    };
    Project(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : BaseModel(container, rs)
    {
        size_t ptr{ query.Offset() };
        load(rs, query.IsRequired(ptr), ptr, name_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, wid_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, color_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, cid_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, active_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, billable_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, client_guid_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, client_name_);
        ptr++;
        ClearDirty();
    }
    Project(ProtectedBase *container, const Json::Value &data)
        : BaseModel(container)
    {
        LoadFromJSON(data);
        updateClientName();
    }
    Project(ProtectedBase *container)
        : BaseModel(container)
    {}
 public:
    friend class ProtectedBase;

    const Poco::UInt64 &WID() const {
        return wid_;
    }
    void SetWID(const Poco::UInt64 value);

    const Poco::UInt64 &CID() const {
        return cid_;
    }
    void SetCID(const Poco::UInt64 value);

    const std::string &ClientGUID() const {
        return client_guid_;
    }
    void SetClientGUID(const std::string &);

    const std::string &Name() const {
        return name_;
    }
    void SetName(const std::string &value);

    const std::string &Color() const {
        return color_;
    }
    void SetColor(const std::string &value);

    std::string ColorCode() const;
    error SetColorCode(const std::string &color_code);

    const bool &Active() const {
        return active_;
    }
    void SetActive(const bool value);

    const bool &IsPrivate() const {
        return private_;
    }
    void SetPrivate(const bool value);

    const bool &Billable() const {
        return billable_;
    }
    void SetBillable(const bool value);

    const std::string &ClientName() const {
        return client_name_;
    }
    void SetClientName(const std::string &value);

    std::string FullName() const;

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    void LoadFromJSON(Json::Value value) override;
    Json::Value SaveToJSON() const override;
    bool DuplicateResource(const toggl::error &err) const override;
    bool ResourceCannotBeCreated(const toggl::error &err) const override;
    error ResolveError(const toggl::error &err) override;

    static std::vector<std::string> ColorCodes;

 private:
    bool clientIsInAnotherWorkspace(const toggl::error &err) const;
    bool onlyAdminsCanChangeProjectVisibility(const toggl::error &err) const;

    // moved over from JSON parsing in User
    void updateClientName();

    std::string name_ { "" };
    std::string color_ { "" };
    std::string client_guid_ { "" };
    std::string client_name_ { "" };
    Poco::UInt64 wid_ { 0 };
    Poco::UInt64 cid_ { 0 };
    bool active_ { false };
    bool private_ { false };
    bool billable_ { false };
};

}  // namespace toggl

#endif  // SRC_PROJECT_H_
