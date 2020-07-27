// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_

#include <string>

#include "types.h"

#include <json/json.h>  // NOLINT

#include <Poco/Types.h>

#include "model/base_model.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT Client : public BaseModel {
 public:
    Client() : BaseModel() {}
    // Before undeleting, see how the copy constructor in BaseModel works
    Client(const Client &o) = delete;
    Client &operator=(const Client &o) = delete;

    Property<Poco::UInt64> WID { 0 };
    Property<std::string> Name { "" };

    void SetWID(Poco::UInt64 value);
    void SetName(const std::string &value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    void LoadFromJSON(const Json::Value &value, bool);
    Json::Value SaveToJSON(int apiVersion = 8) const override;
    Json::Value SyncMetadata() const override;
    Json::Value SyncPayload() const override;
    bool ResolveError(const toggl::error &err) override;
    bool ResourceCannotBeCreated(const toggl::error &err) const override;

 private:
    static bool nameHasAlreadyBeenTaken(const error &err);
};

}  // namespace toggl

#endif  // SRC_CLIENT_H_
