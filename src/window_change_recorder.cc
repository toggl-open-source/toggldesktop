// Copyright 2014 Toggl Desktop developers.

#include "./window_change_recorder.h"

#include <sstream>

#include "./get_focused_window.h"
#include "./timeline_event.h"

#include "Poco/Thread.h"
#include "Poco/NotificationCenter.h"
#include "Poco/Logger.h"

namespace kopsik {

bool WindowChangeRecorder::hasWindowChanged(
    const std::string &title,
    const std::string &filename) const {
    return ((title != last_title_) || (filename != last_filename_));
}

bool WindowChangeRecorder::hasIdlenessChanged(const bool &idle) const {
    return last_idle_ != idle;
}

void WindowChangeRecorder::inspectFocusedWindow() {
    std::string title("");
    std::string filename("");
    bool idle(false);

    int err = getFocusedWindowInfo(&title, &filename, &idle);
    if (err != 0) {
        std::stringstream ss;
        ss << "Failed to get focused window info, error code: " << err;
        Poco::Logger::get("WindowChangeRecorder").error(ss.str());
        return;
    }

    time_t now;
    time(&now);

    if (!hasIdlenessChanged(idle) && !hasWindowChanged(title, filename)) {
        return;
    }
    // We actually record the *previous* event. Meaning, when
    // you have "terminal" open and then switch to "skype",
    // then "terminal" gets recorded here:
    if (last_event_started_at_ > 0) {
        time_t time_delta = now - last_event_started_at_;

        // if window was focussed at least X seconds, save it to timeline
        if (time_delta >= kWindowFocusThresholdSeconds) {
            TimelineEvent event;
            event.start_time = last_event_started_at_;
            event.end_time = now;
            event.filename = last_filename_;
            event.title = last_title_;
            event.idle = last_idle_;

            TimelineEventNotification notification(event);
            Poco::AutoPtr<TimelineEventNotification> ptr(&notification);
            Poco::NotificationCenter::defaultCenter().postNotification(ptr);
        }
    }

    last_title_ = title;
    last_filename_ = filename;
    last_event_started_at_ = now;
}

void WindowChangeRecorder::recordLoop() {
    while (!recording_.isStopped()) {
        inspectFocusedWindow();
        Poco::Thread::sleep(kWindowChangeRecordingIntervalMillis);
    }
}

error WindowChangeRecorder::Shutdown() {
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

}  // namespace kopsik
