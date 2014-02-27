// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_

#include <string>

#include "./types.h"

#include "Poco/Types.h"

#include "./base_model.h"

namespace kopsik {

  class Client : public BaseModel {
  public:
    Client()
      : BaseModel()
      , wid_(0)
      , name_("") {}

    Poco::UInt64 WID() const { return wid_; }
    void SetWID(const Poco::UInt64 value);

    std::string Name() const { return name_; }
    void SetName(const std::string value);

    std::string String() const;

    std::string ModelName() const { return "client"; }
    std::string ModelURL() const { return "/api/v8/clients"; }

  private:
    Poco::UInt64 wid_;
    std::string name_;
    bool dirty_;
  };

  bool CompareClientByName(Client *a, Client *b);

}  // namespace kopsik

#endif  // SRC_CLIENT_H_
