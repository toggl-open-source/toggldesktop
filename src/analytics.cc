
// Copyright 2015 Toggl Desktop developers

// Collect Toggl Desktop usage using Google Analytics Measurement Protocol
// https://developers.google.com/analytics/devguides/collection/protocol/v1/
// or using Toggl's own backend when appropriate.

#include "analytics.h"

#include <sstream>

#include <json/json.h>  // NOLINT

#include "const.h"
#include "https_client.h"
#include "platforminfo.h"
#include "model/settings.h"
#include "urls.h"
#include "model/user.h"
#include "util/logger.h"

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

void Analytics::TrackInAppMessage(const std::string &client_id,
                                  const std::string &id,
                                  const Poco::Int64 type) {
    std::stringstream ss;
    ss << "message/" << id << "/";

    if (type == 0) {
        // in-app message shown on app start
        ss << "show/appstart";
    } else if (type == 1) {
        // in-app message shown with periodic trigger
        ss << "show/periodic";
    } else if (type == 2) {
        // in-app message close clicked
        ss << "click/close";
    } else {
        // in-app message cta clicked
        ss << "click/cta";
    }

    Track(client_id, "in-app-message", ss.str());
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
    HTTPRequest req;
    req.host = "https://ssl.google-analytics.com";
    req.relative_url = relativeURL();

    HTTPResponse resp = TogglClient::GetInstance().silentGet(req);
    if (resp.err != noError) {
        Logger("Analytics").error(resp.err);
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

    if (settings.pomodoro_break) {
        setActionInt("pomodoro_break_minutes-", settings.pomodoro_break_minutes);
        makeReq();
    }

    setActionBool("stop_entry_on_shutdown_sleep-", settings.stop_entry_on_shutdown_sleep);
    makeReq();

    setActionBool("show_touch_bar-", settings.show_touch_bar);
    makeReq();

    setActionBool("start_autotracker_without_suggestions", settings.start_autotracker_without_suggestions);
    makeReq();

    setActionBool("active_tab-", settings.active_tab);
    makeReq();

    setActionInt("color_theme-", settings.color_theme);
    makeReq();

    setActionBool("force_ignore_cert-", settings.force_ignore_cert);
    makeReq();
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
    HTTPRequest req;
    req.host = "https://ssl.google-analytics.com";
    req.relative_url = relativeURL();

    HTTPResponse resp = TogglClient::GetInstance().silentGet(req);
    if (resp.err != noError) {
        Logger("Analytics").error(resp.err);
        return;
    }
}

void Analytics::TrackStartTimeEntry(const std::string &client_id, const std::string& os, const uint8_t tab_index) {
    TrackTimeEntryActivity(client_id, os, "start", tab_index);
}

void Analytics::TrackEditTimeEntry(const std::string &client_id, const std::string& os, const uint8_t tab_index) {
    TrackTimeEntryActivity(client_id, os, "edit", tab_index);
}

void Analytics::TrackDeleteTimeEntry(const std::string &client_id, const std::string& os, const uint8_t tab_index) {
    TrackTimeEntryActivity(client_id, os, "delete", tab_index);
}

void Analytics::TrackLoginWithUsernamePassword(const std::string &client_id) {
    TrackUserAuthentication(client_id, "login", "username_password");
}

void Analytics::TrackLoginWithGoogle(const std::string &client_id) {
    TrackUserAuthentication(client_id, "login", "google");
}

void Analytics::TrackLoginWithApple(const std::string &client_id) {
    TrackUserAuthentication(client_id, "login", "apple");
}

void Analytics::TrackSignupWithUsernamePassword(const std::string &client_id) {
    TrackUserAuthentication(client_id, "signup", "username_password");
}

void Analytics::TrackSignupWithGoogle(const std::string &client_id) {
    TrackUserAuthentication(client_id, "signup", "google");
}

void Analytics::TrackSignupWithApple(const std::string &client_id) {
    TrackUserAuthentication(client_id, "signup", "apple");
}

void Analytics::TrackTimelineResizing(const std::string& client_id, const std::string& os) {
    std::stringstream action;
    action << "edit" << "/" << "resize";
    TrackTimeEntryActivity(client_id, os, action.str(), 1);
}

void Analytics::TrackTimeEntryActivity(const std::string &client_id,
                                       const std::string& os,
                                       const std::string &action,
                                       const uint8_t tab_index) {
    std::string active_view = tab_index == 0 ? "list-view" : "timeline-view";
    std::stringstream ss;
    ss << os << "/" << active_view << "/" << action;
    Track(client_id, active_view, ss.str());
}

void Analytics::TrackUserAuthentication(const std::string &client_id, const std::string &action, const std::string &from) {
    std::stringstream ss;
    ss << action << "/" << from;
    Track(client_id, "user", ss.str());
}

void Analytics::trackTimelineMenuContext(const std::string &client_id,
                                         const std::string &view) {
    std::stringstream ss;
    ss << "click" << "/" << view;
    Track(client_id, "timeline-menu-context", ss.str());
}

void Analytics::TrackTimelineMenuContext(const std::string &client_id, const TimelineMenuContextType type) {
    switch (type) {
        case TimelineMenuContextTypeContinueEntry:
            trackTimelineMenuContext(client_id, "continue-this-entry");
            break;
        case TimelineMenuContextTypeStartEntryFromEnd:
            trackTimelineMenuContext(client_id, "start-entry-from-the-end-of-this-entry");
            break;
        case TimelineMenuContextTypeDelete:
            trackTimelineMenuContext(client_id, "delete");
            break;
        case TimelineMenuContextTypeChangeFirstEntryStopTime:
            trackTimelineMenuContext(client_id, "change-first-entry-stop-time");
            break;
        case TimelineMenuContextTypeChangeLastEntryStartTime:
            trackTimelineMenuContext(client_id, "change-last-entry-start-time");
            break;
    }
}

void Analytics::TrackTimerEdit(const std::string &client_id, const TimerEditActionType action) {
    std::string category = "timer-edit";
    std::string action_str = timer_action_types.at(action);
    Track(client_id, category, action_str);
}

void Analytics::TrackTimerStart(const std::string &client_id, const TimerEditActionType actions) {
    std::string category = "timer-start";
    std::string action_str = "";

    for (auto const& action_pair : timer_action_types) {
        if (actions & action_pair.first) {
            if (!action_str.empty()) {
                action_str += "-";
            }
            action_str += action_pair.second;
        }
    }

    Track(client_id, category, action_str);
}

void Analytics::TrackTimerShortcut(const std::string &client_id, const TimerShortcutActionType action) {
    std::string category = "timer-shortcut";
    std::string action_str = "";

    switch (action) {
        case TimerShortcutActionTypeProjectSelected:
            action_str = "project-select";
            break;
        case TimerShortcutActionTypeTagSelected:
            action_str = "tag-select";
            break;
        case TimerShortcutActionTypeProjectCreated:
            action_str = "project-create";
            break;
        case TimerShortcutActionTypeTagCreated:
            action_str = "tag-create";
            break;
    }

    Track(client_id, category, action_str);
}

void Analytics::TrackDurationDropdown(const std::string &client_id, const DurationDropdownActionType action) {
    std::string category = "timer-duration-dropdown";
    std::string action_str = "";

    switch (action) {
        case DurationDropdownActionTypeStartTimeChange:
            action_str = "start-time-change";
            break;
        case DurationDropdownActionTypeDateChange:
            action_str = "date-change";
            break;
    }

    Track(client_id, category, action_str);
}

}  // namespace toggl
