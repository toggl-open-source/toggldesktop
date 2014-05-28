// Copyright 2014 Toggl Desktop developers.

// All session access should be locked.

#include "./database.h"

#include <limits>
#include <string>
#include <vector>

#include "./user.h"

#include "Poco/Logger.h"
#include "Poco/UUID.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/Stopwatch.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/Binding.h"
#include "Poco/Data/SQLite/SessionImpl.h"
#include "Poco/Data/SQLite/Utility.h"

namespace kopsik {

Database::Database(const std::string db_path)
    : session_(0)
, desktop_id_("") {
    Poco::Data::SQLite::Connector::registerConnector();

    session_ = new Poco::Data::Session("SQLite", db_path);

    {
        int is_sqlite_threadsafe = sqlite3_threadsafe();

        std::stringstream ss;
        ss << "sqlite3_threadsafe()=" << is_sqlite_threadsafe;
        logger().debug(ss.str());

        poco_assert(is_sqlite_threadsafe);
    }

    error err = setJournalMode("wal");
    poco_assert(err == noError);
    {
        std::string mode("");
        error err = journalMode(&mode);
        poco_assert(err == noError);

        std::stringstream ss;
        ss << "PRAGMA journal_mode=" << mode;
        logger().debug(ss.str());

        poco_assert("wal" == mode);
    }

    err = initialize_tables();
    if (err != noError) {
        logger().error(err);
    }
    poco_assert(err == noError);

    Poco::NotificationCenter& nc =
        Poco::NotificationCenter::defaultCenter();

    Poco::Observer<Database, TimelineEventNotification>
    observeCreate(*this,
                  &Database::handleTimelineEventNotification);
    nc.addObserver(observeCreate);

    Poco::Observer<Database, CreateTimelineBatchNotification>
    observeSelect(*this,
                  &Database::handleCreateTimelineBatchNotification);
    nc.addObserver(observeSelect);

    Poco::Observer<Database, DeleteTimelineBatchNotification>
    observeDelete(*this,
                  &Database::handleDeleteTimelineBatchNotification);
    nc.addObserver(observeDelete);
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

    error err = deleteFromTable("users", model->LocalID());
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
    const Poco::Int64 UID) {

    poco_check_ptr(session_);

    poco_assert(UID > 0);
    poco_assert(!table_name.empty());

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "delete from " + table_name + " where uid = :uid",
                  Poco::Data::use(UID),
                  Poco::Data::now;
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
    poco_check_ptr(session_);
    poco_check_ptr(mode);

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "PRAGMA journal_mode",
                  Poco::Data::into(*mode),
                  Poco::Data::now;
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
    poco_check_ptr(session_);

    poco_assert(!mode.empty());

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "PRAGMA journal_mode=" << mode,
                  Poco::Data::now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("setJournalMode");
}

Poco::Logger &Database::logger() const {
    return Poco::Logger::get("database");
}

error Database::deleteFromTable(
    const std::string table_name,
    const Poco::Int64 local_id) {
    poco_check_ptr(session_);

    poco_assert(!table_name.empty());

    if (!local_id) {
        return noError;
    }

    std::stringstream ss;
    ss << "Deleting from table " << table_name
       << ", local ID: " << local_id;
    logger().debug(ss.str());

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "delete from " + table_name +
                  " where local_id = :local_id",
                  Poco::Data::use(local_id),
                  Poco::Data::now;
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
    poco_check_ptr(session_);

    Poco::Mutex::ScopedLock lock(session_m_);

    Poco::Data::SessionImpl* impl = session_->impl();

    Poco::Data::SQLite::SessionImpl* sqlite =
        static_cast<Poco::Data::SQLite::SessionImpl*>(impl);
    std::string last = Poco::Data::SQLite::Utility::lastError(sqlite->db());
    if (last != "not an error") {
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

    std::string api_token("");
    error err = CurrentAPIToken(&api_token);
    if (err != noError) {
        return err;
    }
    if (api_token.empty()) {
        return noError;
    }
    return LoadUserByAPIToken(api_token, user);
}

error Database::LoadSettings(Settings *settings) {
    poco_check_ptr(session_);

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "select use_idle_detection, menubar_timer, dock_icon, "
                  "on_top, reminder, ignore_cert "
                  "from settings",
                  Poco::Data::into(settings->use_idle_detection),
                  Poco::Data::into(settings->menubar_timer),
                  Poco::Data::into(settings->dock_icon),
                  Poco::Data::into(settings->on_top),
                  Poco::Data::into(settings->reminder),
                  Poco::Data::into(settings->ignore_cert),
                  Poco::Data::limit(1),
                  Poco::Data::now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("LoadSettings");
}

error Database::LoadProxySettings(
    bool *use_proxy,
    Proxy *proxy) {

    poco_check_ptr(session_);
    poco_check_ptr(use_proxy);
    poco_check_ptr(proxy);

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "select use_proxy, proxy_host, proxy_port, "
                  "proxy_username, proxy_password "
                  "from settings",
                  Poco::Data::into(*use_proxy),
                  Poco::Data::into(proxy->host),
                  Poco::Data::into(proxy->port),
                  Poco::Data::into(proxy->username),
                  Poco::Data::into(proxy->password),
                  Poco::Data::limit(1),
                  Poco::Data::now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("LoadProxySettings");
}

error Database::SaveSettings(const Settings settings) {
    poco_check_ptr(session_);

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "update settings set "
                  "use_idle_detection = :use_idle_detection, "
                  "menubar_timer = :menubar_timer, "
                  "dock_icon = :dock_icon, "
                  "on_top = :on_top, "
                  "reminder = :reminder, "
                  "ignore_cert = :ignore_cert",
                  Poco::Data::use(settings.use_idle_detection),
                  Poco::Data::use(settings.menubar_timer),
                  Poco::Data::use(settings.dock_icon),
                  Poco::Data::use(settings.on_top),
                  Poco::Data::use(settings.reminder),
                  Poco::Data::use(settings.ignore_cert),
                  Poco::Data::now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("SaveSettings");
}

error Database::SaveProxySettings(
    const bool use_proxy,
    const Proxy proxy) {

    poco_check_ptr(session_);

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "update settings set "
                  "use_proxy = :use_proxy, "
                  "proxy_host = :proxy_host, "
                  "proxy_port = :proxy_port, "
                  "proxy_username = :proxy_username, "
                  "proxy_password = :proxy_password ",
                  Poco::Data::use(use_proxy),
                  Poco::Data::use(proxy.host),
                  Poco::Data::use(proxy.port),
                  Poco::Data::use(proxy.username),
                  Poco::Data::use(proxy.password),
                  Poco::Data::now;
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

    poco_check_ptr(session_);
    poco_check_ptr(update_channel);

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "select update_channel from settings",
                  Poco::Data::into(*update_channel),
                  Poco::Data::limit(1),
                  Poco::Data::now;
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
    const std::string update_channel) {

    poco_check_ptr(session_);
    if (update_channel != "stable" &&
            update_channel != "beta" &&
            update_channel != "dev") {
        return error("Invalid update channel");
    }

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "update settings set "
                  "update_channel = :update_channel",
                  Poco::Data::use(update_channel),
                  Poco::Data::now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("SaveUpdateChannel");
}

error Database::LoadUserByAPIToken(
    const std::string api_token,
    User *model) {

    poco_check_ptr(session_);
    poco_check_ptr(model);

    poco_assert(!api_token.empty());

    Poco::UInt64 uid(0);
    model->SetAPIToken(api_token);

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "select id from users where api_token = :api_token",
                  Poco::Data::into(uid),
                  Poco::Data::use(api_token),
                  Poco::Data::limit(1),
                  Poco::Data::now;
        error err = last_error("LoadUserByAPIToken");
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
    const Poco::UInt64 UID,
    User *user) {

    poco_check_ptr(user);
    poco_check_ptr(session_);

    poco_assert(UID > 0);

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        Poco::Int64 local_id(0);
        Poco::UInt64 id(0);
        std::string api_token("");
        Poco::UInt64 default_wid(0);
        Poco::UInt64 since(0);
        std::string fullname("");
        std::string email("");
        bool record_timeline(false);
        bool store_start_and_stop_time(false);
        std::string timeofday_format("");
        *session_ <<
                  "select local_id, id, api_token, default_wid, since, "
                  "fullname, "
                  "email, record_timeline, store_start_and_stop_time, "
                  "timeofday_format "
                  "from users where id = :id",
                  Poco::Data::into(local_id),
                  Poco::Data::into(id),
                  Poco::Data::into(api_token),
                  Poco::Data::into(default_wid),
                  Poco::Data::into(since),
                  Poco::Data::into(fullname),
                  Poco::Data::into(email),
                  Poco::Data::into(record_timeline),
                  Poco::Data::into(store_start_and_stop_time),
                  Poco::Data::into(timeofday_format),
                  Poco::Data::use(UID),
                  Poco::Data::limit(1),
                  Poco::Data::now;

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
        user->SetAPIToken(api_token);
        user->SetDefaultWID(default_wid);
        user->SetSince(since);
        user->SetFullname(fullname);
        user->SetEmail(email);
        user->SetRecordTimeline(record_timeline);
        user->SetStoreStartAndStopTime(store_start_and_stop_time);
        user->SetTimeOfDayFormat(timeofday_format);
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
    const Poco::UInt64 UID,
    std::vector<Workspace *> *list) {

    poco_assert(UID > 0);

    poco_check_ptr(list);

    list->clear();

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        Poco::Data::Statement select(*session_);
        select <<
               "SELECT local_id, id, uid, name, premium, "
               "only_admins_may_create_projects, admin "
               "FROM workspaces "
               "WHERE uid = :uid "
               "ORDER BY name",
               Poco::Data::use(UID);
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
    const Poco::UInt64 UID,
    std::vector<Client *> *list) {

    poco_assert(UID > 0);

    poco_check_ptr(list);

    list->clear();

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        Poco::Data::Statement select(*session_);
        select << "SELECT local_id, id, uid, name, guid, wid "
               "FROM clients "
               "WHERE uid = :uid "
               "ORDER BY name",
               Poco::Data::use(UID);

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
                model->SetID(rs[1].convert<Poco::UInt64>());
                model->SetUID(rs[2].convert<Poco::UInt64>());
                model->SetName(rs[3].convert<std::string>());
                model->SetGUID(rs[4].convert<std::string>());
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
    const Poco::UInt64 UID,
    std::vector<Project *> *list) {

    poco_assert(UID > 0);

    poco_check_ptr(list);

    list->clear();

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        Poco::Data::Statement select(*session_);
        select << "SELECT local_id, id, uid, name, guid, wid, color, cid, "
               "active, billable "
               "FROM projects "
               "WHERE uid = :uid "
               "ORDER BY name",
               Poco::Data::use(UID);
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
                model->SetID(rs[1].convert<Poco::UInt64>());
                model->SetUID(rs[2].convert<Poco::UInt64>());
                model->SetName(rs[3].convert<std::string>());
                model->SetGUID(rs[4].convert<std::string>());
                model->SetWID(rs[5].convert<Poco::UInt64>());
                model->SetColor(rs[6].convert<std::string>());
                model->SetCID(rs[7].convert<Poco::UInt64>());
                model->SetActive(rs[8].convert<bool>());
                model->SetBillable(rs[9].convert<bool>());
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
    const Poco::UInt64 UID,
    std::vector<Task *> *list) {

    poco_assert(UID > 0);

    poco_check_ptr(list);

    list->clear();

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        Poco::Data::Statement select(*session_);
        select << "SELECT local_id, id, uid, name, wid, pid "
               "FROM tasks "
               "WHERE uid = :uid "
               "ORDER BY name",
               Poco::Data::use(UID);
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
                model->SetID(rs[1].convert<Poco::UInt64>());
                model->SetUID(rs[2].convert<Poco::UInt64>());
                model->SetName(rs[3].convert<std::string>());
                model->SetWID(rs[4].convert<Poco::UInt64>());
                model->SetPID(rs[5].convert<Poco::UInt64>());
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
    const Poco::UInt64 UID,
    std::vector<Tag *> *list) {

    poco_assert(UID > 0);

    poco_check_ptr(list);

    list->clear();

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        Poco::Data::Statement select(*session_);
        select << "SELECT local_id, id, uid, name, wid, guid "
               "FROM tags "
               "WHERE uid = :uid "
               "ORDER BY name",
               Poco::Data::use(UID);
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
                model->SetID(rs[1].convert<Poco::UInt64>());
                model->SetUID(rs[2].convert<Poco::UInt64>());
                model->SetName(rs[3].convert<std::string>());
                model->SetWID(rs[4].convert<Poco::UInt64>());
                model->SetGUID(rs[5].convert<std::string>());
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
    const Poco::UInt64 UID,
    std::vector<TimeEntry *> *list) {

    poco_assert(UID > 0);

    poco_check_ptr(list);

    list->clear();

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        Poco::Data::Statement select(*session_);
        select << "SELECT local_id, id, uid, description, wid, guid, pid, "
               "tid, billable, duronly, ui_modified_at, start, stop, "
               "duration, tags, created_with, deleted_at, updated_at, "
               "project_guid "
               "FROM time_entries "
               "WHERE uid = :uid "
               "ORDER BY start DESC",
               Poco::Data::use(UID);
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
                model->SetID(rs[1].convert<Poco::UInt64>());
                model->SetUID(rs[2].convert<Poco::UInt64>());
                model->SetDescription(rs[3].convert<std::string>());
                model->SetWID(rs[4].convert<Poco::UInt64>());
                model->SetGUID(rs[5].convert<std::string>());
                model->SetPID(rs[6].convert<Poco::UInt64>());
                model->SetTID(rs[7].convert<Poco::UInt64>());
                model->SetBillable(rs[8].convert<bool>());
                model->SetDurOnly(rs[9].convert<bool>());
                model->SetUIModifiedAt(rs[10].convert<Poco::UInt64>());
                model->SetStart(rs[11].convert<Poco::UInt64>());
                model->SetStop(rs[12].convert<Poco::UInt64>());
                model->SetDurationInSeconds(rs[13].convert<Poco::Int64>());
                model->SetTags(rs[14].convert<std::string>());
                model->SetCreatedWith(rs[15].convert<std::string>());
                model->SetDeletedAt(rs[16].convert<Poco::UInt64>());
                model->SetUpdatedAt(rs[17].convert<Poco::UInt64>());
                model->SetProjectGUID(rs[18].convert<std::string>());
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

    poco_assert(UID > 0);

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

typedef kopsik::error (Database::*saveModel)(
    BaseModel *model, std::vector<ModelChange> *changes);

error Database::saveModel(
    TimeEntry *model,
    std::vector<ModelChange> *changes) {

    poco_check_ptr(model);
    poco_check_ptr(session_);
    poco_check_ptr(changes);

    // Time entries need to have a GUID,
    // we expect it everywhere in the UI
    model->EnsureGUID();
    poco_assert(!model->GUID().empty());

    if (!model->NeedsToBeSaved()) {
        return noError;
    }

    Poco::Mutex::ScopedLock lock(session_m_);
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
                          "project_guid = :project_guid "
                          "where local_id = :local_id",
                          Poco::Data::use(model->ID()),
                          Poco::Data::use(model->UID()),
                          Poco::Data::use(model->Description()),
                          Poco::Data::use(model->WID()),
                          Poco::Data::use(model->GUID()),
                          Poco::Data::use(model->PID()),
                          Poco::Data::use(model->TID()),
                          Poco::Data::use(model->Billable()),
                          Poco::Data::use(model->DurOnly()),
                          Poco::Data::use(model->UIModifiedAt()),
                          Poco::Data::use(model->Start()),
                          Poco::Data::use(model->Stop()),
                          Poco::Data::use(model->DurationInSeconds()),
                          Poco::Data::use(model->Tags()),
                          Poco::Data::use(model->CreatedWith()),
                          Poco::Data::use(model->DeletedAt()),
                          Poco::Data::use(model->UpdatedAt()),
                          Poco::Data::use(model->ProjectGUID()),
                          Poco::Data::use(model->LocalID()),
                          Poco::Data::now;
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
                          "project_guid = :project_guid "
                          "where local_id = :local_id",
                          Poco::Data::use(model->UID()),
                          Poco::Data::use(model->Description()),
                          Poco::Data::use(model->WID()),
                          Poco::Data::use(model->GUID()),
                          Poco::Data::use(model->PID()),
                          Poco::Data::use(model->TID()),
                          Poco::Data::use(model->Billable()),
                          Poco::Data::use(model->DurOnly()),
                          Poco::Data::use(model->UIModifiedAt()),
                          Poco::Data::use(model->Start()),
                          Poco::Data::use(model->Stop()),
                          Poco::Data::use(model->DurationInSeconds()),
                          Poco::Data::use(model->Tags()),
                          Poco::Data::use(model->CreatedWith()),
                          Poco::Data::use(model->DeletedAt()),
                          Poco::Data::use(model->UpdatedAt()),
                          Poco::Data::use(model->ProjectGUID()),
                          Poco::Data::use(model->LocalID()),
                          Poco::Data::now;
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
                          "project_guid) "
                          "values(:id, :uid, :description, :wid, "
                          ":guid, :pid, :tid, :billable, "
                          ":duronly, :ui_modified_at, "
                          ":start, :stop, :duration, "
                          ":tags, :created_with, :deleted_at, :updated_at, "
                          ":project_guid)",
                          Poco::Data::use(model->ID()),
                          Poco::Data::use(model->UID()),
                          Poco::Data::use(model->Description()),
                          Poco::Data::use(model->WID()),
                          Poco::Data::use(model->GUID()),
                          Poco::Data::use(model->PID()),
                          Poco::Data::use(model->TID()),
                          Poco::Data::use(model->Billable()),
                          Poco::Data::use(model->DurOnly()),
                          Poco::Data::use(model->UIModifiedAt()),
                          Poco::Data::use(model->Start()),
                          Poco::Data::use(model->Stop()),
                          Poco::Data::use(model->DurationInSeconds()),
                          Poco::Data::use(model->Tags()),
                          Poco::Data::use(model->CreatedWith()),
                          Poco::Data::use(model->DeletedAt()),
                          Poco::Data::use(model->UpdatedAt()),
                          Poco::Data::use(model->ProjectGUID()),
                          Poco::Data::now;
            } else {
                *session_ << "insert into time_entries(uid, description, wid, "
                          "guid, pid, tid, billable, "
                          "duronly, ui_modified_at, "
                          "start, stop, duration, "
                          "tags, created_with, deleted_at, updated_at, "
                          "project_guid "
                          ") values ("
                          ":uid, :description, :wid, "
                          ":guid, :pid, :tid, :billable, "
                          ":duronly, :ui_modified_at, "
                          ":start, :stop, :duration, "
                          ":tags, :created_with, :deleted_at, :updated_at, "
                          ":project_guid)",
                          Poco::Data::use(model->UID()),
                          Poco::Data::use(model->Description()),
                          Poco::Data::use(model->WID()),
                          Poco::Data::use(model->GUID()),
                          Poco::Data::use(model->PID()),
                          Poco::Data::use(model->TID()),
                          Poco::Data::use(model->Billable()),
                          Poco::Data::use(model->DurOnly()),
                          Poco::Data::use(model->UIModifiedAt()),
                          Poco::Data::use(model->Start()),
                          Poco::Data::use(model->Stop()),
                          Poco::Data::use(model->DurationInSeconds()),
                          Poco::Data::use(model->Tags()),
                          Poco::Data::use(model->CreatedWith()),
                          Poco::Data::use(model->DeletedAt()),
                          Poco::Data::use(model->UpdatedAt()),
                          Poco::Data::use(model->ProjectGUID()),
                          Poco::Data::now;
            }
            error err = last_error("saveTimeEntry");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ << "select last_insert_rowid()",
                      Poco::Data::into(local_id),
                      Poco::Data::now;
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

    poco_check_ptr(model);
    poco_check_ptr(session_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }

    Poco::Mutex::ScopedLock lock(session_m_);
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
                      Poco::Data::use(model->ID()),
                      Poco::Data::use(model->UID()),
                      Poco::Data::use(model->Name()),
                      Poco::Data::use(model->Premium()),
                      Poco::Data::use(model->OnlyAdminsMayCreateProjects()),
                      Poco::Data::use(model->Admin()),
                      Poco::Data::use(model->LocalID()),
                      Poco::Data::now;
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
                      Poco::Data::use(model->ID()),
                      Poco::Data::use(model->UID()),
                      Poco::Data::use(model->Name()),
                      Poco::Data::use(model->Premium()),
                      Poco::Data::use(model->OnlyAdminsMayCreateProjects()),
                      Poco::Data::use(model->Admin()),
                      Poco::Data::now;
            error err = last_error("saveWorkspace");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ << "select last_insert_rowid()",
                      Poco::Data::into(local_id),
                      Poco::Data::now;
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

    poco_check_ptr(model);
    poco_check_ptr(session_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }

    Poco::Mutex::ScopedLock lock(session_m_);
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
                          Poco::Data::use(model->ID()),
                          Poco::Data::use(model->UID()),
                          Poco::Data::use(model->Name()),
                          Poco::Data::use(model->WID()),
                          Poco::Data::use(model->LocalID()),
                          Poco::Data::now;
            } else {
                *session_ << "update clients set "
                          "id = :id, uid = :uid, name = :name, guid = :guid, "
                          "wid = :wid "
                          "where local_id = :local_id",
                          Poco::Data::use(model->ID()),
                          Poco::Data::use(model->UID()),
                          Poco::Data::use(model->Name()),
                          Poco::Data::use(model->GUID()),
                          Poco::Data::use(model->WID()),
                          Poco::Data::use(model->LocalID()),
                          Poco::Data::now;
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
                          Poco::Data::use(model->ID()),
                          Poco::Data::use(model->UID()),
                          Poco::Data::use(model->Name()),
                          Poco::Data::use(model->WID()),
                          Poco::Data::now;
            } else {
                *session_ << "insert into clients(id, uid, name, guid, wid) "
                          "values(:id, :uid, :name, :guid, :wid)",
                          Poco::Data::use(model->ID()),
                          Poco::Data::use(model->UID()),
                          Poco::Data::use(model->Name()),
                          Poco::Data::use(model->GUID()),
                          Poco::Data::use(model->WID()),
                          Poco::Data::now;
            }
            error err = last_error("saveClient");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ << "select last_insert_rowid()",
                      Poco::Data::into(local_id),
                      Poco::Data::now;
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

    poco_check_ptr(model);
    poco_check_ptr(session_);

    // Generate GUID only for locally-created
    // projects. User cannot update existing
    // projects, so don't mess with their GUIDs
    if (!model->ID()) {
        model->EnsureGUID();
        poco_assert(!model->GUID().empty());
    }

    if (!model->NeedsToBeSaved()) {
        return noError;
    }

    Poco::Mutex::ScopedLock lock(session_m_);
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
                              "active = :active, billable = :billable "
                              "where local_id = :local_id",
                              Poco::Data::use(model->ID()),
                              Poco::Data::use(model->UID()),
                              Poco::Data::use(model->Name()),
                              Poco::Data::use(model->WID()),
                              Poco::Data::use(model->Color()),
                              Poco::Data::use(model->CID()),
                              Poco::Data::use(model->Active()),
                              Poco::Data::use(model->Billable()),
                              Poco::Data::use(model->LocalID()),
                              Poco::Data::now;
                } else {
                    *session_ << "update projects set "
                              "id = :id, uid = :uid, name = :name, "
                              "guid = :guid,"
                              "wid = :wid, color = :color, cid = :cid, "
                              "active = :active, billable = :billable "
                              "where local_id = :local_id",
                              Poco::Data::use(model->ID()),
                              Poco::Data::use(model->UID()),
                              Poco::Data::use(model->Name()),
                              Poco::Data::use(model->GUID()),
                              Poco::Data::use(model->WID()),
                              Poco::Data::use(model->Color()),
                              Poco::Data::use(model->CID()),
                              Poco::Data::use(model->Active()),
                              Poco::Data::use(model->Billable()),
                              Poco::Data::use(model->LocalID()),
                              Poco::Data::now;
                }
            } else {
                if (model->GUID().empty()) {
                    *session_ << "update projects set "
                              "uid = :uid, name = :name, "
                              "wid = :wid, color = :color, cid = :cid, "
                              "active = :active, billable = :billable "
                              "where local_id = :local_id",
                              Poco::Data::use(model->UID()),
                              Poco::Data::use(model->Name()),
                              Poco::Data::use(model->WID()),
                              Poco::Data::use(model->Color()),
                              Poco::Data::use(model->CID()),
                              Poco::Data::use(model->Active()),
                              Poco::Data::use(model->Billable()),
                              Poco::Data::use(model->LocalID()),
                              Poco::Data::now;
                } else {
                    *session_ << "update projects set "
                              "uid = :uid, name = :name, guid = :guid,"
                              "wid = :wid, color = :color, cid = :cid, "
                              "active = :active, billable = :billable "
                              "where local_id = :local_id",
                              Poco::Data::use(model->UID()),
                              Poco::Data::use(model->Name()),
                              Poco::Data::use(model->GUID()),
                              Poco::Data::use(model->WID()),
                              Poco::Data::use(model->Color()),
                              Poco::Data::use(model->CID()),
                              Poco::Data::use(model->Active()),
                              Poco::Data::use(model->Billable()),
                              Poco::Data::use(model->LocalID()),
                              Poco::Data::now;
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
                              "is_private, billable"
                              ") values("
                              ":id, :uid, :name, :wid, :color, :cid, :active, "
                              ":is_private, :billable"
                              ")",
                              Poco::Data::use(model->ID()),
                              Poco::Data::use(model->UID()),
                              Poco::Data::use(model->Name()),
                              Poco::Data::use(model->WID()),
                              Poco::Data::use(model->Color()),
                              Poco::Data::use(model->CID()),
                              Poco::Data::use(model->Active()),
                              Poco::Data::use(model->IsPrivate()),
                              Poco::Data::use(model->Billable()),
                              Poco::Data::now;
                } else {
                    *session_ <<
                              "insert into projects("
                              "id, uid, name, guid, wid, color, cid, active, "
                              "is_private, billable"
                              ") values("
                              ":id, :uid, :name, :guid, :wid, :color, :cid, "
                              ":active, :is_private, "
                              ":billable"
                              ")",
                              Poco::Data::use(model->ID()),
                              Poco::Data::use(model->UID()),
                              Poco::Data::use(model->Name()),
                              Poco::Data::use(model->GUID()),
                              Poco::Data::use(model->WID()),
                              Poco::Data::use(model->Color()),
                              Poco::Data::use(model->CID()),
                              Poco::Data::use(model->Active()),
                              Poco::Data::use(model->IsPrivate()),
                              Poco::Data::use(model->Billable()),
                              Poco::Data::now;
                }
            } else {
                if (model->GUID().empty()) {
                    *session_ <<
                              "insert into projects("
                              "uid, name, wid, color, cid, active, "
                              "is_private, billable"
                              ") values("
                              ":uid, :name, :wid, :color, :cid, :active, "
                              ":is_private, :billable"
                              ")",
                              Poco::Data::use(model->UID()),
                              Poco::Data::use(model->Name()),
                              Poco::Data::use(model->WID()),
                              Poco::Data::use(model->Color()),
                              Poco::Data::use(model->CID()),
                              Poco::Data::use(model->Active()),
                              Poco::Data::use(model->IsPrivate()),
                              Poco::Data::use(model->Billable()),
                              Poco::Data::now;
                } else {
                    *session_ <<
                              "insert into projects("
                              "uid, name, guid, wid, color, cid, "
                              "active, is_private, billable"
                              ") values("
                              ":uid, :name, :guid, :wid, :color, :cid, "
                              ":active, :is_private, :billable"
                              ")",
                              Poco::Data::use(model->UID()),
                              Poco::Data::use(model->Name()),
                              Poco::Data::use(model->GUID()),
                              Poco::Data::use(model->WID()),
                              Poco::Data::use(model->Color()),
                              Poco::Data::use(model->CID()),
                              Poco::Data::use(model->Active()),
                              Poco::Data::use(model->IsPrivate()),
                              Poco::Data::use(model->Billable()),
                              Poco::Data::now;
                }
            }
            error err = last_error("saveProject");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ << "select last_insert_rowid()",
                      Poco::Data::into(local_id),
                      Poco::Data::now;
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

    poco_check_ptr(model);
    poco_check_ptr(session_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating task " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger().trace(ss.str());

            *session_ << "update tasks set "
                      "id = :id, uid = :uid, name = :name, wid = :wid, "
                      "pid = :pid "
                      "where local_id = :local_id",
                      Poco::Data::use(model->ID()),
                      Poco::Data::use(model->UID()),
                      Poco::Data::use(model->Name()),
                      Poco::Data::use(model->WID()),
                      Poco::Data::use(model->PID()),
                      Poco::Data::use(model->LocalID()),
                      Poco::Data::now;
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
            *session_ << "insert into tasks(id, uid, name, wid, pid) "
                      "values(:id, :uid, :name, :wid, :pid)",
                      Poco::Data::use(model->ID()),
                      Poco::Data::use(model->UID()),
                      Poco::Data::use(model->Name()),
                      Poco::Data::use(model->WID()),
                      Poco::Data::use(model->PID()),
                      Poco::Data::now;
            error err = last_error("saveTask");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ << "select last_insert_rowid()",
                      Poco::Data::into(local_id),
                      Poco::Data::now;
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

    poco_check_ptr(model);
    poco_check_ptr(session_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }

    Poco::Mutex::ScopedLock lock(session_m_);
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
                          Poco::Data::use(model->ID()),
                          Poco::Data::use(model->UID()),
                          Poco::Data::use(model->Name()),
                          Poco::Data::use(model->WID()),
                          Poco::Data::use(model->LocalID()),
                          Poco::Data::now;
            } else {
                *session_ << "update tags set "
                          "id = :id, uid = :uid, name = :name, wid = :wid, "
                          "guid = :guid "
                          "where local_id = :local_id",
                          Poco::Data::use(model->ID()),
                          Poco::Data::use(model->UID()),
                          Poco::Data::use(model->Name()),
                          Poco::Data::use(model->WID()),
                          Poco::Data::use(model->GUID()),
                          Poco::Data::use(model->LocalID()),
                          Poco::Data::now;
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
                          Poco::Data::use(model->ID()),
                          Poco::Data::use(model->UID()),
                          Poco::Data::use(model->Name()),
                          Poco::Data::use(model->WID()),
                          Poco::Data::now;
            } else {
                *session_ << "insert into tags(id, uid, name, wid, guid) "
                          "values(:id, :uid, :name, :wid, :guid)",
                          Poco::Data::use(model->ID()),
                          Poco::Data::use(model->UID()),
                          Poco::Data::use(model->Name()),
                          Poco::Data::use(model->WID()),
                          Poco::Data::use(model->GUID()),
                          Poco::Data::now;
            }
            error err = last_error("saveTag");
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session_ << "select last_insert_rowid()",
                      Poco::Data::into(local_id),
                      Poco::Data::now;
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

    Poco::Mutex::ScopedLock lock(session_m_);

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
                          "api_token = :api_token, default_wid = :default_wid, "
                          "since = :since, id = :id, fullname = :fullname, "
                          "email = :email, record_timeline = :record_timeline, "
                          "store_start_and_stop_time = "
                          " :store_start_and_stop_time, "
                          "timeofday_format = :timeofday_format "
                          "where local_id = :local_id",
                          Poco::Data::use(user->APIToken()),
                          Poco::Data::use(user->DefaultWID()),
                          Poco::Data::use(user->Since()),
                          Poco::Data::use(user->ID()),
                          Poco::Data::use(user->Fullname()),
                          Poco::Data::use(user->Email()),
                          Poco::Data::use(user->RecordTimeline()),
                          Poco::Data::use(user->StoreStartAndStopTime()),
                          Poco::Data::use(user->TimeOfDayFormat()),
                          Poco::Data::use(user->LocalID()),
                          Poco::Data::now;
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
                          "id, api_token, default_wid, since, fullname, email, "
                          "record_timeline, store_start_and_stop_time, "
                          "timeofday_format"
                          ") values("
                          ":id, :api_token, :default_wid, :since, :fullname, "
                          ":email, "
                          ":record_timeline, :store_start_and_stop_time, "
                          ":timeofday_format"
                          ")",
                          Poco::Data::use(user->ID()),
                          Poco::Data::use(user->APIToken()),
                          Poco::Data::use(user->DefaultWID()),
                          Poco::Data::use(user->Since()),
                          Poco::Data::use(user->Fullname()),
                          Poco::Data::use(user->Email()),
                          Poco::Data::use(user->RecordTimeline()),
                          Poco::Data::use(user->StoreStartAndStopTime()),
                          Poco::Data::use(user->TimeOfDayFormat()),
                          Poco::Data::now;
                error err = last_error("SaveUser");
                if (err != noError) {
                    session_->rollback();
                    return err;
                }
                Poco::Int64 local_id(0);
                *session_ << "select last_insert_rowid()",
                          Poco::Data::into(local_id),
                          Poco::Data::now;
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

