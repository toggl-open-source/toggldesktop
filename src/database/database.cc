// Copyright 2014 Toggl Desktop developers.

// All session access should be locked.

#include "database.h"

#include <limits>
#include <string>
#include <vector>

#include "model/autotracker.h"
#include "model/client.h"
#include "model/obm_action.h"
#include "model/project.h"
#include "model/settings.h"
#include "model/tag.h"
#include "model/task.h"
#include "model/time_entry.h"
#include "model/user.h"
#include "model/workspace.h"
#include "const.h"
#include "migrations.h"
#include "proxy.h"

#include <Poco/Data/Binding.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionImpl.h>
#include <Poco/Data/SQLite/SessionImpl.h>
#include <Poco/Data/SQLite/Utility.h>
#include <Poco/Data/Statement.h>
#include <Poco/FileStream.h>
#include <Poco/Stopwatch.h>
#include <Poco/StreamCopier.h>
#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>

namespace toggl {

using Poco::Data::Keywords::useRef;
using Poco::Data::Keywords::limit;
using Poco::Data::Keywords::into;
using Poco::Data::Keywords::now;

Database::Database(const std::string &db_path)
    : session_(nullptr)
, desktop_id_("")
, analytics_client_id_("") {
    Poco::Data::SQLite::Connector::registerConnector();

    session_ = new Poco::Data::Session("SQLite", db_path);

    {
        int is_sqlite_threadsafe = Poco::Data::SQLite::Utility::isThreadSafe();

        logger.debug("sqlite3_threadsafe()=", is_sqlite_threadsafe);

        if (!is_sqlite_threadsafe) {
            logger.error("Database is not thread safe!");
            return;
        }
    }

    error err = setJournalMode("wal");
    if (err != noError) {
        logger.error("Failed to set journal mode to wal!");
        return;
    }

    std::string mode("");
    err = journalMode(&mode);
    if (err != noError) {
        logger.error("Could not detect journal mode!");
        return;
    }

    logger.debug("PRAGMA journal_mode=", mode);

    if ("wal" != mode) {
        logger.error("Failed to enable wal journal mode!");
        return;
    }

    // Remove Time Entries older than 30 days from local db
    Poco::LocalDateTime today;

    Poco::LocalDateTime start =
        today - Poco::Timespan(30 * Poco::Timespan::DAYS);

    err = deleteAllFromTableByDate(
        "time_entries", start.timestamp());

    if (err != noError) {
        logger.error("failed to clean Up Time Entries Data: " + err);
        // but will continue, its not vital
    }

    // Remove Synced Timeline Events older than 9 days from local db
    Poco::LocalDateTime timeline_start =
        today - Poco::Timespan(9 * Poco::Timespan::DAYS);

    err = deleteAllSyncedTimelineEventsByDate(
        timeline_start.timestamp());

    if (err != noError) {
        logger.error("failed to clean Up Timeline Events Data: " + err);
        // but will continue, its not vital
    }

    err = vacuum();
    if (err != noError) {
        logger.error("failed to vacuum: " + err);
        // but will continue, its not vital
    }

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    err = initialize_tables();
    if (err != noError) {
        logger.error(err);
        // We're doomed now; cannot continue without a DB
        throw(err);
    }

    stopwatch.stop();

    logger.debug("Migrated in ", stopwatch.elapsed() / 1000, " ms");
}

Database::~Database() {
    if (session_) {
        delete session_;
        session_ = nullptr;
    }
    Poco::Data::SQLite::Connector::unregisterConnector();
}

error Database::DeleteUser(
    locked<User> &model,
    const bool with_related_data) {

    poco_check_ptr(model);

    error err = DeleteFromTable("sessions", model->LocalID());
    if (err != noError) {
        return err;
    }

    err = DeleteFromTable("users", model->LocalID());
    if (err != noError) {
        return err;
    }
    if (with_related_data) {
        err = deleteAllFromTableByUID("workspaces", model->ID());
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("clients", model->ID());
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("projects", model->ID());
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("tasks", model->ID());
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("tags", model->ID());
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("time_entries", model->ID());
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("autotracker_settings", model->ID());
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("obm_actions", model->ID());
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("timeline_events", model->ID());
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("obm_experiments", model->ID());
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

error Database::deleteAllFromTableByUID(
    const std::string &table_name,
    const Poco::UInt64 &UID) {


    if (!UID) {
        return error("Cannot delete user data without user ID");
    }
    if (table_name.empty()) {
        return error("Cannot delete from table without table name");
    }

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ <<
                  "delete from " + table_name + " where uid = :uid",
                  useRef(UID),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("deleteAllFromTableByUID");
}

error Database::deleteAllFromTableByDate(
    const std::string &table_name,
    const Poco::Timestamp &time) {

    if (table_name.empty()) {
        return error("Cannot delete from table without table name");
    }

    const Poco::Int64 stopTime = time.epochTime();

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ <<
                  "delete from " + table_name + " where "
                  "id NOT NULL and stop < :stop",
                  useRef(stopTime),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("deleteAllFromTableByDate");
}

error Database::deleteAllSyncedTimelineEventsByDate(
    const Poco::Timestamp &time) {
    const Poco::Int64 endTime = time.epochTime();

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ <<
                  "delete from timeline_events where "
                  "uploaded = 1 AND end_time < :end_time",
                  useRef(endTime),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("deleteAllSyncedTimelineEventsByDate");
}


error Database::journalMode(std::string *mode) {
    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);
        poco_check_ptr(mode);

        *session_ <<
                  "PRAGMA journal_mode",
                  into(*mode),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("journalMode");
}

error Database::setJournalMode(const std::string &mode) {
    if (mode.empty()) {
        return error("Cannot set journal mode without a mode");
    }


    try {
        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        *session_ <<
                  "PRAGMA journal_mode=" << mode,
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("setJournalMode");
}

error Database::vacuum() {
    try {
        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        *session_ <<
                  "VACUUM;" << now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("vacuum");
}

error Database::DeleteFromTable(
    const std::string &table_name,
    const Poco::Int64 &local_id) {

    if (table_name.empty()) {
        return error("Cannot delete from table without table name");
    }

    if (!local_id) {
        return noError;
    }

    logger.debug( "Deleting from table ", table_name, ", local ID: ", local_id);

    try {
        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        *session_ <<
                  "delete from " + table_name +
                  " where local_id = :local_id",
                  useRef(local_id),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("DeleteFromTable");
}

error Database::last_error(const std::string &was_doing) {
    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    std::string last = Poco::Data::SQLite::Utility::lastError(*session_);
    if (last != "not an error" && last != "unknown error") {
        return error(was_doing + ": " + last);
    }
    return noError;
}

std::string Database::GenerateGUID() {
    Poco::UUIDGenerator& generator = Poco::UUIDGenerator::defaultGenerator();
    Poco::UUID uuid(generator.createRandom());
    return uuid.toString();
}

error Database::LoadCurrentUser(locked<User> &user) {
    poco_check_ptr(user);

    logger.debug("LoadCurrentUser");

    std::string api_token("");
    Poco::UInt64 uid(0);
    error err = CurrentAPIToken(&api_token, &uid);
    if (err != noError) {
        return err;
    }
    if (api_token.empty()) {
        return noError;
    }
    if (!uid) {
        return noError;
    }
    user->SetAPIToken(api_token);
    return LoadUserByID(uid, user);
}

error Database::LoadSettings(locked<Settings> &settings) {
    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ <<
                  "select use_idle_detection, menubar_timer, "
                  "menubar_project, dock_icon, on_top, reminder,  "
                  "idle_minutes, focus_on_shortcut, reminder_minutes, "
                  "manual_mode, autodetect_proxy, "
                  "remind_starts, remind_ends, "
                  "remind_mon, remind_tue, remind_wed, remind_thu, "
                  "remind_fri, remind_sat, remind_sun, autotrack, "
                  "open_editor_on_shortcut, has_seen_beta_offering, "
                  "pomodoro, pomodoro_minutes, "
                  "pomodoro_break, pomodoro_break_minutes, stop_entry_on_shutdown_sleep, show_touch_bar, active_tab "
                  "from settings "
                  "limit 1",
                  into(settings->use_idle_detection),
                  into(settings->menubar_timer),
                  into(settings->menubar_project),
                  into(settings->dock_icon),
                  into(settings->on_top),
                  into(settings->reminder),
                  into(settings->idle_minutes),
                  into(settings->focus_on_shortcut),
                  into(settings->reminder_minutes),
                  into(settings->manual_mode),
                  into(settings->autodetect_proxy),
                  into(settings->remind_starts),
                  into(settings->remind_ends),
                  into(settings->remind_mon),
                  into(settings->remind_tue),
                  into(settings->remind_wed),
                  into(settings->remind_thu),
                  into(settings->remind_fri),
                  into(settings->remind_sat),
                  into(settings->remind_sun),
                  into(settings->autotrack),
                  into(settings->open_editor_on_shortcut),
                  into(settings->has_seen_beta_offering),
                  into(settings->pomodoro),
                  into(settings->pomodoro_minutes),
                  into(settings->pomodoro_break),
                  into(settings->pomodoro_break_minutes),
                  into(settings->stop_entry_on_shutdown_sleep),
                  into(settings->show_touch_bar),
                  into(settings->active_tab),
                  limit(1),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("LoadSettings");
}

error Database::SaveWindowSettings(
    const Poco::Int64 window_x,
    const Poco::Int64 window_y,
    const Poco::Int64 window_height,
    const Poco::Int64 window_width) {

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ <<
                  "update settings set "
                  "window_x = :window_x, "
                  "window_y = :window_y, "
                  "window_height = :window_height, "
                  "window_width = :window_width ",
                  useRef(window_x),
                  useRef(window_y),
                  useRef(window_height),
                  useRef(window_width),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }

    return last_error("SaveWindowSettings");
}

error Database::SetMiniTimerX(const Poco::Int64 x) {
    return setSettingsValue("mini_timer_x", x);
}

error Database::GetMiniTimerX(Poco::Int64* x) {
    return getSettingsValue("mini_timer_x", x);
}

error Database::SetMiniTimerY(const Poco::Int64 y) {
    return setSettingsValue("mini_timer_y", y);
}

error Database::GetMiniTimerY(Poco::Int64* y) {
    return getSettingsValue("mini_timer_y", y);
}

error Database::SetMiniTimerW(const Poco::Int64 w) {
    return setSettingsValue("mini_timer_w", w);
}

error Database::GetMiniTimerW(Poco::Int64* w) {
    return getSettingsValue("mini_timer_w", w);
}

error Database::LoadWindowSettings(
    Poco::Int64 *window_x,
    Poco::Int64 *window_y,
    Poco::Int64 *window_height,
    Poco::Int64 *window_width) {

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        Poco::Int64 x(0), y(0), height(0), width(0);

        *session_ <<
                  "select window_x, window_y, window_height, window_width "
                  "from settings limit 1",
                  into(x),
                  into(y),
                  into(height),
                  into(width),
                  limit(1),
                  now;

        *window_x = x;
        *window_y = y;
        *window_height = height;
        *window_width = width;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("LoadWindowSettings");
}

error Database::LoadProxySettings(
    bool *use_proxy,
    Proxy *proxy) {

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);
        poco_check_ptr(use_proxy);
        poco_check_ptr(proxy);

        std::string host(""), username(""), password("");
        Poco::UInt64 port(0);
        *session_ <<
                  "select use_proxy, proxy_host, proxy_port, "
                  "proxy_username, proxy_password "
                  "from settings limit 1",
                  into(*use_proxy),
                  into(host),
                  into(port),
                  into(username),
                  into(password),
                  limit(1),
                  now;
        proxy->SetHost(host);
        proxy->SetPort(port);
        proxy->SetUsername(username);
        proxy->SetPassword(password);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("LoadProxySettings");
}

error Database::SetMiniTimerVisible(
    const bool value) {
    return setSettingsValue("mini_timer_visible", value);
}

error Database::GetMiniTimerVisible(bool* result) {
    return getSettingsValue("mini_timer_visible", result);
}

error Database::SetKeepEndTimeFixed(
    const bool value) {
    return setSettingsValue("keep_end_time_fixed", value);
}

error Database::GetKeepEndTimeFixed(bool *result) {
    return getSettingsValue("keep_end_time_fixed", result);
}

error Database::GetShowTouchBar(bool *result) {
    return getSettingsValue("show_touch_bar", result);
}

error Database::GetActiveTab(uint8_t *result) {
    return getSettingsValue("active_tab", result);
}

error Database::SetWindowMaximized(
    const bool value) {
    return setSettingsValue("window_maximized", value);
}

error Database::GetWindowMaximized(bool *result) {
    return getSettingsValue("window_maximized", result);
}

error Database::SetWindowMinimized(
    const bool value) {
    return setSettingsValue("window_minimized", value);
}

error Database::GetWindowMinimized(bool *result) {
    return getSettingsValue("window_minimized", result);
}

error Database::SetWindowEditSizeHeight(
    const Poco::Int64 value) {
    return setSettingsValue("window_edit_size_height", value);
}

error Database::GetWindowEditSizeHeight(Poco::Int64 *result) {
    return getSettingsValue("window_edit_size_height", result);
}

error Database::SetWindowEditSizeWidth(
    const Poco::Int64 value) {
    return setSettingsValue("window_edit_size_width", value);
}

error Database::GetWindowEditSizeWidth(Poco::Int64 *result) {
    return getSettingsValue("window_edit_size_width", result);
}

error Database::SetKeyStart(
    const std::string &value) {
    return setSettingsValue("key_start", value);
}

error Database::GetKeyStart(std::string *result) {
    return getSettingsValue("key_start", result);
}

error Database::SetKeyShow(
    const std::string &value) {
    return setSettingsValue("key_show", value);
}

error Database::GetKeyShow(std::string *result) {
    return getSettingsValue("key_show", result);
}

error Database::SetKeyModifierShow(
    const std::string &value) {
    return setSettingsValue("key_modifier_show", value);
}

error Database::GetKeyModifierShow(std::string *result) {
    return getSettingsValue("key_modifier_show", result);
}

error Database::SetKeyModifierStart(
    const std::string &value) {
    return setSettingsValue("key_modifier_start", value);
}

error Database::GetKeyModifierStart(std::string *result) {
    return getSettingsValue("key_modifier_start", result);
}

error Database::GetMessageSeen(Poco::Int64 *result) {
    return getSettingsValue("message_seen", result);
}

error Database::SetSettingsRemindTimes(
    const std::string &remind_starts,
    const std::string &remind_ends) {

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ <<
                  "update settings set "
                  "remind_starts = :remind_starts, "
                  "remind_ends = :remind_ends ",
                  useRef(remind_starts),
                  useRef(remind_ends),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }

    return last_error("SetSettingsRemindTimes");
}

error Database::SetSettingsRemindDays(
    const bool &remind_mon,
    const bool &remind_tue,
    const bool &remind_wed,
    const bool &remind_thu,
    const bool &remind_fri,
    const bool &remind_sat,
    const bool &remind_sun) {

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ <<
                  "update settings set "
                  "remind_mon = :remind_mon, "
                  "remind_tue = :remind_tue, "
                  "remind_wed = :remind_wed, "
                  "remind_thu = :remind_thu, "
                  "remind_fri = :remind_fri, "
                  "remind_sat = :remind_sat, "
                  "remind_sun = :remind_sun ",
                  useRef(remind_mon),
                  useRef(remind_tue),
                  useRef(remind_wed),
                  useRef(remind_thu),
                  useRef(remind_fri),
                  useRef(remind_sat),
                  useRef(remind_sun),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }

    return last_error("SetSettingsRemindDays");
}

error Database::SetSettingsHasSeenBetaOffering(const bool &value) {
    return setSettingsValue("has_seen_beta_offering", value);
}

error Database::SetSettingsMessageSeen(
    const Poco::UInt64 message_id) {
    return setSettingsValue("message_seen",message_id);
}

error Database::SetSettingsUseIdleDetection(
    const bool &use_idle_detection) {
    return setSettingsValue("use_idle_detection", use_idle_detection);
}

error Database::SetSettingsAutotrack(const bool &value) {
    return setSettingsValue("autotrack", value);
}

error Database::SetSettingsOpenEditorOnShortcut(const bool &value) {
    return setSettingsValue("open_editor_on_shortcut", value);
}

error Database::SetSettingsMenubarTimer(
    const bool &menubar_timer) {
    return setSettingsValue("menubar_timer", menubar_timer);
}

error Database::SetSettingsMenubarProject(
    const bool &menubar_project) {
    return setSettingsValue("menubar_project", menubar_project);
}

error Database::SetSettingsDockIcon(const bool &dock_icon) {
    return setSettingsValue("dock_icon", dock_icon);
}

error Database::SetSettingsOnTop(const bool &on_top) {
    return setSettingsValue("on_top", on_top);
}

error Database::SetSettingsReminder(const bool &reminder) {
    return setSettingsValue("reminder", reminder);
}

error Database::SetSettingsPomodoro(const bool &pomodoro) {
    return setSettingsValue("pomodoro", pomodoro);
}

error Database::SetSettingsPomodoroBreak(const bool &pomodoro_break) {
    return setSettingsValue("pomodoro_break", pomodoro_break);
}

error Database::SetSettingsStopEntryOnShutdownSleep(const bool &stop_entry) {
    return setSettingsValue("stop_entry_on_shutdown_sleep", stop_entry);
}

error Database::SetSettingsIdleMinutes(const Poco::UInt64 idle_minutes) {
    Poco::UInt64 new_value = idle_minutes;
    if (new_value < 1) {
        new_value = 1;
    }
    return setSettingsValue("idle_minutes", new_value);
}

error Database::SetSettingsFocusOnShortcut(const bool &focus_on_shortcut) {
    return setSettingsValue("focus_on_shortcut", focus_on_shortcut);
}

error Database::SetSettingsManualMode(const bool &manual_mode) {
    return setSettingsValue("manual_mode", manual_mode);
}

error Database::SetSettingsAutodetectProxy(const bool &autodetect_proxy) {
    return setSettingsValue("autodetect_proxy", autodetect_proxy);
}

error Database::SetSettingsShowTouchBar(const bool &show_touch_bar) {
    return setSettingsValue("show_touch_bar", show_touch_bar);
}

error Database::SetSettingsActiveTab(const uint8_t &active_tab) {
    return setSettingsValue("active_tab", active_tab);
}

template<typename T>
error Database::setSettingsValue(
    const std::string &field_name,
    const T &value) {

    try {
        poco_check_ptr(session_);

        Poco::Mutex::ScopedLock lock(session_m_);

        *session_ <<
                  "update settings set " + field_name + " = :" + field_name,
                  useRef(value),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("setSettingsValue");
}

template<typename T>
error Database::getSettingsValue(
    const std::string &field_name,
    T *value) {

    try {
        poco_check_ptr(session_);
        poco_check_ptr(value);

        Poco::Mutex::ScopedLock lock(session_m_);

        *session_ <<
                  "select " + field_name + " from settings limit 1",
                  into(*value),
                  limit(1),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("getSettingsValue");
}

error Database::SetSettingsReminderMinutes(
    const Poco::UInt64 reminder_minutes) {
    Poco::UInt64 new_value = reminder_minutes;
    if (new_value < 1) {
        new_value = 1;
    }
    return setSettingsValue("reminder_minutes", new_value);
}

error Database::SetSettingsPomodoroMinutes(
    const Poco::UInt64 pomodoro_minutes) {
    Poco::UInt64 new_value = pomodoro_minutes;
    if (new_value < 1) {
        new_value = 1;
    }
    return setSettingsValue("pomodoro_minutes", new_value);
}

error Database::SetSettingsPomodoroBreakMinutes(
    const Poco::UInt64 pomodoro_break_minutes) {
    Poco::UInt64 new_value = pomodoro_break_minutes;
    if (new_value < 1) {
        new_value = 1;
    }
    return setSettingsValue("pomodoro_break_minutes", new_value);
}

error Database::SaveProxySettings(
    const bool &use_proxy,
    const Proxy &proxy) {

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ <<
                  "update settings set "
                  "use_proxy = :use_proxy, "
                  "proxy_host = :proxy_host, "
                  "proxy_port = :proxy_port, "
                  "proxy_username = :proxy_username, "
                  "proxy_password = :proxy_password ",
                  useRef(use_proxy),
                  useRef(proxy.Host()),
                  useRef(proxy.Port()),
                  useRef(proxy.Username()),
                  useRef(proxy.Password()),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("SaveProxySettings");
}

error Database::Trim(const std::string &text, std::string *result) {
    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);
        poco_check_ptr(result);

        *result = "";

        *session_ <<
                  "select trim(:text) limit 1",
                  into(*result),
                  useRef(text),
                  limit(1),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("Trim");
}

error Database::ResetWindow() {
    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ <<
                  "update settings set "
                  "window_x = 0, "
                  "window_y = 0, "
                  "window_height = 0, "
                  "window_width = 0, "
                  "window_maximized = 0, "
                  "window_minimized = 0, "
                  "window_edit_size_height = 0, "
                  "window_edit_size_width = 0, "
                  "mini_timer_x = 0, "
                  "mini_timer_y = 0, "
                  "mini_timer_w = 0",
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("ResetWindow");
}

error Database::LoadUpdateChannel(
    std::string *update_channel) {

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);
        poco_check_ptr(update_channel);

        *session_ <<
                  "select update_channel from settings limit 1",
                  into(*update_channel),
                  limit(1),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("LoadUpdateChannel");
}

error Database::SaveUpdateChannel(
    const std::string &update_channel) {

    if (update_channel != "stable" &&
            update_channel != "beta" &&
            update_channel != "dev") {
        return error("Invalid update channel");
    }

    return setSettingsValue("update_channel", update_channel);
}

error Database::LoadUserByEmail(
    const std::string &email,
    locked<User> &model) {

    if (email.empty()) {
        return error("Cannot load user by email token without an email");
    }

    Poco::UInt64 uid(0);

    try {
        poco_check_ptr(model);

        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        model->SetEmail(email);

        *session_ <<
                  "select id from users"
                  " where email = :email"
                  " limit 1",
                  into(uid),
                  useRef(email),
                  limit(1),
                  now;
        error err = last_error("LoadUserByEmail");
        if (err != noError) {
            return err;
        }
        if (uid <= 0) {
            return noError;
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return LoadUserByID(uid, model);
}

error Database::loadUsersRelatedData(locked<User> &user) {
    error err = loadModels(user->ID(), user->GetRelatedData()->Workspaces);
    if (err != noError) {
        return err;
    }

    err = loadModels(user->ID(), user->GetRelatedData()->Clients);
    if (err != noError) {
        return err;
    }

    err = loadModels(user->ID(), user->GetRelatedData()->Projects);
    if (err != noError) {
        return err;
    }

    err = loadModels(user->ID(), user->GetRelatedData()->Tasks);
    if (err != noError) {
        return err;
    }

    err = loadModels(user->ID(), user->GetRelatedData()->Tags);
    if (err != noError) {
        return err;
    }

    err = loadModels(user->ID(), user->GetRelatedData()->TimeEntries);
    if (err != noError) {
        return err;
    }

    err = loadModels(user->ID(), user->GetRelatedData()->AutotrackerRules);
    if (err != noError) {
        return err;
    }

    err = loadModels(user->ID(), user->GetRelatedData()->TimelineEvents);
    if (err != noError) {
        return err;
    }

    err = loadModels(user->ID(), user->GetRelatedData()->ObmActions);
    if (err != noError) {
        return err;
    }

    err = loadModels(user->ID(), user->GetRelatedData()->ObmExperiments);
    if (err != noError) {
        return err;
    }

    return noError;
}

error Database::LoadUserByID(
    const Poco::UInt64 &UID,
    locked<User> &user) {

    if (!UID) {
        return error("Cannot load user by ID without an ID");
    }

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    try {
        poco_check_ptr(user);

        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        Poco::Int64 local_id(0);
        Poco::UInt64 id(0);
        Poco::UInt64 default_wid(0);
        Poco::Int64 since(0);
        std::string fullname("");
        std::string email("");
        bool record_timeline(false);
        bool store_start_and_stop_time(false);
        std::string timeofday_format("");
        std::string duration_format("");
        std::string offline_data("");
        Poco::UInt64 default_pid(0);
        Poco::UInt64 default_tid(0);
        bool collapse_entries(false);
        *session_ <<
                  "select local_id, id, default_wid, since, "
                  "fullname, "
                  "email, record_timeline, store_start_and_stop_time, "
                  "timeofday_format, duration_format, offline_data, "
                  "default_pid, default_tid, collapse_entries "
                  "from users where id = :id limit 1",
                  into(local_id),
                  into(id),
                  into(default_wid),
                  into(since),
                  into(fullname),
                  into(email),
                  into(record_timeline),
                  into(store_start_and_stop_time),
                  into(timeofday_format),
                  into(duration_format),
                  into(offline_data),
                  into(default_pid),
                  into(default_tid),
                  into(collapse_entries),
                  useRef(UID),
                  limit(1),
                  now;

        error err = last_error("LoadUserByID");
        if (err != noError) {
            return err;
        }

        if (!id) {
            // No user data found
            return noError;
        }

        user->SetLocalID(local_id);
        user->SetID(id);
        user->SetDefaultWID(default_wid);
        user->SetSince(since);
        user->SetFullname(fullname);
        user->SetEmail(email);
        user->SetRecordTimeline(record_timeline);
        user->SetStoreStartAndStopTime(store_start_and_stop_time);
        user->SetTimeOfDayFormat(timeofday_format);
        user->SetDurationFormat(duration_format);
        user->SetOfflineData(offline_data);
        user->SetDefaultPID(default_pid);
        user->SetDefaultTID(default_tid);
        user->SetCollapseEntries(collapse_entries);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    error err = loadUsersRelatedData(user);
    if (err != noError) {
        return err;
    }

    stopwatch.stop();
    logger.debug("User loaded in ", stopwatch.elapsed() / 1000, " ms");

    return noError;
}

template <typename T>
error Database::saveRelatedModels(const Poco::UInt64 UID,
    const std::string &table_name,
    ProtectedContainer<T> &list,
    std::vector<ModelChange> *changes) {

    if (!UID) {
        return error("Cannot save user related data without an user ID");
    }

    poco_check_ptr(changes);

    for (auto model : list) {
        if (model->IsMarkedAsDeletedOnServer()) {
            error err = DeleteFromTable(table_name, model->LocalID());
            if (err != noError) {
                return err;
            }
            changes->push_back(ModelChange(
                model->ModelName(),
                kChangeTypeDelete,
                model->ID(),
                model->GUID()));
            continue;
        }
        model->SetUID(UID);
        error err = saveModel(model, changes);
        if (err != noError) {
            return err;
        }
    }

    // Purge deleted models from memory
    auto it = list.begin();
    while (it != list.end()) {
        auto model = *it;
        if (model->IsMarkedAsDeletedOnServer()) {
            it = list.erase(it);
        } else {
            ++it;
        }
    }

    return noError;
}

typedef toggl::error (Database::*saveModel)(
    BaseModel *model, std::vector<ModelChange> *changes);

error Database::saveModel(
    locked<TimeEntry> &model,
    std::vector<ModelChange> *changes) {

    try {
        poco_check_ptr(model);
        poco_check_ptr(changes);

        // Time entries need to have a GUID,
        // we expect it everywhere in the UI
        model->EnsureGUID();
        if (model->GUID().empty()) {
            return error("Cannot save time entry without a GUID");
        }

        if (!model->NeedsToBeSaved()) {
            return noError;
        }

        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        if (model->LocalID()) {
            logger.debug("Updating time entry ", model->String(), " in thread ", Poco::Thread::currentTid());

            if (model->ID()) {
                *session_ <<
                          "update time_entries set "
                          "id = :id, uid = :uid, description = :description, "
                          "wid = :wid, guid = :guid, pid = :pid, tid = :tid, "
                          "billable = :billable, "
                          "duronly = :duronly, "
                          "ui_modified_at = :ui_modified_at, "
                          "start = :start, stop = :stop, duration = :duration, "
                          "tags = :tags, created_with = :created_with, "
                          "deleted_at = :deleted_at, "
                          "updated_at = :updated_at, "
                          "project_guid = :project_guid, "
                          "validation_error = :validation_error "
                          "where local_id = :local_id",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Description()),
                          useRef(model->WID()),
                          useRef(model->GUID()),
                          useRef(model->PID()),
                          useRef(model->TID()),
                          useRef(model->Billable()),
                          useRef(model->DurOnly()),
                          useRef(model->UIModifiedAt()),
                          useRef(model->Start()),
                          useRef(model->Stop()),
                          useRef(model->DurationInSeconds()),
                          useRef(model->Tags()),
                          useRef(model->CreatedWith()),
                          useRef(model->DeletedAt()),
                          useRef(model->UpdatedAt()),
                          useRef(model->ProjectGUID()),
                          useRef(model->ValidationError()),
                          useRef(model->LocalID()),
                          now;
            } else {
                *session_ <<
                          "update time_entries set "
                          "uid = :uid, description = :description, wid = :wid, "
                          "guid = :guid, pid = :pid, tid = :tid, "
                          "billable = :billable, "
                          "duronly = :duronly, "
                          "ui_modified_at = :ui_modified_at, "
                          "start = :start, stop = :stop, duration = :duration, "
                          "tags = :tags, created_with = :created_with, "
                          "deleted_at = :deleted_at, "
                          "updated_at = :updated_at, "
                          "project_guid = :project_guid, "
                          "validation_error = :validation_error "
                          "where local_id = :local_id",
                          useRef(model->UID()),
                          useRef(model->Description()),
                          useRef(model->WID()),
                          useRef(model->GUID()),
                          useRef(model->PID()),
                          useRef(model->TID()),
                          useRef(model->Billable()),
                          useRef(model->DurOnly()),
                          useRef(model->UIModifiedAt()),
                          useRef(model->Start()),
                          useRef(model->Stop()),
                          useRef(model->DurationInSeconds()),
                          useRef(model->Tags()),
                          useRef(model->CreatedWith()),
                          useRef(model->DeletedAt()),
                          useRef(model->UpdatedAt()),
                          useRef(model->ProjectGUID()),
                          useRef(model->ValidationError()),
                          useRef(model->LocalID()),
                          now;
            }
            error err = last_error("saveTimeEntry");
            if (err != noError) {
                return err;
            }
            if (model->DeletedAt()) {
                changes->push_back(ModelChange(
                    model->ModelName(),
                    kChangeTypeDelete,
                    model->ID(),
                    model->GUID()));
            } else {
                changes->push_back(ModelChange(
                    model->ModelName(),
                    kChangeTypeUpdate,
                    model->ID(),
                    model->GUID()));
            }
        } else {
            logger.debug("Inserting time entry ", model->String(), " in thread ", Poco::Thread::currentTid());
            if (model->ID()) {
                *session_ <<
                          "insert into time_entries(id, uid, description, "
                          "wid, guid, pid, tid, billable, "
                          "duronly, ui_modified_at, "
                          "start, stop, duration, "
                          "tags, created_with, deleted_at, updated_at, "
                          "project_guid, validation_error) "
                          "values(:id, :uid, :description, :wid, "
                          ":guid, :pid, :tid, :billable, "
                          ":duronly, :ui_modified_at, "
                          ":start, :stop, :duration, "
                          ":tags, :created_with, :deleted_at, :updated_at, "
                          ":project_guid, :validation_error)",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Description()),
                          useRef(model->WID()),
                          useRef(model->GUID()),
                          useRef(model->PID()),
                          useRef(model->TID()),
                          useRef(model->Billable()),
                          useRef(model->DurOnly()),
                          useRef(model->UIModifiedAt()),
                          useRef(model->Start()),
                          useRef(model->Stop()),
                          useRef(model->DurationInSeconds()),
                          useRef(model->Tags()),
                          useRef(model->CreatedWith()),
                          useRef(model->DeletedAt()),
                          useRef(model->UpdatedAt()),
                          useRef(model->ProjectGUID()),
                          useRef(model->ValidationError()),
                          now;
            } else {
                *session_ <<
                          "insert into time_entries(uid, description, wid, "
                          "guid, pid, tid, billable, "
                          "duronly, ui_modified_at, "
                          "start, stop, duration, "
                          "tags, created_with, deleted_at, updated_at, "
                          "project_guid, validation_error "
                          ") values ("
                          ":uid, :description, :wid, "
                          ":guid, :pid, :tid, :billable, "
                          ":duronly, :ui_modified_at, "
                          ":start, :stop, :duration, "
                          ":tags, :created_with, :deleted_at, :updated_at, "
                          ":project_guid, :validation_error)",
                          useRef(model->UID()),
                          useRef(model->Description()),
                          useRef(model->WID()),
                          useRef(model->GUID()),
                          useRef(model->PID()),
                          useRef(model->TID()),
                          useRef(model->Billable()),
                          useRef(model->DurOnly()),
                          useRef(model->UIModifiedAt()),
                          useRef(model->Start()),
                          useRef(model->Stop()),
                          useRef(model->DurationInSeconds()),
                          useRef(model->Tags()),
                          useRef(model->CreatedWith()),
                          useRef(model->DeletedAt()),
                          useRef(model->UpdatedAt()),
                          useRef(model->ProjectGUID()),
                          useRef(model->ValidationError()),
                          now;
            }
            error err = last_error("saveTimeEntry");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ <<
                      "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveTimeEntry");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(),
                kChangeTypeInsert,
                model->ID(),
                model->GUID()));
        }
        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    locked<TimelineEvent> &model,
    std::vector<ModelChange> *changes) {

    try {
        poco_check_ptr(model);
        poco_check_ptr(changes);

        model->EnsureGUID();

        poco_assert(!model->GUID().empty());

        if (!model->NeedsToBeSaved()) {
            return noError;
        }

        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        const int kMaxTimelineStringSize = 300;

        if (model->Filename().length() > kMaxTimelineStringSize) {
            model->SetFilename(
                model->Filename().substr(0, kMaxTimelineStringSize));
        }
        if (model->Title().length() > kMaxTimelineStringSize) {
            model->SetTitle(model->Title().substr(0, kMaxTimelineStringSize));
        }

        if (!model->UID()) {
            return error("Cannot save timeline event without an user ID");
        }
        if (!model->Start()) {
            return error("Cannot save timeline event without start time");
        }
        if (!model->EndTime()) {
            return error("Cannot save timeline event without end time");
        }

        Poco::Int64 start_time(model->Start());
        Poco::Int64 end_time(model->EndTime());

        if (model->LocalID()) {
            logger.trace("Updating timeline event ", model->String(), " in thread ", Poco::Thread::currentTid());

            *session_ <<
                      "update timeline_events set "
                      " guid = :guid, "
                      " title = :title, "
                      " filename = :filename, "
                      " uid = :uid, "
                      " start_time = :start_time, "
                      " end_time = :end_time, "
                      " idle = :idle, "
                      " uploaded = :uploaded, "
                      " chunked = :chunked "
                      "where local_id = :local_id",
                      useRef(model->GUID()),
                      useRef(model->Title()),
                      useRef(model->Filename()),
                      useRef(model->UID()),
                      useRef(start_time),
                      useRef(end_time),
                      useRef(model->Idle()),
                      useRef(model->Uploaded()),
                      useRef(model->Chunked()),
                      useRef(model->LocalID()),
                      now;

            error err = last_error("update timeline event");
            if (err != noError) {
                return err;
            }
            if (model->DeletedAt()) {
                changes->push_back(ModelChange(
                    model->ModelName(),
                    kChangeTypeDelete,
                    model->ID(),
                    model->GUID()));
            } else {
                changes->push_back(ModelChange(
                    model->ModelName(),
                    kChangeTypeUpdate,
                    model->ID(),
                    model->GUID()));
            }
        } else {
            logger.trace("Inserting timeline event ", model->String(), " in thread ", Poco::Thread::currentTid());

            *session_ <<
                      "insert into timeline_events("
                      " guid, "
                      " title, "
                      " filename, "
                      " uid, "
                      " start_time, "
                      " end_time, "
                      " idle, "
                      " uploaded, "
                      " chunked "
                      ") values ("
                      " :guid, "
                      " :title, "
                      " :filename, "
                      " :uid, "
                      " :start_time, "
                      " :end_time, "
                      " :idle, "
                      " :uploaded, "
                      " :chunked "
                      ")",
                      useRef(model->GUID()),
                      useRef(model->Title()),
                      useRef(model->Filename()),
                      useRef(model->UID()),
                      useRef(start_time),
                      useRef(end_time),
                      useRef(model->Idle()),
                      useRef(model->Uploaded()),
                      useRef(model->Chunked()),
                      now;
            error err = last_error("insert timeline event");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ <<
                      "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("select last inserted timeline event ID");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(),
                kChangeTypeInsert,
                model->ID(),
                model->GUID()));
        }

        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    locked<AutotrackerRule> &model,
    std::vector<ModelChange> *changes) {

    try {
        poco_check_ptr(model);

        if (model->LocalID() && !model->Dirty()) {
            return noError;
        }

        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        if (model->LocalID()) {
            logger.trace("Updating autotracker rule ", model->String(), " in thread ", Poco::Thread::currentTid());

            *session_ <<
                      "update autotracker_settings set "
                      "uid = :uid, term = :term, pid = :pid, "
                      "tid = :tid "
                      "where local_id = :local_id",
                      useRef(model->UID()),
                      useRef(model->Term()),
                      useRef(model->PID()),
                      useRef(model->TID()),
                      useRef(model->LocalID()),
                      now;
            error err = last_error("saveAutotrackerRule");
            if (err != noError) {
                return err;
            }
            if (model->DeletedAt()) {
                changes->push_back(ModelChange(
                    model->ModelName(),
                    kChangeTypeDelete,
                    model->ID(),
                    model->GUID()));
            } else {
                changes->push_back(ModelChange(
                    model->ModelName(),
                    kChangeTypeUpdate,
                    model->ID(),
                    model->GUID()));
            }

        } else {
            logger.trace("Inserting autotracker rule ", model->String(), " in thread ", Poco::Thread::currentTid());
            *session_ <<
                      "insert into autotracker_settings(uid, term, pid, tid) "
                      "values(:uid, :term, :pid, :tid)",
                      useRef(model->UID()),
                      useRef(model->Term()),
                      useRef(model->PID()),
                      useRef(model->TID()),
                      now;
            error err = last_error("saveAutotrackerRule");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ <<
                      "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveAutotrackerRule");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(),
                kChangeTypeInsert,
                model->ID(),
                model->GUID()));
        }

        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    locked<ObmAction> &model,
    std::vector<ModelChange> *changes) {

    try {
        poco_check_ptr(model);

        if (model->LocalID() && !model->Dirty()) {
            return noError;
        }

        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        if (model->LocalID()) {
            logger.trace("Updating OBM action ", model->String(), " in thread ", Poco::Thread::currentTid());

            *session_ <<
                      "update obm_actions set "
                      "uid = :uid, "
                      "experiment_id = :experiment_id, "
                      "key = :key, "
                      "value = :value "
                      "where local_id = :local_id",
                      useRef(model->UID()),
                      useRef(model->ExperimentID()),
                      useRef(model->Key()),
                      useRef(model->Value()),
                      useRef(model->LocalID()),
                      now;
            error err = last_error("saveObmAction");
            if (err != noError) {
                return err;
            }
            if (model->DeletedAt()) {
                changes->push_back(ModelChange(
                    model->ModelName(),
                    kChangeTypeDelete,
                    model->ID(),
                    model->GUID()));
            } else {
                changes->push_back(ModelChange(
                    model->ModelName(),
                    kChangeTypeUpdate,
                    model->ID(),
                    model->GUID()));
            }

        } else {
            logger.trace("Inserting OBM action ", model->String(), " in thread ", Poco::Thread::currentTid());
            *session_ <<
                      "insert into obm_actions(uid, experiment_id, key, value) "
                      "values(:uid, :experiment_id, :key, :value)",
                      useRef(model->UID()),
                      useRef(model->ExperimentID()),
                      useRef(model->Key()),
                      useRef(model->Value()),
                      now;
            error err = last_error("saveObmAction");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ <<
                      "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveObmAction");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(),
                kChangeTypeInsert,
                model->ID(),
                model->GUID()));
        }

        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    locked<Workspace> &model,
    std::vector<ModelChange> *changes) {


    try {
        poco_check_ptr(model);

        if (model->LocalID() && !model->Dirty()) {
            return noError;
        }

        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        if (model->LocalID()) {
            logger.trace("Updating workspace ", model->String(), " in thread ", Poco::Thread::currentTid());

            *session_ <<
                      "update workspaces set "
                      "id = :id, uid = :uid, name = :name, premium = :premium, "
                      "only_admins_may_create_projects = "
                      ":only_admins_may_create_projects, admin = :admin, "
                      "projects_billable_by_default = "
                      ":projects_billable_by_default, "
                      "is_business = :is_business, "
                      "locked_time = :locked_time "
                      "where local_id = :local_id",
                      useRef(model->ID()),
                      useRef(model->UID()),
                      useRef(model->Name()),
                      useRef(model->Premium()),
                      useRef(model->OnlyAdminsMayCreateProjects()),
                      useRef(model->Admin()),
                      useRef(model->ProjectsBillableByDefault()),
                      useRef(model->Business()),
                      useRef(model->LockedTime()),
                      useRef(model->LocalID()),
                      now;
            error err = last_error("saveWorkspace");
            if (err != noError) {
                return err;
            }
            changes->push_back(ModelChange(
                model->ModelName(), kChangeTypeUpdate, model->ID(), ""));

        } else {
            logger.trace("Inserting workspace ", model->String(), " in thread ", Poco::Thread::currentTid());
            *session_ <<
                      "insert into workspaces(id, uid, name, premium, "
                      "only_admins_may_create_projects, admin, "
                      "projects_billable_by_default, "
                      "is_business, locked_time) "
                      "values(:id, :uid, :name, :premium, "
                      ":only_admins_may_create_projects, :admin, "
                      ":projects_billable_by_default, "
                      ":is_business, :locked_time)",
                      useRef(model->ID()),
                      useRef(model->UID()),
                      useRef(model->Name()),
                      useRef(model->Premium()),
                      useRef(model->OnlyAdminsMayCreateProjects()),
                      useRef(model->Admin()),
                      useRef(model->ProjectsBillableByDefault()),
                      useRef(model->Business()),
                      useRef(model->LockedTime()),
                      now;
            error err = last_error("saveWorkspace");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ <<
                      "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveWorkspace");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(), kChangeTypeInsert, model->ID(), ""));
        }
        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    locked<Client> &model,
    std::vector<ModelChange> *changes) {

    try {
        poco_check_ptr(model);

        // Generate GUID only for locally-created
        // clients. User cannot update existing
        // clients, so don't mess with their GUIDs
        if (!model->ID()) {
            model->EnsureGUID();
            if (model->GUID().empty()) {
                return error("Cannot save new cient without a GUID");
            }
        }

        if (!model->NeedsToBeSaved()) {
            return noError;
        }

        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        if (model->LocalID()) {
            logger.trace("Updating client ", model->String(), " in thread ", Poco::Thread::currentTid());

            if (model->GUID().empty()) {
                *session_ <<
                          "update clients set "
                          "id = :id, uid = :uid, name = :name, wid = :wid "
                          "where local_id = :local_id",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Name()),
                          useRef(model->WID()),
                          useRef(model->LocalID()),
                          now;
            } else {
                *session_ <<
                          "update clients set "
                          "id = :id, uid = :uid, name = :name, guid = :guid, "
                          "wid = :wid "
                          "where local_id = :local_id",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Name()),
                          useRef(model->GUID()),
                          useRef(model->WID()),
                          useRef(model->LocalID()),
                          now;
            }
            error err = last_error("saveClient");
            if (err != noError) {
                return err;
            }
            changes->push_back(ModelChange(
                model->ModelName(),
                kChangeTypeUpdate,
                model->ID(),
                model->GUID()));

        } else {
            logger.trace("Inserting client ", model->String(), " in thread ", Poco::Thread::currentTid());
            if (model->GUID().empty()) {
                *session_ <<
                          "insert into clients(id, uid, name, wid) "
                          "values(:id, :uid, :name, :wid)",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Name()),
                          useRef(model->WID()),
                          now;
            } else {
                *session_ <<
                          "insert into clients(id, uid, name, guid, wid) "
                          "values(:id, :uid, :name, :guid, :wid)",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Name()),
                          useRef(model->GUID()),
                          useRef(model->WID()),
                          now;
            }
            error err = last_error("saveClient");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ <<
                      "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveClient");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(),
                kChangeTypeInsert,
                model->ID(),
                model->GUID()));
        }
        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    locked<Project> &model,
    std::vector<ModelChange> *changes) {

    try {
        poco_check_ptr(model);

        // Generate GUID only for locally-created
        // projects. User cannot update existing
        // projects, so don't mess with their GUIDs
        if (!model->ID()) {
            model->EnsureGUID();
            if (model->GUID().empty()) {
                return error("Cannot save project without a GUID");
            }
        }

        if (!model->NeedsToBeSaved()) {
            return noError;
        }

        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        if (model->LocalID()) {
            logger.debug("Updating project ", model->String(), " in thread ", Poco::Thread::currentTid());

            if (model->ID()) {
                if (model->GUID().empty()) {
                    *session_ <<
                              "update projects set "
                              "id = :id, uid = :uid, name = :name, "
                              "wid = :wid, color = :color, cid = :cid, "
                              "active = :active, billable = :billable, "
                              "client_guid = :client_guid "
                              "where local_id = :local_id",
                              useRef(model->ID()),
                              useRef(model->UID()),
                              useRef(model->Name()),
                              useRef(model->WID()),
                              useRef(model->Color()),
                              useRef(model->CID()),
                              useRef(model->Active()),
                              useRef(model->Billable()),
                              useRef(model->ClientGUID()),
                              useRef(model->LocalID()),
                              now;
                } else {
                    *session_ <<
                              "update projects set "
                              "id = :id, uid = :uid, name = :name, "
                              "guid = :guid,"
                              "wid = :wid, color = :color, cid = :cid, "
                              "active = :active, billable = :billable, "
                              "client_guid = :client_guid "
                              "where local_id = :local_id",
                              useRef(model->ID()),
                              useRef(model->UID()),
                              useRef(model->Name()),
                              useRef(model->GUID()),
                              useRef(model->WID()),
                              useRef(model->Color()),
                              useRef(model->CID()),
                              useRef(model->Active()),
                              useRef(model->Billable()),
                              useRef(model->ClientGUID()),
                              useRef(model->LocalID()),
                              now;
                }
            } else {
                if (model->GUID().empty()) {
                    *session_ <<
                              "update projects set "
                              "uid = :uid, name = :name, "
                              "wid = :wid, color = :color, cid = :cid, "
                              "active = :active, billable = :billable, "
                              "client_guid = :client_guid "
                              "where local_id = :local_id",
                              useRef(model->UID()),
                              useRef(model->Name()),
                              useRef(model->WID()),
                              useRef(model->Color()),
                              useRef(model->CID()),
                              useRef(model->Active()),
                              useRef(model->Billable()),
                              useRef(model->ClientGUID()),
                              useRef(model->LocalID()),
                              now;
                } else {
                    *session_ <<
                              "update projects set "
                              "uid = :uid, name = :name, guid = :guid,"
                              "wid = :wid, color = :color, cid = :cid, "
                              "active = :active, billable = :billable, "
                              "client_guid = :client_guid "
                              "where local_id = :local_id",
                              useRef(model->UID()),
                              useRef(model->Name()),
                              useRef(model->GUID()),
                              useRef(model->WID()),
                              useRef(model->Color()),
                              useRef(model->CID()),
                              useRef(model->Active()),
                              useRef(model->Billable()),
                              useRef(model->ClientGUID()),
                              useRef(model->LocalID()),
                              now;
                }
            }
            error err = last_error("saveProject");
            if (err != noError) {
                return err;
            }
            changes->push_back(ModelChange(
                model->ModelName(),
                kChangeTypeUpdate,
                model->ID(),
                model->GUID()));

        } else {
            logger.debug("Inserting project ", model->String(), " in thread ", Poco::Thread::currentTid());
            if (model->ID()) {
                if (model->GUID().empty()) {
                    *session_ <<
                              "insert into projects("
                              "id, uid, name, wid, color, cid, active, "
                              "is_private, billable, client_guid"
                              ") values("
                              ":id, :uid, :name, :wid, :color, :cid, :active, "
                              ":is_private, :billable, :client_guid"
                              ")",
                              useRef(model->ID()),
                              useRef(model->UID()),
                              useRef(model->Name()),
                              useRef(model->WID()),
                              useRef(model->Color()),
                              useRef(model->CID()),
                              useRef(model->Active()),
                              useRef(model->IsPrivate()),
                              useRef(model->Billable()),
                              useRef(model->ClientGUID()),
                              now;
                } else {
                    *session_ <<
                              "insert into projects("
                              "id, uid, name, guid, wid, color, cid, "
                              "active, is_private, "
                              "billable, client_guid"
                              ") values("
                              ":id, :uid, :name, :guid, :wid, :color, :cid, "
                              ":active, :is_private, "
                              ":billable, :client_guid"
                              ")",
                              useRef(model->ID()),
                              useRef(model->UID()),
                              useRef(model->Name()),
                              useRef(model->GUID()),
                              useRef(model->WID()),
                              useRef(model->Color()),
                              useRef(model->CID()),
                              useRef(model->Active()),
                              useRef(model->IsPrivate()),
                              useRef(model->Billable()),
                              useRef(model->ClientGUID()),
                              now;
                }
            } else {
                if (model->GUID().empty()) {
                    *session_ <<
                              "insert into projects("
                              "uid, name, wid, color, cid, active, "
                              "is_private, billable, client_guid"
                              ") values("
                              ":uid, :name, :wid, :color, :cid, :active, "
                              ":is_private, :billable, :client_guid"
                              ")",
                              useRef(model->UID()),
                              useRef(model->Name()),
                              useRef(model->WID()),
                              useRef(model->Color()),
                              useRef(model->CID()),
                              useRef(model->Active()),
                              useRef(model->IsPrivate()),
                              useRef(model->Billable()),
                              useRef(model->ClientGUID()),
                              now;
                } else {
                    *session_ <<
                              "insert into projects("
                              "uid, name, guid, wid, color, cid, "
                              "active, is_private, billable, "
                              "client_guid "
                              ") values("
                              ":uid, :name, :guid, :wid, :color, :cid, "
                              ":active, :is_private, :billable, "
                              ":client_guid "
                              ")",
                              useRef(model->UID()),
                              useRef(model->Name()),
                              useRef(model->GUID()),
                              useRef(model->WID()),
                              useRef(model->Color()),
                              useRef(model->CID()),
                              useRef(model->Active()),
                              useRef(model->IsPrivate()),
                              useRef(model->Billable()),
                              useRef(model->ClientGUID()),
                              now;
                }
            }
            error err = last_error("saveProject");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ <<
                      "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveProject");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(),
                kChangeTypeInsert,
                model->ID(),
                model->GUID()));
        }
        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    locked<Task> &model,
    std::vector<ModelChange> *changes) {

    try {
        poco_check_ptr(model);

        if (model->LocalID() && !model->Dirty()) {
            return noError;
        }

        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        if (model->LocalID()) {
            logger.trace("Updating task ", model->String(), " in thread ", Poco::Thread::currentTid());

            *session_ <<
                      "update tasks set "
                      "id = :id, uid = :uid, name = :name, wid = :wid, "
                      "pid = :pid, active = :active "
                      "where local_id = :local_id",
                      useRef(model->ID()),
                      useRef(model->UID()),
                      useRef(model->Name()),
                      useRef(model->WID()),
                      useRef(model->PID()),
                      useRef(model->Active()),
                      useRef(model->LocalID()),
                      now;
            error err = last_error("saveTask");
            if (err != noError) {
                return err;
            }
            changes->push_back(ModelChange(
                model->ModelName(), kChangeTypeUpdate, model->ID(), ""));

        } else {
            logger.trace("Inserting task ", model->String(), " in thread ", Poco::Thread::currentTid());
            *session_ <<
                      "insert into tasks(id, uid, name, wid, pid, active) "
                      "values(:id, :uid, :name, :wid, :pid, :active)",
                      useRef(model->ID()),
                      useRef(model->UID()),
                      useRef(model->Name()),
                      useRef(model->WID()),
                      useRef(model->PID()),
                      useRef(model->Active()),
                      now;
            error err = last_error("saveTask");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ <<
                      "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveTask");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(), kChangeTypeInsert, model->ID(), ""));
        }
        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    locked<Tag> &model,
    std::vector<ModelChange> *changes    ) {

    try {
        poco_check_ptr(model);

        if (model->LocalID() && !model->Dirty()) {
            return noError;
        }

        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        if (model->LocalID()) {
            logger.trace("Updating tag ", model->String(), " in thread ", Poco::Thread::currentTid());

            if (model->GUID().empty()) {
                *session_ <<
                          "update tags set "
                          "id = :id, uid = :uid, name = :name, wid = :wid "
                          "where local_id = :local_id",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Name()),
                          useRef(model->WID()),
                          useRef(model->LocalID()),
                          now;
            } else {
                *session_ <<
                          "update tags set "
                          "id = :id, uid = :uid, name = :name, wid = :wid, "
                          "guid = :guid "
                          "where local_id = :local_id",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Name()),
                          useRef(model->WID()),
                          useRef(model->GUID()),
                          useRef(model->LocalID()),
                          now;
            }
            error err = last_error("saveTag");
            if (err != noError) {
                return err;
            }
            changes->push_back(ModelChange(
                model->ModelName(),
                kChangeTypeUpdate,
                model->ID(),
                model->GUID()));

        } else {
            logger.trace("Inserting tag ", model->String(), " in thread ", Poco::Thread::currentTid());
            if (model->GUID().empty()) {
                *session_ <<
                          "insert into tags(id, uid, name, wid) "
                          "values(:id, :uid, :name, :wid)",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Name()),
                          useRef(model->WID()),
                          now;
            } else {
                *session_ <<
                          "insert into tags(id, uid, name, wid, guid) "
                          "values(:id, :uid, :name, :wid, :guid)",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Name()),
                          useRef(model->WID()),
                          useRef(model->GUID()),
                          now;
            }
            error err = last_error("saveTag");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ <<
                      "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveTag");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(),
                kChangeTypeInsert,
                model->ID(),
                model->GUID()));
        }
        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}


