// Copyright 2015 Toggl Desktop developers.

#include "../src/timeline_event.h"

#include <sstream>
#include <cstring>

#include "./const.h"

namespace toggl {

std::string TimelineEvent::String() const {
    std::stringstream ss;
    ss << start_time
       << ";"
       << end_time
       << ";"
       << filename
       << ";"
       << title;
    return ss.str();
}

std::string TimelineEvent::ModelName() const {
    return kModelTimelineEvent;
}

std::string TimelineEvent::ModelURL() const {
    return "";
}

void TimelineEvent::SetTitle(const std::string value) {
    if (title != value) {
        title = value;
        SetDirty();
    }
}

}   // namespace toggl
