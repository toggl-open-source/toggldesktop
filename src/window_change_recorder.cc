// Copyright (c) 2013 Toggl

#include "./window_change_recorder.h"

#include <sstream>

#include "./get_focused_window.h"
#include "./timeline_event.h"

#include "Poco/Thread.h"
#include "Poco/Util/Option.h"
#include "Poco/Logger.h"
#include "Poco/Util/OptionSet.h"

namespace kopsik {

void WindowChangeRecorder::start_recording() {
    if (user_id_ > 0) {
        std::stringstream out;
        out << "start_recording, user_id = " << user_id_;
        Poco::Logger &logger = Poco::Logger::get("window_change_recorder");
        logger.debug(out.str());
        if (!recording_.isRunning()) {
            recording_.start();

            // Tell all the people
            Poco::NotificationCenter& nc =
                Poco::NotificationCenter::defaultCenter();
            RecordingStatusNotification notification(true, user_id_);
            Poco::AutoPtr<RecordingStatusNotification> ptr(&notification);
            nc.postNotification(ptr);
        }
    }
}

void WindowChangeRecorder::stop_recording() {
    if (recording_.isRunning()) {
        recording_.stop();
        recording_.wait();

        // Tell all the people
        Poco::NotificationCenter& nc =
            Poco::NotificationCenter::defaultCenter();
        RecordingStatusNotification notification(false, user_id_);
        Poco::AutoPtr<RecordingStatusNotification> ptr(&notification);
        nc.postNotification(ptr);
    }
}

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
        Poco::Logger &logger = Poco::Logger::get("window_change_recorder");
        logger.debug("record_loop");
        inspect_focused_window();
        Poco::Thread::sleep(recording_interval_ms_);
    }
}

void WindowChangeRecorder::handleConfigureNotification(
        ConfigureNotification* notification) {
    std::stringstream out;
    out << "handleConfigureNotification, user_id = " << notification->user_id;
    Poco::Logger &logger = Poco::Logger::get("window_change_recorder");
    logger.debug(out.str());
    if (user_id_ != notification->user_id) {
        logger.debug("user has changed, stopping recording");
        stop_recording();
    }
    // Setting the user ID won't start timeline recording, first
    // we'll need confirmation from server, that recording is really
    // enabled.
    user_id_ = notification->user_id;
}

void WindowChangeRecorder::handleUserTimelineSettingsNotification(
        UserTimelineSettingsNotification* notification) {
    Poco::Logger &logger = Poco::Logger::get("window_change_recorder");

    {
        std::stringstream out;
        out << "handleUserTimelineSettingsNotification"
            << ", user_id = " << notification->user_id
            << ", record_timeline = " << notification->record_timeline;
        logger.debug(out.str());
    }

    if (user_id_ == notification->user_id) {
        if (notification->record_timeline) {
            start_recording();
        } else if (!notification->record_timeline) {
            stop_recording();
        }
    } else {
        std::stringstream out;
        out << "ignoring user " << notification->user_id <<
            " settings, I know about user " << user_id_;
        logger.warning(out.str());
    }
}

}  // namespace kopsik
