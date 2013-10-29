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
    TimelineUploader() :
            user_id_(0),
            upload_token_(""),
            upload_interval_seconds_(kTimelineUploadIntervalSeconds),
            current_upload_interval_seconds_(kTimelineUploadIntervalSeconds),
            max_upload_interval_seconds_(kTimelineUploadMaxBackoffSeconds),
            upload_host_(kTimelineUploadHost),
            uploading_(this, &TimelineUploader::upload_loop_activity) {
        Poco::NotificationCenter& nc =
            Poco::NotificationCenter::defaultCenter();

        Poco::Observer<TimelineUploader, ConfigureNotification>
            observeUser(*this,
                &TimelineUploader::handleConfigureNotification);
        nc.addObserver(observeUser);

        Poco::Observer<TimelineUploader, TimelineBatchReadyNotification>
            observeUpload(*this,
                &TimelineUploader::handleTimelineBatchReadyNotification);
        nc.addObserver(observeUpload);
    }

 protected:
    // Subsystem overrides
    const char* name() const { return "timeline_uploader"; }

    // Notification handlers
    void handleConfigureNotification(ConfigureNotification* notification);
    void handleTimelineBatchReadyNotification(
        TimelineBatchReadyNotification *notification);

    // Handle command line params
    void handleConfigUploadInterval(
        const std::string& name, const std::string& value);
    void handleConfigUploadHost(
        const std::string& name, const std::string& value);

    // Activity callback
    void upload_loop_activity();

 private:
    // Sync with server
    bool sync(const unsigned int user_id,
        const std::string &upload_token,
        const std::vector<TimelineEvent> &timeline_events,
        const std::string &desktop_id);
    std::string convert_timeline_to_json(
        const std::vector<TimelineEvent> &timeline_events,
        const std::string &desktop_id);
    void json_to_timeline_settings(
        const std::string &json, bool &record_timeline);

    // Actity start/stop
    void start_uploading();
    void stop_uploading();

    // Hopefully an authenticated user sending the timeline events.
    unsigned int user_id_;
    std::string upload_token_;

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
