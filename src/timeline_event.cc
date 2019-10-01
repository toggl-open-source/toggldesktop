// Copyright 2015 Toggl Desktop developers.

#include "../src/timeline_event.h"

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
    if (title_ != value) {
        title_ = value;
        SetDirty();
    }
}

void TimelineEvent::SetStart(const Poco::Int64 value) {
    if (start_time_ != value) {
        start_time_ = value;
        SetDirty();
    }
}

void TimelineEvent::SetEndTime(const Poco::Int64 value) {
    if (end_time_ != value) {
        end_time_ = value;
        SetDirty();
    }
}

void TimelineEvent::SetIdle(const bool value) {
    if (idle_ != value) {
        idle_ = value;
        SetDirty();
    }
}

void TimelineEvent::SetFilename(const std::string &value) {
    if (filename_ != value) {
        filename_ = value;
        SetDirty();
    }
}

void TimelineEvent::SetChunked(const bool value) {
    if (chunked_ != value) {
        chunked_ = value;
        SetDirty();
    }
}

void TimelineEvent::SetUploaded(const bool value) {
    if (uploaded_ != value) {
        uploaded_ = value;
        SetDirty();
    }
}

Json::Value TimelineEvent::SaveToJSON() const {
    Json::Value n;
    n["guid"] = GUID();
    n["filename"] = Filename();
    n["title"] = Title();
    n["start_time"] = Json::Int64(Start());
    n["end_time"] = Json::Int64(EndTime());
    n["created_with"] = "timeline";
    return n;
}

}   // namespace toggl
