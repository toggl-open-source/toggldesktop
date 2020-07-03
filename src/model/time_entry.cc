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
    if (durationTooLarge(err) && StopTime() && StartTime()) {
        Poco::Int64 seconds =
            (std::min)(StopTime() - StartTime(),
                       Poco::Int64(kMaxTimeEntryDurationSeconds));
        SetDurationInSeconds(seconds, true);
        return true;
    }
    if (startTimeWrongYear(err) && StopTime() && StartTime()) {
        Poco::Int64 seconds =
            (std::min)(StopTime() - StartTime(),
                       Poco::Int64(kMaxTimeEntryDurationSeconds));
        SetDurationInSeconds(seconds, true);
        SetStartTime(StopTime() - Duration(), true);
        return true;
    }
    if (stopTimeMustBeAfterStartTime(err) && StopTime() && StartTime()) {
        SetStopTime(StartTime() + DurationInSeconds(), true);
        return true;
    }
    if (userCannotAccessWorkspace(err)) {
        SetWID(0);
        SetPID(0, true);
        SetTID(0, true);
        return true;
    }
    if (userCannotAccessTheSelectedProject(err)) {
        SetPID(0, true);
        SetTID(0, true);
        return true;
    }
    if (userCannotAccessSelectedTask(err)) {
        SetTID(0, true);
        return true;
    }
    if (billableIsAPremiumFeature(err)) {
        SetBillable(false, true);
        return true;
    }
    if (isMissingCreatedWith(err)) {
        SetCreatedWith(HTTPClient::Config.UserAgent());
        return true;
    }
    return false;
}

