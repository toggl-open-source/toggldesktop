// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIMELINE_NOTIFICATIONS_H_
#define SRC_TIMELINE_NOTIFICATIONS_H_

#include "./timeline_event.h"

#include <vector>
#include <string>

#include "./types.h"

#include "Poco/Types.h"

namespace toggl {

class TimelineBatch {
 public:
    TimelineBatch()
        : user_id_(0)
    , api_token_("")
    , desktop_id_("") {}

    ~TimelineBatch() {}

    Poco::UInt64 &UserID() {
        return user_id_;
    }
    void SetUserID(const Poco::UInt64 value) {
        user_id_ = value;
    }

    std::string &APIToken() {
        return api_token_;
    }
    void SetAPIToken(const std::string &value) {
        api_token_ = value;
    }

    std::vector<TimelineEvent> &Events() {
        return events_;
    }
    void SetEvents(const std::vector<TimelineEvent> &value) {
        events_ = value;
    }

    std::string &DesktopID() {
        return desktop_id_;
    }
    void SetDesktopID(const std::string &value) {
        desktop_id_ = value;
    }

 private:
    Poco::UInt64 user_id_;
    std::string api_token_;
    std::vector<TimelineEvent> events_;
    std::string desktop_id_;
};

class TimelineDatasource {
 public:
    virtual ~TimelineDatasource() {}

    // A autotracker event has started
    virtual error StartAutotrackerEvent(const TimelineEvent &event) = 0;

    // A timeline event is detected, window has changes
    // or there's an idle event.
    virtual error StartTimelineEvent(TimelineEvent *event) = 0;

    // Find timeline events for upload,
    virtual error CreateCompressedTimelineBatchForUpload(
        TimelineBatch *batch) = 0;

    // A batch of timeline events has been upladed and is marked assuch.
    virtual error MarkTimelineBatchAsUploaded(
        const std::vector<TimelineEvent> &events) = 0;
};

};  // namespace toggl

#endif  // SRC_TIMELINE_NOTIFICATIONS_H_
