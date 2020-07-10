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

    Property<std::string> Name { "" };
    Property<Poco::UInt64> WID { 0 };

    void SetName(const std::string &value);
    void SetWID(Poco::UInt64 value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    void LoadFromJSON(const Json::Value &data);
};

}  // namespace toggl

#endif  // SRC_TAG_H_
