// Copyright 2014 Toggl Desktop developers.

#include "./time_entry.h"

#include <sstream>

#include "./formatter.h"

#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include "Poco/LocalDateTime.h"

namespace kopsik {

bool TimeEntry::NeedsPush() {
    return NeedsPOST() || NeedsPUT() || NeedsDELETE();
}

void TimeEntry::StopAt(const Poco::Int64 at) {
    poco_assert(at);
    SetDurationInSeconds(at + DurationInSeconds());
    poco_assert(DurationInSeconds() > 0);
    SetStop(at);
    SetUIModifiedAt(time(0));
}

bool TimeEntry::NeedsPOST() {
    // No server side ID yet, meaning it's not POSTed yet
    return !id_ && !(deleted_at_ > 0);
}

bool TimeEntry::NeedsPUT() {
    // User has modified model via UI, needs a PUT
    return ui_modified_at_ > 0 && !(deleted_at_ > 0);
}

bool TimeEntry::NeedsDELETE() {
    // TE is deleted, needs a DELETE on server side
    return id_ && (deleted_at_ > 0);
}

std::string TimeEntry::String() {
    std::stringstream ss;
    ss  << "ID=" << id_
        << " local_id=" << local_id_
        << " description=" << description_
        << " wid=" << wid_
        << " guid=" << guid_
        << " pid=" << pid_
        << " tid=" << tid_
        << " start=" << start_
        << " stop=" << stop_
        << " duration=" << duration_in_seconds_
        << " billable=" << billable_
        << " duronly=" << duronly_
        << " tags=" << Tags()
        << " created_with=" << CreatedWith()
        << " ui_modified_at=" << ui_modified_at_
        << " deleted_at=" << deleted_at_
        << " updated_at=" << updated_at_;
    return ss.str();
}

void TimeEntry::SetDurOnly(bool value) {
    if (duronly_ != value) {
        duronly_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetDeletedAt(Poco::UInt64 value) {
    if (deleted_at_ != value) {
        deleted_at_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetStart(Poco::UInt64 value) {
    if (start_ != value) {
        start_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetUpdatedAt(Poco::UInt64 value) {
    if (updated_at_ != value) {
        updated_at_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetDescription(std::string value) {
    if (description_ != value) {
        description_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetGUID(std::string value) {
    if (guid_ != value) {
        guid_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetUIModifiedAt(Poco::UInt64 value) {
    if (ui_modified_at_ != value) {
        ui_modified_at_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetStopString(std::string value) {
    Poco::Int64 stop = Formatter::Parse8601(value);
    if (duration_in_seconds_ >= 0) {
        SetDurationInSeconds(stop - start_);
    }
    SetStop(stop);
}

void TimeEntry::SetCreatedWith(std::string value) {
    if (created_with_ != value) {
        created_with_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetBillable(bool value) {
    if (billable_ != value) {
        billable_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetWID(Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetStop(Poco::UInt64 value) {
    if (stop_ != value) {
        stop_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetTID(Poco::UInt64 value) {
    if (tid_ != value) {
        tid_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetUID(Poco::UInt64 value) {
    if (uid_ != value) {
        uid_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetTags(std::string tags) {
    if (Tags() != tags) {
        TagNames.clear();
        if (!tags.empty()) {
          std::stringstream ss(tags);
          while (ss.good()) {
              std::string tag;
              getline(ss, tag, '|');
              TagNames.push_back(tag);
          }
        }
        dirty_ = true;
    }
}

void TimeEntry::SetPID(Poco::UInt64 value) {
    if (pid_ != value) {
        pid_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetID(Poco::UInt64 value) {
    if (id_ != value) {
        id_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetDurationInSeconds(Poco::Int64 value) {
    if (duration_in_seconds_ != value) {
        duration_in_seconds_ = value;
        dirty_ = true;
    }
}

void TimeEntry::SetStartString(std::string value) {
    Poco::Int64 start = Formatter::Parse8601(value);
    if (duration_in_seconds_ < 0) {
        SetDurationInSeconds(-start);
    } else {
        SetStop(start + duration_in_seconds_);
    }
    SetStart(start);
}

void TimeEntry::SetDurationString(const std::string value) {
    int seconds = Formatter::ParseDurationString(value);
    if (duration_in_seconds_ < 0) {
        time_t now = time(0);
        time_t start = now - seconds;
        SetStart(start);
        SetDurationInSeconds(-start);
    } else {
        SetDurationInSeconds(seconds);
        SetStop(start_ + seconds);
    }
}

std::string TimeEntry::Tags() {
    std::stringstream ss;
    for (std::vector<std::string>::const_iterator it =
            TagNames.begin();
            it != TagNames.end();
            it++) {
        if (it != TagNames.begin()) {
            ss << "|";
        }
        ss << *it;
    }
    return ss.str();
}

std::string TimeEntry::DateHeaderString() {
    return Formatter::FormatDateHeader(start_);
}

std::string TimeEntry::DurationString() {
    return Formatter::FormatDurationInSecondsHHMMSS(duration_in_seconds_);
}

void TimeEntry::SetUpdatedAtString(std::string value) {
    SetUpdatedAt(Formatter::Parse8601(value));
}

std::string TimeEntry::StopString() {
    return Formatter::Format8601(stop_);
}

std::string TimeEntry::StartString() {
    return Formatter::Format8601(start_);
}

bool TimeEntry::IsToday() {
  Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(Start());
  Poco::LocalDateTime datetime(ts);
  Poco::LocalDateTime today;
  return today.year() == datetime.year() &&
      today.month() == datetime.month() &&
      today.day() == datetime.day();
}

bool CompareTimeEntriesByStart(TimeEntry *a, TimeEntry *b) {
    return a->Start() > b->Start();
}

}   // namespace kopsik
