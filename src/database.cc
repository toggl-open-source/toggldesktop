// Copyright 2014 Toggl Desktop developers.

// All session access should be locked.

#include "../src/database.h"

#include <limits>
#include <string>
#include <vector>

#include "./client.h"
#include "./const.h"
#include "./project.h"
#include "./proxy.h"
#include "./settings.h"
#include "./tag.h"
#include "./task.h"
#include "./time_entry.h"
#include "./user.h"
#include "./workspace.h"

#include "Poco/Data/Binding.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/SessionImpl.h"
#include "Poco/Data/SQLite/SessionImpl.h"
#include "Poco/Data/SQLite/Utility.h"
#include "Poco/Data/Statement.h"
#include "Poco/FileStream.h"
#include "Poco/Logger.h"
#include "Poco/Stopwatch.h"
#include "Poco/StreamCopier.h"
#include "Poco/UUID.h"
#include "Poco/UUIDGenerator.h"

namespace toggl {

using Poco::Data::Keywords::useRef;
using Poco::Data::Keywords::limit;
using Poco::Data::Keywords::into;
using Poco::Data::Keywords::now;

Database::Database(const std::string db_path)
    : session_(0)
, desktop_id_("")
, analytics_client_id_("") {
    Poco::Data::SQLite::Connector::registerConnector();

    session_ = new Poco::Data::Session("SQLite", db_path);

    {
        int is_sqlite_threadsafe = sqlite3_threadsafe();

        std::stringstream ss;
        ss << "sqlite3_threadsafe()=" << is_sqlite_threadsafe;
        logger().debug(ss.str());

        if (!is_sqlite_threadsafe) {
            logger().error("Database is not thread safe!");
            return;
        }
    }

    error err = setJournalMode("wal");
    if (err != noError) {
        logger().error("Failed to set journal mode to wal!");
        return;
    }

    std::string mode("");
    err = journalMode(&mode);
    if (err != noError) {
        logger().error("Could not detect journal mode!");
        return;
    }

    {
        std::stringstream ss;
        ss << "PRAGMA journal_mode=" << mode;
        logger().debug(ss.str());
    }

    if ("wal" != mode) {
        logger().error("Failed to enable wal journal mode!");
        return;
    }

    err = vacuum();
    if (err != noError) {
        logger().error("failed to vacuum: " + err);
        // but will continue, its not vital
    }

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    err = initialize_tables();
    if (err != noError) {
        logger().error(err);
        return;
    }

    stopwatch.stop();

    {
        std::stringstream ss;
        ss  << "Migrated in "
            << stopwatch.elapsed() / 1000 << " ms";
        logger().debug(ss.str());
    }
}

Database::~Database() {
    if (session_) {
        delete session_;
        session_ = 0;
    }
    Poco::Data::SQLite::Connector::unregisterConnector();
}

error Database::DeleteUser(
    User *model,
    const bool with_related_data) {

    poco_check_ptr(model);

    error err = deleteFromTable("sessions", model->LocalID());
    if (err != noError) {
        return err;
    }

    err = deleteFromTable("users", model->LocalID());
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
    }
    return noError;
}

error Database::deleteAllFromTableByUID(
    const std::string table_name,
    const Poco::Int64 &UID) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    if (!UID) {
        return error("Cannot delete user data without user ID");
    }
    if (table_name.empty()) {
        return error("Cannot delete from table without table name");
    }

    try {
        *session_ << "delete from " + table_name + " where uid = :uid",
                  useRef(UID),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("deleteAllFromTableByUID");
}

error Database::journalMode(std::string *mode) {
    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);
    poco_check_ptr(mode);

    try {
        *session_ << "PRAGMA journal_mode",
                  into(*mode),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("journalMode");
}

