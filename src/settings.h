// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include <sstream>
#include <string>

#include <json/json.h>  // NOLINT

#include "base_model.h"

#include <Poco/Types.h>

namespace toggl {

class TOGGL_INTERNAL_EXPORT Settings : public BaseModel {
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
    , pomodoro(false)
    , pomodoro_break(false)
    , pomodoro_minutes(0)
    , pomodoro_break_minutes(0)
    , stop_entry_on_shutdown_sleep(false)
    , show_touch_bar(true)
    , start_autotracker_without_suggestions(false)
    , active_tab(0)
    , color_theme(0) {}

    virtual ~Settings() {}

    bool use_idle_detection;
    bool menubar_timer;
    bool menubar_project;
    bool dock_icon;
    bool on_top;
    bool reminder;
    Poco::Int64 idle_minutes;
    bool focus_on_shortcut;
    Poco::Int64 reminder_minutes;
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
    bool pomodoro;
    bool pomodoro_break;
    Poco::Int64 pomodoro_minutes;
    Poco::Int64 pomodoro_break_minutes;
    bool stop_entry_on_shutdown_sleep;
    bool show_touch_bar;
    bool start_autotracker_without_suggestions;
    Poco::UInt8 active_tab;
    Poco::UInt8 color_theme;

    bool IsSame(const Settings &other) const;

    // Override BaseModel

    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    Json::Value SaveToJSON() const override;
};

}  // namespace toggl

#endif  // SRC_SETTINGS_H_
