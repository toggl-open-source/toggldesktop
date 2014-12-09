// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include "Poco/Types.h"

namespace toggl {

class Settings {
 public:
    Settings()
        : use_idle_detection(false)
    , menubar_timer(false)
    , dock_icon(false)
    , on_top(false)
    , reminder(false)
    , idle_minutes(0)
    , focus_on_shortcut(false) {}
    ~Settings() {}

    bool use_idle_detection;
    bool menubar_timer;
    bool dock_icon;
    bool on_top;
    bool reminder;
    Poco::UInt64 idle_minutes;
    bool focus_on_shortcut;
};

}  // namespace toggl

#endif  // SRC_SETTINGS_H_
