// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include <sstream>
#include <string>

#include <json/json.h>  // NOLINT

#include "model/base_model.h"

#include <Poco/Types.h>

namespace toggl {

class TOGGL_INTERNAL_EXPORT Settings : public BaseModel {
 public:
    Settings() {}

    Property<bool> use_idle_detection { false };
    Property<bool> menubar_timer { false };
    Property<bool> menubar_project { false };
    Property<bool> dock_icon { false };
    Property<bool> on_top { false };
    Property<bool> reminder { false };
    Property<Poco::Int64> idle_minutes { 0 };
    Property<bool> focus_on_shortcut { true };
    Property<Poco::Int64> reminder_minutes { 0 };
    Property<bool> manual_mode { false };
    Property<bool> autodetect_proxy { true };
    Property<bool> remind_mon { true };
    Property<bool> remind_tue { true };
    Property<bool> remind_wed { true };
    Property<bool> remind_thu { true };
    Property<bool> remind_fri { true };
    Property<bool> remind_sat { true };
    Property<bool> remind_sun { true };
    Property<std::string> remind_starts { "" };
    Property<std::string> remind_ends { "" };
    Property<bool> autotrack { false };
    Property<bool> open_editor_on_shortcut { false };
    Property<bool> has_seen_beta_offering { false };
    Property<bool> pomodoro { false };
    Property<bool> pomodoro_break { false };
    Property<Poco::Int64> pomodoro_minutes { 0 };
    Property<Poco::Int64> pomodoro_break_minutes { 0 };
    Property<bool> stop_entry_on_shutdown_sleep { false };
    Property<bool> show_touch_bar { true };
    Property<Poco::UInt8> active_tab { 0 };
    Property<Poco::UInt8> color_theme { 0 };

    bool IsSame(const Settings &other) const;

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    Json::Value SaveToJSON(int apiVersion = 8) const override;
};

}  // namespace toggl

#endif  // SRC_SETTINGS_H_
