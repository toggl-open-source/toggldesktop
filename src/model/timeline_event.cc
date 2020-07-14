// Copyright 2015 Toggl Track developers.

#include "model/timeline_event.h"

#include <sstream>
#include <cstring>

namespace toggl {

std::string TimelineEvent::String() const {
    std::stringstream ss;
    ss << "TimelineEvent"
       << " guid=" << GUID()
       << " local_id=" << LocalID()
       << " start_time=" << Start()
       << " end_time=" << EndTime()
       << " filename=" << Filename()
       << " title=" << Title()
       << " duration=" << Duration();
    return ss.str();
}

std::string TimelineEvent::ModelName() const {
    return kModelTimelineEvent;
}

std::string TimelineEvent::ModelURL() const {
    return "";
}

void TimelineEvent::SetTitle(const std::string &value) {
    if (Title.Set(value))
        SetDirty();
}

void TimelineEvent::SetStartTime(Poco::Int64 value) {
    if (StartTime.Set(value)) {
        updateDuration();
        SetDirty();
    }
}

void TimelineEvent::SetEndTime(Poco::Int64 value) {
    if (EndTime.Set(value))  {
        updateDuration();
        SetDirty();
    }
}

void TimelineEvent::SetIdle(bool value) {
    if (Idle.Set(value))
        SetDirty();
}

void TimelineEvent::SetFilename(const std::string &value) {
    if (Filename.Set(value))
        SetDirty();
}

void TimelineEvent::SetChunked(bool value) {
    if (Chunked.Set(value))
        SetDirty();
}

void TimelineEvent::SetUploaded(bool value) {
    if (Uploaded.Set(value))
        SetDirty();
}

Json::Value TimelineEvent::SaveToJSON(int) const {
    Json::Value n;
    n["guid"] = GUID();
    n["filename"] = Filename();
    n["title"] = Title();
    n["start_time"] = Json::Int64(Start());
    n["end_time"] = Json::Int64(EndTime());
    n["created_with"] = "timeline";
    return n;
}

void TimelineEvent::updateDuration() {
    Poco::Int64 value = EndTime() - StartTime();
    DurationInSeconds.Set(value < 0 ? 0 : value);
}

}   // namespace toggl
