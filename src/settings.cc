// Copyright 2014 Toggl Desktop developers.

#include "../src/settings.h"

#include "./formatter.h"

namespace toggl {

Json::Value Settings::SaveToJSON() const {
    Json::Value json;
    json["use_idle_detection"] = use_idle_detection;
    json["menubar_timer"] = menubar_timer;
    json["menubar_project"] = menubar_project;
    json["dock_icon"] = dock_icon;
    json["on_top"] = on_top;
    json["reminder"] = reminder;
    json["idle_minutes"] = Json::UInt64(idle_minutes);
    json["focus_on_shortcut"] = focus_on_shortcut;
    json["reminder_minutes"] = Json::UInt64(reminder_minutes);
    json["manual_mode"] = manual_mode;
    json["autodetect_proxy"] = autodetect_proxy;
    json["remind_mon"] = remind_mon;
    json["remind_tue"] = remind_tue;
    json["remind_wed"] = remind_wed;
    json["remind_thu"] = remind_thu;
    json["remind_fri"] = remind_fri;
    json["remind_sat"] = remind_sat;
    json["remind_sun"] = remind_sun;
    json["remind_starts"] = Formatter::EscapeJSONString(remind_starts);
    json["remind_ends"] = Formatter::EscapeJSONString(remind_ends);
    json["autotrack"] = autotrack;
    json["open_editor_on_shortcut"] = open_editor_on_shortcut;
    json["has_seen_beta_offering"] = has_seen_beta_offering;
    json["render_timeline"] = render_timeline;
    return json;
}

std::string Settings::String() const {
    std::stringstream ss;
    ss << "Settings"
       << " use_idle_detection=" << use_idle_detection
       << " menubar_timer=" << menubar_timer
       << " menubar_project=" << menubar_project
       << " dock_icon=" << dock_icon
       << " on_top=" << on_top
       << " reminder=" << reminder
       << " idle_minutes=" << idle_minutes
       << " focus_on_shortcut=" << focus_on_shortcut
       << " reminder_minutes=" << reminder_minutes
       << " manual_mode=" << manual_mode
       << " autodetect_proxy=" << autodetect_proxy
       << " remind_mon=" << remind_mon
       << " remind_tue=" << remind_tue
       << " remind_wed=" << remind_wed
       << " remind_thu=" << remind_thu
       << " remind_fri=" << remind_fri
       << " remind_sat=" << remind_sat
       << " remind_sun=" << remind_sun
       << " remind_starts=" << remind_starts
       << " remind_ends=" << remind_ends
       << " autotrack=" << autotrack
       << " open_editor_on_shortcut=" << open_editor_on_shortcut
       << " has_seen_beta_offering=" << has_seen_beta_offering
       << " render_timeline=" << render_timeline;
    return ss.str();
}

}   // namespace toggl
