// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TAG_H_
#define SRC_TAG_H_

#include <string>

#include <Poco/Types.h>

#include "base_model.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT Tag : public BaseModel {
    Tag(ProtectedBase *container)
        : BaseModel(container)
    , wid_(0)
    , name_("") {}

    Tag(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : BaseModel(container, rs)
    {
        auto parentCount = BaseModel::DatabaseColumns().size();
        loadFromDatabase(rs, parentCount + 0, name_, false);
        loadFromDatabase(rs, parentCount + 1, wid_);
        ClearDirty();
    }
 public:
    friend class ProtectedBase;

    const Poco::UInt64 &WID() const {
        return wid_;
    }
    void SetWID(const Poco::UInt64 value);

    const std::string &Name() const {
        return name_;
    }
    void SetName(const std::string &value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    void LoadFromJSON(Json::Value data) override;

    static std::string DatabaseTable() { return "tags"; }
    static std::list<std::string> DatabaseColumns() {
        std::list<std::string> columns = BaseModel::DatabaseColumns();
        columns.splice(columns.end(), {"name", "wid"});
        return columns;
    }
    static std::list<std::string> DatabaseOrder() {
        return { "name" };
    }

    inline static const Query query {
        Table {"tags"},
        Columns {"name", "wid"},
        Join { },
        OrderBy { "name" },
        &BaseModel::query
    };

 private:
    Poco::UInt64 wid_;
    std::string name_;
};

}  // namespace toggl

#endif  // SRC_TAG_H_
