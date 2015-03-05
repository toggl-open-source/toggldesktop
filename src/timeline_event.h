// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIMELINE_EVENT_H_
#define SRC_TIMELINE_EVENT_H_

#include <time.h>
#include <string>

#include "Poco/Types.h"

class TimelineEvent {
 public:
    TimelineEvent() :
    id(0),
    user_id(0),
    title(""),
    filename(""),
    start_time(0),
    end_time(0),
    idle(false),
    chunked(false) {}

    Poco::Int64 id;
    Poco::UInt64 user_id;
    std::string title;
    std::string filename;
    time_t start_time;
    time_t end_time;
    bool idle;
    bool chunked;
};

#endif  // SRC_TIMELINE_EVENT_H_
