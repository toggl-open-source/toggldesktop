// Copyright (c) 2013 Toggl

#ifndef SRC_TIMELINE_UPLOADER_H_
#define SRC_TIMELINE_UPLOADER_H_

#include "./timeline_event.h"
#include "./timeline_notifications.h"
#include "./timeline_constants.h"

#include <string>
#include <vector>

#include "Poco/Activity.h"
#include "Poco/Observer.h"
#include "Poco/NotificationCenter.h"

namespace kopsik {

class TimelineUploader {
 public:
    TimelineUploader(const unsigned int user_id,
                const std::string api_token) :
            user_id_(user_id),
            api_token_(api_token),
            upload_interval_seconds_(kTimelineUploadIntervalSeconds),
            current_upload_interval_seconds_(kTimelineUploadIntervalSeconds),
            max_upload_interval_seconds_(kTimelineUploadMaxBackoffSeconds),
            upload_host_(kTimelineUploadHost),
            uploading_(this, &TimelineUploader::upload_loop_activity) {
        Poco::NotificationCenter& nc =
            Poco::NotificationCenter::defaultCenter();

        Poco::Observer<TimelineUploader, TimelineBatchReadyNotification>
            observeUpload(*this,
                &TimelineUploader::handleTimelineBatchReadyNotification);
        nc.addObserver(observeUpload);

        poco_assert(!api_token_.empty());
        poco_assert(user_id_ > 0);
        uploading_.start();
    }

    ~TimelineUploader() {
        if (uploading_.isRunning()) {
            uploading_.stop();
            uploading_.wait();
        }
    }

 protected:
    // Notification handlers
    void handleTimelineBatchReadyNotification(
        TimelineBatchReadyNotification *notification);

    // Activity callback
    void upload_loop_activity();

 private:
    // Sync with server
    bool sync(const unsigned int user_id,
        const std::string api_token,
        const std::vector<TimelineEvent> &timeline_events,
        const std::string desktop_id);
    std::string convert_timeline_to_json(
        const std::vector<TimelineEvent> &timeline_events,
        const std::string &desktop_id);

    // Hopefully an authenticated user sending the timeline events.
    unsigned int user_id_;
    std::string api_token_;

    // How many seconds to wait before send next batch of timeline
    // events to backend.
    unsigned int upload_interval_seconds_;

    // Exponential backoff implementation
    void exponential_backoff();
    void reset_backoff();
    unsigned int current_upload_interval_seconds_;
    unsigned int max_upload_interval_seconds_;

    // Toggl API host
    std::string upload_host_;

    // An Activity is a possibly long running void/no arguments
    // member function running in its own thread.
    Poco::Activity<TimelineUploader> uploading_;
};

}  // namespace kopsik

#endif  // SRC_TIMELINE_UPLOADER_H_
