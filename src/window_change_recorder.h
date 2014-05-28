// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_WINDOW_CHANGE_RECORDER_H_
#define SRC_WINDOW_CHANGE_RECORDER_H_

#include <string>

#include "./timeline_event.h"
#include "./timeline_notifications.h"
#include "./timeline_constants.h"
#include "./types.h"

#include "Poco/Activity.h"
#include "Poco/Logger.h"

namespace kopsik {

class WindowChangeRecorder {
 public:
    WindowChangeRecorder()
        : last_title_("")
    , last_filename_("")
    , last_event_started_at_(0)
    , recording_(this, &WindowChangeRecorder::recordLoop) {
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

    bool hasTheWindowChanged(const std::string &title,
                             const std::string &filename);

    // Last window focus event data
    std::string last_title_;
    std::string last_filename_;
    time_t last_event_started_at_;

    Poco::Activity<WindowChangeRecorder> recording_;
};

}  // namespace kopsik

#endif  // SRC_WINDOW_CHANGE_RECORDER_H_
