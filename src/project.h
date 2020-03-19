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
 public:
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

    static std::string DatabaseTable() {
        return "projects";
    }
    static std::list<std::string> DatabaseColumns() {
        auto columns = BaseModel::DatabaseColumns();
        columns.splice(columns.end(), {"name", "wid", "color", "cid", "active", "billable", "client_guid", "clients.name"});
        return columns;
    };
    static std::list<std::string> DatabaseJoin() {
        return {"clients ON projects.cid = clients.id",
            "workspaces ON projects.wid = workspaces.id"};
    }
    static std::list<std::string> DatabaseOrder() {
        return {};
    }
    inline static const Query query {
        Table { "projects" },
        Columns { "name", "wid", "color", "cid", "active", "billable", "client_guid", "clients.name" },
        Join {
            "LEFT JOIN clients ON projects.cid = clients.id",
            "LEFT JOIN workspaces ON projects.wid = workspaces.id"
        },
        OrderBy {
            "workspaces.name COLLATE NOCASE ASC",
            "clients.name COLLATE NOCASE ASC",
            "projects.name COLLATE NOCASE ASC"
        },
        &BaseModel::query
    };
    Project(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : BaseModel(container, rs)
    {
        auto offset = BaseModel::DatabaseColumns().size();
        loadFromDatabase(rs, offset + 0, name_);
        loadFromDatabase(rs, offset + 1, wid_);
        loadFromDatabase(rs, offset + 2, color_, false);
        loadFromDatabase(rs, offset + 3, cid_, false);
        loadFromDatabase(rs, offset + 4, active_);
        loadFromDatabase(rs, offset + 5, billable_);
        loadFromDatabase(rs, offset + 6, client_guid_, false);
        loadFromDatabase(rs, offset + 7, client_name_, false);
        ClearDirty();
    }

    static std::vector<std::string> ColorCodes;

 private:
    bool clientIsInAnotherWorkspace(const toggl::error &err) const;
    bool onlyAdminsCanChangeProjectVisibility(const toggl::error &err) const;

    Poco::UInt64 wid_ { 0 };
    Poco::UInt64 cid_ { 0 };
    std::string name_ { "" };
    std::string color_ { "" };
    std::string client_guid_ { "" };
    std::string client_name_ { "" };
    bool active_ { false };
    bool private_ { false };
    bool billable_ { false };
};

template<typename T, size_t N> T *end(T (&ra)[N]);

}  // namespace toggl

#endif  // SRC_PROJECT_H_
