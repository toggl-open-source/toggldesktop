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
    // Before undeleting, see how the copy constructor in BaseModel works
    ObmAction(const ObmAction &o) = delete;
    ObmAction &operator=(const ObmAction &o) = delete;

    Property<Poco::UInt64> ExperimentID { 0 };
    Property<std::string> Key { "" };
    Property<std::string> Value { "" };

    void SetKey(const std::string &value);
    void SetValue(const std::string &value);
    void SetExperimentID(Poco::UInt64 value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    Json::Value SaveToJSON(int apiVersion = 8) const override;
};

class TOGGL_INTERNAL_EXPORT ObmExperiment : public BaseModel {
 public:
    ObmExperiment() : BaseModel() {}
    // Before undeleting, see how the copy constructor in BaseModel works
    ObmExperiment(const ObmExperiment &o) = delete;
    ObmExperiment &operator=(const ObmExperiment &o) = delete;

    Property<std::string> Actions { "" };
    Property<Poco::UInt64> Nr { 0 };
    Property<bool> Included { false };
    Property<bool> HasSeen { false };

    void SetActions(const std::string &value);
    void SetNr(Poco::UInt64 value);
    void SetIncluded(bool value);
    void SetHasSeen(bool value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
};

}  // namespace toggl

#endif  // SRC_OBM_ACTION_H_
