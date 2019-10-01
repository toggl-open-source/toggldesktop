
// Copyright 2015 Toggl Desktop developers

// Collect Toggl Desktop usage using Google Analytics Measurement Protocol
// https://developers.google.com/analytics/devguides/collection/protocol/v1/
// or using Toggl's own backend when appropriate.

#include "../src/analytics.h"

#include <sstream>

#include <json/json.h>  // NOLINT

#include "./const.h"
#include "./https_client.h"
#include "./platforminfo.h"
#include "./settings.h"
#include "./urls.h"
#include "./user.h"

#include "Poco/Logger.h"

namespace toggl {


void Analytics::Track(const std::string &client_id,
                      const std::string &category,
                      const std::string &action) {
    start(new GoogleAnalyticsEvent(
        client_id, category, action, "", 1));
}

void Analytics::TrackChannel(const std::string &client_id,
                             const std::string &channel) {
    std::stringstream ss;
    ss << "channel-"
       << channel;

    Track(client_id, "channel", ss.str());
}

void Analytics::TrackOs(const std::string &client_id,
                        const std::string &os) {
    std::stringstream ss;
    ss << "os-"
       << os;

    Track(client_id, "os", ss.str());
}

void Analytics::TrackOSDetails(const std::string &client_id) {
    std::stringstream ss;

    RetrieveOsDetails(ss);

    Track(client_id, "osdetails", ss.str());
}

void Analytics::TrackWindowSize(const std::string &client_id,
                                const std::string &os,
                                const toggl::Rectangle rect) {
    TrackSize(client_id, os, "mainsize", rect);
}

void Analytics::TrackEditSize(const std::string &client_id,
                              const std::string &os,
                              const toggl::Rectangle rect) {
    TrackSize(client_id, os, "editsize", rect);
}

void Analytics::TrackSize(const std::string &client_id,
                          const std::string &os,
                          const std::string &name,
                          const toggl::Rectangle rect) {
    std::stringstream ss;
    ss << os << "/" << name << "-" << rect.str();

    Track(client_id, "stats", ss.str());
}

void Analytics::TrackSettings(const std::string &client_id,
                              const bool record_timeline,
                              const Settings &settings,
                              const bool use_proxy,
                              const Proxy &proxy) {
    Poco::LocalDateTime now;
    if (now.year() != settings_sync_date.year()
            || now.month() != settings_sync_date.month()
            || now.day() != settings_sync_date.day()) {
        settings_sync_date = Poco::LocalDateTime();
        start(new GoogleAnalyticsSettingsEvent(
            client_id, "settings", record_timeline,
            settings, use_proxy, proxy));
    }
}

void Analytics::TrackIdleDetectionClick(const std::string &client_id,
                                        const std::string &button) {
    std::stringstream ss;
    ss << "reminder/"
       << button;

    Track(client_id, "reminder", ss.str());
}

void Analytics::TrackAutocompleteUsage(const std::string &client_id,
                                       const bool was_using_autocomplete) {
    std::stringstream ss;
    ss << "timer/autocomplete-";
    if (was_using_autocomplete) {
        ss << "true";
    } else {
        ss << "false";
    }

    Track(client_id, "timer", ss.str());
}

const std::string GoogleAnalyticsEvent::relativeURL() {
    std::stringstream ss;
    ss << "/collect"
       << "?v=" << "1"
       << "&tid=" << "UA-3215787-27"
       << "&cid=" << client_id_
       << "&t=" << "event"
       << "&ec=" << category_
       << "&ea=" << action_
       << "&el=" << opt_label_
       << "&ev=" << opt_value_;
    return ss.str();
}

void GoogleAnalyticsEvent::runTask() {
    HTTPSRequest req;
    req.host = "https://ssl.google-analytics.com";
    req.relative_url = relativeURL();

    HTTPSClient client;
    HTTPSResponse resp = client.Get(req);
    if (resp.err != noError) {
        Poco::Logger::get("Analytics").error(resp.err);
        return;
    }
}

const std::string GoogleAnalyticsSettingsEvent::relativeURL() {
    std::stringstream ss;
    ss << "/collect"
       << "?v=" << "1"
       << "&tid=" << "UA-3215787-27"
       << "&cid=" << client_id_
       << "&t=" << "event"
       << "&ec=" << category_
       << "&ea=" << action_;
    return ss.str();
}

void GoogleAnalyticsSettingsEvent::runTask() {
    setActionBool("record_timeline-", record_timeline);
    makeReq();

    setActionBool("uses_proxy-", uses_proxy);
    makeReq();

    // Settings struct

    if (uses_proxy) {
        setActionBool("autodetect_proxy-", settings.autodetect_proxy);
        makeReq();
    }

    setActionBool("dock_icon-", settings.dock_icon);
    makeReq();

    setActionBool("menubar_timer-", settings.menubar_timer);
    makeReq();

    setActionBool("menubar_project-", settings.menubar_project);
    makeReq();

    setActionBool("on_top-", settings.on_top);
    makeReq();


    setActionBool("use_idle_detection-", settings.use_idle_detection);
    makeReq();

    if (settings.use_idle_detection) {
        setActionInt("idle_minutes-", settings.idle_minutes);
        makeReq();
    }

    setActionBool("focus_on_shortcut-", settings.focus_on_shortcut);
    makeReq();

    setActionBool("manual_mode-", settings.manual_mode);
    makeReq();

    setActionBool("autotrack-", settings.autotrack);
    makeReq();

    setActionBool("open_editor_on_shortcut-", settings.open_editor_on_shortcut);
    makeReq();

    setActionBool("reminder-", settings.reminder);
    makeReq();

    if (settings.reminder) {
        setActionBool("reminder_day_mon-", settings.remind_mon);
        makeReq();

        setActionBool("reminder_day_tue-", settings.remind_tue);
        makeReq();

        setActionBool("reminder_day_wed-", settings.remind_wed);
        makeReq();

        setActionBool("reminder_day_thu-", settings.remind_thu);
        makeReq();

        setActionBool("reminder_day_fri-", settings.remind_fri);
        makeReq();

        setActionBool("reminder_day_sat-", settings.remind_sat);
        makeReq();

        setActionBool("reminder_day_sun-", settings.remind_sun);
        makeReq();

        setActionInt("reminder_minutes-", settings.reminder_minutes);
        makeReq();

        setActionString("remind_starts-", settings.remind_starts);
        makeReq();

        setActionString("remind_ends-", settings.remind_ends);
        makeReq();
    }

    setActionBool("pomodoro-", settings.pomodoro);
    makeReq();

    if (settings.pomodoro) {
        setActionInt("pomodoro_minutes-", settings.pomodoro_minutes);
        makeReq();
    }

    setActionBool("pomodoro_break-", settings.pomodoro_break);
    makeReq();

    setActionBool("stop_entry_on_shutdown_sleep-", settings.stop_entry_on_shutdown_sleep);
    makeReq();

    if (settings.pomodoro_break) {
        setActionInt("pomodoro_break_minutes-",
                     settings.pomodoro_break_minutes);
        makeReq();
    }
}

void GoogleAnalyticsSettingsEvent::setActionBool(const std::string &type, bool value) {
    std::stringstream ss;
    ss << "settings/"
       << type
       << value;
    action_ = ss.str();
}

void GoogleAnalyticsSettingsEvent::setActionInt(const std::string &type,
        Poco::Int64 value) {
    std::stringstream ss;
    ss << "settings/"
       << type
       << value;
    action_ = ss.str();
}

void GoogleAnalyticsSettingsEvent::setActionString(const std::string &type,
        const std::string &value) {
    std::stringstream ss;
    ss << "settings/"
       << type
       << value;
    action_ = ss.str();
}

void GoogleAnalyticsSettingsEvent::makeReq() {
    HTTPSRequest req;
    req.host = "https://ssl.google-analytics.com";
    req.relative_url = relativeURL();

    HTTPSClient client;
    HTTPSResponse resp = client.Get(req);
    if (resp.err != noError) {
        Poco::Logger::get("Analytics").error(resp.err);
        return;
    }
}

}  // namespace toggl
