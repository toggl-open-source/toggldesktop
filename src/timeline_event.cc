// Copyright 2015 Toggl Desktop developers.

#include "../src/timeline_event.h"

#include <sstream>
#include <cstring>

namespace toggl {

std::string TimelineEvent::String() const {
    std::stringstream ss;
    ss << start_time_
       << ";"
       << end_time_
       << ";"
       << filename_
       << ";"
       << title_
       << ";"
       << Duration();
    return ss.str();
}

std::string TimelineEvent::ModelName() const {
    return kModelTimelineEvent;
}

std::string TimelineEvent::ModelURL() const {
    return "";
}

void TimelineEvent::SetTitle(const std::string value) {
    if (title_ != value) {
        title_ = value;
        SetDirty();
    }
}

void TimelineEvent::SetStart(const Poco::UInt64 value) {
    if (start_time_ != value) {
        start_time_ = value;
        SetDirty();
    }
}

void TimelineEvent::SetEndTime(const Poco::UInt64 value) {
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

void TimelineEvent::SetFilename(const std::string value) {
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

}   // namespace toggl
