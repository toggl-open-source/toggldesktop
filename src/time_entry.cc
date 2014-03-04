// Copyright 2014 Toggl Desktop developers.

// NB! Setters should not directly calculate
// anything besides setting the field asked.
// This is because same setters are used when
// loading from database, JSON etc. If time entry
// needs to be recalculated after some user
// action, the recalculation should be started
// from context, using specific functions, not
// setters.

#include "./time_entry.h"

#include <sstream>

#include "./formatter.h"
#include "./json.h"

#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include "Poco/LocalDateTime.h"

namespace kopsik {

void TimeEntry::StopAt(const Poco::Int64 at) {
    poco_assert(at);
    SetDurationInSeconds(at + DurationInSeconds());
    poco_assert(DurationInSeconds() > 0);
    SetStop(at);
    SetUIModifiedAt(time(0));
}

std::string TimeEntry::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " description=" << description_
        << " wid=" << wid_
        << " guid=" << GUID()
        << " pid=" << pid_
        << " tid=" << tid_
        << " start=" << start_
        << " stop=" << stop_
        << " duration=" << duration_in_seconds_
        << " billable=" << billable_
        << " duronly=" << duronly_
        << " tags=" << Tags()
        << " created_with=" << CreatedWith()
        << " ui_modified_at=" << UIModifiedAt()
        << " deleted_at=" << DeletedAt()
        << " updated_at=" << UpdatedAt();
    return ss.str();
}

void TimeEntry::SetDurOnly(const bool value) {
    if (duronly_ != value) {
        duronly_ = value;
        SetDirty();
    }
}

void TimeEntry::SetStart(const Poco::UInt64 value) {
    if (start_ != value) {
        start_ = value;
        SetDirty();
    }
}

void TimeEntry::SetDescription(const std::string value) {
    if (description_ != value) {
        description_ = value;
        SetDirty();
    }
}

void TimeEntry::SetStopString(const std::string value) {
    SetStop(Formatter::Parse8601(value));
}

void TimeEntry::SetCreatedWith(const std::string value) {
    if (created_with_ != value) {
        created_with_ = value;
        SetDirty();
    }
}

void TimeEntry::SetBillable(const bool value) {
    if (billable_ != value) {
        billable_ = value;
        SetDirty();
    }
}

void TimeEntry::SetWID(const Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        SetDirty();
    }
}

void TimeEntry::recalculateDuration() {
  if (duration_in_seconds_ >= 0) {
    SetDurationInSeconds(stop_ - start_);
  }
}

void TimeEntry::SetStop(const Poco::UInt64 value) {
  if (stop_ != value) {
    stop_ = value;
    SetDirty();
  }
  /* FIXME:
  stop_ = value;
  SetDirty();
  if (stop_ >= start_) {
    recalculateDuration();
    return;
  }
  // Stop time cannot be before start time, it'll get an error from backend.
  Poco::Timestamp ts =
    Poco::Timestamp::fromEpochTime(stop_) + 1*Poco::Timespan::DAYS;
  stop_ = ts.epochTime();
  poco_assert(stop_ >= start_);
  recalculateDuration();
  */
}

void TimeEntry::SetTID(const Poco::UInt64 value) {
    if (tid_ != value) {
        tid_ = value;
        SetDirty();
    }
}

void TimeEntry::SetTags(const std::string tags) {
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
        SetDirty();
    }
}

void TimeEntry::SetPID(const Poco::UInt64 value) {
    if (pid_ != value) {
        pid_ = value;
        SetDirty();
    }
}

void TimeEntry::SetDurationInSeconds(const Poco::Int64 value) {
    if (duration_in_seconds_ != value) {
        duration_in_seconds_ = value;
        SetDirty();
    }
}

void TimeEntry::SetStartString(const std::string value) {
    Poco::Int64 start = Formatter::Parse8601(value);
    /* FIXME: 
    if (duration_in_seconds_ < 0) {
        SetDurationInSeconds(-start);
    } else {
        SetStop(start + duration_in_seconds_);
    }
    */
    SetStart(start);
}

void TimeEntry::SetDurationString(const std::string value) {
    int seconds = Formatter::ParseDurationString(value);
    /* FIXME:
    if (duration_in_seconds_ < 0) {
        time_t now = time(0);
        time_t start = now - seconds;
        SetStart(start);
        SetDurationInSeconds(-start);
    } else {
        SetDurationInSeconds(seconds);
        SetStop(start_ + seconds);
    }
    */
    SetDurationInSeconds(seconds);
}

void TimeEntry::SetProjectGUID(const std::string value) {
    if (project_guid_ != value) {
        project_guid_ = value;
        SetDirty();
    }
}

std::string TimeEntry::Tags() const {
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

std::string TimeEntry::DateHeaderString() const {
    return Formatter::FormatDateHeader(start_);
}

std::string TimeEntry::DurationString() const {
    return Formatter::FormatDurationInSecondsHHMMSS(duration_in_seconds_);
}

std::string TimeEntry::StopString() const {
    return Formatter::Format8601(stop_);
}

std::string TimeEntry::StartString() const {
    return Formatter::Format8601(start_);
}

bool TimeEntry::IsToday() const {
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

void TimeEntry::LoadFromJSONNode(JSONNODE * const data) {
  poco_assert(data);

  Poco::UInt64 ui_modified_at =
      GetUIModifiedAtFromJSONNode(data);
  if (UIModifiedAt() > ui_modified_at) {
      Poco::Logger &logger = Poco::Logger::get("json");
      std::stringstream ss;
      ss  << "Will not overwrite time entry "
          << String()
          << " with server data because we have a ui_modified_at";
      logger.debug(ss.str());
      return;
  }

  JSONNODE_ITERATOR current_node = json_begin(data);
  JSONNODE_ITERATOR last_node = json_end(data);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "id") == 0) {
      SetID(json_as_int(*current_node));
    } else if (strcmp(node_name, "description") == 0) {
      SetDescription(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "guid") == 0) {
      SetGUID(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "wid") == 0) {
      SetWID(json_as_int(*current_node));
    } else if (strcmp(node_name, "pid") == 0) {
      SetPID(json_as_int(*current_node));
    } else if (strcmp(node_name, "tid") == 0) {
      SetTID(json_as_int(*current_node));
    } else if (strcmp(node_name, "start") == 0) {
      SetStartString(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "stop") == 0) {
      SetStopString(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "duration") == 0) {
      SetDurationInSeconds(json_as_int(*current_node));
    } else if (strcmp(node_name, "billable") == 0) {
      SetBillable(json_as_bool(*current_node));
    } else if (strcmp(node_name, "duronly") == 0) {
      SetDurOnly(json_as_bool(*current_node));
    } else if (strcmp(node_name, "tags") == 0) {
      loadTagsFromJSONNode(*current_node);
    } else if (strcmp(node_name, "created_with") == 0) {
      SetCreatedWith(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "at") == 0) {
      SetUpdatedAtString(std::string(json_as_string(*current_node)));
    }
    ++current_node;
  }

  SetUIModifiedAt(0);
}

void TimeEntry::loadTagsFromJSONNode(JSONNODE * const list) {
  poco_assert(list);

  TagNames.clear();

  JSONNODE_ITERATOR current_node = json_begin(list);
  JSONNODE_ITERATOR last_node = json_end(list);
  while (current_node != last_node) {
    std::string tag = std::string(json_as_string(*current_node));
    if (!tag.empty()) {
      TagNames.push_back(tag);
    }
    ++current_node;
  }
}

}   // namespace kopsik
