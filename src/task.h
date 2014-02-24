// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TASK_H_
#define SRC_TASK_H_

#include <string>

#include "Poco/Types.h"

namespace kopsik {

    class Task {
    public:
        Task() : local_id_(0), id_(0), name_(""), wid_(0), pid_(0), uid_(0),
            dirty_(false), is_marked_as_deleted_on_server_(false) {}

        Poco::Int64 LocalID() { return local_id_; }
        void SetLocalID(Poco::Int64 value) { local_id_ = value; }
        Poco::UInt64 ID() { return id_; }
        void SetID(Poco::UInt64 value);
        std::string Name() { return name_; }
        void SetName(std::string value);
        Poco::UInt64 WID() { return wid_; }
        void SetWID(Poco::UInt64 value);
        Poco::UInt64 PID() { return pid_; }
        void SetPID(Poco::UInt64 value);
        Poco::UInt64 UID() { return uid_; }
        void SetUID(Poco::UInt64 value);
        bool Dirty() { return dirty_; }
        void ClearDirty() { dirty_ = false; }

        std::string String();

        bool IsMarkedAsDeletedOnServer() {
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
        Poco::UInt64 wid_;
        Poco::UInt64 pid_;
        Poco::UInt64 uid_;
        bool dirty_;
        bool is_marked_as_deleted_on_server_;
    };

}  // namespace kopsik

#endif  // SRC_TASK_H_
