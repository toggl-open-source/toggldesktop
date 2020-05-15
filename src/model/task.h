// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TASK_H_
#define SRC_TASK_H_

#include <string>

#include <json/json.h>  // NOLINT

#include <Poco/Types.h>

#include "model/base_model.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT Task : public BaseModel {
 public:
    Task() : BaseModel() {}

    virtual bool IsDirty() const override {
        return BaseModel::IsDirty() || IsAnyPropertyDirty(Name, WID, PID, Active);
    }
    virtual void ClearDirty() override {
        BaseModel::ClearDirty();
        AllPropertiesClearDirty(Name, WID, PID, Active);
    }

    Property<std::string> Name { "" };
    Property<Poco::UInt64> WID { 0 };
    Property<Poco::UInt64> PID { 0 };
    Property<bool> Active { false };

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    void LoadFromJSON(Json::Value value) override;
};

}  // namespace toggl

#endif  // SRC_TASK_H_
