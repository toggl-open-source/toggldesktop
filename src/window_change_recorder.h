// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_WINDOW_CHANGE_RECORDER_H_
#define SRC_WINDOW_CHANGE_RECORDER_H_

#include <string>
#include <map>

#include "./timeline_notifications.h"
#include "./types.h"

#include "Poco/Activity.h"

namespace Poco {
class Logger;
}

namespace toggl {

class WindowChangeRecorder {
 public:
    explicit WindowChangeRecorder(TimelineDatasource *datasource)
        : last_title_("")
    , last_filename_("")
    , last_event_started_at_(0)
    , last_idle_(false)
    , timeline_datasource_(datasource)
    , recording_(this, &WindowChangeRecorder::recordLoop)
    , last_autotracker_title_("")
    , shutdown_(false)
    , timeline_errors_() {
        recording_.start();
    }

    ~WindowChangeRecorder() {
        Shutdown();
    }

    error Shutdown();

 protected:
    void recordLoop();

 private:
    void inspectFocusedWindow();

    bool hasWindowChanged(
        const std::string &title,
        const std::string &filename) const;

    bool hasIdlenessChanged(const bool &idle) const;

    Poco::Logger &logger();

    // Last window focus event data
    std::string last_title_;
    std::string last_filename_;
    time_t last_event_started_at_;
    bool last_idle_;

    TimelineDatasource *timeline_datasource_;

    Poco::Activity<WindowChangeRecorder> recording_;

    std::string last_autotracker_title_;

    Poco::Mutex shutdown_m_;
    bool shutdown_;

    std::map<const int, int> timeline_errors_;
};

}  // namespace toggl

#endif  // SRC_WINDOW_CHANGE_RECORDER_H_