error Database::setJournalMode(const std::string mode) {
    if (mode.empty()) {
        return error("Cannot set journal mode without a mode");
    }

    Poco::Mutex::ScopedLock lock(session_m_);
    poco_check_ptr(session_);

    try {
        *session_ << "PRAGMA journal_mode=" << mode,
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("setJournalMode");
}

error Database::vacuum() {
    Poco::Mutex::ScopedLock lock(session_m_);
    poco_check_ptr(session_);

    try {
        *session_ << "VACUUM;" << now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("vacuum");
}

Poco::Logger &Database::logger() const {
    return Poco::Logger::get("database");
}

error Database::deleteFromTable(
    const std::string table_name,
    const Poco::Int64 &local_id) {

    if (table_name.empty()) {
        return error("Cannot delete from table without table name");
    }

    Poco::Mutex::ScopedLock lock(session_m_);
    poco_check_ptr(session_);

    if (!local_id) {
        return noError;
    }

    std::stringstream ss;
    ss << "Deleting from table " << table_name
       << ", local ID: " << local_id;
    logger().debug(ss.str());

    try {
        *session_ << "delete from " + table_name +
                  " where local_id = :local_id",
                  useRef(local_id),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("deleteFromTable");
}

error Database::last_error(const std::string was_doing) {
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

error Database::LoadCurrentUser(User *user) {
    poco_check_ptr(user);

    logger().debug("LoadCurrentUser");

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

error Database::LoadSettings(Settings *settings) {
    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    try {
        *session_ << "select use_idle_detection, menubar_timer, "
                  "menubar_project, dock_icon, on_top, reminder,  "
                  "idle_minutes, focus_on_shortcut, reminder_minutes, "
                  "manual_mode, autodetect_proxy "
                  "from settings limit 1",
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
                  limit(1),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("LoadSettings");
}

error Database::SaveWindowSettings(
    const Poco::Int64 window_x,
    const Poco::Int64 window_y,
    const Poco::Int64 window_height,
    const Poco::Int64 window_width) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    try {
        *session_ << "update settings set "
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
    } catch(const std::string& ex) {
        return ex;
    }

    return last_error("SaveWindowSettings");
}

error Database::LoadWindowSettings(
    Poco::Int64 *window_x,
    Poco::Int64 *window_y,
    Poco::Int64 *window_height,
    Poco::Int64 *window_width) {
    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    Poco::Int64 x(0), y(0), height(0), width(0);

    try {
        *session_ << "select window_x, window_y, window_height, window_width "
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
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("LoadWindowSettings");
}

error Database::LoadProxySettings(
    bool *use_proxy,
    Proxy *proxy) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);
    poco_check_ptr(use_proxy);
    poco_check_ptr(proxy);

    try {
        std::string host(""), username(""), password("");
        Poco::UInt64 port(0);
        *session_ << "select use_proxy, proxy_host, proxy_port, "
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
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("LoadProxySettings");
}

error Database::SetSettingsUseIdleDetection(
    const bool &use_idle_detection) {
    return setSettingsValue("use_idle_detection", use_idle_detection);
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

template<typename T>
error Database::setSettingsValue(
    const std::string field_name,
    const T &value) {
    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    try {
        *session_ << "update settings set " + field_name + " = :" + field_name,
                  useRef(value),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("setSettingsValue");
}

error Database::SetSettingsReminderMinutes(
    const Poco::UInt64 reminder_minutes) {
    Poco::UInt64 new_value = reminder_minutes;
    if (new_value < 1) {
        new_value = 1;
    }
    return setSettingsValue("reminder_minutes", new_value);
}

error Database::SaveProxySettings(
    const bool &use_proxy,
    const Proxy &proxy) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    try {
        *session_ << "update settings set "
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
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("SaveProxySettings");
}

error Database::LoadUpdateChannel(
    std::string *update_channel) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);
    poco_check_ptr(update_channel);

    try {
        *session_ << "select update_channel from settings limit 1",
                  into(*update_channel),
                  limit(1),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
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
    User *model) {

    if (email.empty()) {
        return error("Cannot load user by email token without an email");
    }

    poco_check_ptr(model);

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    Poco::UInt64 uid(0);
    model->SetEmail(email);

    try {
        *session_ << "select id from users"
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
    } catch(const std::string& ex) {
        return ex;
    }
    return LoadUserByID(uid, model);
}

error Database::loadUsersRelatedData(User *user) {
    error err = loadWorkspaces(user->ID(), &user->related.Workspaces);
    if (err != noError) {
        return err;
    }

    err = loadClients(user->ID(), &user->related.Clients);
    if (err != noError) {
        return err;
    }

    err = loadProjects(user->ID(), &user->related.Projects);
    if (err != noError) {
        return err;
    }

    err = loadTasks(user->ID(), &user->related.Tasks);
    if (err != noError) {
        return err;
    }

    err = loadTags(user->ID(), &user->related.Tags);
    if (err != noError) {
        return err;
    }

    err = loadTimeEntries(user->ID(), &user->related.TimeEntries);
    if (err != noError) {
        return err;
    }

    return noError;
}

error Database::LoadUserByID(
    const Poco::UInt64 &UID,
    User *user) {

    if (!UID) {
        return error("Cannot load user by ID without an ID");
    }

    poco_check_ptr(user);

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    try {
        Poco::Int64 local_id(0);
        Poco::UInt64 id(0);
        Poco::UInt64 default_wid(0);
        Poco::UInt64 since(0);
        std::string fullname("");
        std::string email("");
        bool record_timeline(false);
        bool store_start_and_stop_time(false);
        std::string timeofday_format("");
        std::string duration_format("");
        std::string offline_data("");
        *session_ <<
                  "select local_id, id, default_wid, since, "
                  "fullname, "
                  "email, record_timeline, store_start_and_stop_time, "
                  "timeofday_format, duration_format, offline_data "
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
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    error err = loadUsersRelatedData(user);
    if (err != noError) {
        return err;
    }

    stopwatch.stop();
    std::stringstream ss;
    ss << "User loaded in " << stopwatch.elapsed() / 1000 << " ms";
    logger().debug(ss.str());

    return noError;
}

error Database::loadWorkspaces(
    const Poco::UInt64 &UID,
    std::vector<Workspace *> *list) {

    if (!UID) {
        return error("Cannot load user workspaces without an user ID");
    }

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(list);

    list->clear();

    try {
        Poco::Data::Statement select(*session_);
        select <<
               "SELECT local_id, id, uid, name, premium, "
               "only_admins_may_create_projects, admin "
               "FROM workspaces "
               "WHERE uid = :uid "
               "ORDER BY name",
               useRef(UID);
        error err = last_error("loadWorkspaces");
        if (err != noError) {
            return err;
        }
        Poco::Data::RecordSet rs(select);
        while (!select.done()) {
            select.execute();
            bool more = rs.moveFirst();
            while (more) {
                Workspace *model = new Workspace();
                model->SetLocalID(rs[0].convert<Poco::Int64>());
                model->SetID(rs[1].convert<Poco::UInt64>());
                model->SetUID(rs[2].convert<Poco::UInt64>());
                model->SetName(rs[3].convert<std::string>());
                model->SetPremium(rs[4].convert<bool>());
                model->SetOnlyAdminsMayCreateProjects(rs[5].convert<bool>());
                model->SetAdmin(rs[6].convert<bool>());
                model->ClearDirty();
                list->push_back(model);
                more = rs.moveNext();
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("loadWorkspaces");
}

error Database::loadClients(
    const Poco::UInt64 &UID,
    std::vector<Client *> *list) {

    if (!UID) {
        return error("Cannot load user clients without an user ID");
    }

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(list);

    list->clear();

    try {
        Poco::Data::Statement select(*session_);
        select << "SELECT local_id, id, uid, name, guid, wid "
               "FROM clients "
               "WHERE uid = :uid "
               "ORDER BY name",
               useRef(UID);

        error err = last_error("loadClients");
        if (err != noError) {
            return err;
        }
        Poco::Data::RecordSet rs(select);
        while (!select.done()) {
            select.execute();
            bool more = rs.moveFirst();
            while (more) {
                Client *model = new Client();
                model->SetLocalID(rs[0].convert<Poco::Int64>());
                if (rs[1].isEmpty()) {
                    model->SetID(0);
                } else {
                    model->SetID(rs[1].convert<Poco::UInt64>());
                }
                model->SetUID(rs[2].convert<Poco::UInt64>());
                model->SetName(rs[3].convert<std::string>());
                if (rs[4].isEmpty()) {
                    model->SetGUID("");
                } else {
                    model->SetGUID(rs[4].convert<std::string>());
                }
                model->SetWID(rs[5].convert<Poco::UInt64>());
                model->ClearDirty();
                list->push_back(model);
                more = rs.moveNext();
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("loadClients");
}

error Database::loadProjects(
    const Poco::UInt64 &UID,
    std::vector<Project *> *list) {

    if (!UID) {
        return error("Cannot load user projects without an user ID");
    }

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(list);

    list->clear();

    try {
        Poco::Data::Statement select(*session_);
        select << "SELECT local_id, id, uid, name, guid, wid, color, cid, "
               "active, billable, client_guid "
               "FROM projects "
               "WHERE uid = :uid "
               "ORDER BY name",
               useRef(UID);
        error err = last_error("loadProjects");
        if (err != noError) {
            return err;
        }
        Poco::Data::RecordSet rs(select);
        while (!select.done()) {
            select.execute();
            bool more = rs.moveFirst();
            while (more) {
                Project *model = new Project();
                model->SetLocalID(rs[0].convert<Poco::Int64>());
                if (rs[1].isEmpty()) {
                    model->SetID(0);
                } else {
                    model->SetID(rs[1].convert<Poco::UInt64>());
                }
                model->SetUID(rs[2].convert<Poco::UInt64>());
                model->SetName(rs[3].convert<std::string>());
                if (rs[4].isEmpty()) {
                    model->SetGUID("");
                } else {
                    model->SetGUID(rs[4].convert<std::string>());
                }
                model->SetWID(rs[5].convert<Poco::UInt64>());
                if (rs[6].isEmpty()) {
                    model->SetColor("");
                } else {
                    model->SetColor(rs[6].convert<std::string>());
                }
                if (rs[7].isEmpty()) {
                    model->SetCID(0);
                } else {
                    model->SetCID(rs[7].convert<Poco::UInt64>());
                }
                model->SetActive(rs[8].convert<bool>());
                model->SetBillable(rs[9].convert<bool>());
                if (rs[10].isEmpty()) {
                    model->SetClientGUID("");
                } else {
                    model->SetClientGUID(rs[10].convert<std::string>());
                }
                model->ClearDirty();
                list->push_back(model);
                more = rs.moveNext();
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("loadProjects");
}

error Database::loadTasks(
    const Poco::UInt64 &UID,
    std::vector<Task *> *list) {

    if (!UID) {
        return error("Cannot load user tasks without an user ID");
    }

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(list);

    list->clear();

    try {
        Poco::Data::Statement select(*session_);
        select << "SELECT local_id, id, uid, name, wid, pid, active "
               "FROM tasks "
               "WHERE uid = :uid "
               "ORDER BY name",
               useRef(UID);
        error err = last_error("loadTasks");
        if (err != noError) {
            return err;
        }
        Poco::Data::RecordSet rs(select);
        while (!select.done()) {
            select.execute();
            bool more = rs.moveFirst();
            while (more) {
                Task *model = new Task();
                model->SetLocalID(rs[0].convert<Poco::Int64>());
                if (rs[1].isEmpty()) {
                    model->SetID(0);
                } else {
                    model->SetID(rs[1].convert<Poco::UInt64>());
                }
                model->SetUID(rs[2].convert<Poco::UInt64>());
                model->SetName(rs[3].convert<std::string>());
                model->SetWID(rs[4].convert<Poco::UInt64>());
                if (rs[5].isEmpty()) {
                    model->SetPID(0);
                } else {
                    model->SetPID(rs[5].convert<Poco::UInt64>());
                }
                model->SetActive(rs[6].convert<bool>());
                model->ClearDirty();
                list->push_back(model);
                more = rs.moveNext();
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("loadTasks");
}

error Database::loadTags(
    const Poco::UInt64 &UID,
    std::vector<Tag *> *list) {

    if (!UID) {
        return error("Cannot load user tags without an user ID");
    }

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(list);

    list->clear();

    try {
        Poco::Data::Statement select(*session_);
        select << "SELECT local_id, id, uid, name, wid, guid "
               "FROM tags "
               "WHERE uid = :uid "
               "ORDER BY name",
               useRef(UID);
        error err = last_error("loadTags");
        if (err != noError) {
            return err;
        }
        Poco::Data::RecordSet rs(select);
        while (!select.done()) {
            select.execute();
            bool more = rs.moveFirst();
            while (more) {
                Tag *model = new Tag();
                model->SetLocalID(rs[0].convert<Poco::Int64>());
                if (rs[1].isEmpty()) {
                    model->SetID(0);
                } else {
                    model->SetID(rs[1].convert<Poco::UInt64>());
                }
                model->SetUID(rs[2].convert<Poco::UInt64>());
                model->SetName(rs[3].convert<std::string>());
                model->SetWID(rs[4].convert<Poco::UInt64>());
                if (rs[5].isEmpty()) {
                    model->SetGUID("");
                } else {
                    model->SetGUID(rs[5].convert<std::string>());
                }
                model->ClearDirty();
                list->push_back(model);
                more = rs.moveNext();
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("loadTags");
}

error Database::loadTimeEntries(
    const Poco::UInt64 &UID,
    std::vector<TimeEntry *> *list) {

    if (!UID) {
        return error("Cannot load user time entries without an user ID");
    }

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(list);

    list->clear();

    try {
        Poco::Data::Statement select(*session_);
        select << "SELECT local_id, id, uid, description, wid, guid, pid, "
               "tid, billable, duronly, ui_modified_at, start, stop, "
               "duration, tags, created_with, deleted_at, updated_at, "
               "project_guid, validation_error "
               "FROM time_entries "
               "WHERE uid = :uid "
               "ORDER BY start DESC",
               useRef(UID);
        error err = last_error("loadTimeEntries");
        if (err != noError) {
            return err;
        }
        err = loadTimeEntriesFromSQLStatement(&select, list);
        if (err != noError) {
            return err;
        }

        // Ensure all time entries have a GUID.
        for (std::vector<TimeEntry *>::iterator it = list->begin();
                it != list->end();
                ++it) {
            TimeEntry *te = *it;
            te->EnsureGUID();
            if (te->Dirty()) {
                te->SetUIModified();
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Database::loadTimeEntriesFromSQLStatement(
    Poco::Data::Statement *select,
    std::vector<TimeEntry *> *list) {

    poco_check_ptr(select);
    poco_check_ptr(list);

    try {
        Poco::Data::RecordSet rs(*select);
        while (!select->done()) {
            select->execute();
            bool more = rs.moveFirst();
            while (more) {
                TimeEntry *model = new TimeEntry();
                model->SetLocalID(rs[0].convert<Poco::Int64>());
                if (rs[1].isEmpty()) {
                    model->SetID(0);
                } else {
                    model->SetID(rs[1].convert<Poco::UInt64>());
                }
                model->SetUID(rs[2].convert<Poco::UInt64>());
                if (rs[3].isEmpty()) {
                    model->SetDescription("");
                } else {
                    model->SetDescription(rs[3].convert<std::string>());
                }
                model->SetWID(rs[4].convert<Poco::UInt64>());
                model->SetGUID(rs[5].convert<std::string>());
                if (rs[6].isEmpty()) {
                    model->SetPID(0);
                } else {
                    model->SetPID(rs[6].convert<Poco::UInt64>());
                }
                if (rs[7].isEmpty()) {
                    model->SetTID(0);
                } else {
                    model->SetTID(rs[7].convert<Poco::UInt64>());
                }
                model->SetBillable(rs[8].convert<bool>());
                model->SetDurOnly(rs[9].convert<bool>());
                if (rs[10].isEmpty()) {
                    model->SetUIModifiedAt(0);
                } else {
                    model->SetUIModifiedAt(rs[10].convert<Poco::UInt64>());
                }
                model->SetStart(rs[11].convert<Poco::UInt64>());
                if (rs[12].isEmpty()) {
                    model->SetStop(0);
                } else {
                    model->SetStop(rs[12].convert<Poco::UInt64>());
                }
                model->SetDurationInSeconds(rs[13].convert<Poco::Int64>());
                if (rs[14].isEmpty()) {
                    model->SetTags("");
                } else {
                    model->SetTags(rs[14].convert<std::string>());
                }
                if (rs[15].isEmpty()) {
                    model->SetCreatedWith("");
                } else {
                    model->SetCreatedWith(rs[15].convert<std::string>());
                }
                if (rs[16].isEmpty()) {
                    model->SetDeletedAt(0);
                } else {
                    model->SetDeletedAt(rs[16].convert<Poco::UInt64>());
                }
                if (rs[17].isEmpty()) {
                    model->SetUpdatedAt(0);
                } else {
                    model->SetUpdatedAt(rs[17].convert<Poco::UInt64>());
                }
                if (rs[18].isEmpty()) {
                    model->SetProjectGUID("");
                } else {
                    model->SetProjectGUID(rs[18].convert<std::string>());
                }
                if (rs[19].isEmpty()) {
                    model->SetValidationError("");
                } else {
                    model->SetValidationError(rs[19].convert<std::string>());
                }
                model->ClearDirty();
                list->push_back(model);
                more = rs.moveNext();
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

template <typename T>
error Database::saveRelatedModels(
    const Poco::UInt64 UID,
    const std::string table_name,
    std::vector<T *> *list,
    std::vector<ModelChange> *changes) {

    if (!UID) {
        return error("Cannot save user related data without an user ID");
    }

    poco_check_ptr(list);
    poco_check_ptr(changes);

    typedef typename std::vector<T *>::iterator iterator;

    for (size_t i = 0; i < list->size(); i++) {
        T *model = list->at(i);
        if (model->IsMarkedAsDeletedOnServer()) {
            error err = deleteFromTable(table_name, model->LocalID());
            if (err != noError) {
                return err;
            }
            changes->push_back(ModelChange(
                model->ModelName(),
                "delete",
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
    iterator it = list->begin();
    while (it != list->end()) {
        T *model = *it;
        if (model->IsMarkedAsDeletedOnServer()) {
            it = list->erase(it);
        } else {
            ++it;
        }
    }

    return noError;
}

typedef toggl::error (Database::*saveModel)(
    BaseModel *model, std::vector<ModelChange> *changes);

error Database::saveModel(
    TimeEntry *model,
    std::vector<ModelChange> *changes) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(model);
    poco_check_ptr(session_);
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

    try {
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating time entry " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().debug(ss.str());

            if (model->ID()) {
                *session_ << "update time_entries set "
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
                *session_ << "update time_entries set "
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
                    model->ModelName(), "delete", model->ID(), model->GUID()));
            } else {
                changes->push_back(ModelChange(
                    model->ModelName(), "update", model->ID(), model->GUID()));
            }
        } else {
            std::stringstream ss;
            ss << "Inserting time entry " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().debug(ss.str());
            if (model->ID()) {
                *session_ << "insert into time_entries(id, uid, description, "
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
                *session_ << "insert into time_entries(uid, description, wid, "
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
            *session_ << "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveTimeEntry");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(), "insert", model->ID(), model->GUID()));
        }
        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    Workspace *model,
    std::vector<ModelChange> *changes) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(model);
    poco_check_ptr(session_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }

    try {
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating workspace " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().trace(ss.str());

            *session_ << "update workspaces set "
                      "id = :id, uid = :uid, name = :name, premium = :premium, "
                      "only_admins_may_create_projects = "
                      ":only_admins_may_create_projects, admin = :admin "
                      "where local_id = :local_id",
                      useRef(model->ID()),
                      useRef(model->UID()),
                      useRef(model->Name()),
                      useRef(model->Premium()),
                      useRef(model->OnlyAdminsMayCreateProjects()),
                      useRef(model->Admin()),
                      useRef(model->LocalID()),
                      now;
            error err = last_error("saveWorkspace");
            if (err != noError) {
                return err;
            }
            changes->push_back(ModelChange(
                model->ModelName(), "update", model->ID(), ""));

        } else {
            std::stringstream ss;
            ss << "Inserting workspace " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().trace(ss.str());
            *session_ <<
                      "insert into workspaces(id, uid, name, premium, "
                      "only_admins_may_create_projects, admin) "
                      "values(:id, :uid, :name, :premium, "
                      ":only_admins_may_create_projects, :admin)",
                      useRef(model->ID()),
                      useRef(model->UID()),
                      useRef(model->Name()),
                      useRef(model->Premium()),
                      useRef(model->OnlyAdminsMayCreateProjects()),
                      useRef(model->Admin()),
                      now;
            error err = last_error("saveWorkspace");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ << "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveWorkspace");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(), "insert", model->ID(), ""));
        }
        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    Client *model,
    std::vector<ModelChange> *changes) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(model);
    poco_check_ptr(session_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }

    try {
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating client " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().trace(ss.str());

            if (model->GUID().empty()) {
                *session_ << "update clients set "
                          "id = :id, uid = :uid, name = :name, wid = :wid "
                          "where local_id = :local_id",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Name()),
                          useRef(model->WID()),
                          useRef(model->LocalID()),
                          now;
            } else {
                *session_ << "update clients set "
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
                model->ModelName(), "update", model->ID(), model->GUID()));

        } else {
            std::stringstream ss;
            ss << "Inserting client " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().trace(ss.str());
            if (model->GUID().empty()) {
                *session_ << "insert into clients(id, uid, name, wid) "
                          "values(:id, :uid, :name, :wid)",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Name()),
                          useRef(model->WID()),
                          now;
            } else {
                *session_ << "insert into clients(id, uid, name, guid, wid) "
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
            *session_ << "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveClient");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(), "insert", model->ID(), model->GUID()));
        }
        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    Project *model,
    std::vector<ModelChange> *changes) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(model);
    poco_check_ptr(session_);

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

    try {
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating project " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().debug(ss.str());

            if (model->ID()) {
                if (model->GUID().empty()) {
                    *session_ << "update projects set "
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
                    *session_ << "update projects set "
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
                    *session_ << "update projects set "
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
                    *session_ << "update projects set "
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
                model->ModelName(), "update", model->ID(), model->GUID()));

        } else {
            std::stringstream ss;
            ss << "Inserting project " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().debug(ss.str());
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
            *session_ << "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveProject");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(), "insert", model->ID(), model->GUID()));
        }
        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    Task *model,
    std::vector<ModelChange> *changes) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(model);
    poco_check_ptr(session_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }

    try {
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating task " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().trace(ss.str());

            *session_ << "update tasks set "
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
                model->ModelName(), "update", model->ID(), ""));

        } else {
            std::stringstream ss;
            ss << "Inserting task " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().trace(ss.str());
            *session_ << "insert into tasks(id, uid, name, wid, pid, active) "
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
            *session_ << "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveTask");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(), "insert", model->ID(), ""));
        }
        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Database::saveModel(
    Tag *model,
    std::vector<ModelChange> *changes    ) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(model);
    poco_check_ptr(session_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }

    try {
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating tag " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().trace(ss.str());

            if (model->GUID().empty()) {
                *session_ << "update tags set "
                          "id = :id, uid = :uid, name = :name, wid = :wid "
                          "where local_id = :local_id",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Name()),
                          useRef(model->WID()),
                          useRef(model->LocalID()),
                          now;
            } else {
                *session_ << "update tags set "
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
                model->ModelName(), "update", model->ID(), model->GUID()));

        } else {
            std::stringstream ss;
            ss << "Inserting tag " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().trace(ss.str());
            if (model->GUID().empty()) {
                *session_ << "insert into tags(id, uid, name, wid) "
                          "values(:id, :uid, :name, :wid)",
                          useRef(model->ID()),
                          useRef(model->UID()),
                          useRef(model->Name()),
                          useRef(model->WID()),
                          now;
            } else {
                *session_ << "insert into tags(id, uid, name, wid, guid) "
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
            *session_ << "select last_insert_rowid()",
                      into(local_id),
                      now;
            err = last_error("saveTag");
            if (err != noError) {
                return err;
            }
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
                model->ModelName(), "insert", model->ID(), model->GUID()));
        }
        model->ClearDirty();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Database::SaveUser(
    User *user,
    const bool with_related_data,
    std::vector<ModelChange> *changes) {

    Poco::Mutex::ScopedLock lock(session_m_);

    // Do nothing, if user has already logged out
    if (!user) {
        logger().warning("Cannot save user, user is logged out");
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
                std::stringstream ss;
                ss << "Updating user " + user->String()
                   << " in thread " << Poco::Thread::currentTid();
                logger().trace(ss.str());

                *session_ << "update users set "
                          "default_wid = :default_wid, "
                          "since = :since, id = :id, fullname = :fullname, "
                          "email = :email, record_timeline = :record_timeline, "
                          "store_start_and_stop_time = "
                          " :store_start_and_stop_time, "
                          "timeofday_format = :timeofday_format, "
                          "duration_format = :duration_format, "
                          "offline_data = :offline_data "
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
                          useRef(user->LocalID()),
                          now;
                error err = last_error("SaveUser");
                if (err != noError) {
                    session_->rollback();
                    return err;
                }
                changes->push_back(ModelChange(
                    user->ModelName(), "update", user->ID(), ""));
            } else {
                std::stringstream ss;
                ss << "Inserting user " + user->String()
                   << " in thread " << Poco::Thread::currentTid();
                logger().trace(ss.str());
                *session_ << "insert into users("
                          "id, default_wid, since, fullname, email, "
                          "record_timeline, store_start_and_stop_time, "
                          "timeofday_format, duration_format, offline_data "
                          ") values("
                          ":id, :default_wid, :since, :fullname, "
                          ":email, "
                          ":record_timeline, :store_start_and_stop_time, "
                          ":timeofday_format, :duration_format, :offline_data "
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
                          now;
                error err = last_error("SaveUser");
                if (err != noError) {
                    session_->rollback();
                    return err;
                }
                Poco::Int64 local_id(0);
                *session_ << "select last_insert_rowid()",
                          into(local_id),
                          now;
                err = last_error("SaveUser");
                if (err != noError) {
                    session_->rollback();
                    return err;
                }
                user->SetLocalID(local_id);
                changes->push_back(ModelChange(
                    user->ModelName(), "insert", user->ID(), ""));
            }
            user->ClearDirty();
        } catch(const Poco::Exception& exc) {
            session_->rollback();
            return exc.displayText();
        } catch(const std::exception& ex) {
            session_->rollback();
            return ex.what();
        } catch(const std::string& ex) {
            session_->rollback();
            return ex;
        }
    }

    if (with_related_data) {
        // Workspaces
        std::vector<ModelChange> workspace_changes;
        error err = saveRelatedModels(user->ID(),
                                      "workspaces",
                                      &user->related.Workspaces,
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
                                &user->related.Clients,
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
                                &user->related.Projects,
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
                                &user->related.Tasks,
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

        err = saveRelatedModels(user->ID(),
                                "tags",
                                &user->related.Tags,
                                changes);
        if (err != noError) {
            session_->rollback();
            return err;
        }

        err = saveRelatedModels(user->ID(),
                                "time_entries",
                                &user->related.TimeEntries,
                                changes);
        if (err != noError) {
            session_->rollback();
            return err;
        }
    }

    session_->commit();

    stopwatch.stop();

    {
        std::stringstream ss;
        ss  << "User with_related_data=" << with_related_data << " saved in "
            << stopwatch.elapsed() / 1000 << " ms in thread "
            << Poco::Thread::currentTid();
        logger().debug(ss.str());
    }

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

    err = migrateUsers();
    if (err != noError) {
        return err;
    }

    err = migrateWorkspaces();
    if (err != noError) {
        return err;
    }

    err = migrateClients();
    if (err != noError) {
        return err;
    }

    err = migrateProjects();
    if (err != noError) {
        return err;
    }

    err = migrateTasks();
    if (err != noError) {
        return err;
    }

    err = migrateTags();
    if (err != noError) {
        return err;
    }

    err = migrateTimeEntries();
    if (err != noError) {
        return err;
    }

    err = migrateSessions();
    if (err != noError) {
        return err;
    }

    err = migrateSettings();
    if (err != noError) {
        return err;
    }

    err = migrateTimeline();
    if (err != noError) {
        return err;
    }

    err = migrateAnalytics();
    if (err != noError) {
        return err;
    }

    return noError;
}

error Database::migrateClients() {
    error err = migrate(
        "clients",
        "create table clients("
        "local_id integer primary key,"
        "id integer, "  // ID can be null when its not pushed to server yet
        "uid integer not null, "
        "name varchar not null, "
        "guid varchar, "
        "wid integer not null, "
        "constraint fk_clients_wid foreign key (wid) "
        "   references workpaces(id) on delete no action on update no action,"
        "constraint fk_clients_uid foreign key (uid) "
        "   references users(id) on delete no action on update no action"
        "); ");
    if (err != noError) {
        return err;
    }

    err = migrate("clients.id",
                  "CREATE UNIQUE INDEX id_clients_id ON clients (uid, id); ");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "clients.guid",
        "CREATE UNIQUE INDEX id_clients_guid ON clients (uid, guid);");
    if (err != noError) {
        return err;
    }

    return err;
}

error Database::migrateTasks() {
    error err = migrate(
        "tasks",
        "create table tasks("
        "local_id integer primary key, "
        "id integer not null, "
        "uid integer not null, "
        "name varchar not null, "
        "wid integer not null, "
        "pid integer, "
        "constraint fk_tasks_wid foreign key (wid) "
        "   references workpaces(id) on delete no action on update no action, "
        "constraint fk_tasks_pid foreign key (pid) "
        "   references projects(id) on delete no action on update no action, "
        "constraint fk_tasks_uid foreign key (uid) "
        "   references users(id) on delete no action on update no action "
        "); ");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "tasks.id",
        "CREATE UNIQUE INDEX id_tasks_id ON tasks (uid, id);");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "tasks.active",
        "alter table tasks add column active integer not null default 1;");
    if (err != noError) {
        return err;
    }

    return noError;
}

error Database::migrateTags() {
    error err = migrate(
        "tags",
        "create table tags("
        "local_id integer primary key, "
        "id integer not null, "
        "uid integer not null, "
        "name varchar not null, "
        "wid integer not null, "
        "guid varchar, "
        "constraint fk_tags_wid foreign key (wid) "
        "   references workspaces(id) on delete no action on update no action,"
        "constraint fk_tags_uid foreign key (uid) "
        "   references users(id) on delete no action on update no action"
        "); ");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "tags.id",
        "CREATE UNIQUE INDEX id_tags_id ON tags (uid, id); ");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "tags.guid",
        "CREATE UNIQUE INDEX id_tags_guid ON tags (uid, guid); ");
    if (err != noError) {
        return err;
    }

    return noError;
}

error Database::migrateSessions() {
    error err = migrate("sessions",
                        "create table sessions("
                        "local_id integer primary key, "
                        "api_token varchar not null, "
                        "active integer not null default 1 "
                        "); ");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "sessions.active",
        "CREATE UNIQUE INDEX id_sessions_active ON sessions (active); ");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "sessions.uid",
        "alter table sessions add column uid integer references users (id);");
    if (err != noError) {
        return err;
    }

    return err;
}

