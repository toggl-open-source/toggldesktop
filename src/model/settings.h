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
    Settings(ProtectedBase *container)
        : BaseModel(container)
    {}
 public:
    friend class ProtectedBase;

    virtual ~Settings() {}

    bool use_idle_detection { false };
    bool menubar_timer { false };
    bool menubar_project { false };
    bool dock_icon { false };
    bool on_top { false };
    bool reminder { false };
    Poco::Int64 idle_minutes { 0};
    bool focus_on_shortcut { true };
    Poco::Int64 reminder_minutes { 0 };
    bool manual_mode { false };
    bool autodetect_proxy { true };
    bool remind_mon { true };
    bool remind_tue { true };
    bool remind_wed { true };
    bool remind_thu { true };
    bool remind_fri { true };
    bool remind_sat { true };
    bool remind_sun { true };
    std::string remind_starts { "" };
    std::string remind_ends { "" };
    bool autotrack { false };
    bool open_editor_on_shortcut { false };
    bool has_seen_beta_offering { false };
    bool pomodoro { false };
    bool pomodoro_break { false };
    Poco::Int64 pomodoro_minutes { 0 };
    Poco::Int64 pomodoro_break_minutes { 0 };
    bool stop_entry_on_shutdown_sleep { false };
    bool show_touch_bar { true };
    Poco::UInt8 active_tab { 0 };

    bool IsSame(locked<Settings> &other) const;

    // Override BaseModel

    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    Json::Value SaveToJSON() const override;
};

}  // namespace toggl

#endif  // SRC_SETTINGS_H_
