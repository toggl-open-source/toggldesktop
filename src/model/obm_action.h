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

    Property<Poco::UInt64> ExperimentID { 0 };
    Property<std::string> Key { "" };
    Property<std::string> Value { "" };

    void SetKey(const std::string &value);
    void SetValue(const std::string &value);
    void SetExperimentID(const Poco::UInt64 value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    Json::Value SaveToJSON() const override;
};

class TOGGL_INTERNAL_EXPORT ObmExperiment : public BaseModel {
 public:
    ObmExperiment() : BaseModel() {}

    Property<std::string> Actions { "" };
    Property<Poco::UInt64> Nr { 0 };
    Property<bool> Included { false };
    Property<bool> HasSeen { false };

    void SetActions(const std::string &value);
    void SetNr(const Poco::UInt64 value);
    void SetIncluded(const bool value);
    void SetHasSeen(const bool value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
};

}  // namespace toggl

#endif  // SRC_OBM_ACTION_H_
