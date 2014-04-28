// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_PROJECT_H_
#define SRC_PROJECT_H_

#include <vector>
#include <string>

#include "./types.h"
#include "./base_model.h"

#include "Poco/Types.h"

namespace kopsik {

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
    , billable_(false) {}

    Poco::UInt64 WID() const {
        return wid_;
    }
    void SetWID(const Poco::UInt64 value);

    Poco::UInt64 CID() const {
        return cid_;
    }
    void SetCID(const Poco::UInt64 value);

    std::string UppercaseName() const;
    std::string Name() const {
        return name_;
    }
    void SetName(const std::string value);

    std::string Color() const {
        return color_;
    }
    void SetColor(const std::string value);
    std::string ColorCode() const;

    bool Active() const {
        return active_;
    }
    void SetActive(const bool value);

    bool IsPrivate() const {
        return private_;
    }
    void SetPrivate(const bool value);

    std::string String() const;

    static std::vector<std::string> color_codes;

    bool Billable() const {
        return billable_;
    }
    void SetBillable(const bool value);

    std::string ModelName() const {
        return "project";
    }
    std::string ModelURL() const {
        return "/api/v8/projects";
    }

    void LoadFromJSONNode(JSONNODE * const);
    JSONNODE *SaveToJSONNode() const;

    bool DuplicateResource(const kopsik::error) const;
    bool ResourceCannotBeCreated(const kopsik::error) const;
    bool ResolveError(const kopsik::error);

 private:
    bool userCannotAddOrEditProjectsInWorkspace(const error) const;

    Poco::UInt64 wid_;
    Poco::UInt64 cid_;
    std::string name_;
    std::string color_;
    bool active_;
    bool private_;
    bool billable_;
};

}  // namespace kopsik

#endif  // SRC_PROJECT_H_
