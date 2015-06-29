// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIMELINE_EVENT_H_
#define SRC_TIMELINE_EVENT_H_

#include <time.h>

#include <sstream>
#include <string>

#include "Poco/Types.h"

#include "./base_model.h"

namespace toggl {

class TimelineEvent  : public BaseModel {
 public:
    TimelineEvent()
        : BaseModel()
    , id(0)
    , user_id(0)
    , title("")
    , filename("")
    , start_time(0)
    , end_time(0)
    , idle(false)
    , chunked(false)
    , uploaded(false) {}

    virtual ~TimelineEvent() {}

    void SetTitle(const std::string value);


    Poco::Int64 id;
    Poco::UInt64 user_id;
    std::string title;
    std::string filename;
    time_t start_time;
    time_t end_time;
    bool idle;
    bool chunked;
    bool uploaded;

    // Override BaseModel

    std::string String() const;
    std::string ModelName() const;
    std::string ModelURL() const;
};

}  // namespace toggl

#endif  // SRC_TIMELINE_EVENT_H_
