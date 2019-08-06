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

class TOGGL_INTERNAL_EXPORT WindowChangeRecorder {
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
    , isLocked_(false)
    , isSleeping_(false)
    , timeline_errors_() {
        recording_.start();
    }

    ~WindowChangeRecorder() {
        Shutdown();
    }

    void SetIsLocked(bool isLocked) {
        Poco::Mutex::ScopedLock lock(isLocked_m_);
        isLocked_ = isLocked;
    }

    void SetIsSleeping(bool isSleeping) {
        Poco::Mutex::ScopedLock lock(isSleeping_m_);
        isSleeping_ = isSleeping;
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

    bool getIsLocked() {
        Poco::Mutex::ScopedLock lock(isLocked_m_);
        return isLocked_;
    }

    bool getIsSleeping() {
        Poco::Mutex::ScopedLock lock(isSleeping_m_);
        return isSleeping_;
    }

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

    Poco::Mutex isLocked_m_;
    bool isLocked_;

    Poco::Mutex isSleeping_m_;
    bool isSleeping_;

    std::map<const int, int> timeline_errors_;
};

}  // namespace toggl

#endif  // SRC_WINDOW_CHANGE_RECORDER_H_
