// Copyright (c) 2013 Toggl

#ifndef SRC_WINDOW_CHANGE_RECORDER_H_
#define SRC_WINDOW_CHANGE_RECORDER_H_

#include <string>

#include "./timeline_event.h"
#include "./timeline_notifications.h"
#include "./timeline_constants.h"

#include "Poco/NotificationCenter.h"
#include "Poco/Activity.h"
#include "Poco/Observer.h"

namespace kopsik {

class WindowChangeRecorder {
 public:
    WindowChangeRecorder() :
            user_id_(0),
            last_title_(""),
            last_filename_(""),
            last_event_started_at_(0),
            window_focus_seconds_(kWindowFocusThresholdSeconds),
            recording_interval_ms_(kWindowChangeRecordingIntervalMillis),
            recording_(this, &WindowChangeRecorder::record_loop) {
        Poco::NotificationCenter& nc =
            Poco::NotificationCenter::defaultCenter();

        Poco::Observer<WindowChangeRecorder, ConfigureNotification>
            observeUser(*this,
                &WindowChangeRecorder::handleConfigureNotification);
        nc.addObserver(observeUser);

        Poco::Observer<WindowChangeRecorder, UserTimelineSettingsNotification>
            observeUserTimelineSettings(*this,
                &WindowChangeRecorder::handleUserTimelineSettingsNotification);
        nc.addObserver(observeUserTimelineSettings);
    }

    // Handle notifications
    void handleConfigureNotification(ConfigureNotification* notification);
    void handleUserTimelineSettingsNotification(
        UserTimelineSettingsNotification* notification);

    void Start();
    void Stop();

 protected:
    // Subsystem overrides
    const char* name() const { return "window_change_recorder"; }

    // Activity callback
    void record_loop();

 private:
    void inspect_focused_window();

    // User who is recording the events.
    unsigned int user_id_;

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
