// Copyright 2014 Toggl Desktop developers.

// NB! Setters should not directly calculate
// anything besides setting the field asked.
// This is because same setters are used when
// loading from database, JSON etc. If time entry
// needs to be recalculated after some user
// action, the recalculation should be started
// from context, using specific functions, not
// setters.

#include "model/time_entry.h"

#include <sstream>
#include <algorithm>

#include <json/json.h>  // NOLINT

#include "https_client.h"
#include "util/formatter.h"

#include <Poco/DateTime.h>
#include <Poco/LocalDateTime.h>
#include <Poco/Logger.h>
#include <Poco/NumberParser.h>
#include <Poco/Timestamp.h>
#include "toggl_api_private.h"

namespace toggl {

bool TimeEntry::ResolveError(const error &err) {
    if (durationTooLarge(err) && Stop() && Start()) {
        Poco::Int64 seconds =
            (std::min)(Stop() - Start(),
                       Poco::Int64(kMaxTimeEntryDurationSeconds));
        SetDurationInSeconds(seconds);
        return true;
    }
    if (startTimeWrongYear(err) && Stop() && Start()) {
        Poco::Int64 seconds =
            (std::min)(Stop() - Start(),
                       Poco::Int64(kMaxTimeEntryDurationSeconds));
        SetDurationInSeconds(seconds);
        SetStart(Stop() - Duration());
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
    if (isMissingCreatedWith(err)) {
        SetCreatedWith(HTTPClient::Config.UserAgent());
        return true;
    }
    return false;
}

bool TimeEntry::isNotFound(const error &err) const {
    return std::string::npos != std::string(err).find(
        "Time entry not found");
}
bool TimeEntry::isMissingCreatedWith(const error &err) const {
    return std::string::npos != std::string(err).find(
        "created_with needs to be provided an a valid string");
}

bool TimeEntry::userCannotAccessTheSelectedProject(
    const error &err) const {
    return (std::string::npos != std::string(err).find(
        "User cannot access the selected project"));
}

bool TimeEntry::userCannotAccessSelectedTask(
    const error &err) const {
    return (std::string::npos != std::string(err).find(
        "User cannot access selected task"));
}

bool TimeEntry::durationTooLarge(const error &err) const {
    return (std::string::npos != std::string(err).find(
        "Max allowed duration per 1 time entry is 999 hours"));
}

bool TimeEntry::startTimeWrongYear(const error &err) const {
    return (std::string::npos != std::string(err).find(
        "Start time year must be between 2006 and 2030"));
}

bool TimeEntry::stopTimeMustBeAfterStartTime(const error &err) const {
    return (std::string::npos != std::string(err).find(
        "Stop time must be after start time"));
}

bool TimeEntry::billableIsAPremiumFeature(const error &err) const {
    return (std::string::npos != std::string(err).find(
        "Billable is a premium feature"));
}

void TimeEntry::DiscardAt(const Poco::Int64 at) {
    if (!IsTracking()) {
        logger().error("Cannot discard time entry that is not tracking");
        return;
    }

    if (!at) {
        logger().error("Cannot discard time entry without a timestamp");
        return;
    }

    if (at < Start()) {
        logger().error("Cannot discard time entry with start time bigger than current moment");
        return;
    }

    Poco::Int64 duration = at - Start();

    if (duration < 0) {
        logger().error("Discarding with this time entry would result in negative duration");  // NOLINT
        return;
    }

    SetDurationInSeconds(duration);
    SetStop(at);
    SetUIModified();
}

void TimeEntry::StopTracking() {
    DiscardAt(time(nullptr));
}

std::string TimeEntry::String() const {
    std::stringstream ss;
    ss  << "TimeEntry"
        << " ID=" << ID()
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
        << " unsynced=" << unsynced_
        << " duronly=" << duronly_
        << " tags=" << Tags()
        << " created_with=" << CreatedWith()
        << " ui_modified_at=" << UIModifiedAt()
        << " deleted_at=" << DeletedAt()
        << " updated_at=" << UpdatedAt();
    return ss.str();
}

void TimeEntry::SetLastStartAt(const Poco::Int64 value) {
    if (last_start_at_ != value) {
        last_start_at_ = value;
    }
}

void TimeEntry::SetDurOnly(const bool value) {
    if (duronly_ != value) {
        duronly_ = value;
        SetDirty();
    }
}

void TimeEntry::SetStart(const Poco::Int64 value) {
    if (start_ != value) {
        start_ = value;
        SetDirty();
    }
}

void TimeEntry::SetStop(const Poco::Int64 value) {
    if (stop_ != value) {
        stop_ = value;
        SetDirty();
    }
}

void TimeEntry::SetDescription(const std::string &value) {
    const std::string &trimValue = trim_whitespace(value);
    if (description_ != trimValue) {
        description_ = trimValue;
        SetDirty();
    }
}

void TimeEntry::SetStopString(const std::string &value) {
    SetStop(Formatter::Parse8601(value));
}

void TimeEntry::SetCreatedWith(const std::string &value) {
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

void TimeEntry::SetStopUserInput(const std::string &value) {
    SetStopString(value);

    if (Stop() < Start()) {
        // Stop time cannot be before start time,
        // it'll get an error from backend.
        Poco::Timestamp ts =
            Poco::Timestamp::fromEpochTime(Stop()) + 1*Poco::Timespan::DAYS;
        SetStop(ts.epochTime());
    }

    if (Stop() < Start()) {
        logger().error("Stop time must be after start time!");
        return;
    }

    if (!IsTracking()) {
        SetDurationInSeconds(Stop() - Start());
    }

    if (Dirty()) {
        ClearValidationError();
        SetUIModified();
    }
}

void TimeEntry::SetTID(const Poco::UInt64 value) {
    if (tid_ != value) {
        tid_ = value;
        SetDirty();
    }
}

static const char kTagSeparator = '\t';

void TimeEntry::SetTags(const std::string &tags) {
    if (Tags() != tags) {
        TagNames.clear();
        if (!tags.empty()) {
            std::stringstream ss(tags);
            while (ss.good()) {
                std::string tag;
                getline(ss, tag, kTagSeparator);
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

void TimeEntry::SetStartUserInput(const std::string &value,
                                  const bool keepEndTimeFixed) {
    Poco::Int64 start = Formatter::Parse8601(value);
    if (IsTracking()) {
        SetDurationInSeconds(-start);
    } else {
        auto stop = Stop();
        if (keepEndTimeFixed && stop > start) {
            SetDurationInSeconds(stop - start);
        } else {
            SetStop(start + DurationInSeconds());
        }
    }
    SetStart(start);

    if (Dirty()) {
        ClearValidationError();
        SetUIModified();
    }
}

void TimeEntry::SetStartString(const std::string &value) {
    SetStart(Formatter::Parse8601(value));
}

void TimeEntry::SetDurationUserInput(const std::string &value) {
    int seconds = Formatter::ParseDurationString(value);
    if (IsTracking()) {
        time_t now = time(nullptr);
        time_t start = now - seconds;
        SetStart(start);
        SetDurationInSeconds(-start);
    } else {
        SetDurationInSeconds(seconds);
    }
    SetStop(Start() + seconds);

    if (Dirty()) {
        ClearValidationError();
        SetUIModified();
    }
}

void TimeEntry::SetProjectGUID(const std::string &value) {
    if (project_guid_ != value) {
        project_guid_ = value;
        SetDirty();
    }
}

const std::string TimeEntry::Tags() const {
    std::stringstream ss;
    for (auto it = TagNames.begin(); it != TagNames.end(); ++it) {
        if (it != TagNames.begin()) {
            ss << kTagSeparator;
        }
        ss << *it;
    }
    return ss.str();
}

const std::string TimeEntry::TagsHash() const {
    std::vector<std::string> sortedTagNames(TagNames);
    sort(sortedTagNames.begin(), sortedTagNames.end());
    std::stringstream ss;
    for (auto it = sortedTagNames.begin(); it != sortedTagNames.end(); ++it) {
        if (it != sortedTagNames.begin()) {
            ss << kTagSeparator;
        }
        ss << *it;
    }
    return ss.str();
}

std::string TimeEntry::StopString() const {
    return Formatter::Format8601(stop_);
}

std::string TimeEntry::StartString() const {
    return Formatter::Format8601(start_);
}

const std::string TimeEntry::GroupHash() const {
    std::stringstream ss;
    ss << toggl::Formatter::FormatDateHeader(Start())
       << Description()
       << WID()
       << PID()
       << TID()
       << ProjectGUID()
       << Billable()
       << TagsHash();
    return ss.str();
}

bool TimeEntry::IsToday() const {
    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(Start());
    Poco::LocalDateTime datetime(ts);
    Poco::LocalDateTime today;
    return today.year() == datetime.year() &&
           today.month() == datetime.month() &&
           today.day() == datetime.day();
}

void TimeEntry::LoadFromJSON(Json::Value data) {
    // No ui_modified_at in server responses.
    // Compare updated_at with ui_modified_at to see if ui has been changed
    Json::Value at = data["at"];
    Poco::Int64 updated_at(0);
    if (at.isString()) {
        updated_at = Formatter::Parse8601(at.asString());
    } else {
        updated_at = at.asInt64();
    }

    if (updated_at != 0 &&
            (UIModifiedAt() >= updated_at ||
             UpdatedAt() >= updated_at)) {
        std::stringstream ss;
        ss  << "Will not overwrite time entry "
            << "[" << String() << "]"
            << " with server data because we have a newer or same updated_at"
            << " [Server updated_at: " << updated_at << "]";
        logger().debug(ss.str());
        return;
    }

    if (data.isMember("tags")) {
        loadTagsFromJSON(data["tags"]);
    }

    if (data.isMember("created_with")) {
        SetCreatedWith(data["created_with"].asString());
    }

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
    ClearUnsynced();
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

    if (PID()) {
        n["pid"] = Json::UInt64(PID());
    } else {
        n["pid"] = Json::nullValue;
    }

    if (TID()) {
        n["tid"] = Json::UInt64(TID());
    } else {
        n["tid"] = Json::nullValue;
    }

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
    if (TagNames.size() > 0) {
        for (std::vector<std::string>::const_iterator it = TagNames.begin();
                it != TagNames.end();
                ++it) {
            std::string tag_name = Formatter::EscapeJSONString(*it);
            tag_nodes.append(Json::Value(tag_name));
        }
    } else {
        Json::Reader reader;
        reader.parse("[]", tag_nodes);
    }
    n["tags"] = tag_nodes;

    return n;
}

Poco::Int64 TimeEntry::RealDurationInSeconds() const {
    auto now = time(nullptr);

    return now + DurationInSeconds();
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

std::string TimeEntry::ModelName() const {
    return kModelTimeEntry;
}

std::string TimeEntry::ModelURL() const {
    std::stringstream relative_url;
    relative_url << "/api/v9/workspaces/"
                 << WID() << "/time_entries";

    if (ID()) {
        relative_url << "/" << ID();
    }

    return relative_url.str();
}

void TimeEntry::SetSkipPomodoro(const bool value) {
    skipPomodoro = value;
}

}   // namespace toggl
