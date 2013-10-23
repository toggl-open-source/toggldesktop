// Copyright (c) 2013 Toggl

#ifndef SRC_TIMELINE_EVENT_H_
#define SRC_TIMELINE_EVENT_H_

#include <time.h>
#include <string>

class TimelineEvent {
 public:
    TimelineEvent() :
        id(0),
        user_id(0),
        title(""),
        filename(""),
        start_time(0),
        end_time(0),
        idle(false) {
    };

    unsigned int id;
    unsigned int user_id;
    std::string title;
    std::string filename;
    time_t start_time;
    time_t end_time;
    bool idle;
};

#endif  // SRC_TIMELINE_EVENT_H_
