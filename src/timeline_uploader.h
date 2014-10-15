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
#include "Poco/Logger.h"

namespace toggl {

class TimelineUploader {
 public:
    TimelineUploader(const std::string upload_url, TimelineDatasource *ds)
        : current_upload_interval_seconds_(kTimelineUploadIntervalSeconds)
    , upload_url_(upload_url)
    , timeline_datasource_(ds)
    , uploading_(this, &TimelineUploader::upload_loop_activity) {
        start();
    }

    ~TimelineUploader() {
        Shutdown();
    }

    error Shutdown();

 protected:
    // Activity callback
    void upload_loop_activity();

 private:
    error start();

    error upload(TimelineBatch *batch);

    static std::string convert_timeline_to_json(
        const std::vector<TimelineEvent> &timeline_events,
        const std::string &desktop_id);

    // How many seconds to wait before send next batch of timeline
    // events to backend.
    unsigned int current_upload_interval_seconds_;
    void backoff();
    void reset_backoff();

    Poco::Logger &logger() const {
        return Poco::Logger::get("timeline_uploader");
    }

    error process();
    void sleep();

    std::string upload_url_;

    TimelineDatasource *timeline_datasource_;

    // An Activity is a possibly long running void/no arguments
    // member function running in its own thread.
    Poco::Activity<TimelineUploader> uploading_;
};

}  // namespace toggl

#endif  // SRC_TIMELINE_UPLOADER_H_
