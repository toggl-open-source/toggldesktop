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
    , menubar_project(false)
    , dock_icon(false)
    , on_top(false)
    , reminder(false)
    , idle_minutes(0)
    , focus_on_shortcut(true)
    , reminder_minutes(0)
    , manual_mode(false)
    , autodetect_proxy(true) {}
    ~Settings() {}

    bool use_idle_detection;
    bool menubar_timer;
    bool menubar_project;
    bool dock_icon;
    bool on_top;
    bool reminder;
    Poco::UInt64 idle_minutes;
    bool focus_on_shortcut;
    Poco::UInt64 reminder_minutes;
    bool manual_mode;
    bool autodetect_proxy;
};

}  // namespace toggl

#endif  // SRC_SETTINGS_H_
