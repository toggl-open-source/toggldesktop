// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIME_ENTRY_H_
#define SRC_TIME_ENTRY_H_

#include <string>
#include <vector>

#include "./types.h"
#include "./base_model.h"

#include "Poco/Types.h"

namespace kopsik {

  class TimeEntry : public BaseModel {
  public:
    TimeEntry()
      : BaseModel()
      , wid_(0)
      , pid_(0)
      , tid_(0)
      , billable_(false)
      , start_(0)
      , stop_(0)
      , duration_in_seconds_(0)
      , description_("")
      , duronly_(false)
      , created_with_("") {}
    virtual ~TimeEntry() {}

    std::string Tags() const;
    void SetTags(const std::string tags);

    Poco::UInt64 WID() const { return wid_; }
    void SetWID(const Poco::UInt64 value);

    Poco::UInt64 PID() const { return pid_; }
    void SetPID(const Poco::UInt64 value);

    Poco::UInt64 TID() const { return tid_; }
    void SetTID(const Poco::UInt64 value);

    bool Billable() const { return billable_; }
    void SetBillable(const bool value);

    Poco::Int64 DurationInSeconds() const { return duration_in_seconds_; }
    void SetDurationInSeconds(const Poco::Int64 value);

    std::string DurationString() const;
    void SetDurationString(const std::string value);

    bool DurOnly() const { return duronly_; }
    void SetDurOnly(const bool value);

    std::string Description() const { return description_; }
    void SetDescription(const std::string value);

    std::string StartString() const;
    void SetStartString(const std::string value);

    Poco::UInt64 Start() const { return start_; }
    void SetStart(const Poco::UInt64 value);

    std::string DateHeaderString() const;

    std::string StopString() const;
    void SetStopString(const std::string value);

    Poco::UInt64 Stop() { return stop_; }
    void SetStop(const Poco::UInt64 value);

    std::string CreatedWith() const { return created_with_; }
    void SetCreatedWith(const std::string value);

    void StopAt(const Poco::Int64 at);

    std::string String() const;

    bool IsToday() const;

    std::vector<std::string> TagNames;

    std::string ModelName() const { return "time_entry"; }
    std::string ModelURL() const { return "/api/v8/time_entries"; }

  private:
    Poco::UInt64 wid_;
    Poco::UInt64 pid_;
    Poco::UInt64 tid_;
    bool billable_;
    Poco::UInt64 start_;
    Poco::UInt64 stop_;
    Poco::Int64 duration_in_seconds_;
    std::string description_;
    bool duronly_;
    std::string created_with_;

    bool setDurationStringHHMMSS(const std::string value);
    bool setDurationStringHHMM(const std::string value);
    bool setDurationStringMMSS(const std::string value);
  };

  bool CompareTimeEntriesByStart(TimeEntry *a, TimeEntry *b);

}  // namespace kopsik

#endif  // SRC_TIME_ENTRY_H_
