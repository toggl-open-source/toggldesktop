// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIMELINE_NOTIFICATIONS_H_
#define SRC_TIMELINE_NOTIFICATIONS_H_

#include "./timeline_event.h"

#include <vector>
#include <string>

#include "Poco/Notification.h"

// A timeline event is detected, window has changes
// or there's and idle event.
class TimelineEventNotification : public Poco::Notification {
 public:
    explicit TimelineEventNotification(TimelineEvent _event) : event(_event) {}
    TimelineEvent event;
};

// Find timeline events (for upload).
class CreateTimelineBatchNotification : public Poco::Notification {
 public:
    CreateTimelineBatchNotification() {}
};

// A batch of timeline events has been found in database, that
// is ready for upload.
class TimelineBatchReadyNotification : public Poco::Notification {
 public:
    TimelineBatchReadyNotification(const Poco::UInt64 user_id,
                                   std::string api_token,
                                   std::vector<TimelineEvent> events,
                                   std::string desktop_id)
        : user_id_(user_id)
    , api_token_(api_token)
    , events_(events)
    , desktop_id_(desktop_id) {}

    ~TimelineBatchReadyNotification() {}

    Poco::UInt64 &UserID() {
        return user_id_;
    }
    std::string &APIToken() {
        return api_token_;
    }
    std::vector<TimelineEvent> &Events() {
        return events_;
    }
    std::string &DesktopID() {
        return desktop_id_;
    }

 private:
    Poco::UInt64 user_id_;
    std::string api_token_;
    std::vector<TimelineEvent> events_;
    std::string desktop_id_;
};

// A batch of timeline events has been upladed and may be deleted.
class DeleteTimelineBatchNotification : public Poco::Notification {
 public:
    explicit DeleteTimelineBatchNotification(std::vector<TimelineEvent> _batch)
        : batch(_batch) {}
    std::vector<TimelineEvent> batch;
};

#endif  // SRC_TIMELINE_NOTIFICATIONS_H_
