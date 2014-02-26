// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_PROJECT_H_
#define SRC_PROJECT_H_

#include <vector>
#include <string>

#include "./types.h"

#include "Poco/Types.h"

namespace kopsik {

  class Project {
  public:
    Project() : local_id_(0), id_(0), guid_(""), wid_(0), cid_(0),
      name_(""), uid_(0), dirty_(false), color_(""),
      active_(false), is_marked_as_deleted_on_server_(false),
      billable_(false) {}

    Poco::Int64 LocalID() const { return local_id_; }
    void SetLocalID(const Poco::Int64 value) { local_id_ = value; }

    Poco::UInt64 ID() const { return id_; }
    void SetID(const Poco::UInt64 value);

    guid GUID() const { return guid_; }
    void SetGUID(const std::string value);

    Poco::UInt64 WID() const { return wid_; }
    void SetWID(const Poco::UInt64 value);

    Poco::UInt64 CID() const { return cid_; }
    void SetCID(const Poco::UInt64 value);

    std::string UppercaseName() const;
    std::string Name() const { return name_; }
    void SetName(const std::string value);

    Poco::UInt64 UID() const { return uid_; }
    void SetUID(const Poco::UInt64 value);

    bool Dirty() const { return dirty_; }
    void ClearDirty() { dirty_ = false; }

    std::string Color() const { return color_; }
    void SetColor(const std::string value);
    std::string ColorCode() const;

    bool Active() const { return active_; }
    void SetActive(const bool value);

    std::string String() const;

    static std::vector<std::string> color_codes;

    bool IsMarkedAsDeletedOnServer() const {
      return is_marked_as_deleted_on_server_;
    }
    void MarkAsDeletedOnServer() {
      is_marked_as_deleted_on_server_ = true;
      dirty_ = true;
    }

    bool Billable() const { return billable_; }
    void SetBillable(const bool value);

  private:
    Poco::Int64 local_id_;
    Poco::UInt64 id_;
    guid guid_;
    Poco::UInt64 wid_;
    Poco::UInt64 cid_;
    std::string name_;
    Poco::UInt64 uid_;
    bool dirty_;
    std::string color_;
    bool active_;
    bool is_marked_as_deleted_on_server_;
    bool billable_;
  };

}  // namespace kopsik

#endif  // SRC_PROJECT_H_