error Database::initialize_tables() {
    poco_check_ptr(session_);

    Poco::Mutex::ScopedLock lock(session_m_);

    std::string table_name;
    // Check if we have migrations table
    *session_ <<
              "select name from sqlite_master "
              "where type='table' and name='kopsik_migrations'",
              Poco::Data::into(table_name),
              Poco::Data::limit(1),
              Poco::Data::now;
    error err = last_error("initialize_tables");
    if (err != noError) {
        return err;
    }

    if (table_name.length() == 0) {
        *session_ <<
                  "create table kopsik_migrations(id integer primary key, "
                  "name varchar not null)",
                  Poco::Data::now;
        error err = last_error("initialize_tables");
        if (err != noError) {
            return err;
        }
        *session_ <<
                  "CREATE UNIQUE INDEX id_kopsik_migrations_name "
                  "ON kopsik_migrations (name);",
                  Poco::Data::now;
        err = last_error("initialize_tables");
        if (err != noError) {
            return err;
        }
    }

    err = migrate("users",
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

    err = migrate("users.email",
                  "CREATE UNIQUE INDEX id_users_email ON users (email);");
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

    err = migrate(
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

    err = migrate(
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

    err = migrate(
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

    err = migrate(
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

    err = migrate("sessions",
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

    err = migrate("settings",
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

    err = migrate(
        "settings.default",
        "INSERT INTO settings(update_channel) "
        "SELECT 'stable' WHERE NOT EXISTS (SELECT 1 FROM settings LIMIT 1);");
    if (err != noError) {
        return err;
    }

    err = migrate("settings.menubar_timer",
                  "ALTER TABLE settings "
                  "ADD COLUMN menubar_timer integer not null default 0;");
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

    err = migrate("timeline_installation",
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
        err = SaveDesktopID();
        if (err != noError) {
            return err;
        }
    }

    return noError;
}

error Database::CurrentAPIToken(std::string *token) {
    poco_check_ptr(session_);
    poco_check_ptr(token);

    *token = "";

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "select api_token from sessions",
                  Poco::Data::into(*token),
                  Poco::Data::limit(1),
                  Poco::Data::now;
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
    poco_check_ptr(session_);

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "delete from sessions", Poco::Data::now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("ClearCurrentAPIToken");
}

error Database::SetCurrentAPIToken(const std::string &token) {
    poco_check_ptr(session_);

    Poco::Mutex::ScopedLock lock(session_m_);

    error err = ClearCurrentAPIToken();
    if (err != noError) {
        return err;
    }
    try {
        *session_ << "insert into sessions(api_token) values(:api_token)",
                  Poco::Data::use(token),
                  Poco::Data::now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("SetCurrentAPIToken");
}

error Database::SaveDesktopID() {
    poco_check_ptr(session_);

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << "INSERT INTO timeline_installation(desktop_id) "
                  "VALUES(:desktop_id)",
                  Poco::Data::use(desktop_id_),
                  Poco::Data::now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error("SaveDesktopID");
}

error Database::migrate(
    const std::string name,
    const std::string sql) {

    poco_check_ptr(session_);

    poco_assert(!name.empty());
    poco_assert(!sql.empty());

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        int count = 0;
        *session_ << "select count(*) from kopsik_migrations where name=:name",
                  Poco::Data::into(count),
                  Poco::Data::use(name),
                  Poco::Data::now;
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
                  Poco::Data::use(name),
                  Poco::Data::now;
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

    poco_check_ptr(session_);

    poco_assert(!sql.empty());

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        *session_ << sql, Poco::Data::now;
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

error Database::select_timeline_batch(
    const Poco::UInt64 user_id,
    std::vector<TimelineEvent> *timeline_events) {
    std::stringstream out;
    out << "select_batch, user_id = " << user_id;
    logger().debug(out.str());

    poco_assert(user_id > 0);
    poco_assert(timeline_events->empty());
    if (!session_) {
        logger().warning("select_batch database is not open, ignoring request");
        return noError;
    }

    Poco::Mutex::ScopedLock lock(session_m_);

    Poco::Data::Statement select(*session_);
    select << "SELECT id, title, filename, start_time, end_time, idle "
           "FROM timeline_events WHERE user_id = :user_id "
           "LIMIT 100",
           Poco::Data::use(user_id);
    Poco::Data::RecordSet rs(select);
    while (!select.done()) {
        select.execute();
        bool more = rs.moveFirst();
        while (more) {
            TimelineEvent event;
            event.id = rs[0].convert<unsigned int>();
            event.title = rs[1].convert<std::string>();
            event.filename = rs[2].convert<std::string>();
            event.start_time = rs[3].convert<int>();
            event.end_time = rs[4].convert<int>();
            event.idle = rs[5].convert<bool>();
            event.user_id = static_cast<unsigned int>(user_id);
            timeline_events->push_back(event);
            more = rs.moveNext();
        }
    }

    std::stringstream event_count;
    event_count << "select_batch found " << timeline_events->size()
                <<  " events.";
    logger().debug(event_count.str());

    return last_error("select_timeline_batch");
}

error Database::insert_timeline_event(const TimelineEvent& event) {
    std::stringstream out;
    out << "insert " << event.start_time << ";" << event.end_time << ";"
        << event.filename << ";" << event.title;
    logger().debug(out.str());

    poco_assert(event.user_id > 0);
    poco_assert(event.start_time > 0);
    poco_assert(event.end_time > 0);
    if (!session_) {
        logger().warning("insert database is not open, ignoring request");
        return noError;
    }

    Poco::Mutex::ScopedLock lock(session_m_);

    *session_ << "INSERT INTO timeline_events("
              "user_id, title, filename, start_time, end_time, idle"
              ") VALUES ("
              ":user_id, :title, :filename, :start_time, :end_time, :idle"
              ")",
              Poco::Data::use(event.user_id),
              Poco::Data::use(event.title),
              Poco::Data::use(event.filename),
              Poco::Data::use(event.start_time),
              Poco::Data::use(event.end_time),
              Poco::Data::use(event.idle),
              Poco::Data::now;
    return last_error("insert_timeline_event");
}

error Database::delete_timeline_batch(
    const std::vector<TimelineEvent> &timeline_events) {
    std::stringstream out;
    out << "delete_batch " << timeline_events.size() << " events.";
    logger().debug(out.str());

    poco_assert(!timeline_events.empty());
    if (!session_) {
        logger().warning("delete_batch database is not open, ignoring request");
        return noError;
    }
    std::vector<int> ids;
    for (std::vector<TimelineEvent>::const_iterator i = timeline_events.begin();
            i != timeline_events.end();
            ++i) {
        const TimelineEvent &event = *i;
        ids.push_back(event.id);
    }

    Poco::Mutex::ScopedLock lock(session_m_);

    *session_ << "DELETE FROM timeline_events WHERE id = :id",
              Poco::Data::use(ids),
              Poco::Data::now;
    return last_error("delete_timeline_batch");
}

void Database::handleTimelineEventNotification(
    TimelineEventNotification* notification) {
    logger().debug("handleTimelineEventNotification");
    insert_timeline_event(notification->event);
}

void Database::handleCreateTimelineBatchNotification(
    CreateTimelineBatchNotification* notification) {
    logger().debug("handleCreateTimelineBatchNotification");
    std::vector<TimelineEvent> batch;
    select_timeline_batch(notification->user_id, &batch);
    if (batch.empty()) {
        return;
    }
    Poco::NotificationCenter& nc = Poco::NotificationCenter::defaultCenter();
    TimelineBatchReadyNotification response(
        notification->user_id, batch, desktop_id_);
    Poco::AutoPtr<TimelineBatchReadyNotification> ptr(&response);
    nc.postNotification(ptr);
}

void Database::handleDeleteTimelineBatchNotification(
    DeleteTimelineBatchNotification* notification) {
    logger().debug("handleDeleteTimelineBatchNotification");

    poco_assert(!notification->batch.empty());

    delete_timeline_batch(notification->batch);
}

error Database::String(
    const std::string sql,
    std::string *result) {

    poco_check_ptr(session_);
    poco_check_ptr(result);

    poco_assert(!sql.empty());

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        std::string value("");
        *session_ << sql,
        Poco::Data::into(value),
        Poco::Data::now;
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

    poco_check_ptr(session_);
    poco_check_ptr(result);

    poco_assert(!sql.empty());

    Poco::Mutex::ScopedLock lock(session_m_);
    try {
        Poco::UInt64 value(0);
        *session_ << sql,
        Poco::Data::into(value),
        Poco::Data::now;
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

}   // namespace kopsik
