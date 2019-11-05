// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIMELINE_UPLOADER_H_
#define SRC_TIMELINE_UPLOADER_H_

#include <string>
#include <vector>

#include "./timeline_event.h"
#include "./timeline_notifications.h"
#include "./types.h"

#include "Poco/Activity.h"

namespace Poco {
class Logger;
}

namespace toggl {

std::string convertTimelineToJSON(
    const std::vector<TimelineEvent> &timeline_events,
    const std::string &desktop_id);

class TOGGL_INTERNAL_EXPORT TimelineUploader {
 public:
    explicit TimelineUploader(TimelineDatasource *ds)
        : current_upload_interval_seconds_(kTimelineUploadIntervalSeconds)
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

    // How many seconds to wait before send next batch of timeline
    // events to backend.
    unsigned int current_upload_interval_seconds_;
    void backoff();
    void reset_backoff();

    Poco::Logger &logger() const;

    error process();
    void sleep();

    TimelineDatasource *timeline_datasource_;

    // An Activity is a possibly long running void/no arguments
    // member function running in its own thread.
    Poco::Activity<TimelineUploader> uploading_;
};

}  // namespace toggl

#endif  // SRC_TIMELINE_UPLOADER_H_