error Database::migrateWorkspaces() {
    error err = migrate(
        "workspaces",
        "create table workspaces("
        "local_id integer primary key,"
        "id integer not null, "
        "uid integer not null, "
        "name varchar not null, "
        "constraint fk_workspaces_uid foreign key (uid) "
        "   references users(id) "
        "     on delete no action on update no action"
        "); ");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "workspaces.id",
        "CREATE UNIQUE INDEX id_workspaces_id ON workspaces (uid, id);");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "workspaces.premium",
        "alter table workspaces add column premium int default 0;");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "workspaces.only_admins_may_create_projects",
        "alter table workspaces add column "
        "   only_admins_may_create_projects integer not null default 0; ");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "workspaces.admin",
        "alter table workspaces add column "
        "   admin integer not null default 0; ");
    if (err != noError) {
        return err;
    }

    return err;
}

error Database::migrateProjects() {
    error err = migrate(
        "projects",
        "create table projects("
        "local_id integer primary key, "
        "id integer, "  // project ID can be null, when its created client side
        "uid integer not null, "
        "name varchar not null, "
        "guid varchar, "
        "color varchar, "
        "wid integer not null, "
        "cid integer, "
        "active integer not null default 1,"
        "constraint fk_projects_wid foreign key (wid) "
        "   references workpaces(id) on delete no action on update no action,"
        "constraint fk_projects_cid foreign key (cid) "
        "   references clients(id) on delete no action on update no action,"
        "constraint fk_projects_uid foreign key (uid) "
        "   references users(id) ON DELETE NO ACTION ON UPDATE NO ACTION"
        "); ");
    if (err != noError) {
        return err;
    }

    err = migrate("projects.billable",
                  "ALTER TABLE projects ADD billable INT NOT NULL DEFAULT 0");
    if (err != noError) {
        return err;
    }

    err = migrate("projects.is_private",
                  "ALTER TABLE projects ADD is_private INT NOT NULL DEFAULT 0");
    if (err != noError) {
        return err;
    }

    err = migrate("projects.client_guid",
                  "ALTER TABLE projects "
                  "ADD COLUMN client_guid VARCHAR;");
    if (err != noError) {
        return err;
    }

    err = migrate("projects.id",
                  "CREATE UNIQUE INDEX id_projects_id ON projects (uid, id);");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "projects.guid",
        "CREATE UNIQUE INDEX id_projects_guid ON projects (uid, guid);");
    if (err != noError) {
        return err;
    }

    return err;
}

