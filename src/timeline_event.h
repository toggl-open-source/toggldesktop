// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIMELINE_EVENT_H_
#define SRC_TIMELINE_EVENT_H_

#include <time.h>

#include <sstream>
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
    chunked(false),
    uploaded(false) {}

    Poco::Int64 id;
    Poco::UInt64 user_id;
    std::string title;
    std::string filename;
    time_t start_time;
    time_t end_time;
    bool idle;
    bool chunked;
    bool uploaded;

    std::string String() const {
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
};

#endif  // SRC_TIMELINE_EVENT_H_
