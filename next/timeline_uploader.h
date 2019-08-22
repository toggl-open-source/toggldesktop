// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIMELINE_UPLOADER_H_
#define SRC_TIMELINE_UPLOADER_H_

#include <string>
#include <vector>

#include "model/timeline_event.h"
#include "timeline_notifications.h"
#include "types.h"
#include "event_queue.h"

namespace Poco {
class Logger;
}

namespace toggl {

std::string convertTimelineToJSON(
    const std::vector<TimelineEvent> &timeline_events,
    const std::string &desktop_id);

class TimelineUploader : public Event {
 public:
    explicit TimelineUploader(TimelineDatasource *ds, EventQueue *queue)
        : Event(queue)
        , current_upload_interval_seconds_(kTimelineUploadIntervalSeconds)
    , timeline_datasource_(ds)
    {
        start();
    }

    ~TimelineUploader() {
        Shutdown();
    }

    error Shutdown();

 protected:
    // Activity callback
    void execute() override;

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

    TimelineDatasource *timeline_datasource_;
};

}  // namespace toggl

#endif  // SRC_TIMELINE_UPLOADER_H_
