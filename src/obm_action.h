// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_OBM_ACTION_H_
#define SRC_OBM_ACTION_H_

#include <string>

#include <json/json.h>  // NOLINT

#include <Poco/Types.h>

#include "base_model.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT ObmAction : public BaseModel {
    ObmAction(ProtectedContainerBase *container)
        : BaseModel(container)
    , experiment_id_(0)
    , key_("")
    , value_("") {}
 public:
    friend class ProtectedContainer<ObmAction>;

    const std::string &Key() const {
        return key_;
    }
    void SetKey(const std::string &value);

    const std::string &Value() const {
        return value_;
    }
    void SetValue(const std::string &value);

    const Poco::UInt64 &ExperimentID() const {
        return experiment_id_;
    }
    void SetExperimentID(const Poco::UInt64 value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    Json::Value SaveToJSON() const override;

 private:
    Poco::UInt64 experiment_id_;
    std::string key_;
    std::string value_;
};

class TOGGL_INTERNAL_EXPORT ObmExperiment : public BaseModel {
    ObmExperiment(ProtectedContainerBase *container)
        : BaseModel(container)
    , included_(false)
    , nr_(0)
    , has_seen_(false)
    , actions_("") {}
 public:
   friend class ProtectedContainer<ObmExperiment>;

    const bool &Included() const {
        return included_;
    }
    void SetIncluded(const bool value);

    const bool &HasSeen() const {
        return has_seen_;
    }
    void SetHasSeen(const bool value);

    const Poco::UInt64 &Nr() const {
        return nr_;
    }
    void SetNr(const Poco::UInt64 value);

    const std::string &Actions() const {
        return actions_;
    }
    void SetActions(const std::string &value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;

 private:
    bool included_;
    Poco::UInt64 nr_;
    bool has_seen_;
    std::string actions_;
};

}  // namespace toggl

#endif  // SRC_OBM_ACTION_H_
