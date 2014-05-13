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
#include <algorithm>

#include "./formatter.h"
#include "./json.h"
#include "./const.h"

#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include "Poco/LocalDateTime.h"

namespace kopsik {

bool TimeEntry::ResolveError(const kopsik::error err) {
    if (durationTooLarge(err) && Stop() && Start()) {
        Poco::UInt64 seconds =
            (std::min)(Stop() - Start(),
                       Poco::UInt64(kMaxTimeEntryDurationSeconds));
        SetDurationInSeconds(seconds);
        return true;
    }
    if (stopTimeMustBeAfterStartTime(err) && Stop() && Start()) {
        SetStop(Start() + DurationInSeconds());
        return true;
    }
    if (userCannotAccessWorkspace(err)) {
        SetWID(0);
        SetPID(0);
        SetTID(0);
        return true;
    }
    if (userCannotAccessTheSelectedProject(err)) {
        SetPID(0);
        SetTID(0);
        return true;
    }
    if (userCannotAccessSelectedTask(err)) {
        SetTID(0);
        return true;
    }
    return false;
}

bool TimeEntry::userCannotAccessTheSelectedProject(
    const kopsik::error err) const {
    return (std::string::npos != std::string(err).find(
        "User cannot access the selected project"));
}

bool TimeEntry::userCannotAccessSelectedTask(
    const kopsik::error err) const {
    return (std::string::npos != std::string(err).find(
        "User cannot access selected task."));
}

bool TimeEntry::durationTooLarge(const kopsik::error err) const {
    return (std::string::npos != std::string(err).find(
        "Max allowed duration per 1 time entry is 1000 hours"));
}

bool TimeEntry::stopTimeMustBeAfterStartTime(const kopsik::error err) const {
    return (std::string::npos != std::string(err).find(
        "Stop time must be after start time"));
}

void TimeEntry::StopAt(const Poco::UInt64 at) {
    if (!IsTracking()) {
        return;
    }

    poco_assert(at);

    Poco::Int64 duration = at + DurationInSeconds();
    if (duration < 0) {
        duration = -1 * duration;
    }

    SetDurationInSeconds(duration);

    poco_assert(DurationInSeconds() >= 0);

    SetStop(at);

    SetUIModified();
}

void TimeEntry::StopTracking() {
    StopAt(time(0));
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

void TimeEntry::SetStop(const Poco::UInt64 value) {
    if (stop_ != value) {
        stop_ = value;
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

void TimeEntry::SetStopUserInput(const std::string value) {
    SetStopString(value);

    if (Stop() < Start()) {
        // Stop time cannot be before start time,
        // it'll get an error from backend.
        Poco::Timestamp ts =
            Poco::Timestamp::fromEpochTime(Stop()) + 1*Poco::Timespan::DAYS;
        SetStop(ts.epochTime());
    }

    poco_assert(Stop() >= Start());

    if (!IsTracking()) {
        SetDurationInSeconds(Stop() - Start());
    }
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

void TimeEntry::SetStartUserInput(const std::string value) {
    Poco::Int64 start = Formatter::Parse8601(value);
    if (IsTracking()) {
        SetDurationInSeconds(-start);
    } else {
        SetStop(start + DurationInSeconds());
    }
    SetStart(start);
}

void TimeEntry::SetStartString(const std::string value) {
    SetStart(Formatter::Parse8601(value));
}

void TimeEntry::SetDurationUserInput(const std::string value) {
    int seconds = Formatter::ParseDurationString(value);
    if (IsTracking()) {
        time_t now = time(0);
        time_t start = now - seconds;
        SetStart(start);
        SetDurationInSeconds(-start);
    } else {
        SetDurationInSeconds(seconds);
        SetStop(Start() + seconds);
    }
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
    return a->Start() < b->Start();
}

void TimeEntry::LoadFromJSONNode(JSONNODE * const data) {
    poco_check_ptr(data);

    Poco::UInt64 ui_modified_at = UIModifiedAtFromJSONNode(data);
    if (UIModifiedAt() > ui_modified_at) {
        std::stringstream ss;
        ss  << "Will not overwrite time entry "
            << String()
            << " with server data because we have a newer ui_modified_at";
        logger().debug(ss.str());
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

JSONNODE *TimeEntry::SaveToJSONNode() const {
    JSONNODE *n = json_new(JSON_NODE);
    json_set_name(n, ModelName().c_str());
    if (ID()) {
        json_push_back(n, json_new_i("id", (json_int_t)ID()));
    }
    json_push_back(n,
                   json_new_a(
                       "description",
                       Formatter::EscapeJSONString(Description()).c_str()));
    // Workspace ID can't be 0 on server side. So don't
    // send 0 if we have no default workspace ID, because
    // NULL is not 0
    if (WID()) {
        json_push_back(n, json_new_i("wid", (json_int_t)WID()));
    }
    json_push_back(n, json_new_a("guid", GUID().c_str()));
    if (!PID() && !ProjectGUID().empty()) {
        json_push_back(n, json_new_a("pid", ProjectGUID().c_str()));
    } else {
        json_push_back(n, json_new_i("pid", (json_int_t)PID()));
    }
    json_push_back(n, json_new_i("tid", (json_int_t)TID()));
    json_push_back(n, json_new_a("start", StartString().c_str()));
    if (Stop()) {
        json_push_back(n, json_new_a("stop", StopString().c_str()));
    }
    json_push_back(n, json_new_i("duration",
                                 (json_int_t)DurationInSeconds()));
    json_push_back(n, json_new_b("billable", Billable()));
    json_push_back(n, json_new_b("duronly", DurOnly()));
    json_push_back(n, json_new_i("ui_modified_at",
                                 (json_int_t)UIModifiedAt()));
    json_push_back(n,
                   json_new_a(
                       "created_with",
                       Formatter::EscapeJSONString(CreatedWith()).c_str()));

    JSONNODE *tag_nodes = json_new(JSON_ARRAY);
    json_set_name(tag_nodes, "tags");
    for (std::vector<std::string>::const_iterator it = TagNames.begin();
            it != TagNames.end();
            it++) {
        std::string tag_name = *it;
        json_push_back(
            tag_nodes,
            json_new_a(NULL,
                       Formatter::EscapeJSONString(tag_name).c_str()));
    }
    json_push_back(n, tag_nodes);

    return n;
}

void TimeEntry::loadTagsFromJSONNode(JSONNODE * const list) {
    poco_check_ptr(list);

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
