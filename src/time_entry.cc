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

#include <json/json.h>  // NOLINT

#include "./formatter.h"
#include "./const.h"

#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include "Poco/LocalDateTime.h"
#include "Poco/NumberParser.h"

namespace toggl {

bool TimeEntry::ResolveError(const toggl::error err) {
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
    if (billableIsAPremiumFeature(err)) {
        SetBillable(false);
        return true;
    }
    return false;
}

bool TimeEntry::userCannotAccessTheSelectedProject(
    const toggl::error err) const {
    return (std::string::npos != std::string(err).find(
        "User cannot access the selected project"));
}

bool TimeEntry::userCannotAccessSelectedTask(
    const toggl::error err) const {
    return (std::string::npos != std::string(err).find(
        "User cannot access selected task"));
}

bool TimeEntry::durationTooLarge(const toggl::error err) const {
    return (std::string::npos != std::string(err).find(
        "Max allowed duration per 1 time entry is 1000 hours"));
}

bool TimeEntry::stopTimeMustBeAfterStartTime(const toggl::error err) const {
    return (std::string::npos != std::string(err).find(
        "Stop time must be after start time"));
}

bool TimeEntry::billableIsAPremiumFeature(const toggl::error err) const {
    return (std::string::npos != std::string(err).find(
        "Billable is a premium feature"));
}

void TimeEntry::DiscardAt(const Poco::UInt64 at) {
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
    DiscardAt(time(0));
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

    if (Dirty()) {
        SetUIModified();
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

    if (Dirty()) {
        SetUIModified();
    }
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
    }
    SetStop(Start() + seconds);

    if (Dirty()) {
        SetUIModified();
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

void TimeEntry::LoadFromJSON(Json::Value data) {
    Json::Value modified = data["ui_modified_at"];
    Poco::UInt64 ui_modified_at(0);
    if (modified.isString()) {
        ui_modified_at = Poco::NumberParser::parseUnsigned64(
            modified.asString());
    } else {
        ui_modified_at = modified.asUInt64();
    }
    if (UIModifiedAt() > ui_modified_at) {
        std::stringstream ss;
        ss  << "Will not overwrite time entry "
            << String()
            << " with server data because we have a newer ui_modified_at";
        logger().debug(ss.str());
        return;
    }

    if (data.isMember("guid")) {
        SetGUID(data["guid"].asString());
    }

    if (data.isMember("tags")) {
        loadTagsFromJSON(data["tags"]);
    }

    if (data.isMember("created_with")) {
        SetCreatedWith(data["created_with"].asString());
    }

    SetID(data["id"].asUInt64());
    SetDescription(data["description"].asString());
    if (data.isMember("wid")) {
        SetWID(data["wid"].asUInt64());
    } else {
        SetWID(0);
    }
    if (data.isMember("pid")) {
        SetPID(data["pid"].asUInt64());
    } else {
        SetPID(0);
    }
    if (data.isMember("tid")) {
        SetTID(data["tid"].asUInt64());
    } else {
        SetTID(0);
    }
    SetStartString(data["start"].asString());
    SetStopString(data["stop"].asString());
    SetDurationInSeconds(data["duration"].asInt64());
    SetBillable(data["billable"].asBool());
    SetDurOnly(data["duronly"].asBool());
    SetUpdatedAtString(data["at"].asString());

    SetUIModifiedAt(0);
}

Json::Value TimeEntry::SaveToJSON() const {
    Json::Value n;
    if (ID()) {
        n["id"] = Json::UInt64(ID());
    }
    n["description"] = Formatter::EscapeJSONString(Description());
    // Workspace ID can't be 0 on server side. So don't
    // send 0 if we have no default workspace ID, because
    // NULL is not 0
    if (WID()) {
        n["wid"] = Json::UInt64(WID());
    }
    n["guid"] = GUID();
    if (!PID() && !ProjectGUID().empty()) {
        n["pid"] = ProjectGUID();
    } else {
        n["pid"] = Json::UInt64(PID());
    }
    n["tid"] = Json::UInt64(TID());
    n["start"] = StartString();
    if (Stop()) {
        n["stop"] = StopString();
    }
    n["duration"] = Json::Int64(DurationInSeconds());
    n["billable"] = Billable();
    n["duronly"] = DurOnly();
    n["ui_modified_at"] = Json::UInt64(UIModifiedAt());
    n["created_with"] = Formatter::EscapeJSONString(CreatedWith());

    Json::Value tag_nodes;
    for (std::vector<std::string>::const_iterator it = TagNames.begin();
            it != TagNames.end();
            it++) {
        std::string tag_name = Formatter::EscapeJSONString(*it);
        tag_nodes.append(Json::Value(tag_name));
    }
    n["tags"] = tag_nodes;

    return n;
}

Poco::UInt64 TimeEntry::AbsDuration(const Poco::Int64 value) {
    Poco::Int64 duration = value;

    // Duration is negative when time is tracking
    if (duration < 0) {
        duration += time(0);
    }
    // If after calculation time is still negative,
    // either computer clock is wrong or user
    // has set start time to the future. Render positive
    // duration only:
    if (duration < 0) {
        duration *= -1;
    }

    return static_cast<Poco::UInt64>(duration);
}

void TimeEntry::loadTagsFromJSON(Json::Value list) {
    TagNames.clear();

    for (unsigned int i = 0; i < list.size(); i++) {
        std::string tag = list[i].asString();
        if (!tag.empty()) {
            TagNames.push_back(tag);
        }
    }
}

void TimeEntry::Assign(TimeEntry *existing) {
    SetDescription(existing->Description());
    SetDurOnly(existing->DurOnly());
    SetWID(existing->WID());
    SetPID(existing->PID());
    SetTID(existing->TID());
    SetBillable(existing->Billable());
    SetTags(existing->Tags());
}

}   // namespace toggl
