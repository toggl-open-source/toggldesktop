// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_PROJECT_H_
#define SRC_PROJECT_H_

#include <string>
#include <vector>

#include "model/base_model.h"
#include "types.h"

#include <Poco/Types.h>

namespace toggl {

class TOGGL_INTERNAL_EXPORT Project : public BaseModel {
 public:
    Project() : BaseModel() {}

    virtual bool IsDirty() const override {
        return BaseModel::IsDirty() || IsAnyPropertyDirty(Name, Color, ClientGUID, ClientName, WID, CID, Active, Private, Billable);
    }
    virtual void ClearDirty() override {
        BaseModel::ClearDirty();
        AllPropertiesClearDirty(Name, Color, ClientGUID, ClientName, WID, CID, Active, Private, Billable);
    }

    Property<std::string> Name { "" };
    Property<std::string> Color { "" };
    Property<std::string> ClientGUID { "" };
    Property<std::string> ClientName { "" };
    Property<Poco::UInt64> WID { 0 };
    Property<Poco::UInt64> CID { 0 };
    Property<bool> Active { false };
    Property<bool> Private { false };
    Property<bool> Billable { false };

    std::string ColorCode() const;
    error SetColorCode(const std::string &color_code);

    std::string FullName() const;

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    void LoadFromJSON(Json::Value value) override;
    Json::Value SaveToJSON(int apiVersion = 8) const override;
    bool DuplicateResource(const toggl::error &err) const override;
    bool ResourceCannotBeCreated(const toggl::error &err) const override;
    bool ResolveError(const toggl::error &err) override;

    static std::vector<std::string> ColorCodes;

 private:
    bool clientIsInAnotherWorkspace(const toggl::error &err) const;
    bool onlyAdminsCanChangeProjectVisibility(const toggl::error &err) const;
};

template<typename T, size_t N> T *end(T (&ra)[N]);

}  // namespace toggl

#endif  // SRC_PROJECT_H_
