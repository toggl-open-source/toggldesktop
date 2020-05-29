// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIMELINE_EVENT_H_
#define SRC_TIMELINE_EVENT_H_

#include <time.h>

#include <sstream>
#include <string>

#include "model/base_model.h"
#include "util/formatter.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT TimelineEvent : public BaseModel, public TimedEvent {
 public:
    TimelineEvent() : BaseModel() {}
    virtual ~TimelineEvent() {}

    Property<std::string> Title { "" };
    Property<std::string> Filename { "" };
    Property<Poco::Int64> StartTime { 0 };
    Property<Poco::Int64> EndTime { 0 };
    Property<Poco::Int64> DurationInSeconds { 0 };
    Property<bool> Idle { false };
    Property<bool> Chunked { false };
    Property<bool> Uploaded { false };

    void SetTitle(const std::string &value);
    void SetStart(const Poco::Int64 value);
    void SetEndTime(const Poco::Int64 value);
    void SetIdle(const bool value);
    void SetFilename(const std::string &value);
    void SetChunked(const bool value);
    void SetUploaded(const bool value);

    // Derived data
    bool VisibleToUser() const {
        return !Uploaded() && !DeletedAt() && Chunked();
    }

    // Override TimedEvent
    const Poco::Int64 &Start() const  override {
        return StartTime();
    }
    const Poco::Int64 &Duration() const {
        return DurationInSeconds();
    }

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    Json::Value SaveToJSON() const override;

 private:

    void updateDuration();
};

}  // namespace toggl

#endif  // SRC_TIMELINE_EVENT_H_
