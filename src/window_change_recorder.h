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
  explicit WindowChangeRecorder(const Poco::UInt64 user_id) :
            user_id_(user_id),
            last_title_(""),
            last_filename_(""),
            last_event_started_at_(0),
            window_focus_seconds_(kWindowFocusThresholdSeconds),
            recording_interval_ms_(kWindowChangeRecordingIntervalMillis),
            recording_(this, &WindowChangeRecorder::record_loop) {
        poco_assert(user_id_);
        recording_.start();
    }

    error Stop() {
        try {
            if (recording_.isRunning()) {
                recording_.stop();
                recording_.wait();
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

    ~WindowChangeRecorder() {
        Stop();
    }

 protected:
    // Activity callback
    void record_loop();

 private:
    void inspect_focused_window();

    // User who is recording the events.
    Poco::UInt64 user_id_;

    // Last window focus event data
    std::string last_title_;
    std::string last_filename_;
    time_t last_event_started_at_;

    // How many seconds must be a window focussed,
    // until the window change is recorded.
    unsigned int window_focus_seconds_;

    unsigned int recording_interval_ms_;

    Poco::Activity<WindowChangeRecorder> recording_;
};

}  // namespace kopsik

#endif  // SRC_WINDOW_CHANGE_RECORDER_H_
