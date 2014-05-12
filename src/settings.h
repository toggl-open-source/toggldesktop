// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

namespace kopsik {

class Settings {
 public:
    Settings()
        : use_idle_detection(false)
    , menubar_timer(false)
    , dock_icon(false)
    , on_top(false)
    , reminder(false) {}
    ~Settings() {}

    bool use_idle_detection;
    bool menubar_timer;
    bool dock_icon;
    bool on_top;
    bool reminder;
};

}  // namespace kopsik

#endif  // SRC_SETTINGS_H_
