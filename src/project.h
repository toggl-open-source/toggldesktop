// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_PROJECT_H_
#define SRC_PROJECT_H_

#include <string>
#include <vector>

#include "./base_model.h"
#include "./types.h"

#include "Poco/Types.h"

namespace toggl {

class Project : public BaseModel {
 public:
    Project()
        : BaseModel()
    , wid_(0)
    , cid_(0)
    , name_("")
    , color_("")
    , active_(false)
    , private_(false)
    , billable_(false)
    , client_guid_("")
    , client_name_("") {}

    const Poco::UInt64 &WID() const {
        return wid_;
    }
    void SetWID(const Poco::UInt64 value);

    const Poco::UInt64 &CID() const {
        return cid_;
    }
    void SetCID(const Poco::UInt64 value);

    const std::string &ClientGUID() const {
        return client_guid_;
    }
    void SetClientGUID(const std::string &);

    const std::string &Name() const {
        return name_;
    }
    void SetName(const std::string &value);

    const std::string &Color() const {
        return color_;
    }
    void SetColor(const std::string &value);

    std::string ColorCode() const;
    error SetColorCode(const std::string &color_code);

    const bool &Active() const {
        return active_;
    }
    void SetActive(const bool value);

    const bool &IsPrivate() const {
        return private_;
    }
    void SetPrivate(const bool value);

    const bool &Billable() const {
        return billable_;
    }
    void SetBillable(const bool value);

    const std::string &ClientName() const {
        return client_name_;
    }
    void SetClientName(const std::string &value);

    std::string FullName() const;

    // Override BaseModel
    std::string String() const;
    std::string ModelName() const;
    std::string ModelURL() const;
    void LoadFromJSON(Json::Value value);
    Json::Value SaveToJSON() const;
    bool DuplicateResource(const toggl::error &err) const;
    bool ResourceCannotBeCreated(const toggl::error &err) const;
    bool ResolveError(const toggl::error &err);

    static std::vector<std::string> ColorCodes;

 private:
    bool clientIsInAnotherWorkspace(const toggl::error &err) const;
    bool onlyAdminsCanChangeProjectVisibility(const toggl::error &err) const;

    Poco::UInt64 wid_;
    Poco::UInt64 cid_;
    std::string name_;
    std::string color_;
    bool active_;
    bool private_;
    bool billable_;
    std::string client_guid_;
    std::string client_name_;
};

template<typename T, size_t N> T *end(T (&ra)[N]);

}  // namespace toggl

#endif  // SRC_PROJECT_H_
