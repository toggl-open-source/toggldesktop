// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TASK_H_
#define SRC_TASK_H_

#include <string>

#include <json/json.h>  // NOLINT

#include <Poco/Types.h>

#include "base_model.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT Task : public BaseModel {
    inline static const std::string modelName{ kModelTask };
    inline static const Query query{
        Query::Table{"tasks"},
        Query::Columns {
            { "name", true },
            { "wid", true },
            { "pid", false },
            { "active", true }
        },
        Query::Join{},
        Query::OrderBy{"name"},
        &BaseModel::query
    };
    Task(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : BaseModel(container, rs)
    {
        size_t ptr{ query.Offset() };
        load(rs, query.IsRequired(ptr), ptr, name_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, wid_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, pid_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, active_);
        ptr++;
        ClearDirty();
    }
    Task(ProtectedBase *container, const Json::Value &data)
        : BaseModel(container)
    {
        LoadFromJSON(data);
        ClearDirty();
    }
    Task(ProtectedBase *container)
        : BaseModel(container)
    {}
 public:
    friend class ProtectedBase;

    const std::string &Name() const {
        return name_;
    }
    void SetName(const std::string &value);

    const Poco::UInt64 &WID() const {
        return wid_;
    }
    void SetWID(const Poco::UInt64 value);

    const Poco::UInt64 &PID() const {
        return pid_;
    }
    void SetPID(const Poco::UInt64 value);

    const bool &Active() const {
        return active_;
    }
    void SetActive(const bool value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    void LoadFromJSON(Json::Value value) override;

 private:
    std::string name_ { " "};
    Poco::UInt64 wid_ { 0 };
    Poco::UInt64 pid_ { 0 };
    bool active_ { false };
};

}  // namespace toggl

#endif  // SRC_TASK_H_