error Database::SaveUser(
    locked<User> &user,
    const bool with_related_data,
    std::vector<ModelChange> *changes) {

    Poco::Mutex::ScopedLock lock(session_m_);

    // Do nothing, if user has already logged out
    if (!user) {
        logger.warning("Cannot save user, user is logged out");
        return noError;
    }

    poco_check_ptr(session_);
    poco_check_ptr(changes);

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    if (user->Email().empty()) {
        return error("Missing user e-mail, cannot save user");
    }
    if (user->APIToken().empty()) {
        return error("Missing user API token, cannot save user");
    }
    if (!user->ID()) {
        return error("Missing user ID, cannot save user");
    }

    session_->begin();

    // Check if we really need to save model,
    // *but* do not return if we don't need to.
    // We might need to save related models, still.
    if (!user->LocalID() || user->Dirty()) {
        try {
            if (user->LocalID()) {
                logger.trace("Updating user ", user->String(), " in thread ", Poco::Thread::currentTid());

                *session_ <<
                          "update users set "
                          "default_wid = :default_wid, "
                          "since = :since, id = :id, fullname = :fullname, "
                          "email = :email, record_timeline = :record_timeline, "
                          "store_start_and_stop_time = "
                          " :store_start_and_stop_time, "
                          "timeofday_format = :timeofday_format, "
                          "duration_format = :duration_format, "
                          "offline_data = :offline_data, "
                          "default_pid = :default_pid, "
                          "default_tid = :default_tid, "
                          "collapse_entries = :collapse_entries "
                          "where local_id = :local_id",
                          useRef(user->DefaultWID()),
                          useRef(user->Since()),
                          useRef(user->ID()),
                          useRef(user->Fullname()),
                          useRef(user->Email()),
                          useRef(user->RecordTimeline()),
                          useRef(user->StoreStartAndStopTime()),
                          useRef(user->TimeOfDayFormat()),
                          useRef(user->DurationFormat()),
                          useRef(user->OfflineData()),
                          useRef(user->DefaultPID()),
                          useRef(user->DefaultTID()),
                          useRef(user->CollapseEntries()),
                          useRef(user->LocalID()),
                          now;
                error err = last_error("SaveUser");
                if (err != noError) {
                    session_->rollback();
                    return err;
                }
                changes->push_back(ModelChange(
                    user->ModelName(), kChangeTypeUpdate, user->ID(), ""));
            } else {
                logger.trace("Inserting user ", user->String(), " in thread ", Poco::Thread::currentTid());
                *session_ <<
                          "insert into users("
                          "id, default_wid, since, fullname, email, "
                          "record_timeline, store_start_and_stop_time, "
                          "timeofday_format, duration_format, offline_data, "
                          "default_pid, default_tid"
                          ") values("
                          ":id, :default_wid, :since, :fullname, "
                          ":email, "
                          ":record_timeline, :store_start_and_stop_time, "
                          ":timeofday_format, :duration_format, :offline_data, "
                          ":default_pid, :default_tid"
                          ")",
                          useRef(user->ID()),
                          useRef(user->DefaultWID()),
                          useRef(user->Since()),
                          useRef(user->Fullname()),
                          useRef(user->Email()),
                          useRef(user->RecordTimeline()),
                          useRef(user->StoreStartAndStopTime()),
                          useRef(user->TimeOfDayFormat()),
                          useRef(user->DurationFormat()),
                          useRef(user->OfflineData()),
                          useRef(user->DefaultPID()),
                          useRef(user->DefaultTID()),
                          now;
                error err = last_error("SaveUser");
                if (err != noError) {
                    session_->rollback();
                    return err;
                }
                Poco::Int64 local_id(0);
                *session_ <<
                          "select last_insert_rowid()",
                          into(local_id),
                          now;
                err = last_error("SaveUser");
                if (err != noError) {
                    session_->rollback();
                    return err;
                }
                user->SetLocalID(local_id);
                changes->push_back(ModelChange(
                    user->ModelName(),
                    kChangeTypeInsert,
                    user->ID(),
                    ""));
            }
            user->ClearDirty();
        } catch(const Poco::Exception& exc) {
            session_->rollback();
            return exc.displayText();
        } catch(const std::exception& ex) {
            session_->rollback();
            return ex.what();
        } catch(const std::string & ex) {
            session_->rollback();
            return ex;
        }
    }

    if (with_related_data) {
        // Workspaces
        std::vector<ModelChange> workspace_changes;
        error err = saveRelatedModels(user->ID(),
                                      "workspaces",
                                      user->GetRelatedData()->Workspaces,
                                      &workspace_changes);
        if (err != noError) {
            session_->rollback();
            return err;
        }
        for (std::vector<ModelChange>::const_iterator
                it = workspace_changes.begin();
                it != workspace_changes.end();
                ++it) {
            ModelChange change = *it;
            if (change.IsDeletion()) {
                user->DeleteRelatedModelsWithWorkspace(change.ModelID());
            }
            changes->push_back(change);
        }

        // Clients
        std::vector<ModelChange> client_changes;
        err = saveRelatedModels(user->ID(),
                                "clients",
                                user->GetRelatedData()->Clients,
                                &client_changes);
        if (err != noError) {
            session_->rollback();
            return err;
        }
        for (std::vector<ModelChange>::const_iterator
                it = client_changes.begin();
                it != client_changes.end();
                ++it) {
            ModelChange change = *it;
            if (change.IsDeletion()) {
                user->RemoveClientFromRelatedModels(change.ModelID());
            }
            changes->push_back(change);
        }

        // Projects
        std::vector<ModelChange> project_changes;
        err = saveRelatedModels(user->ID(),
                                "projects",
                                user->GetRelatedData()->Projects,
                                &project_changes);
        if (err != noError) {
            session_->rollback();
            return err;
        }
        for (std::vector<ModelChange>::const_iterator
                it = project_changes.begin();
                it != project_changes.end();
                ++it) {
            ModelChange change = *it;
            if (change.IsDeletion()) {
                user->RemoveProjectFromRelatedModels(change.ModelID());
            }
            changes->push_back(change);
        }

        // Tasks
        std::vector<ModelChange> task_changes;
        err = saveRelatedModels(user->ID(),
                                "tasks",
                                user->GetRelatedData()->Tasks,
                                &task_changes);
        if (err != noError) {
            session_->rollback();
            return err;
        }
        for (std::vector<ModelChange>::const_iterator
                it = task_changes.begin();
                it != task_changes.end();
                ++it) {
            ModelChange change = *it;
            if (change.IsDeletion()) {
                user->RemoveTaskFromRelatedModels(change.ModelID());
            }
            changes->push_back(change);
        }

        // Tags
        err = saveRelatedModels(user->ID(),
                                "tags",
                                user->GetRelatedData()->Tags,
                                changes);
        if (err != noError) {
            session_->rollback();
            return err;
        }

        // Time entries
        err = saveRelatedModels(user->ID(),
                                "time_entries",
                                user->GetRelatedData()->TimeEntries,
                                changes);
        if (err != noError) {
            session_->rollback();
            return err;
        }

        // Autotracker rules
        err = saveRelatedModels(user->ID(),
                                "autotracker_settings",
                                user->GetRelatedData()->AutotrackerRules,
                                changes);
        if (err != noError) {
            session_->rollback();
            return err;
        }

        // OBM actions
        err = saveRelatedModels(user->ID(),
                                "obm_actions",
                                user->GetRelatedData()->ObmActions,
                                changes);
        if (err != noError) {
            session_->rollback();
            return err;
        }

        // OBM experiments
        err = saveRelatedModels(user->ID(),
                                "obm_experiments",
                                user->GetRelatedData()->ObmExperiments,
                                changes);
        if (err != noError) {
            session_->rollback();
            return err;
        }

        // Timeline events
        err = saveRelatedModels(user->ID(),
                                "timeline_events",
                                user->GetRelatedData()->TimelineEvents,
                                changes);
        if (err != noError) {
            session_->rollback();
            return err;
        }
    }

    session_->commit();

    stopwatch.stop();

    logger.debug("User with_related_data=", with_related_data, " saved in ", stopwatch.elapsed() / 1000, " ms in thread ", Poco::Thread::currentTid());

    return noError;
}

