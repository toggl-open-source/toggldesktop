// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_

#include <string>

#include "types.h"

#include <json/json.h>  // NOLINT

#include <Poco/Types.h>

#include "base_model.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT Client : public BaseModel {
    inline static const std::string modelName{ kModelClient };
    inline static const Query query{
        Query::Table{"clients"},
        Query::Columns {
            { "name", true },
            { "wid", true }
        },
        Query::Join{},
        Query::OrderBy{"name"},
        &BaseModel::query
    };
    Client(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : BaseModel(container, rs)
    {
        size_t ptr{ query.Offset() };
        load(rs, query.IsRequired(ptr), ptr, name_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, wid_);
        ptr++;
        ClearDirty();
    }
    Client(ProtectedBase *container, const Json::Value &data)
        : BaseModel(container)
    {
        LoadFromJSON(data);
        EnsureGUID();
    }
    Client(ProtectedBase *container)
        : BaseModel(container)
    {}
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
    void LoadFromJSON(Json::Value value) override;
    Json::Value SaveToJSON() const override;
    error ResolveError(const toggl::error &err) override;
    bool ResourceCannotBeCreated(const toggl::error &err) const override;

 private:
    Poco::UInt64 wid_ { 0 };
    std::string name_ { "" };

    static bool nameHasAlreadyBeenTaken(const error &err);
};

}  // namespace toggl

#endif  // SRC_CLIENT_H_
