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
    Tag(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : BaseModel(container, rs)
    {
        if (!rs[4].isEmpty())
            name_ = rs[4].convert<decltype(name_)>();
        if (!rs[5].isEmpty())
            wid_ = rs[5].convert<decltype(wid_)>();
    }

 private:
    Poco::UInt64 wid_;
    std::string name_;
};

}  // namespace toggl

#endif  // SRC_TAG_H_