error Database::ensureMigrationTable() {
    std::string table_name;
    // Check if we have migrations table
    *session_ <<
              "select name from sqlite_master "
              "where type='table' and name='kopsik_migrations'",
              into(table_name),
              limit(1),
              now;
    error err = last_error("initialize_tables");
    if (err != noError) {
        return err;
    }

    if (table_name.length() == 0) {
        *session_ <<
                  "create table kopsik_migrations(id integer primary key, "
                  "name varchar not null)",
                  now;
        error err = last_error("initialize_tables");
        if (err != noError) {
            return err;
        }
        *session_ <<
                  "CREATE UNIQUE INDEX id_kopsik_migrations_name "
                  "ON kopsik_migrations (name);",
                  now;
        err = last_error("initialize_tables");
        if (err != noError) {
            return err;
        }
    }

    return noError;
}

error Database::initialize_tables() {
    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    error err = ensureMigrationTable();
    if (err != noError) {
        return err;
    }

    err = Migrations(this).Run();
    if (err != noError) {
        return err;
    }

    return noError;
}

error Database::CurrentAPIToken(
    std::string *token,
    Poco::UInt64 *uid) {

    try {
        poco_check_ptr(token);
        poco_check_ptr(uid);

        poco_check_ptr(session_);
        Poco::Mutex::ScopedLock lock(session_m_);

        *token = "";
        *uid = 0;

        *session_ <<
                  "select api_token, uid "
                  " from sessions limit 1",
                  into(*token),
                  into(*uid),
                  limit(1),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("CurrentAPIToken");
}

error Database::ClearCurrentAPIToken() {
    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ <<
                  "delete from sessions", now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("ClearCurrentAPIToken");
}

error Database::SetCurrentAPIToken(
    const std::string &token,
    const Poco::UInt64 &uid) {
    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        if (token.empty()) {
            return error("cannot start session without API token");
        }
        if (!uid) {
            return error("cannot start session without user ID");
        }

        poco_check_ptr(session_);

        error err = ClearCurrentAPIToken();
        if (err != noError) {
            return err;
        }

        *session_ <<
                  "insert into sessions(api_token, uid) "
                  " values(:api_token, :uid)",
                  useRef(token),
                  useRef(uid),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("SetCurrentAPIToken");
}

error Database::EnsureTimelineGUIDS() {
    try {
        while (true) {
            Poco::UInt64 local_id_without_guid(0);
            error err = UInt(
                "select local_id "
                "from timeline_events "
                "where guid is null or guid = '' "
                "limit 1", &local_id_without_guid);
            if (err != noError) {
                return err;
            }
            if (!local_id_without_guid) {
                return noError;
            }
            std::string guid = GenerateGUID();

            Poco::Mutex::ScopedLock lock(session_m_);

            poco_check_ptr(session_);

            *session_ <<
                      "update timeline_events "
                      "set guid = :guid "
                      "where local_id = :local_id",
                      useRef(guid),
                      useRef(local_id_without_guid),
                      now;
            err = last_error("EnsureTimelineGUIDS");
            if (err != noError) {
                return err;
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
}

error Database::EnsureAnalyticsClientID() {
    error err = String(
        "SELECT analytics_client_id "
        "FROM analytics_settings "
        "LIMIT 1",
        &analytics_client_id_);
    if (err != noError) {
        return err;
    }
    if (analytics_client_id_.empty()) {
        analytics_client_id_ = GenerateGUID();
        err = saveAnalyticsClientID();
        if (err != noError) {
            return err;
        }
    }

    return noError;
}

error Database::EnsureDesktopID() {
    error err = String(
        "SELECT desktop_id "
        "FROM timeline_installation "
        "LIMIT 1",
        &desktop_id_);
    if (err != noError) {
        return err;
    }
    if (desktop_id_.empty()) {
        desktop_id_ = GenerateGUID();
        err = saveDesktopID();
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

error Database::saveAnalyticsClientID() {
    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ <<
                  "INSERT INTO analytics_settings(analytics_client_id) "
                  "VALUES(:analytics_client_id)",
                  useRef(analytics_client_id_),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("saveAnalyticsClientID");
}

error Database::LoadMigrations(
    std::vector<std::string> *list) {

    try {
        poco_check_ptr(list);

        list->clear();

        Poco::Mutex::ScopedLock lock(session_m_);

        Poco::Data::Statement select(*session_);
        select <<
               "SELECT name FROM kopsik_migrations";
        error err = last_error("LoadMigrations");
        if (err != noError) {
            return err;
        }
        Poco::Data::RecordSet rs(select);
        while (!select.done()) {
            select.execute();
            bool more = rs.moveFirst();
            while (more) {
                std::string name(rs[0].convert<std::string>());
                list->push_back(name);
                more = rs.moveNext();
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("LoadMigrations");
}

error Database::Migrate(
    const std::string &name,
    const std::string &sql) {

    if (name.empty()) {
        return error("Cannot run a migration without name");
    }
    if (sql.empty()) {
        return error("Cannot run a migration without SQL");
    }

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        int count = 0;
        *session_ <<
                  "select count(*) from kopsik_migrations where name=:name",
                  into(count),
                  useRef(name),
                  now;
        error err = last_error("migrate");
        if (err != noError) {
            return err;
        }

        if (count) {
            return noError;
        }

        logger.debug("Migrating", "\n", name, "\n", sql, "\n");

        err = execute(sql);
        if (err != noError) {
            return err;
        }

        *session_ <<
                  "insert into kopsik_migrations(name) values(:name)",
                  useRef(name),
                  now;
        err = last_error("migrate");
        if (err != noError) {
            return err;
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Database::execute(
    const std::string &sql) {


    if (sql.empty()) {
        return error("Cannot execute empty SQL");
    }

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ << sql, now;
        error err = last_error("execute");
        if (err != noError) {
            return err;
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

error Database::String(
    const std::string &sql,
    std::string *result) {

    if (sql.empty()) {
        return error("Cannot select from database with empty SQL");
    }

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);
        poco_check_ptr(result);

        std::string value("");
        *session_ << sql,
        into(value),
        now;
        *result = value;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("String");
}

error Database::UInt(
    const std::string &sql,
    Poco::UInt64 *result) {

    if (sql.empty()) {
        return error("Cannot select a numeric from database with empty SQL");
    }

    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);
        poco_check_ptr(result);

        Poco::UInt64 value(0);
        *session_ << sql,
        into(value),
        now;
        *result = value;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("UInt");
}

error Database::saveDesktopID() {
    try {
        Poco::Mutex::ScopedLock lock(session_m_);

        poco_check_ptr(session_);

        *session_ <<
                  "INSERT INTO timeline_installation(desktop_id) "
                  "VALUES(:desktop_id)",
                  useRef(desktop_id_),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("saveDesktopID");
}

error Database::saveModel(
    locked<ObmExperiment> &model,
    std::vector<ModelChange> *changes) {

    try {
        poco_check_ptr(model);

        if (model->LocalID() && !model->Dirty()) {
            return noError;
        }

        Poco::Mutex::ScopedLock lock(session_m_);
        poco_check_ptr(session_);

        if (model->LocalID()) {
            logger.trace("Updating OBM experiment ", model->String(), " in thread ", Poco::Thread::currentTid());

            *session_ <<
                      "update obm_experiments set "
                      "uid = :uid, "
                      "nr = :nr, "
                      "included = :included, "
                      "has_seen = :has_seen, "
                      "actions = :actions "
                      "where local_id = :local_id",
                      useRef(model->UID()),
                      useRef(model->Nr()),
                      useRef(model->Included()),
                      useRef(model->HasSeen()),
                      useRef(model->Actions()),
                      useRef(model->LocalID()),
                      now;
            error err = last_error("saveObmExperiment");
            if (err != noError) {
                return err;
            }
            if (model->DeletedAt()) {
                changes->push_back(ModelChange(
                    model->ModelName(),
                    kChangeTypeDelete,
                    model->ID(),
                    model->GUID()));
            } else {
                changes->push_back(ModelChange(
                    model->ModelName(),
                    kChangeTypeUpdate,
                    model->ID(),
                    model->GUID()));
            }

        } else {
            logger.trace("Inserting OBM action ", model->String(), " in thread ", Poco::Thread::currentTid());
            *session_ <<
                      "insert into obm_experiments("
                      "uid, nr, included, has_seen, actions "
                      ") values("
                      ":uid, :nr, :included, :has_seen, :actions"
                      ")",
                      useRef(model->UID()),
                      useRef(model->Nr()),
                      useRef(model->Included()),
                      useRef(model->HasSeen()),
                      useRef(model->Actions()),
                      now;
            error err = last_error("saveObmExperiment");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ <<
                      "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveObmExperiment");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(),
                kChangeTypeInsert,
                model->ID(),
                model->GUID()));
        }

        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

template<typename T>
error Database::loadModels(const Poco::UInt64 &UID, ProtectedContainer<T> &list) {
    if (!UID) {
        return error("Cannot load user workspaces without an user ID");
    }

    try {
        list.clear();

        Poco::Mutex::ScopedLock lock(session_m_);

        Poco::Data::Statement select(*session_);
        select << list.GetSelect("uid"),
               useRef(UID);
        error err = last_error("load_" + list.ModelName());
        if (err != noError) {
            return err;
        }
        Poco::Data::RecordSet rs(select);
        while (!select.done()) {
            select.execute();
            bool more = rs.moveFirst();
            while (more) {
                auto model = list.create(rs);
                model->ClearDirty();
                more = rs.moveNext();
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }
    return last_error("loadWorkspaces");
}

}   // namespace toggl
