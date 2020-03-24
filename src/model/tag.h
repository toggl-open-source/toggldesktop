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
    void LoadFromJSON(Json::Value data) override;

 private:
    Poco::UInt64 wid_ { 0 };
    std::string name_ { "" };
};

}  // namespace toggl

#endif  // SRC_TAG_H_