error Database::migrateAnalytics() {
    error err = migrate("analytics_settings",
                        "CREATE TABLE analytics_settings("
                        "id INTEGER PRIMARY KEY, "
                        "analytics_client_id VARCHAR NOT NULL"
                        ")");
    if (err != noError) {
        return err;
    }

    err = migrate("analytics_settings.analytics_client_id",
                  "CREATE UNIQUE INDEX id_analytics_settings_client_id "
                  "ON analytics_settings(analytics_client_id);");
    if (err != noError) {
        return err;
    }

    err = String("SELECT analytics_client_id FROM analytics_settings LIMIT 1",
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

error Database::migrateTimeline() {
    error err = migrate("timeline_installation",
                        "CREATE TABLE timeline_installation("
                        "id INTEGER PRIMARY KEY, "
                        "desktop_id VARCHAR NOT NULL"
                        ")");
    if (err != noError) {
        return err;
    }

    err = migrate("timeline_installation.desktop_id",
                  "CREATE UNIQUE INDEX id_timeline_installation_desktop_id "
                  "ON timeline_installation(desktop_id);");
    if (err != noError) {
        return err;
    }

    err = migrate("timeline_events",
                  "CREATE TABLE timeline_events("
                  "id INTEGER PRIMARY KEY, "
                  "user_id INTEGER NOT NULL, "
                  "title VARCHAR, "
                  "filename VARCHAR, "
                  "start_time INTEGER NOT NULL, "
                  "end_time INTEGER, "
                  "idle INTEGER NOT NULL"
                  ")");
    if (err != noError) {
        return err;
    }

    err = String("SELECT desktop_id FROM timeline_installation LIMIT 1",
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

    err = migrate(
        "timeline_events.chunked",
        "alter table timeline_events"
        "   add column chunked integer not null default 0;");
    if (err != noError) {
        return err;
    }

    return noError;
}

error Database::migrateUsers() {
    error err = migrate("users",
                        "create table users("
                        "local_id integer primary key, "
                        "id integer not null, "
                        "api_token varchar not null, "
                        "default_wid integer, "
                        "since integer, "
                        "fullname varchar, "
                        "email varchar not null, "
                        "record_timeline integer not null default 0"
                        "); ");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "users.store_start_and_stop_time",
        "ALTER TABLE users "
        "ADD COLUMN store_start_and_stop_time INT NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "users.timeofday_format",
        "ALTER TABLE users "
        "ADD COLUMN timeofday_format varchar NOT NULL DEFAULT 'HH:mm';");
    if (err != noError) {
        return err;
    }

    err = migrate("users.id",
                  "CREATE UNIQUE INDEX id_users_id ON users (id);");
    if (err != noError) {
        return err;
    }

    err = migrate("users.duration_format",
                  "alter table users "
                  "add column duration_format varchar "
                  "not null default 'classic';");
    if (err != noError) {
        return err;
    }

    err = migrate("drop users.email index",
                  "DROP INDEX IF EXISTS id_users_email;");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "users.api_token",
        "CREATE UNIQUE INDEX id_users_api_token ON users (api_token);");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "users.offline_data",
        "alter table users"
        " add column offline_data varchar");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "no api token step #1",
        "ALTER TABLE users RENAME TO tmp_users");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "no api token step #2",
        "create table users("
        "   local_id integer primary key, "
        "   id integer not null unique, "
        "   default_wid integer, "
        "   since integer, "
        "   fullname varchar, "
        "   email varchar not null, "
        "   record_timeline integer not null default 0, "
        "   store_start_and_stop_time INT NOT NULL DEFAULT 0, "
        "   timeofday_format varchar NOT NULL DEFAULT 'HH:mm', "
        "   duration_format varchar not null default 'classic', "
        "   offline_data varchar"
        ")");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "no api token step #3",
        "insert into users"
        " select local_id, id, default_wid, since, fullname, email,"
        " record_timeline, store_start_and_stop_time, timeofday_format,"
        " duration_format, offline_data"
        " from tmp_users");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "no api token step #4",
        "drop table tmp_users");
    if (err != noError) {
        return err;
    }

    return err;
}

