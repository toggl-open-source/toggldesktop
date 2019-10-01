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
        auto it = timeline_errors_.find(err);
        int count = 1;

        if (it == timeline_errors_.end()) {
            std::stringstream ss;
            ss << "Failed to get focused window info, error code: " << err;
            logger().error(ss.str());
        } else {
            count += timeline_errors_[err];
        }

        timeline_errors_[err] = count;

        if (count % 100 == 0) {
            std::stringstream ss;
            ss << "Failed to get focused window info, error code: "
               << err << " [" << count << " times]";
            logger().error(ss.str());
        }

        // Allow erroneous timeline event in Windows
        // to test windows empty title apps
#if !defined(_WIN32) && !defined(WIN32)
        return;
#endif
    }

    time_t now;
    time(&now);

    time_t time_delta = now - last_event_started_at_;

    if (last_event_started_at_ > 0) {
        if (!last_idle_ && last_autotracker_title_ != title) {
            // Notify that the timeline event has started
            // we'll use this in auto tracking
            last_autotracker_title_ = title;

            TimelineEvent event;
            event.SetStart(last_event_started_at_);
            event.SetEndTime(now);
            event.SetTitle(title);
            event.SetIdle(false);
            timeline_datasource_->StartAutotrackerEvent(event);
        }
    }
    idle = idle || getIsLocked() || getIsSleeping();
    bool idleChanged = hasIdlenessChanged(idle);

    if (idleChanged) {
        last_autotracker_title_ = "";
    }

    if (!idleChanged && !hasWindowChanged(title, filename)) {
        return;
    }

    // We actually record the *previous* event. Meaning, when
    // you have "terminal" open and then switch to "skype",
    // then "terminal" gets recorded here:
    if (last_event_started_at_ > 0) {
        // if window was focussed at least X seconds, save it to timeline
        if (time_delta >= kWindowFocusThresholdSeconds && !last_idle_) {
            TimelineEvent *event = new TimelineEvent();
            event->SetStart(last_event_started_at_);
            event->SetEndTime(now);
            event->SetFilename(last_filename_);
            event->SetTitle(last_title_);
            event->SetIdle(false);
            error err = timeline_datasource_->StartTimelineEvent(event);
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

#define kWindowRecorderSleepMillis 250

void WindowChangeRecorder::recordLoop() {
    while (!recording_.isStopped()) {
        {
            Poco::Mutex::ScopedLock lock(shutdown_m_);
            if (shutdown_) {
                break;
            }
        }

        inspectFocusedWindow();

        if (recording_.isStopped()) {
            break;
        }

        Poco::Thread::sleep(kWindowRecorderSleepMillis);

        if (recording_.isStopped()) {
            break;
        }

        Poco::Thread::sleep(kWindowRecorderSleepMillis);
    }
}

error WindowChangeRecorder::Shutdown() {
    try {
        {
            Poco::Mutex::ScopedLock lock(shutdown_m_);
            shutdown_ = true;
        }
        if (recording_.isRunning()) {
            recording_.stop();
            recording_.wait(5);
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

}  // namespace toggl
