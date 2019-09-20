// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_MODEL_CHANGE_H_
#define SRC_MODEL_CHANGE_H_

#include <string>

#include "Poco/Types.h"

#include "./const.h"

namespace toggl {

class ModelChange {
 public:
    ModelChange(
        const std::string &model_type,
        const std::string &change_type,
        const Poco::UInt64 model_id,
        const std::string &GUID)
        : model_type_(model_type)
    , change_type_(change_type)
    , model_id_(model_id)
    , GUID_(GUID) {}

    std::string GUID() const {
        return GUID_;
    }
    std::string ModelType() const {
        return model_type_;
    }
    Poco::UInt64 ModelID() const {
        return model_id_;
    }
    std::string ChangeType() const {
        return change_type_;
    }
    bool IsDeletion() const {
        return kChangeTypeDelete == change_type_;
    }

 private:
    std::string model_type_;
    std::string change_type_;
    Poco::UInt64 model_id_;
    std::string GUID_;
};

}  // namespace toggl

#endif  // SRC_MODEL_CHANGE_H_
