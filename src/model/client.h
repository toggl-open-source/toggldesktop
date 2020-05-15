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

    virtual bool IsDirty() const override {
        return BaseModel::IsDirty() || IsAnyPropertyDirty(WID, Name);
    }
    virtual void ClearDirty() override {
        BaseModel::ClearDirty();
        AllPropertiesClearDirty(WID, Name);
    }

    Property<Poco::UInt64> WID;
    Property<std::string> Name;

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    void LoadFromJSON(Json::Value value) override;
    Json::Value SaveToJSON(int apiVersion = 8) const override;
    bool ResolveError(const toggl::error &err) override;
    bool ResourceCannotBeCreated(const toggl::error &err) const override;

 private:
    static bool nameHasAlreadyBeenTaken(const error &err);
};

}  // namespace toggl

#endif  // SRC_CLIENT_H_
