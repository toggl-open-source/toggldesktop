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

    std::string Tags() const;
    void SetTags(const std::string tags);

    Poco::UInt64 ID() const { return id_; }
    void SetID(const Poco::UInt64 value);

    Poco::UInt64 WID() const { return wid_; }
    void SetWID(const Poco::UInt64 value);

    Poco::UInt64 UID() const { return uid_; }
    void SetUID(const Poco::UInt64 value);

    Poco::UInt64 PID() const { return pid_; }
    void SetPID(const Poco::UInt64 value);

    Poco::UInt64 TID() const { return tid_; }
    void SetTID(const Poco::UInt64 value);

    Poco::UInt64 UIModifiedAt() const { return ui_modified_at_; }
    void SetUIModifiedAt(const Poco::UInt64 value);

    bool Billable() const { return billable_; }
    void SetBillable(const bool value);

    Poco::Int64 DurationInSeconds() const { return duration_in_seconds_; }
    void SetDurationInSeconds(const Poco::Int64 value);

    std::string DurationString() const;
    void SetDurationString(const std::string value);

    Poco::Int64 LocalID() const { return local_id_; }
    void SetLocalID(const Poco::Int64 value) { local_id_ = value; }

    bool DurOnly() const { return duronly_; }
    void SetDurOnly(const bool value);

    std::string Description() const { return description_; }
    void SetDescription(const std::string value);

    std::string GUID() const { return guid_; }
    void SetGUID(const std::string value);

    std::string StartString() const;
    void SetStartString(const std::string value);

    Poco::UInt64 Start() const { return start_; }
    void SetStart(const Poco::UInt64 value);

    std::string DateHeaderString() const;

    std::string StopString() const;
    void SetStopString(const std::string value);

    Poco::UInt64 Stop() { return stop_; }
    void SetStop(const Poco::UInt64 value);

    bool Dirty() const { return dirty_; }
    void ClearDirty() { dirty_ = false; }

    std::string CreatedWith() const { return created_with_; }
    void SetCreatedWith(const std::string value);
    // Deleting a time entry hides it from
    // UI and flags it for removal from
    // server:
    Poco::UInt64 DeletedAt() const { return deleted_at_; }
    void SetDeletedAt(const Poco::UInt64 value);

    Poco::UInt64 UpdatedAt() const { return updated_at_; }
    void SetUpdatedAt(const Poco::UInt64 value);

    std::string UpdatedAtString() const;
    void SetUpdatedAtString(const std::string value);

    // When time entry is finally deleted
    // on server, it will be removed from local
    // DB using this flag:
    bool IsMarkedAsDeletedOnServer() const {
        return is_marked_as_deleted_on_server_;
    }
    void MarkAsDeletedOnServer() {
        is_marked_as_deleted_on_server_ = true;
        dirty_ = true;
    }

    void StopAt(const Poco::Int64 at);

    void LoadFromJSONString(const std::string json);

    std::string String() const;

    bool NeedsPush() const;
    bool NeedsPOST() const;
    bool NeedsPUT() const;
    bool NeedsDELETE() const;

    bool IsToday() const;

    std::vector<std::string> TagNames;

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

  bool CompareTimeEntriesByStart(TimeEntry *a, TimeEntry *b);

}  // namespace kopsik

#endif  // SRC_TIME_ENTRY_H_
