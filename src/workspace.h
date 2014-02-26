// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_WORKSPACE_H_
#define SRC_WORKSPACE_H_

#include <string>

#include "Poco/Types.h"

namespace kopsik {

  class Workspace {
  public:
    Workspace() : local_id_(0), id_(0), name_(""), uid_(0), dirty_(false),
      is_marked_as_deleted_on_server_(false), premium_(false) {}

    std::string String() const;

    Poco::Int64 LocalID() const { return local_id_; }
    void SetLocalID(const Poco::Int64 value) { local_id_ = value; }

    Poco::UInt64 ID() const { return id_; }
    void SetID(const Poco::UInt64 value);

    std::string Name() const { return name_; }
    void SetName(const std::string value);

    Poco::UInt64 UID() const { return uid_; }
    void SetUID(const Poco::UInt64 value);

    bool Dirty() const { return dirty_; }
    void ClearDirty() { dirty_ = false; }

    bool Premium() const { return premium_; }
    void SetPremium(const bool value);

    bool IsMarkedAsDeletedOnServer() const {
      return is_marked_as_deleted_on_server_;
    }
    void MarkAsDeletedOnServer() {
      is_marked_as_deleted_on_server_ = true;
      dirty_ = true;
    }

  private:
    Poco::Int64 local_id_;
    Poco::UInt64 id_;
    std::string name_;
    Poco::UInt64 uid_;
    bool dirty_;
    bool is_marked_as_deleted_on_server_;
    bool premium_;
  };

  bool CompareWorkspaceByName(Workspace *a, Workspace *b);

}  // namespace kopsik

#endif  // SRC_WORKSPACE_H_
