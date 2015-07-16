// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include <sstream>
#include <string>

#include <json/json.h>  // NOLINT

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
    , autodetect_proxy(true)
    , remind_mon(true)
    , remind_tue(true)
    , remind_wed(true)
    , remind_thu(true)
    , remind_fri(true)
    , remind_sat(true)
    , remind_sun(true)
    , remind_starts("")
    , remind_ends("")
    , autotrack(false)
    , open_editor_on_shortcut(false)
    , has_seen_beta_offering(false)
    , render_timeline(false) {}
    ~Settings() {}

    bool IsSame(const Settings &other) const {
        return ((use_idle_detection == other.use_idle_detection)
                && (menubar_timer == other.menubar_timer)
                && (menubar_project == other.menubar_project)
                && (dock_icon == other.dock_icon)
                && (on_top == other.on_top)
                && (reminder == other.reminder)
                && (idle_minutes == other.idle_minutes)
                && (focus_on_shortcut == other.focus_on_shortcut)
                && (reminder_minutes == other.reminder_minutes)
                && (manual_mode == other.manual_mode)
                && (autodetect_proxy == other.autodetect_proxy)
                && (remind_mon == other.remind_mon)
                && (remind_tue == other.remind_tue)
                && (remind_wed == other.remind_wed)
                && (remind_thu == other.remind_thu)
                && (remind_fri == other.remind_fri)
                && (remind_sat == other.remind_sat)
                && (remind_sun == other.remind_sun)
                && (remind_starts == other.remind_starts)
                && (remind_ends == other.remind_ends)
                && (autotrack == other.autotrack)
                && (open_editor_on_shortcut == other.open_editor_on_shortcut)
                && (has_seen_beta_offering == other.has_seen_beta_offering)
                && (render_timeline == other.render_timeline));
    }

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
    bool remind_mon;
    bool remind_tue;
    bool remind_wed;
    bool remind_thu;
    bool remind_fri;
    bool remind_sat;
    bool remind_sun;
    std::string remind_starts;
    std::string remind_ends;
    bool autotrack;
    bool open_editor_on_shortcut;
    bool has_seen_beta_offering;
    bool render_timeline;

    // FIXME: will implement BaseModel here

    Json::Value SaveToJSON() const;
    std::string String() const;
};

}  // namespace toggl

#endif  // SRC_SETTINGS_H_
