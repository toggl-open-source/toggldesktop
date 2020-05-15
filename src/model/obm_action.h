// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_OBM_ACTION_H_
#define SRC_OBM_ACTION_H_

#include <string>

#include <json/json.h>  // NOLINT

#include <Poco/Types.h>

#include "model/base_model.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT ObmAction : public BaseModel {
 public:
    ObmAction() : BaseModel() {}

    virtual bool IsDirty() const override {
        return BaseModel::IsDirty() || IsAnyPropertyDirty(ExperimentID, Key, Value);
    }
    virtual void ClearDirty() override {
        BaseModel::ClearDirty();
        AllPropertiesClearDirty(ExperimentID, Key, Value);
    }

    Property<Poco::UInt64> ExperimentID { 0 };
    Property<std::string> Key { "" };
    Property<std::string> Value { "" };

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    Json::Value SaveToJSON(int apiVersion = 8) const override;

 private:
};

class TOGGL_INTERNAL_EXPORT ObmExperiment : public BaseModel {
 public:
    ObmExperiment() : BaseModel() {}

    virtual bool IsDirty() const override {
        return BaseModel::IsDirty() || IsAnyPropertyDirty(Included, HasSeen, Nr, Actions);
    }
    virtual void ClearDirty() override {
        BaseModel::ClearDirty();
        AllPropertiesClearDirty(Included, HasSeen, Nr, Actions);
    }

    Property<bool> Included { false };
    Property<Poco::UInt64> Nr { 0 };
    Property<bool> HasSeen { false };
    Property<std::string> Actions { "" };

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
};

}  // namespace toggl

#endif  // SRC_OBM_ACTION_H_