bool TimeEntry::isNotFound(const error &err) {
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

    if (at < StartTime()) {
        logger().error("Cannot discard time entry with start time bigger than current moment");
        return;
    }

    Poco::Int64 duration = at - StartTime();

    if (duration < 0) {
        logger().error("Discarding with this time entry would result in negative duration");  // NOLINT
        return;
    }

    SetDurationInSeconds(duration, true);
    SetStopTime(at, true);
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

void TimeEntry::SetLastStartAt(Poco::Int64 value) {
    if (LastStartAt() != value) {
        LastStartAt.Set(value);
    }
}

void TimeEntry::SetDurOnly(bool value) {
    if (DurOnly() != value) {
        DurOnly.Set(value);
        SetDirty();
    }
}

void TimeEntry::SetStartTime(Poco::Int64 value, bool userModified) {
    if (StartTime() != value) {
        StartTime.Set(value, userModified);
        SetDirty();
    }
}

void TimeEntry::SetStopTime(Poco::Int64 value, bool userModified) {
    if (StopTime() != value) {
        StopTime.Set(value, userModified);
        SetDirty();
    }
}

void TimeEntry::SetDescription(const std::string &value, bool userModified) {
    const std::string &trimValue = trim_whitespace(value);
    if (Description() != trimValue) {
        Description.Set(trimValue, userModified);
        SetDirty();
    }
}

void TimeEntry::SetStopString(const std::string &value, bool userModified) {
    SetStopTime(Formatter::Parse8601(value), userModified);
}

void TimeEntry::SetCreatedWith(const std::string &value) {
    if (CreatedWith() != value) {
        CreatedWith.Set(value, false);
        SetDirty();
    }
}

void TimeEntry::SetBillable(bool value, bool userModified) {
    if (Billable() != value) {
        Billable.Set(value, userModified);
        SetDirty();
    }
}

void TimeEntry::SetWID(Poco::UInt64 value) {
    if (WID() != value) {
        WID.Set(value);
        SetDirty();
    }
}

void TimeEntry::SetStopUserInput(const std::string &value) {
    SetStopString(value, true);

    if (StopTime() < StartTime()) {
        // Stop time cannot be before start time,
        // it'll get an error from backend.
        Poco::Timestamp ts =
            Poco::Timestamp::fromEpochTime(StopTime()) + 1*Poco::Timespan::DAYS;
        SetStopTime(ts.epochTime(), true);
    }

    if (StopTime() < StartTime()) {
        logger().error("Stop time must be after start time!");
        return;
    }

    if (!IsTracking()) {
        SetDurationInSeconds(StopTime() - StartTime(), true);
    }

    if (Dirty()) {
        ClearValidationError();
        SetUIModified();
    }
}

void TimeEntry::SetTID(Poco::UInt64 value, bool userModified) {
    if (TID() != value) {
        TID.Set(value, userModified);
        SetDirty();
    }
}

static const char kTagSeparator = '\t';

void TimeEntry::SetTags(const std::string &tags, bool userModified) {
    if (Tags() != tags) {
        decltype(TagNames)::value_type tmp;
        if (!tags.empty()) {
            std::stringstream ss(tags);
            while (ss.good()) {
                std::string tag;
                getline(ss, tag, kTagSeparator);
                tmp.push_back(tag);
            }
        }
        TagNames.Set(std::move(tmp), userModified);
        SetDirty();
    }
}

void TimeEntry::SetPID(Poco::UInt64 value, bool userModified) {
    if (PID() != value) {
        PID.Set(value, userModified);
        SetDirty();
    }
}

void TimeEntry::SetDurationInSeconds(Poco::Int64 value, bool userModified) {
    if (DurationInSeconds() != value) {
        DurationInSeconds.Set(value, userModified);
        SetDirty();
    }
}

void TimeEntry::SetStartUserInput(const std::string &value,
                                  bool keepEndTimeFixed) {
    Poco::Int64 start = Formatter::Parse8601(value);
    if (IsTracking()) {
        SetDurationInSeconds(-start, true);
    } else {
        auto stop = StopTime();
        if (keepEndTimeFixed && stop > start) {
            SetDurationInSeconds(stop - start, true);
        } else {
            SetStopTime(start + DurationInSeconds(), true);
        }
    }
    SetStartTime(start, true);

    if (Dirty()) {
        ClearValidationError();
        SetUIModified();
    }
}

void TimeEntry::SetStartString(const std::string &value, bool userModified) {
    SetStartTime(Formatter::Parse8601(value), userModified);
}

void TimeEntry::SetDurationUserInput(const std::string &value) {
    int seconds = Formatter::ParseDurationString(value);
    if (IsTracking()) {
        time_t now = time(nullptr);
        time_t start = now - seconds;
        SetStartTime(start, true);
        SetDurationInSeconds(-start, true);
    } else {
        SetDurationInSeconds(seconds, true);
    }
    SetStopTime(StartTime() + seconds, true);

    if (Dirty()) {
        ClearValidationError();
        SetUIModified();
    }
}

void TimeEntry::SetProjectGUID(const std::string &value, bool userModified) {
    if (ProjectGUID() != value) {
        ProjectGUID.Set(value, userModified);
        SetDirty();
    }
}

const std::string TimeEntry::Tags() const {
    std::stringstream ss;
    for (auto it = TagNames->begin(); it != TagNames->end(); ++it) {
        if (it != TagNames->begin()) {
            ss << kTagSeparator;
        }
        ss << *it;
    }
    return ss.str();
}

const std::string TimeEntry::TagsHash() const {
    std::vector<std::string> sortedTagNames(TagNames());
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
    ss << toggl::Formatter::FormatDateHeader(StartTime())
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
    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(StartTime());
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

    SetDescription(data["description"].asString(), false);

    if (data.isMember("wid")) {
        SetWID(data["wid"].asUInt64());
    } else if (data.isMember("workspace_id")) {
        SetWID(data["workspace_id"].asUInt64());
    } else {
        SetWID(0);
    }
    if (data.isMember("pid")) {
        SetPID(data["pid"].asUInt64(), false);
    } else if (data.isMember("project_id")) {
        SetPID(data["project_id"].asUInt64(), false);
    } else {
        SetPID(0, false);
    }
    if (data.isMember("tid")) {
        SetTID(data["tid"].asUInt64(), false);
    } else if (data.isMember("task_id")) {
        SetTID(data["task_id"].asUInt64(), false);
    } else {
        SetTID(0, false);
    }
    SetStartString(data["start"].asString(), false);
    SetStopString(data["stop"].asString(), false);
    SetDurationInSeconds(data["duration"].asInt64(), false);
    SetBillable(data["billable"].asBool(), false);
    SetDurOnly(data["duronly"].asBool());
    SetUpdatedAtString(data["at"].asString());

    SetUIModifiedAt(0);
    ClearUnsynced();
}

Json::Value TimeEntry::SaveToJSON(int apiVersion) const {
    Json::Value n;
    if (ID()) {
        n["id"] = Json::UInt64(ID());
    }
    n["description"] = Formatter::EscapeJSONString(Description());
    if (apiVersion == 8) {
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
    }
    else {
        // Workspace ID can't be 0 on server side. So don't
        // send 0 if we have no default workspace ID, because
        // NULL is not 0
        if (WID()) {
            n["workspace_id"] = Json::UInt64(WID());
        }
        n["guid"] = GUID();
        if (!PID() && !ProjectGUID().empty()) {
            n["project_id"] = ProjectGUID();
        } else {
            n["project_id"] = Json::UInt64(PID());
        }

        if (PID()) {
            n["project_id"] = Json::UInt64(PID());
        } else {
            n["project_id"] = Json::nullValue;
        }

        if (TID()) {
            n["task_id"] = Json::UInt64(TID());
        } else {
            n["task_id"] = Json::nullValue;
        }
    }

    n["start"] = StartString();
    if (StopTime()) {
        n["stop"] = StopString();
    }
    n["duration"] = Json::Int64(DurationInSeconds());
    // TODO billable is a premium feature. It will now be handled in Context but it really should be omitted here, not anywhere else
    n["billable"] = Billable();
    n["duronly"] = DurOnly();
    n["ui_modified_at"] = Json::UInt64(UIModifiedAt());
    n["created_with"] = Formatter::EscapeJSONString(CreatedWith());

    Json::Value tag_nodes;
    if (TagNames->size() > 0) {
        for (std::vector<std::string>::const_iterator it = TagNames->begin();
                it != TagNames->end();
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

Json::Value TimeEntry::SyncMetadata() const {
    Json::Value result;
    if (NeedsPOST()) {
        result["client_assigned_id"] = std::to_string(-LocalID());
    }
    else if (NeedsPUT() || NeedsDELETE()) {
        if (ID() > 0)
            result["id"] = Json::Int64(ID());
        else // and this really shouldn't happen
            result["id"] = std::to_string(-LocalID());
        result["workspace_id"] = Json::Int64(WID());
    }
    return result;
}

Json::Value TimeEntry::SyncPayload() const {
    Json::Value result;
    if (NeedsPOST()) {
        result["id"] = Json::Int64(-LocalID());
        result["workspace_id"] = Json::Int64(WID());
    }
    if (NeedsPOST() || NeedsPUT()) {
        auto insertIfValue = [&, this](auto &name, auto &property, auto value) {
            if (NeedsPOST() || property.IsDirty()) {
                result[name] = value;
            }
        };
        auto insertIf = [&](auto &name, auto &property) {
            insertIfValue(name, property, property());
        };

        if (PID() > 0) {
            insertIfValue("project_id", PID, Json::Int64(PID()));
        }
        else {
            insertIf("project_id", ProjectGUID);
        }
        if (TID() > 0)  {
            insertIfValue("task_id", TID, Json::Int64(TID()));
        }

        insertIf("billable", Billable);
        insertIfValue("start", StartTime, StartString());
        if (StopTime())
            insertIfValue("stop", StopTime, StopString());
        insertIfValue("duration", DurationInSeconds, Json::Int64(DurationInSeconds()));
        insertIf("description", Description);
        insertIfValue("created_with", CreatedWith, Formatter::EscapeJSONString(CreatedWith()));

        Json::Value tag_nodes;
        if (TagNames->size() > 0) {
            for (std::vector<std::string>::const_iterator it = TagNames->begin();
                    it != TagNames->end();
                    ++it) {
                std::string tag_name = Formatter::EscapeJSONString(*it);
                tag_nodes.append(Json::Value(tag_name));
            }
        } else {
            Json::Reader reader;
            reader.parse("[]", tag_nodes);
        }
        insertIfValue("tags", TagNames, tag_nodes);
    }
    return result;
}


Poco::Int64 TimeEntry::RealDurationInSeconds() const {
    auto now = time(nullptr);

    return now + DurationInSeconds();
}

void TimeEntry::loadTagsFromJSON(Json::Value list) {
    TagNames->clear();

    for (unsigned int i = 0; i < list.size(); i++) {
        std::string tag = list[i].asString();
        if (!tag.empty()) {
            TagNames->push_back(tag);
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

void TimeEntry::SetSkipPomodoro(bool value) {
    SkipPomodoro.Set(value);
}

}   // namespace toggl
