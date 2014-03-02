// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TAG_H_
#define SRC_TAG_H_

#include <string>

#include "./types.h"

#include "Poco/Types.h"

#include "./base_model.h"

namespace kopsik {

  class Tag : public BaseModel {
  public:
    Tag()
      : BaseModel()
      , wid_(0)
      , name_("") {}

    Poco::UInt64 WID() const { return wid_; }
    void SetWID(const Poco::UInt64 value);

    std::string Name() const { return name_; }
    void SetName(const std::string value);

    std::string String() const;

    std::string ModelName() const { return "tag"; }
    std::string ModelURL() const { return "/api/v8/tags"; }

    void LoadFromJSONNode(JSONNODE * const data);

  private:
    Poco::UInt64 wid_;
    std::string name_;
  };

}  // namespace kopsik

#endif  // SRC_TAG_H_