error Database::migrateTimeEntries() {
    error err = migrate(
        "time_entries",
        "create table time_entries("
        "local_id integer primary key, "
        "id integer, "  // ID can be null when its not pushed to server yet
        "uid integer not null, "
        "description varchar, "
        "wid integer not null, "
        "guid varchar, "
        "pid integer, "
        "tid integer, "
        "billable integer not null default 0,"
        "duronly integer not null default 0, "
        "ui_modified_at integer, "
        "start integer not null, "
        "stop integer, "
        "duration integer not null,"
        "tags text,"
        "created_with varchar,"
        "deleted_at integer,"
        "updated_at integer,"
        "constraint fk_time_entries_wid foreign key (wid) "
        "   references workspaces(id) on delete no action on update no action, "
        "constraint fk_time_entries_pid foreign key (pid) "
        "   references projects(id) on delete no action on update no action, "
        "constraint fk_time_entries_tid foreign key (tid) "
        "   references tasks(id) on delete no action on update no action, "
        "constraint fk_time_entries_uid foreign key (uid) "
        "   references users(id) on delete no action on update no action"
        "); ");
    if (err != noError) {
        return err;
    }

    err = migrate("time_entries.id",
                  "CREATE UNIQUE INDEX id_time_entries_id "
                  "ON time_entries (uid, id); ");
    if (err != noError) {
        return err;
    }

    err = migrate("time_entries.guid",
                  "CREATE UNIQUE INDEX id_time_entries_guid "
                  "ON time_entries (uid, guid); ");
    if (err != noError) {
        return err;
    }

    err = migrate("time_entries.project_guid",
                  "ALTER TABLE time_entries "
                  "ADD COLUMN project_guid VARCHAR;");
    if (err != noError) {
        return err;
    }

    err = migrate("time_entries.guid not null, step 1",
                  "ALTER TABLE time_entries RENAME TO tmp_time_entries; ");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "time_entries.guid not null, step 2",
        "create table time_entries("
        "local_id integer primary key, "
        "id integer, "
        "uid integer not null, "
        "description varchar, "
        "wid integer not null, "
        "guid varchar NOT NULL, "
        "pid integer, "
        "tid integer, "
        "billable integer not null default 0,"
        "duronly integer not null default 0, "
        "ui_modified_at integer, "
        "start integer not null, "
        "stop integer, "
        "duration integer not null,"
        "tags text,"
        "created_with varchar,"
        "deleted_at integer,"
        "updated_at integer,"
        "project_guid VARCHAR,"
        "constraint fk_time_entries_wid foreign key (wid) "
        "   references workspaces(id) on delete no action on update no action, "
        "constraint fk_time_entries_pid foreign key (pid) "
        "   references projects(id) on delete no action on update no action, "
        "constraint fk_time_entries_tid foreign key (tid) "
        "   references tasks(id) on delete no action on update no action, "
        "constraint fk_time_entries_uid foreign key (uid) "
        "   references users(id) on delete no action on update no action"
        "); ");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "time_entries.guid not null, step 3",
        "insert into time_entries("
        "   local_id, id, uid, description, wid, guid, pid, tid, billable, "
        "   duronly, ui_modified_at, start, stop, duration, tags, "
        "   created_with, deleted_at, updated_at, project_guid) "
        "select "
        "   local_id, id, uid, description, wid, guid, pid, tid, billable, "
        "   duronly, ui_modified_at, start, stop, duration, tags, "
        "   created_with, deleted_at, updated_at, project_guid "
        "from tmp_time_entries;");
    if (err != noError) {
        return err;
    }

    err = migrate("time_entries.guid not null, step 4",
                  "drop table tmp_time_entries;");
    if (err != noError) {
        return err;
    }

    err = migrate("time_entries.guid not null, step 5",
                  "CREATE UNIQUE INDEX id_time_entries_id "
                  "   ON time_entries (uid, id); ");
    if (err != noError) {
        return err;
    }

    err = migrate("time_entries.guid not null, step 6",
                  "CREATE UNIQUE INDEX id_time_entries_guid "
                  "   ON time_entries (uid, guid); ");
    if (err != noError) {
        return err;
    }

    err = migrate("time_entries.validation_error",
                  "ALTER TABLE time_entries "
                  "ADD COLUMN validation_error VARCHAR;");
    if (err != noError) {
        return err;
    }

    return noError;
}

