// Copyright 2014 Toggl Desktop developers.

#include "../src/window_change_recorder.h"

#include <sstream>

#include "./get_focused_window.h"
#include "./const.h"

#include "Poco/Logger.h"
#include "Poco/Thread.h"

namespace toggl {

Poco::Logger &WindowChangeRecorder::logger() {
    return Poco::Logger::get("WindowChangeRecorder");
}

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
        logger().error(ss.str());
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
        if (time_delta >= kWindowFocusThresholdSeconds && !last_idle_) {
            TimelineEvent event;
            event.start_time = last_event_started_at_;
            event.end_time = now;
            event.filename = last_filename_;
            event.title = last_title_;
            event.idle = false;

            error err = timeline_datasource_->SaveTimelineEvent(&event);
            if (err != noError) {
                logger().error(err);
            }
        }
    }

    last_title_ = title;
    last_filename_ = filename;
    last_idle_ = idle;
    last_event_started_at_ = now;
}

void WindowChangeRecorder::recordLoop() {
    while (!recording_.isStopped()) {
        inspectFocusedWindow();

        if (recording_.isStopped()) {
            break;
        }

        Poco::Thread::sleep(250);

        if (recording_.isStopped()) {
            break;
        }

        Poco::Thread::sleep(250);
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

}  // namespace toggl
