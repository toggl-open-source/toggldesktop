// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIMELINE_UPLOADER_H_
#define SRC_TIMELINE_UPLOADER_H_

#include <string>
#include <vector>

#include "./timeline_event.h"
#include "./timeline_notifications.h"
#include "./timeline_constants.h"
#include "./types.h"

#include "Poco/Activity.h"
#include "Poco/Observer.h"
#include "Poco/NotificationCenter.h"
#include "Poco/Logger.h"

namespace kopsik {

class TimelineUploader {
 public:
    explicit TimelineUploader(const std::string upload_url)
        : current_upload_interval_seconds_(kTimelineUploadIntervalSeconds)
    , upload_url_(upload_url)
    , uploading_(this, &TimelineUploader::upload_loop_activity)
    , observer_(*this,
                &TimelineUploader::handleTimelineBatchReadyNotification) {
        start();
    }

    ~TimelineUploader() {
        Shutdown();
    }

    error Shutdown();

 protected:
    // Notification handlers
    void handleTimelineBatchReadyNotification(
        TimelineBatchReadyNotification *notification);

    // Activity callback
    void upload_loop_activity();

 private:
    error start();

    bool sync(TimelineBatchReadyNotification *notification);

    static std::string convert_timeline_to_json(
        const std::vector<TimelineEvent> &timeline_events,
        const std::string &desktop_id);

    // How many seconds to wait before send next batch of timeline
    // events to backend.
    unsigned int current_upload_interval_seconds_;
    void backoff();
    void reset_backoff();

    std::string upload_url_;

    // An Activity is a possibly long running void/no arguments
    // member function running in its own thread.
    Poco::Activity<TimelineUploader> uploading_;

    Poco::Observer<TimelineUploader, TimelineBatchReadyNotification> observer_;

    Poco::Logger &logger() const {
        return Poco::Logger::get("timeline_uploader");
    }
};

}  // namespace kopsik

#endif  // SRC_TIMELINE_UPLOADER_H_
