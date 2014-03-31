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
    TimelineUploader(
        const Poco::UInt64 user_id,
        const std::string api_token,
        const std::string timeline_upload_url,
        const std::string app_name,
        const std::string app_version) :
    user_id_(user_id),
    api_token_(api_token),
    upload_interval_seconds_(kTimelineUploadIntervalSeconds),
    current_upload_interval_seconds_(kTimelineUploadIntervalSeconds),
    max_upload_interval_seconds_(kTimelineUploadMaxBackoffSeconds),
    timeline_upload_url_(timeline_upload_url),
    app_name_(app_name),
    app_version_(app_version),
    uploading_(this, &TimelineUploader::upload_loop_activity) {
        Poco::NotificationCenter& nc =
            Poco::NotificationCenter::defaultCenter();

        Poco::Observer<TimelineUploader, TimelineBatchReadyNotification>
        observeUpload(*this,
                      &TimelineUploader::handleTimelineBatchReadyNotification);
        nc.addObserver(observeUpload);

        poco_assert(!api_token_.empty());
        poco_assert(user_id_ > 0);
        poco_assert(!timeline_upload_url.empty());

        uploading_.start();
    }

    error Stop() {
        try {
            if (uploading_.isRunning()) {
                uploading_.stop();
                uploading_.wait();
            }
        } catch(const Poco::Exception& exc) {
            return exc.displayText();
        } catch(const std::exception& ex) {
            return ex.what();
        } catch(const std::string& ex) {
            return ex;
        }
        return noError;
    }

    ~TimelineUploader() {
        Stop();
    }

 protected:
    // Notification handlers
    void handleTimelineBatchReadyNotification(
        TimelineBatchReadyNotification *notification);

    // Activity callback
    void upload_loop_activity();

 private:
    // Sync with server
    bool sync(
        const Poco::UInt64 user_id,
        const std::string api_token,
        const std::vector<TimelineEvent> &timeline_events,
        const std::string desktop_id);
    static std::string convert_timeline_to_json(
        const std::vector<TimelineEvent> &timeline_events,
        const std::string &desktop_id);

    Poco::UInt64 user_id_;
    std::string api_token_;

    // How many seconds to wait before send next batch of timeline
    // events to backend.
    unsigned int upload_interval_seconds_;

    // Exponential backoff implementation
    void exponential_backoff();
    void reset_backoff();
    unsigned int current_upload_interval_seconds_;
    unsigned int max_upload_interval_seconds_;

    std::string timeline_upload_url_;
    std::string app_name_;
    std::string app_version_;

    // An Activity is a possibly long running void/no arguments
    // member function running in its own thread.
    Poco::Activity<TimelineUploader> uploading_;

    Poco::Logger &logger() const {
        return Poco::Logger::get("timeline_uploader");
    }
};

}  // namespace kopsik

#endif  // SRC_TIMELINE_UPLOADER_H_
