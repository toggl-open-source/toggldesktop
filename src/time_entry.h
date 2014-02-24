// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIME_ENTRY_H_
#define SRC_TIME_ENTRY_H_

#include <string>
#include <vector>

#include "./types.h"

#include "Poco/Types.h"

namespace kopsik {

    class TimeEntry {
    public:
        TimeEntry() : local_id_(0),
            id_(0), guid_(""), wid_(0), pid_(0), tid_(0), billable_(false),
            start_(0), stop_(0), duration_in_seconds_(0), description_(""),
            duronly_(false), ui_modified_at_(0), uid_(0), dirty_(false),
            created_with_(""), deleted_at_(0),
            is_marked_as_deleted_on_server_(false),
            updated_at_(0) {}

        std::string Tags();
        void SetTags(std::string tags);

        Poco::UInt64 ID() { return id_; }
        void SetID(Poco::UInt64 value);

        Poco::UInt64 WID() { return wid_; }
        void SetWID(Poco::UInt64 value);

        Poco::UInt64 UID() { return uid_; }
        void SetUID(Poco::UInt64 value);

        Poco::UInt64 PID() { return pid_; }
        void SetPID(Poco::UInt64 value);

        Poco::UInt64 TID() { return tid_; }
        void SetTID(Poco::UInt64 value);

        Poco::UInt64 UIModifiedAt() { return ui_modified_at_; }
        void SetUIModifiedAt(Poco::UInt64 value);

        bool Billable() { return billable_; }
        void SetBillable(bool value);

        Poco::Int64 DurationInSeconds() { return duration_in_seconds_; }
        void SetDurationInSeconds(Poco::Int64 value);
        std::string DurationString();
        void SetDurationString(const std::string value);

        Poco::Int64 LocalID() { return local_id_; }
        void SetLocalID(Poco::Int64 value) { local_id_ = value; }

        bool DurOnly() { return duronly_; }
        void SetDurOnly(bool value);

        std::string Description() { return description_; }
        void SetDescription(std::string value);

        std::string GUID() { return guid_; }
        void SetGUID(std::string value);

        std::string StartString();
        void SetStartString(std::string value);
        Poco::UInt64 Start() { return start_; }
        void SetStart(Poco::UInt64 value);
        std::string DateHeaderString();

        std::string StopString();
        void SetStopString(std::string value);
        Poco::UInt64 Stop() { return stop_; }
        void SetStop(Poco::UInt64 value);

        bool Dirty() { return dirty_; }
        void ClearDirty() { dirty_ = false; }

        std::string CreatedWith() { return created_with_; }
        void SetCreatedWith(std::string value);
        // Deleting a time entry hides it from
        // UI and flags it for removal from
        // server:
        Poco::UInt64 DeletedAt() { return deleted_at_; }
        void SetDeletedAt(Poco::UInt64 value);

        Poco::UInt64 UpdatedAt() { return updated_at_; }
        void SetUpdatedAt(Poco::UInt64 value);
        std::string UpdatedAtString();
        void SetUpdatedAtString(std::string value);

        // When time entry is finally deleted
        // on server, it will be removed from local
        // DB using this flag:
        bool IsMarkedAsDeletedOnServer() {
            return is_marked_as_deleted_on_server_;
        }
        void MarkAsDeletedOnServer() {
            is_marked_as_deleted_on_server_ = true;
            dirty_ = true;
        }

        void StopAt(const Poco::Int64 at);

        std::vector<std::string> TagNames;

        void LoadFromJSONString(std::string json);

        std::string String();

        bool NeedsPush();
        bool NeedsPOST();
        bool NeedsPUT();
        bool NeedsDELETE();

        bool IsToday();

    private:
        Poco::Int64 local_id_;
        Poco::UInt64 id_;
        guid guid_;
        Poco::UInt64 wid_;
        Poco::UInt64 pid_;
        Poco::UInt64 tid_;
        bool billable_;
        Poco::UInt64 start_;
        Poco::UInt64 stop_;
        Poco::Int64 duration_in_seconds_;
        std::string description_;
        bool duronly_;
        // TE is the only model that can actually
        // be updated by user.
        Poco::UInt64 ui_modified_at_;
        Poco::UInt64 uid_;
        bool dirty_;
        std::string created_with_;
        Poco::UInt64 deleted_at_;
        bool is_marked_as_deleted_on_server_;
        Poco::UInt64 updated_at_;

        bool setDurationStringHHMMSS(const std::string value);
        bool setDurationStringHHMM(const std::string value);
        bool setDurationStringMMSS(const std::string value);
    };

}  // namespace kopsik

#endif  // SRC_TIME_ENTRY_H_
