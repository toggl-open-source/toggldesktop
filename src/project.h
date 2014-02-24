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

    Poco::Int64 LocalID() { return local_id_; }
    void SetLocalID(Poco::Int64 value) { local_id_ = value; }
    Poco::UInt64 ID() { return id_; }
    void SetID(Poco::UInt64 value);
    guid GUID() { return guid_; }
    void SetGUID(std::string value);
    Poco::UInt64 WID() { return wid_; }
    void SetWID(Poco::UInt64 value);
    Poco::UInt64 CID() { return cid_; }
    void SetCID(Poco::UInt64 value);

    std::string UppercaseName();
    std::string Name() { return name_; }
    void SetName(std::string value);

    Poco::UInt64 UID() { return uid_; }
    void SetUID(Poco::UInt64 value);

    bool Dirty() { return dirty_; }
    void ClearDirty() { dirty_ = false; }

    std::string Color() { return color_; }
    void SetColor(std::string value);
    std::string ColorCode();

    bool Active() { return active_; }
    void SetActive(const bool value);

    std::string String();

    static std::vector<std::string> color_codes;

    bool IsMarkedAsDeletedOnServer() {
      return is_marked_as_deleted_on_server_;
    }
    void MarkAsDeletedOnServer() {
      is_marked_as_deleted_on_server_ = true;
      dirty_ = true;
    }

    bool Billable() { return billable_; }
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
