// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_

#include <string>

#include "./types.h"

#include <json/json.h>  // NOLINT

#include "Poco/Types.h"

#include "./base_model.h"

namespace toggl {

class Client : public BaseModel {
 public:
    Client()
        : BaseModel()
    , wid_(0)
    , name_("") {}

    const Poco::UInt64 &WID() const {
        return wid_;
    }
    void SetWID(const Poco::UInt64 value);

    const std::string &Name() const {
        return name_;
    }
    void SetName(const std::string &value);

    // Override BaseModel
    std::string String() const;
    std::string ModelName() const;
    std::string ModelURL() const;
    void LoadFromJSON(Json::Value value);
    Json::Value SaveToJSON() const;
    bool ResolveError(const toggl::error &err);
    bool ResourceCannotBeCreated(const toggl::error &err) const;

 private:
    Poco::UInt64 wid_;
    std::string name_;

    static bool nameHasAlreadyBeenTaken(const error &err);
};

}  // namespace toggl

#endif  // SRC_CLIENT_H_
