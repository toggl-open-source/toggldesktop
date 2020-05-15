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
    if (durationTooLarge(err) && StopTime() && Start()) {
        Poco::Int64 seconds =
            (std::min)(StopTime() - Start(),
                       Poco::Int64(kMaxTimeEntryDurationSeconds));
        DurationInSeconds.Set(seconds);
        return true;
    }
    if (startTimeWrongYear(err) && StopTime() && Start()) {
        Poco::Int64 seconds =
            (std::min)(StopTime() - Start(),
                       Poco::Int64(kMaxTimeEntryDurationSeconds));
        DurationInSeconds.Set(seconds);
        StartTime.Set(StopTime() - Duration());
        return true;
    }
    if (stopTimeMustBeAfterStartTime(err) && StopTime() && Start()) {
        StopTime.Set(Start() + DurationInSeconds());
        return true;
    }
    if (userCannotAccessWorkspace(err)) {
        WID.Set(0);
        PID.Set(0);
        TID.Set(0);
        return true;
    }
    if (userCannotAccessTheSelectedProject(err)) {
        PID.Set(0);
        TID.Set(0);
        return true;
    }
    if (userCannotAccessSelectedTask(err)) {
        TID.Set(0);
        return true;
    }
    if (billableIsAPremiumFeature(err)) {
        Billable.Set(false);
        return true;
    }
    if (isMissingCreatedWith(err)) {
        CreatedWith.Set(HTTPClient::Config.UserAgent());
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

    DurationInSeconds.Set(duration);
    StopTime.Set(at);
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
        << " description=" << Description()
        << " wid=" << WID()
        << " guid=" << GUID()
        << " pid=" << PID()
        << " tid=" << TID()
        << " start=" << StartTime()
        << " stop=" << StopTime()
        << " duration=" << DurationInSeconds()
        << " billable=" << Billable()
        << " unsynced=" << Unsynced()
        << " duronly=" << DurOnly()
        << " tags=" << Tags()
        << " created_with=" << CreatedWith()
        << " ui_modified_at=" << UIModifiedAt()
        << " deleted_at=" << DeletedAt()
        << " updated_at=" << UpdatedAt();
    return ss.str();
}


void TimeEntry::SetStopString(const std::string &value) {
    StopTime.Set(Formatter::Parse8601(value));
}

void TimeEntry::SetStopUserInput(const std::string &value) {
    SetStopString(value);

    if (StopTime() < Start()) {
        // Stop time cannot be before start time,
        // it'll get an error from backend.
        Poco::Timestamp ts =
            Poco::Timestamp::fromEpochTime(StopTime()) + 1*Poco::Timespan::DAYS;
        StopTime.Set(ts.epochTime());
    }

    if (StopTime() < Start()) {
        logger().error("Stop time must be after start time!");
        return;
    }

    if (!IsTracking()) {
        DurationInSeconds.Set(StopTime() - Start());
    }

    if (Dirty()) {
        ValidationError.Set(noError);
        SetUIModified();
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


void TimeEntry::SetStartUserInput(const std::string &value,
                                  const bool keepEndTimeFixed) {
    Poco::Int64 start = Formatter::Parse8601(value);
    if (IsTracking()) {
        DurationInSeconds.Set(-start);
    } else {
        auto stop = StopTime();
        if (keepEndTimeFixed && stop > start) {
            DurationInSeconds.Set(stop - start);
        } else {
            StopTime.Set(start + DurationInSeconds());
        }
    }
    StartTime.Set(start);

    if (Dirty()) {
        ValidationError.Set(noError);
        SetUIModified();
    }
}

void TimeEntry::SetStartString(const std::string &value) {
    StartTime.Set(Formatter::Parse8601(value));
}

void TimeEntry::SetDurationUserInput(const std::string &value) {
    int seconds = Formatter::ParseDurationString(value);
    if (IsTracking()) {
        time_t now = time(nullptr);
        time_t start = now - seconds;
        StartTime.Set(start);
        DurationInSeconds.Set(-start);
    } else {
        DurationInSeconds.Set(seconds);
    }
    StopTime.Set(Start() + seconds);

    if (Dirty()) {
        ValidationError.Set(noError);
        SetUIModified();
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
    return Formatter::Format8601(StopTime());
}

std::string TimeEntry::StartString() const {
    return Formatter::Format8601(StartTime());
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
        CreatedWith.Set(data["created_with"].asString());
    }

    Description.Set(data["description"].asString());

    if (data.isMember("wid")) {
        WID.Set(data["wid"].asUInt64());
    } else {
        WID.Set(0);
    }
    if (data.isMember("pid")) {
        PID.Set(data["pid"].asUInt64());
    } else {
        PID.Set(0);
    }
    if (data.isMember("tid")) {
        TID.Set(data["tid"].asUInt64());
    } else {
        TID.Set(0);
    }
    SetStartString(data["start"].asString());
    SetStopString(data["stop"].asString());
    DurationInSeconds.Set(data["duration"].asInt64());
    Billable.Set(data["billable"].asBool());
    DurOnly.Set(data["duronly"].asBool());
    SetUpdatedAtString(data["at"].asString());

    UIModifiedAt.Set(0);
    Unsynced.Set(false);
}

Json::Value TimeEntry::SaveToJSON(int apiVersion) const {
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
        n["workspace_id"] = Json::UInt64(WID());
    }
    if (apiVersion == 8)
        n["guid"] = GUID();
    if (!PID() && !ProjectGUID().empty()) {
        if (apiVersion == 8)
            n["pid"] = ProjectGUID();
        else
            n["project_id"] = ProjectGUID();
    } else {
        if (apiVersion == 8)
            n["pid"] = Json::UInt64(PID());
    }

    if (PID()) {
        if (apiVersion == 8)
            n["pid"] = Json::UInt64(PID());
        else
            n["project_id"] = Json::UInt64(PID());
    } else {
        if (apiVersion == 8)
            n["pid"] = Json::nullValue;
    }

    if (TID()) {
        if (apiVersion == 8)
            n["tid"] = Json::UInt64(TID());
        else
            n["task_id"] = Json::UInt64(TID());
    } else {
        if (apiVersion == 8)
            n["tid"] = Json::nullValue;
    }

    n["start"] = StartString();
    if (StopTime() && apiVersion == 8) {
        n["stop"] = StopString();
    }
    n["duration"] = Json::Int64(DurationInSeconds());
    if (apiVersion == 8)
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

}   // namespace toggl
