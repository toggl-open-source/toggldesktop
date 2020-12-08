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
bool TimeEntry::isLocked(const error& err) {
    return std::string::npos != std::string(err).find(
        "Entries can't be added or edited in this period");
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
        << " uid=" << UID()
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
    if (DurOnly.Set(value))
        SetDirty();
}

void TimeEntry::SetStartTime(Poco::Int64 value, bool userModified) {
    if (StartTime.Set(value, userModified))
        SetDirty();
}

void TimeEntry::SetStopTime(Poco::Int64 value, bool userModified) {
    if (StopTime.Set(value, userModified))
        SetDirty();
}

TimeEntry::TimeEntry(const TimeEntry &o)
    : BaseModel(o)
    , Description { o.Description }
    , CreatedWith { o.CreatedWith }
    , ProjectGUID { o.ProjectGUID }
    , TagNames { o.TagNames }
    , WID { o.WID }
    , PID { o.PID }
    , TID { o.TID }
    , StartTime { o.StartTime }
    , StopTime { o.StopTime }
    , DurationInSeconds { o.DurationInSeconds }
    , LastStartAt { o.LastStartAt }
    , Billable { o.Billable }
    , DurOnly { o.DurOnly }
    , SkipPomodoro { o.SkipPomodoro }
{

}

void TimeEntry::SetDescription(const std::string &value, bool userModified) {
    const std::string &trimValue = trim_whitespace(value);
    if (Description.Set(trimValue, userModified))
        SetDirty();
}

void TimeEntry::SetCreatedWith(const std::string &value) {
    if (CreatedWith.Set(value, false))
        SetDirty();
}

void TimeEntry::SetBillable(bool value, bool userModified) {
    if (Billable.Set(value, userModified))
        SetDirty();
}

void TimeEntry::SetWID(Poco::UInt64 value) {
    if (WID.Set(value))
        SetDirty();
}

void TimeEntry::SetStopUserInput(const std::string &value) {
    std::time_t stopTime = Formatter::Parse8601(value);

    if (stopTime < StartTime()) {
        // Stop time cannot be before start time,
        // it'll get an error from backend.
        Poco::Timestamp ts =
            Poco::Timestamp::fromEpochTime(stopTime) + 1*Poco::Timespan::DAYS;
        stopTime = ts.epochTime();
    }

    if (stopTime < StartTime()) {
        logger().error("Stop time must be after start time!");
        return;
    }

    SetStopTime(stopTime, true);

    if (!IsTracking()) {
        SetDurationInSeconds(StopTime() - StartTime(), true);
    }

    if (Dirty()) {
        ClearValidationError();
        SetUIModified();
    }
}

void TimeEntry::SetTID(Poco::UInt64 value, bool userModified) {
    if (TID.Set(value, userModified))
        SetDirty();
}

static const char kTagSeparator = '\t';

void TimeEntry::SetTags(const std::string &tags, bool userModified) {
    decltype(TagNames)::value_type tmp;
    if (!tags.empty()) {
        std::stringstream ss(tags);
        while (ss.good()) {
            std::string tag;
            getline(ss, tag, kTagSeparator);
            tmp.push_back(tag);
        }
    }
    if (TagNames.Set(std::move(tmp), userModified))
        SetDirty();
}

void TimeEntry::SetPID(Poco::UInt64 value, bool userModified) {
    if (PID.Set(value, userModified))
        SetDirty();
}

void TimeEntry::SetDurationInSeconds(Poco::Int64 value, bool userModified) {
    if (DurationInSeconds.Set(value, userModified))
        SetDirty();
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
    if (ProjectGUID.Set(value, userModified))
        SetDirty();
}

const std::string &TimeEntry::Tags() const {
    return TagsVectorToString(TagNames());
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

std::vector<std::string> TimeEntry::TagsStringToVector(const std::string &str) {
    std::vector<std::string> tmp;
    if (!str.empty()) {
        std::stringstream ss(str);
        while (ss.good()) {
            std::string tag;
            getline(ss, tag, kTagSeparator);
            if (!tag.empty())
                tmp.push_back(tag);
        }
    }
    return tmp;
}

const std::string &TimeEntry::TagsVectorToString(const std::vector<std::string> &vec) {
    static thread_local std::string cache;
    cache.clear();
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        if (it != vec.begin()) {
            cache += kTagSeparator;
        }
        cache += *it;
    }
    return cache;
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

void TimeEntry::LoadFromJSON(const Json::Value &data, bool syncServer) {
    // Helper function to convert a string value to a timestamp
    auto convertTimeString = [](const Json::Value &json) -> Poco::Int64 {
        return Formatter::Parse8601(json.asString());
    };
    // Function that checks the JSON for the field in question and updates the property according to both sync server and legacy specs
    auto updateMergeablePropertyConvert = [this, &data, &syncServer](const std::string &field, auto &property, auto &convert) -> bool {
        // no member -> no update
        if (!data.isMember(field))
            return false;
        // extract the value using the helper in util/json or any other supplied conversion function
        auto serverValue = convert(data[field]);
        // if we're using the sync server and the previous value is the same as what server has, don't change anything
        if (syncServer && serverValue == property.GetPrevious())
            return true;
        // now if the current version is different from the server value, we need to update it
        if (serverValue != property.Get()) {
            property.Set(std::move(serverValue), false);
            SetDirty();
        }
        return true;
    };
    // This is just a wrapper so we don't have to supply JsonHelper::convert<T> for each call. It determines the type of the property automatically.
    auto updateMergeableProperty = [&updateMergeablePropertyConvert](const std::string &field, auto &property) -> bool {
        // decltype(property) gets us Property<T>&
        // std::remove_reference removes the reference -> Property<T>
        // Then we need to actually access the Property<T> type and use our own value_type to get T
        using type = typename std::remove_reference<decltype(property)>::type::value_type;
        return updateMergeablePropertyConvert(field, property, JsonHelper::convert<type>);
    };

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

    // WID should be static
    if (data.isMember("wid")) {
        SetWID(data["wid"].asUInt64());
    } else if (data.isMember("workspace_id")) {
        SetWID(data["workspace_id"].asUInt64());
    } else {
        SetWID(0);
    }

    updateMergeableProperty("tags", TagNames);
    updateMergeableProperty("created_with", CreatedWith);
    updateMergeableProperty("description", Description);
    if (!updateMergeableProperty("project_id", PID))
        if (!updateMergeableProperty("pid", PID))
            SetPID(0, false);
    if (!updateMergeableProperty("task_id", TID))
        if (!updateMergeableProperty("tid", TID))
            SetTID(0, false);
    updateMergeableProperty("billable", Billable);
    updateMergeableProperty("duration", DurationInSeconds);
    updateMergeablePropertyConvert("start", StartTime, convertTimeString);
    updateMergeablePropertyConvert("stop", StopTime, convertTimeString);

    // These properties are not sync-server-mergeable
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
        else if (PID.IsDirty()) {
            result["project_id"] = Json::nullValue;
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
