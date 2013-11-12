// Copyright (c) 2013 Toggl

#include "./window_change_recorder.h"

#include <sstream>

#include "./get_focused_window.h"
#include "./timeline_event.h"

#include "Poco/Thread.h"
#include "Poco/NotificationCenter.h"

namespace kopsik {

void WindowChangeRecorder::inspect_focused_window() {
    std::string title("");
    std::string filename("");

    GetFocusedWindowInfo(&title, &filename);

    time_t now;
    time(&now);

    // Has the window changed?
    if ((title != last_title_) || (filename != last_filename_)) {
        // We actually record the *previous* event. Meaning, when
        // you have "terminal" open and then switch to "skype",
        // then "terminal" gets recorded here:
        if (last_event_started_at_ > 0) {
            time_t time_delta = now - last_event_started_at_;

            // if window was focussed at least X seconds, save it to timeline
            if (time_delta >= window_focus_seconds_) {
                poco_assert(user_id_ > 0);
                TimelineEvent event;
                event.start_time = last_event_started_at_;
                event.end_time = now;
                event.filename = last_filename_;
                event.title = last_title_;
                event.user_id = user_id_;
                Poco::NotificationCenter& nc =
                    Poco::NotificationCenter::defaultCenter();
                TimelineEventNotification notification(event);
                Poco::AutoPtr<TimelineEventNotification> ptr(&notification);
                nc.postNotification(ptr);
            }
        }

        last_title_ = title;
        last_filename_ = filename;
        last_event_started_at_ = now;
    }
}

void WindowChangeRecorder::record_loop() {
    while (!recording_.isStopped()) {
        inspect_focused_window();
        Poco::Thread::sleep(recording_interval_ms_);
    }
}

}  // namespace kopsik
