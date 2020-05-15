// Copyright 2015 Toggl Desktop developers.

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

Json::Value TimelineEvent::SaveToJSON(int apiVersion) const {
    Json::Value n;
    n["guid"] = GUID();
    n["filename"] = Filename();
    n["title"] = Title();
    n["start_time"] = Json::Int64(Start());
    n["end_time"] = Json::Int64(EndTime());
    n["created_with"] = "timeline";
    return n;
}

const Poco::Int64 &TimelineEvent::Start() const {
    return StartTime();
}

const Poco::Int64 &TimelineEvent::Duration() const {
    static thread_local Poco::Int64 value = EndTime() - StartTime();
    value = value < 0 ? 0 : value;
    return value;
}

}   // namespace toggl
