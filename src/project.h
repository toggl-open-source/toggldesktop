// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_PROJECT_H_
#define SRC_PROJECT_H_

#include <vector>
#include <string>

#include "./types.h"
#include "./base_model.h"

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
    , client_guid_("") {}

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
    void SetClientGUID(const std::string);

    const std::string &Name() const {
        return name_;
    }
    void SetName(const std::string value);

    const std::string &Color() const {
        return color_;
    }
    void SetColor(const std::string value);
    std::string ColorCode() const;

    const bool &Active() const {
        return active_;
    }
    void SetActive(const bool value);

    const bool &IsPrivate() const {
        return private_;
    }
    void SetPrivate(const bool value);

    std::string String() const;

    const bool &Billable() const {
        return billable_;
    }
    void SetBillable(const bool value);

    std::string ModelName() const {
        return "project";
    }
    std::string ModelURL() const {
        return "/api/v8/projects";
    }

    void LoadFromJSON(Json::Value value);
    Json::Value SaveToJSON() const;

    bool DuplicateResource(const toggl::error) const;
    bool ResourceCannotBeCreated(const toggl::error) const;
    bool ResolveError(const toggl::error);

 private:
    bool clientIsInAnotherWorkspace(const toggl::error) const;

    static std::vector<std::string> color_codes;

    Poco::UInt64 wid_;
    Poco::UInt64 cid_;
    std::string name_;
    std::string color_;
    bool active_;
    bool private_;
    bool billable_;
    std::string client_guid_;
};

template<typename T, size_t N> T *end(T (&ra)[N]);

}  // namespace toggl

#endif  // SRC_PROJECT_H_
