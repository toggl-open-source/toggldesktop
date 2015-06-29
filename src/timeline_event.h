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
    , title_("")
    , filename_("")
    , start_time_(0)
    , end_time_(0)
    , idle_(false)
    , chunked_(false)
    , uploaded_(false) {}

    virtual ~TimelineEvent() {}

    const std::string &Title() const {
        return title_;
    }
    void SetTitle(const std::string value);

    const time_t &StartTime() const {
        return start_time_;
    }
    void SetStartTime(const time_t value);

    const time_t &EndTime() const {
        return end_time_;
    }
    void SetEndTime(const time_t value);

    const bool &Idle() const {
        return idle_;
    }
    void SetIdle(const bool value);

    const std::string &Filename() const {
        return filename_;
    }
    void SetFilename(const std::string value);

    const bool &Chunked() const {
        return chunked_;
    }
    void SetChunked(const bool value);

    const bool &Uploaded() const {
        return uploaded_;
    }
    void SetUploaded(const bool value);

    // Override BaseModel

    std::string String() const;
    std::string ModelName() const;
    std::string ModelURL() const;

 private:
    std::string title_;
    std::string filename_;
    time_t start_time_;
    time_t end_time_;
    bool idle_;
    bool chunked_;
    bool uploaded_;
};

}  // namespace toggl

#endif  // SRC_TIMELINE_EVENT_H_