error Database::migrateSettings() {
    error err = migrate(
        "settings",
        "create table settings("
        "local_id integer primary key, "
        "use_proxy integer not null default 0, "
        "proxy_host varchar, "
        "proxy_port integer, "
        "proxy_username varchar, "
        "proxy_password varchar, "
        "use_idle_detection integer not null default 1)");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "settings.update_channel",
        "ALTER TABLE settings "
        "ADD COLUMN update_channel varchar not null default 'stable';");
    if (err != noError) {
        return err;
    }

    // for 5% users, set the update channel to 'beta' instead of 'stable'
    Poco::UInt64 has_settings(0);
    err = UInt("select count(1) from settings", &has_settings);
    if (err != noError) {
        return err;
    }
    if (!has_settings) {
        Poco::Random random;
        random.seed();
        std::string channel("stable");
        Poco::UInt32 r = random.next(100);
        if (r < kBetaChannelPercentage) {
            channel = "beta";
        }
        err = migrate(
            "settings.default",
            "INSERT INTO settings(update_channel) VALUES('" + channel + "')");
        if (err != noError) {
            return err;
        }
    }

    err = migrate("settings.menubar_timer",
                  "ALTER TABLE settings "
                  "ADD COLUMN menubar_timer integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = migrate("settings.menubar_project",
                  "ALTER TABLE settings "
                  "ADD COLUMN menubar_project integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = migrate("settings.dock_icon",
                  "ALTER TABLE settings "
                  "ADD COLUMN dock_icon INTEGER NOT NULL DEFAULT 1;");
    if (err != noError) {
        return err;
    }

    err = migrate("settings.on_top",
                  "ALTER TABLE settings "
                  "ADD COLUMN on_top INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = migrate("settings.reminder",
                  "ALTER TABLE settings "
                  "ADD COLUMN reminder INTEGER NOT NULL DEFAULT 1;");
    if (err != noError) {
        return err;
    }

    err = migrate("settings.ignore_cert",
                  "ALTER TABLE settings "
                  "ADD COLUMN ignore_cert INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = migrate("settings.idle_minutes",
                  "ALTER TABLE settings "
                  "ADD COLUMN idle_minutes INTEGER NOT NULL DEFAULT 5;");
    if (err != noError) {
        return err;
    }

    err = migrate("settings.focus_on_shortcut",
                  "ALTER TABLE settings "
                  "ADD COLUMN focus_on_shortcut INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = migrate("settings.reminder_minutes",
                  "ALTER TABLE settings "
                  "ADD COLUMN reminder_minutes INTEGER NOT NULL DEFAULT 10;");
    if (err != noError) {
        return err;
    }

    err = migrate("settings.manual_mode",
                  "ALTER TABLE settings "
                  "ADD COLUMN manual_mode INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "focus on shortcut by default #1",
        "ALTER TABLE settings RENAME TO tmp_settings");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "focus on shortcut by default #2",
        "create table settings("
        "   local_id integer primary key, "
        "   use_proxy integer not null default 0, "
        "   proxy_host varchar, "
        "   proxy_port integer, "
        "   proxy_username varchar, "
        "   proxy_password varchar, "
        "   use_idle_detection integer not null default 1, "
        "   update_channel varchar not null default 'stable', "
        "   menubar_timer integer not null default 0, "
        "   menubar_project integer not null default 0, "
        "   dock_icon INTEGER NOT NULL DEFAULT 1, "
        "   on_top INTEGER NOT NULL DEFAULT 0, "
        "   reminder INTEGER NOT NULL DEFAULT 1, "
        "   ignore_cert INTEGER NOT NULL DEFAULT 0, "
        "   idle_minutes INTEGER NOT NULL DEFAULT 5, "
        "   focus_on_shortcut INTEGER NOT NULL DEFAULT 1, "
        "   reminder_minutes INTEGER NOT NULL DEFAULT 10, "
        "   manual_mode INTEGER NOT NULL DEFAULT 0 "
        ")");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "focus on shortcut by default #3",
        "insert into settings"
        " select local_id, use_proxy, "
        " proxy_host, proxy_port, proxy_username, proxy_password, "
        " use_idle_detection, update_channel, menubar_timer, menubar_project, "
        " dock_icon, on_top, reminder, ignore_cert, idle_minutes, "
        " focus_on_shortcut, reminder_minutes, manual_mode "
        " from tmp_settings");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "focus on shortcut by default #4",
        "drop table tmp_settings");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "focus on shortcut by default #5",
        "update settings set focus_on_shortcut = 1");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "settings.autodetect_proxy",
        "ALTER TABLE settings "
        "ADD COLUMN autodetect_proxy INTEGER NOT NULL DEFAULT 1;");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "settings.window_x",
        "ALTER TABLE settings "
        "ADD COLUMN window_x integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "settings.window_y",
        "ALTER TABLE settings "
        "ADD COLUMN window_y integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "settings.window_height",
        "ALTER TABLE settings "
        "ADD COLUMN window_height integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = migrate(
        "settings.window_width",
        "ALTER TABLE settings "
        "ADD COLUMN window_width integer not null default 0;");
    if (err != noError) {
        return err;
    }

    return noError;
}

