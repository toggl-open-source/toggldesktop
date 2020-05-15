// Copyright 2014 Toggl Desktop developers.

#include "window_change_recorder.h"

#include <sstream>

#include "get_focused_window.h"
#include "const.h"

#include <Poco/Thread.h>

#if defined(__APPLE__)
extern bool isCatalinaOSX(void);
#endif

namespace toggl {

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
            logger.error("Failed to get focused window info, error code: ", err);
        } else {
            count += timeline_errors_[err];
        }

        timeline_errors_[err] = count;

        if (count % 100 == 0) {
            logger.error("Failed to get focused window info, error code: ", err, " [", count, " times]");
        }

        // Allow erroneous timeline event in Windows
        // to test windows empty title apps
#if !defined(_WIN32) && !defined(WIN32)
        return;
#endif
    }

    // Check if we need ScreenRecording permission in order to receive the Window's title
    if (is_catalina_OSX) {

        // It's lite version of Timeline recording
        // 10.15+ and title is empty
        // Just set it as a filename
        if (title.empty()) {
            title = std::string(filename);
        }
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
            event.StartTime.Set(last_event_started_at_);
            event.EndTime.Set(now);
            event.Title.Set(title);
            event.Idle.Set(false);
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

    // Lite version of timeline recorder
    // Since we don't have Screen Recording permission yet => title will be empty
    // So we only track the primary timeline (treat title is filename)
    if (is_catalina_OSX && last_title_ == title && last_filename_ == filename) {
        return;
    }

    // We actually record the *previous* event. Meaning, when
    // you have "terminal" open and then switch to "skype",
    // then "terminal" gets recorded here:
    if (last_event_started_at_ > 0) {
        // if window was focussed at least X seconds, save it to timeline
        if (time_delta >= kWindowFocusThresholdSeconds && !last_idle_) {
            TimelineEvent *event = new TimelineEvent();
            event->StartTime.Set(last_event_started_at_);
            event->EndTime.Set(now);
            event->Filename.Set(last_filename_);
            event->Title.Set(last_title_);
            event->Idle.Set(false);
            error err = timeline_datasource_->StartTimelineEvent(event);
            if (err != noError) {
                logger.error(err);
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
