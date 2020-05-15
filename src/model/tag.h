// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TAG_H_
#define SRC_TAG_H_

#include <string>

#include <Poco/Types.h>

#include "model/base_model.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT Tag : public BaseModel {
 public:
    Tag() : BaseModel() {}

    virtual bool IsDirty() const override {
        return BaseModel::IsDirty() || IsAnyPropertyDirty(WID, Name);
    }
    virtual void ClearDirty() override {
        BaseModel::ClearDirty();
        AllPropertiesClearDirty(WID, Name);
    }

    Property<Poco::UInt64> WID { 0 };
    Property<std::string> Name { "" };

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    void LoadFromJSON(Json::Value data) override;
};

}  // namespace toggl

#endif  // SRC_TAG_H_