error Database::CurrentAPIToken(
    std::string *token,
    Poco::UInt64 *uid) {

    poco_check_ptr(token);
    poco_check_ptr(uid);

    poco_check_ptr(session_);
    Poco::Mutex::ScopedLock lock(session_m_);

    *token = "";
    *uid = 0;

    try {
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
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("CurrentAPIToken");
}

error Database::ClearCurrentAPIToken() {
    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    try {
        *session_ << "delete from sessions", now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("ClearCurrentAPIToken");
}

error Database::SetCurrentAPIToken(
    const std::string &token,
    const Poco::UInt64 &uid) {
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
    try {
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
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("SetCurrentAPIToken");
}

error Database::saveDesktopID() {
    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    try {
        *session_ << "INSERT INTO timeline_installation(desktop_id) "
                  "VALUES(:desktop_id)",
                  useRef(desktop_id_),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("saveDesktopID");
}

error Database::saveAnalyticsClientID() {
    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    try {
        *session_ << "INSERT INTO analytics_settings(analytics_client_id) "
                  "VALUES(:analytics_client_id)",
                  useRef(analytics_client_id_),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("saveAnalyticsClientID");
}

error Database::migrate(
    const std::string &name,
    const std::string sql) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    if (name.empty()) {
        return error("Cannot run a migration without name");
    }
    if (sql.empty()) {
        return error("Cannot run a migration without SQL");
    }

    try {
        int count = 0;
        *session_ << "select count(*) from kopsik_migrations where name=:name",
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

        std::stringstream ss;
        ss  << "Migrating" << "\n"
            << name << "\n"
            << sql << "\n";
        logger().debug(ss.str());

        err = execute(sql);
        if (err != noError) {
            return err;
        }

        *session_ << "insert into kopsik_migrations(name) values(:name)",
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
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Database::execute(
    const std::string sql) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    if (sql.empty()) {
        return error("Cannot execute empty SQL");
    }

    try {
        *session_ << sql, now;
        error err = last_error("execute");
        if (err != noError) {
            return err;
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Database::deleteTooOldTimeline(
    const Poco::UInt64 &UID) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);

    if (!UID) {
        return error("Cannot delete old timeline without UID");
    }

    time_t minimum_time = time(0) - kTimelineSecondsToKeep;

    try {
        *session_ << "delete from timeline_events "
                  "where user_id = :uid "
                  "and start_time < :minimum_time",
                  useRef(UID),
                  useRef(minimum_time),
                  now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("deleteTooOldTimeline");
}

error Database::SelectTimelineBatch(
    const Poco::UInt64 &user_id,
    std::vector<TimelineEvent> *timeline_events) {

    error err = deleteTooOldTimeline(user_id);
    if (err != noError) {
        return err;
    }

    // Load all uncompressed timeline events into memory
    std::vector<TimelineEvent> uncompressed;
    err = selectUnompressedTimelineEvents(user_id, &uncompressed);
    if (err != noError) {
        return err;
    }

    // Group events by app name into chunks
    std::map<std::string, TimelineEvent> compressed;
    for (std::vector<TimelineEvent>::iterator i = uncompressed.begin();
            i != uncompressed.end();
            ++i) {
        TimelineEvent &event = *i;
        poco_assert(!event.chunked);

        time_t chunk_start_time =
            (event.start_time / kTimelineChunkSeconds) * kTimelineChunkSeconds;

        std::stringstream ss;
        ss << event.filename;
        ss << "::";
        ss << event.title;
        ss << "::";
        ss << event.idle;
        ss << "::";
        ss << chunk_start_time;
        std::string key = ss.str();

        time_t duration = event.end_time - event.start_time;
        if (duration < 0) {
            duration = 0;
        }

        if (compressed.find(key) == compressed.end()) {
            TimelineEvent chunk;
            chunk.user_id = user_id;
            chunk.start_time = event.start_time;
            chunk.end_time = chunk.start_time + duration;
            chunk.filename = event.filename;
            chunk.title = event.title;
            chunk.idle = event.idle;
            chunk.chunked = true;
            compressed[key] = chunk;
        } else {
            TimelineEvent chunk = compressed[key];
            chunk.end_time = chunk.end_time + duration;
            compressed[key] = chunk;
        }
    }

    // Save the new, chunked events
    for (std::map<std::string, TimelineEvent>::iterator i = compressed.begin();
            i != compressed.end();
            ++i) {
        TimelineEvent &event = i->second;
        poco_assert(event.chunked);
        err = InsertTimelineEvent(&event);
        if (err != noError) {
            return err;
        }
    }

    // Delete the uncompressed events now
    err = DeleteTimelineBatch(uncompressed);
    if (err != noError) {
        return err;
    }

    err = selectCompressedTimelineBatch(
        user_id, timeline_events);
    if (err != noError) {
        return err;
    }

    return noError;
}

error Database::selectUnompressedTimelineEvents(
    const Poco::UInt64 &user_id,
    std::vector<TimelineEvent> *timeline_events) {

    if (!user_id) {
        return error("Cannot load timeline without a user ID");
    }

    if (!timeline_events->empty()) {
        return error("Timeline events already loaded");
    }

    if (!session_) {
        logger().warning("select_batch database is not open, ignoring request");
        return noError;
    }

    time_t chunk_up_to = time(0) - kTimelineChunkSeconds;

    {
        std::stringstream s;
        s << "selectUnompressedTimelineEvents user_id = "
          << user_id
          << ", chunk_up_to = " << chunk_up_to;
        logger().debug(s.str());
    }

    Poco::Mutex::ScopedLock lock(session_m_);

    try {
        Poco::Data::Statement select(*session_);
        select <<
               "SELECT id, title, filename, start_time, end_time, idle, "
               "chunked "
               "FROM timeline_events "
               "WHERE user_id = :user_id "
               "AND start_time < :seconds_ago "
               "AND NOT chunked ",
               useRef(user_id),
               useRef(chunk_up_to);
        loadTimelineEvents(user_id, &select, timeline_events);

        {
            std::stringstream s;
            s << "selectUnompressedTimelineEvents found "
              << timeline_events->size()
              << " events.";
            logger().debug(s.str());
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("selectUnompressedTimelineEvents");
}

error Database::selectCompressedTimelineBatch(
    const Poco::UInt64 &user_id,
    std::vector<TimelineEvent> *timeline_events) {

    if (!user_id) {
        return error("Cannot load timeline without a user ID");
    }

    if (!timeline_events->empty()) {
        return error("Timeline events already loaded");
    }

    if (!session_) {
        logger().warning("select_batch database is not open, ignoring request");
        return noError;
    }

    std::stringstream out;
    out << "selectCompressedTimelineBatch user_id = " << user_id;
    logger().debug(out.str());

    Poco::Mutex::ScopedLock lock(session_m_);

    try {
        Poco::Data::Statement select(*session_);
        select <<
               "SELECT id, title, filename, start_time, end_time, idle, "
               "chunked "
               "FROM timeline_events "
               "WHERE user_id = :user_id "
               "AND chunked "
               "LIMIT 100",
               useRef(user_id);
        loadTimelineEvents(user_id, &select, timeline_events);

        std::stringstream event_count;
        event_count << "selectCompressedTimelineBatch found "
                    << timeline_events->size()
                    << " events.";
        logger().debug(event_count.str());
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("selectCompressedTimelineBatch");
}

void loadTimelineEvents(
    const Poco::UInt64 &user_id,
    Poco::Data::Statement *select,
    std::vector<TimelineEvent> *timeline_events) {

    Poco::Data::RecordSet rs(*select);

    while (!select->done()) {
        select->execute();
        bool more = rs.moveFirst();
        while (more) {
            TimelineEvent event;
            event.id = rs[0].convert<unsigned int>();
            if (!rs[1].isEmpty()) {
                event.title = rs[1].convert<std::string>();
            }
            if (!rs[2].isEmpty()) {
                event.filename = rs[2].convert<std::string>();
            }
            event.start_time = rs[3].convert<int>();
            if (!rs[4].isEmpty()) {
                event.end_time = rs[4].convert<int>();
            }
            event.idle = rs[5].convert<bool>();
            event.user_id = static_cast<unsigned int>(user_id);
            event.chunked = rs[6].convert<bool>();
            timeline_events->push_back(event);
            more = rs.moveNext();
        }
    }
}

const int kMaxTimelineStringSize = 300;

error Database::InsertTimelineEvent(TimelineEvent *event) {
    Poco::Mutex::ScopedLock lock(session_m_);

    if (event->filename.length() > kMaxTimelineStringSize) {
        event->filename = event->filename.substr(0, kMaxTimelineStringSize);
    }
    if (event->title.length() > kMaxTimelineStringSize) {
        event->title = event->title.substr(0, kMaxTimelineStringSize);
    }

    std::stringstream out;
    out << "InsertTimelineEvent " << event->start_time
        << ";"
        << event->end_time
        << ";"
        << event->filename
        << ";"
        << event->title;
    logger().debug(out.str());

    if (!event->user_id) {
        return error("Cannot save timeline event without an user ID");
    }
    if (!event->start_time) {
        return error("Cannot save timeline event without start time");
    }
    if (!event->end_time) {
        return error("Cannot save timeline event without end time");
    }

    if (!session_) {
        logger().warning("InsertTimelineEvent db closed, ignoring request");
        return noError;
    }

    Poco::Int64 start_time(event->start_time);
    Poco::Int64 end_time(event->end_time);

    *session_ << "INSERT INTO timeline_events("
              "user_id, title, filename, start_time, end_time, idle, "
              "chunked"
              ") VALUES ("
              ":user_id, :title, :filename, :start_time, :end_time, :idle, "
              ":chunked"
              ")",
              useRef(event->user_id),
              useRef(event->title),
              useRef(event->filename),
              useRef(start_time),
              useRef(end_time),
              useRef(event->idle),
              useRef(event->chunked),
              now;
    return last_error("InsertTimelineEvent");
}

error Database::DeleteTimelineBatch(
    const std::vector<TimelineEvent> &timeline_events) {

    if (timeline_events.empty()) {
        return noError;
    }

    Poco::Mutex::ScopedLock lock(session_m_);

    std::stringstream out;
    out << "DeleteTimelineBatch " << timeline_events.size() << " events.";
    logger().debug(out.str());

    if (!session_) {
        logger().warning("DeleteTimelineBatch db closed, ignoring request");
        return noError;
    }
    std::vector<Poco::Int64> ids;
    for (std::vector<TimelineEvent>::const_iterator i = timeline_events.begin();
            i != timeline_events.end();
            ++i) {
        const TimelineEvent &event = *i;
        ids.push_back(event.id);
    }

    *session_ << "DELETE FROM timeline_events WHERE id = :id",
              useRef(ids),
              now;
    return last_error("DeleteTimelineBatch");
}

error Database::String(
    const std::string sql,
    std::string *result) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);
    poco_check_ptr(result);

    if (sql.empty()) {
        return error("Cannot select from database with empty SQL");
    }

    try {
        std::string value("");
        *session_ << sql,
        into(value),
        now;
        *result = value;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("String");
}

error Database::UInt(
    const std::string sql,
    Poco::UInt64 *result) {

    Poco::Mutex::ScopedLock lock(session_m_);

    poco_check_ptr(session_);
    poco_check_ptr(result);

    if (sql.empty()) {
        return error("Cannot select a numeric from database with empty SQL");
    }

    try {
        Poco::UInt64 value(0);
        *session_ << sql,
        into(value),
        now;
        *result = value;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("UInt");
}

}   // namespace toggl
