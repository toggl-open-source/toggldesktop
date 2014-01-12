// Copyright 2013 Tanel Lebedev

#include "./database.h"

#include <limits>
#include <string>
#include <vector>

#include "./toggl_api_client.h"

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

error Database::DeleteUser(
        User *model,
        const bool with_related_data) {
    poco_assert(model);

    Poco::Mutex::ScopedLock lock(mutex_);

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
    poco_assert(session);
    poco_assert(UID > 0);
    poco_assert(!table_name.empty());

    try {
        *session << "delete from " + table_name + " where uid = :uid",
            Poco::Data::use(UID),
            Poco::Data::now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error();
}

error Database::deleteFromTable(
        const std::string table_name,
        const Poco::Int64 local_id) {
    poco_assert(session);
    poco_assert(!table_name.empty());
    poco_assert(local_id);

    std::stringstream ss;
    ss << "Deleting from table " << table_name
        << ", local ID: " << local_id;
    Poco::Logger &logger = Poco::Logger::get("database");
    logger.debug(ss.str());
    try {
        *session << "delete from " + table_name +
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
    return last_error();
}

error Database::last_error() {
    poco_assert(session);

    Poco::Data::SessionImpl* impl = session->impl();
    Poco::Data::SQLite::SessionImpl* sqlite =
        static_cast<Poco::Data::SQLite::SessionImpl*>(impl);
    std::string last = Poco::Data::SQLite::Utility::lastError(sqlite->db());
    if (last != "not an error") {
        return error(last);
    }
    return noError;
}

std::string Database::generateGUID() {
    Poco::UUIDGenerator& generator =
        Poco::UUIDGenerator::defaultGenerator();
    Poco::UUID uuid(generator.createRandom());
    return uuid.toString();
}

error Database::LoadCurrentUser(
        User *user,
        const bool with_related_data) {
    poco_assert(user);

    Poco::Mutex::ScopedLock lock(mutex_);

    std::string api_token("");
    error err = CurrentAPIToken(&api_token);
    if (err != noError) {
        return err;
    }
    if (api_token.empty()) {
        return noError;
    }
    return LoadUserByAPIToken(api_token, user, with_related_data);
}

error Database::LoadSettings(
        bool *use_proxy,
        Proxy *proxy,
        bool *use_idle_detection) {
    poco_assert(session);
    poco_assert(use_proxy);
    poco_assert(proxy);
    poco_assert(use_idle_detection);

    Poco::Mutex::ScopedLock lock(mutex_);

    int has_settings = 0;

    try {
        *session << "select use_proxy, proxy_host, proxy_port, "
                "proxy_username, proxy_password, use_idle_detection, 1 "
                "from settings",
            Poco::Data::into(*use_proxy),
            Poco::Data::into(proxy->host),
            Poco::Data::into(proxy->port),
            Poco::Data::into(proxy->username),
            Poco::Data::into(proxy->password),
            Poco::Data::into(*use_idle_detection),
            Poco::Data::into(has_settings),
            Poco::Data::limit(1),
            Poco::Data::now;
      if (!has_settings) {
        // Defaults:
        *use_idle_detection = 1;
      }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error();
}

error Database::SaveSettings(
        const bool use_proxy,
        const Proxy *proxy,
        const bool use_idle_detection) {
    poco_assert(session);

    Poco::Mutex::ScopedLock lock(mutex_);

    try {
        *session << "delete from settings",
            Poco::Data::now;
        kopsik::error err = last_error();
        if (err != kopsik::noError) {
            return err;
        }
        *session << "insert into settings "
            "(use_proxy, proxy_host, proxy_port, proxy_username, "
            "proxy_password, use_idle_detection) "
            "values "
            "(:use_proxy, :proxy_host, :proxy_port, :proxy_username, "
            ":proxy_password, :use_idle_detection)",
            Poco::Data::use(use_proxy),
            Poco::Data::use(proxy->host),
            Poco::Data::use(proxy->port),
            Poco::Data::use(proxy->username),
            Poco::Data::use(proxy->password),
            Poco::Data::use(use_idle_detection),
            Poco::Data::now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error();
}

error Database::LoadUserByAPIToken(
        const std::string api_token,
        User *model,
        const bool with_related_data) {
    poco_assert(session);
    poco_assert(model);
    poco_assert(!api_token.empty());

    Poco::Mutex::ScopedLock lock(mutex_);

    Poco::UInt64 uid(0);
    model->SetAPIToken(api_token);
    try {
        *session << "select id from users where api_token = :api_token",
            Poco::Data::into(uid),
            Poco::Data::use(api_token),
            Poco::Data::limit(1),
            Poco::Data::now;
        error err = last_error();
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
    return LoadUserByID(uid, model, with_related_data);
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

    return loadTimeEntries(user->ID(), &user->related.TimeEntries);
}

error Database::LoadUserByID(
        const Poco::UInt64 UID,
        User *user,
        const bool with_related_data) {
    poco_assert(user);
    poco_assert(session);
    poco_assert(UID > 0);

    Poco::Mutex::ScopedLock lock(mutex_);

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    Poco::Logger &logger = Poco::Logger::get("database");

    try {
        Poco::Int64 local_id(0);
        Poco::UInt64 id(0);
        std::string api_token("");
        Poco::UInt64 default_wid(0);
        Poco::UInt64 since(0);
        std::string fullname("");
        std::string email("");
        bool record_timeline(false);
        *session <<
            "select local_id, id, api_token, default_wid, since, fullname, "
            "email, record_timeline "
            "from users where id = :id",
            Poco::Data::into(local_id),
            Poco::Data::into(id),
            Poco::Data::into(api_token),
            Poco::Data::into(default_wid),
            Poco::Data::into(since),
            Poco::Data::into(fullname),
            Poco::Data::into(email),
            Poco::Data::into(record_timeline),
            Poco::Data::use(UID),
            Poco::Data::limit(1),
            Poco::Data::now;

        error err = last_error();
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
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    if (with_related_data) {
        error err = loadUsersRelatedData(user);
        if (err != noError) {
            return err;
        }
    }

    stopwatch.stop();
    std::stringstream ss;
    ss << "User with_related_data=" << with_related_data << " loaded in "
        << stopwatch.elapsed() / 1000 << " ms";
    logger.debug(ss.str());

    return noError;
}

error Database::UInt(
        const std::string sql,
        Poco::UInt64 *result) {
    poco_assert(session);
    poco_assert(result);
    poco_assert(!sql.empty());

    Poco::Mutex::ScopedLock lock(mutex_);

    try {
        Poco::UInt64 value(0);
        *session << sql,
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
    return last_error();
}

error Database::String(
        const std::string sql,
        std::string *result) {
    poco_assert(session);
    poco_assert(result);
    poco_assert(!sql.empty());

    Poco::Mutex::ScopedLock lock(mutex_);

    try {
        std::string value("");
        *session << sql,
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
    return last_error();
}

error Database::loadWorkspaces(
        const Poco::UInt64 UID,
        std::vector<Workspace *> *list) {
    poco_assert(UID > 0);
    poco_assert(list);

    list->clear();

    try {
        Poco::Data::Statement select(*session);
        select << "SELECT local_id, id, uid, name "
            "FROM workspaces WHERE uid = :uid "
            "ORDER BY name",
            Poco::Data::use(UID);
        error err = last_error();
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
    return last_error();
}

error Database::loadClients(
        const Poco::UInt64 UID,
        std::vector<Client *> *list) {
    poco_assert(UID > 0);
    poco_assert(list);

    list->clear();

    try {
        Poco::Data::Statement select(*session);
        select << "SELECT local_id, id, uid, name, guid, wid "
            "FROM clients WHERE uid = :uid "
            "ORDER BY name",
            Poco::Data::use(UID);

        error err = last_error();
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
    return last_error();
}

error Database::loadProjects(
        const Poco::UInt64 UID,
        std::vector<Project *> *list) {
    poco_assert(UID > 0);
    poco_assert(list);

    list->clear();

    try {
        Poco::Data::Statement select(*session);
        select << "SELECT local_id, id, uid, name, guid, wid, color, cid, "
            "active "
            "FROM projects WHERE uid = :uid "
            "ORDER BY name",
            Poco::Data::use(UID);
        error err = last_error();
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
    return last_error();
}

error Database::loadTasks(
        const Poco::UInt64 UID,
        std::vector<Task *> *list) {
    poco_assert(UID > 0);
    poco_assert(list);

    list->clear();

    try {
        Poco::Data::Statement select(*session);
        select << "SELECT local_id, id, uid, name, wid, pid "
            "FROM tasks WHERE uid = :uid "
            "ORDER BY name",
            Poco::Data::use(UID);
        error err = last_error();
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
    return last_error();
}

error Database::loadTags(
        const Poco::UInt64 UID,
        std::vector<Tag *> *list) {
    poco_assert(UID > 0);
    poco_assert(list);

    list->clear();

    try {
        Poco::Data::Statement select(*session);
        select << "SELECT local_id, id, uid, name, wid, guid "
            "FROM tags WHERE uid = :uid "
            "ORDER BY name",
            Poco::Data::use(UID);
        error err = last_error();
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
    return last_error();
}

error Database::loadTimeEntries(
        const Poco::UInt64 UID,
        std::vector<TimeEntry *> *list) {
    poco_assert(UID > 0);
    poco_assert(list);

    list->clear();

    try {
        Poco::Data::Statement select(*session);
        select << "SELECT local_id, id, uid, description, wid, guid, pid, "
            "tid, billable, duronly, ui_modified_at, start, stop, "
            "duration, tags, created_with, deleted_at, updated_at "
            "FROM time_entries "
            "WHERE uid = :uid "
            "ORDER BY start DESC",
            Poco::Data::use(UID);
        error err = last_error();
        if (err != noError) {
            return err;
        }
        return loadTimeEntriesFromSQLStatement(&select, list);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error();
}

error Database::loadTimeEntriesFromSQLStatement(
        Poco::Data::Statement *select,
        std::vector<TimeEntry *> *list) {
    poco_assert(select);
    poco_assert(list);
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

// FIXME: user generic saveModels instead, delete this
error Database::saveWorkspaces(
        const Poco::UInt64 UID,
        std::vector<Workspace *> *list,
        std::vector<ModelChange> *changes) {
    poco_assert(UID > 0);
    poco_assert(list);
    for (std::vector<Workspace *>::iterator it = list->begin();
            it != list->end(); ++it) {
        Workspace *model = *it;
        model->SetUID(UID);
        error err = SaveWorkspace(model, changes);
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

// FIXME: user generic saveModels instead, delete this
error Database::saveClients(
        const Poco::UInt64 UID,
        std::vector<Client *> *list,
        std::vector<ModelChange> *changes) {
    poco_assert(UID > 0);
    poco_assert(list);
    for (std::vector<Client *>::iterator it = list->begin();
            it != list->end(); ++it) {
        Client *model = *it;
        model->SetUID(UID);
        error err = SaveClient(model, changes);
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

// FIXME: user generic saveModels instead, delete this
error Database::saveProjects(
        const Poco::UInt64 UID,
         std::vector<Project *> *list,
         std::vector<ModelChange> *changes) {
    poco_assert(UID > 0);
    poco_assert(list);
    for (std::vector<Project *>::iterator it = list->begin();
            it != list->end(); ++it) {
        Project *model = *it;
        model->SetUID(UID);
        error err = SaveProject(model, changes);
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

// FIXME: user generic saveModels instead, delete this
error Database::saveTasks(
        const Poco::UInt64 UID,
        std::vector<Task *> *list,
        std::vector<ModelChange> *changes) {
    poco_assert(UID > 0);
    poco_assert(list);
    for (std::vector<Task *>::iterator it = list->begin();
            it != list->end(); ++it) {
        Task *model = *it;
        model->SetUID(UID);
        error err = SaveTask(model, changes);
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

// FIXME: user generic saveModels instead, delete this
error Database::saveTags(
        const Poco::UInt64 UID,
        std::vector<Tag *> *list,
        std::vector<ModelChange> *changes) {
    poco_assert(UID > 0);
    poco_assert(list);
    for (std::vector<Tag *>::iterator it = list->begin();
            it != list->end(); ++it) {
        Tag *model = *it;
        model->SetUID(UID);
        error err = SaveTag(model, changes);
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

// FIXME: make this generic saveModels or similar
error Database::saveTimeEntries(
        const Poco::UInt64 UID,
        std::vector<TimeEntry *> *list,
        std::vector<ModelChange> *changes) {
    poco_assert(UID > 0);
    poco_assert(list);
    poco_assert(changes);

    {
        std::stringstream ss;
        ss << "Saving time entries in thread " << Poco::Thread::currentTid();
        Poco::Logger &logger = Poco::Logger::get("database");
        logger.debug(ss.str());
    }

    for (std::vector<TimeEntry *>::iterator it = list->begin();
            it != list->end(); ++it) {
        TimeEntry *model = *it;
        if (model->IsMarkedAsDeletedOnServer()) {
            error err = deleteFromTable("time_entries", model->LocalID());
            if (err != noError) {
                return err;
            }
            changes->push_back(ModelChange(
                "time_entry", "delete", model->ID(), model->GUID()));
            continue;
        }
        model->SetUID(UID);
        error err = SaveTimeEntry(model, changes);
        if (err != noError) {
            return err;
        }
    }

    // Purge deleted time entries from memory
    std::vector<TimeEntry *>::iterator it = list->begin();
    while (it != list->end()) {
        TimeEntry *te = *it;
        if (te->IsMarkedAsDeletedOnServer()) {
            it = list->erase(it);
        } else {
            ++it;
        }
    }

    {
        std::stringstream ss;
        ss << "Finished saving time entries in thread " <<
            Poco::Thread::currentTid();
        Poco::Logger &logger = Poco::Logger::get("database");
        logger.debug(ss.str());
    }

    return noError;
}

error Database::SaveTimeEntry(
        TimeEntry *model,
        std::vector<ModelChange> *changes) {
    poco_assert(model);
    poco_assert(session);
    poco_assert(changes);

    Poco::Mutex::ScopedLock lock(mutex_);

    if (model->LocalID() && !model->Dirty() && !model->GUID().empty()) {
        return noError;
    }
    if (model->GUID().empty()) {
        model->SetGUID(generateGUID());
    }
    try {
        Poco::Logger &logger = Poco::Logger::get("database");
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating time entry " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger.debug(ss.str());

            if (model->ID()) {
                *session << "update time_entries set "
                    "id = :id, uid = :uid, description = :description, "
                    "wid = :wid, guid = :guid, pid = :pid, tid = :tid, "
                    "billable = :billable, "
                    "duronly = :duronly, ui_modified_at = :ui_modified_at, "
                    "start = :start, stop = :stop, duration = :duration, "
                    "tags = :tags, created_with = :created_with, "
                    "deleted_at = :deleted_at, "
                    "updated_at = :updated_at "
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
                    Poco::Data::use(model->LocalID()),
                    Poco::Data::now;
            } else {
                *session << "update time_entries set "
                    "uid = :uid, description = :description, wid = :wid, "
                    "guid = :guid, pid = :pid, tid = :tid, "
                    "billable = :billable, "
                    "duronly = :duronly, ui_modified_at = :ui_modified_at, "
                    "start = :start, stop = :stop, duration = :duration, "
                    "tags = :tags, created_with = :created_with, "
                    "deleted_at = :deleted_at, "
                    "updated_at = :updated_at "
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
                    Poco::Data::use(model->LocalID()),
                    Poco::Data::now;
            }
            error err = last_error();
            if (err != noError) {
                return err;
            }
            if (model->DeletedAt()) {
                changes->push_back(ModelChange(
                    "time_entry", "delete", model->ID(), model->GUID()));
            } else {
                changes->push_back(ModelChange(
                    "time_entry", "update", model->ID(), model->GUID()));
            }
        } else {
            std::stringstream ss;
            ss << "Inserting time entry " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger.debug(ss.str());
            if (model->ID()) {
                *session << "insert into time_entries(id, uid, description, "
                    "wid, guid, pid, tid, billable, "
                    "duronly, ui_modified_at, "
                    "start, stop, duration, "
                    "tags, created_with, deleted_at, updated_at) "
                    "values(:id, :uid, :description, :wid, "
                    ":guid, :pid, :tid, :billable, "
                    ":duronly, :ui_modified_at, "
                    ":start, :stop, :duration, "
                    ":tags, :created_with, :deleted_at, :updated_at)",
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
                    Poco::Data::now;
            } else {
                *session << "insert into time_entries(uid, description, wid, "
                    "guid, pid, tid, billable, "
                    "duronly, ui_modified_at, "
                    "start, stop, duration, "
                    "tags, created_with, deleted_at, updated_at) "
                    "values(:uid, :description, :wid, "
                    ":guid, :pid, :tid, :billable, "
                    ":duronly, :ui_modified_at, "
                    ":start, :stop, :duration, "
                    ":tags, :created_with, :deleted_at, :updated_at)",
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
                    Poco::Data::now;
                }
            error err = last_error();
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session << "select last_insert_rowid()",
                Poco::Data::into(local_id),
                Poco::Data::now;
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
              "time_entry", "insert", model->ID(), model->GUID()));
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

error Database::SaveWorkspace(
        Workspace *model,
        std::vector<ModelChange> *changes) {
    poco_assert(model);
    poco_assert(session);

    Poco::Mutex::ScopedLock lock(mutex_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }
    try {
        Poco::Logger &logger = Poco::Logger::get("database");
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating workspace " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger.debug(ss.str());

            *session << "update workspaces set "
                "id = :id, uid = :uid, name = :name "
                "where local_id = :local_id",
                Poco::Data::use(model->ID()),
                Poco::Data::use(model->UID()),
                Poco::Data::use(model->Name()),
                Poco::Data::use(model->LocalID()),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
            changes->push_back(ModelChange(
                    "workspace", "update", model->ID(), ""));

        } else {
            std::stringstream ss;
            ss << "Inserting workspace " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger.debug(ss.str());
            *session << "insert into workspaces(id, uid, name) "
                "values(:id, :uid, :name)",
                Poco::Data::use(model->ID()),
                Poco::Data::use(model->UID()),
                Poco::Data::use(model->Name()),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session << "select last_insert_rowid()",
                Poco::Data::into(local_id),
                Poco::Data::now;
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
              "workspace", "insert", model->ID(), ""));
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

error Database::SaveClient(
        Client *model,
        std::vector<ModelChange> *changes) {
    poco_assert(model);
    poco_assert(session);

    Poco::Mutex::ScopedLock lock(mutex_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }
    try {
        Poco::Logger &logger = Poco::Logger::get("database");
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating client " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger.debug(ss.str());

            // FIXME: check how to property insert null :S
            if (model->GUID().empty()) {
                *session << "update clients set "
                    "id = :id, uid = :uid, name = :name, wid = :wid "
                    "where local_id = :local_id",
                    Poco::Data::use(model->ID()),
                    Poco::Data::use(model->UID()),
                    Poco::Data::use(model->Name()),
                    Poco::Data::use(model->WID()),
                    Poco::Data::use(model->LocalID()),
                    Poco::Data::now;
            } else {
                *session << "update clients set "
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
            error err = last_error();
            if (err != noError) {
                return err;
            }
            changes->push_back(ModelChange(
              "client", "update", model->ID(), model->GUID()));

        } else {
            std::stringstream ss;
            ss << "Inserting client " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger.debug(ss.str());
            // FIXME: check how to property insert null :S
            if (model->GUID().empty()) {
                *session << "insert into clients(id, uid, name, wid) "
                    "values(:id, :uid, :name, :wid)",
                    Poco::Data::use(model->ID()),
                    Poco::Data::use(model->UID()),
                    Poco::Data::use(model->Name()),
                    Poco::Data::use(model->WID()),
                    Poco::Data::now;
            } else {
                *session << "insert into clients(id, uid, name, guid, wid) "
                    "values(:id, :uid, :name, :guid, :wid)",
                    Poco::Data::use(model->ID()),
                    Poco::Data::use(model->UID()),
                    Poco::Data::use(model->Name()),
                    Poco::Data::use(model->GUID()),
                    Poco::Data::use(model->WID()),
                    Poco::Data::now;
            }
            error err = last_error();
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session << "select last_insert_rowid()",
                Poco::Data::into(local_id),
                Poco::Data::now;
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
              "client", "insert", model->ID(), model->GUID()));
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

error Database::SaveProject(
        Project *model,
        std::vector<ModelChange> *changes) {
    poco_assert(model);
    poco_assert(session);

    Poco::Mutex::ScopedLock lock(mutex_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }
    try {
        Poco::Logger &logger = Poco::Logger::get("database");
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating project " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger.debug(ss.str());

            // FIXME: check how to property insert null :S
            if (model->GUID().empty()) {
                *session << "update projects set "
                    "id = :id, uid = :uid, name = :name, "
                    "wid = :wid, color = :color, cid = :cid, "
                    "active = :active "
                    "where local_id = :local_id",
                    Poco::Data::use(model->ID()),
                    Poco::Data::use(model->UID()),
                    Poco::Data::use(model->Name()),
                    Poco::Data::use(model->WID()),
                    Poco::Data::use(model->Color()),
                    Poco::Data::use(model->CID()),
                    Poco::Data::use(model->Active()),
                    Poco::Data::use(model->LocalID()),
                    Poco::Data::now;
            } else {
                *session << "update projects set "
                    "id = :id, uid = :uid, name = :name, guid = :guid,"
                    "wid = :wid, color = :color, cid = :cid, "
                    "active = :active "
                    "where local_id = :local_id",
                    Poco::Data::use(model->ID()),
                    Poco::Data::use(model->UID()),
                    Poco::Data::use(model->Name()),
                    Poco::Data::use(model->GUID()),
                    Poco::Data::use(model->WID()),
                    Poco::Data::use(model->Color()),
                    Poco::Data::use(model->CID()),
                    Poco::Data::use(model->Active()),
                    Poco::Data::use(model->LocalID()),
                    Poco::Data::now;
            }
            error err = last_error();
            if (err != noError) {
                return err;
            }
            changes->push_back(ModelChange(
                    "project", "update", model->ID(), model->GUID()));

        } else {
            std::stringstream ss;
            ss << "Inserting project " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger.debug(ss.str());
            // FIXME: check how to property insert null :S
            if (model->GUID().empty()) {
                *session <<
                    "insert into projects("
                    "id, uid, name, wid, color, cid, active"
                    ") values("
                    ":id, :uid, :name, :wid, :color, :cid, :active"
                    ")",
                    Poco::Data::use(model->ID()),
                    Poco::Data::use(model->UID()),
                    Poco::Data::use(model->Name()),
                    Poco::Data::use(model->WID()),
                    Poco::Data::use(model->Color()),
                    Poco::Data::use(model->CID()),
                    Poco::Data::use(model->Active()),
                    Poco::Data::now;
            } else {
                *session <<
                    "insert into projects("
                    "id, uid, name, guid, wid, color, cid, active"
                    ") values("
                    ":id, :uid, :name, :guid, :wid, :color, :cid, :active"
                    ")",
                    Poco::Data::use(model->ID()),
                    Poco::Data::use(model->UID()),
                    Poco::Data::use(model->Name()),
                    Poco::Data::use(model->GUID()),
                    Poco::Data::use(model->WID()),
                    Poco::Data::use(model->Color()),
                    Poco::Data::use(model->CID()),
                    Poco::Data::use(model->Active()),
                    Poco::Data::now;
            }
            error err = last_error();
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session << "select last_insert_rowid()",
                Poco::Data::into(local_id),
                Poco::Data::now;
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
              "project", "insert", model->ID(), model->GUID()));
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

error Database::SaveTask(
        Task *model,
        std::vector<ModelChange> *changes) {
    poco_assert(model);
    poco_assert(session);

    Poco::Mutex::ScopedLock lock(mutex_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }
    try {
        Poco::Logger &logger = Poco::Logger::get("database");
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating task " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger.debug(ss.str());

            *session << "update tasks set "
                "id = :id, uid = :uid, name = :name, wid = :wid, pid = :pid "
                "where local_id = :local_id",
                Poco::Data::use(model->ID()),
                Poco::Data::use(model->UID()),
                Poco::Data::use(model->Name()),
                Poco::Data::use(model->WID()),
                Poco::Data::use(model->PID()),
                Poco::Data::use(model->LocalID()),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
              return err;
            }
            changes->push_back(ModelChange(
              "task", "update", model->ID(), ""));

        } else {
            std::stringstream ss;
            ss << "Inserting task " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger.debug(ss.str());
            *session << "insert into tasks(id, uid, name, wid, pid) "
                "values(:id, :uid, :name, :wid, :pid)",
                Poco::Data::use(model->ID()),
                Poco::Data::use(model->UID()),
                Poco::Data::use(model->Name()),
                Poco::Data::use(model->WID()),
                Poco::Data::use(model->PID()),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session << "select last_insert_rowid()",
                Poco::Data::into(local_id),
                Poco::Data::now;
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
              "task", "insert", model->ID(), ""));
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

error Database::SaveTag(
        Tag *model,
        std::vector<ModelChange> *changes    ) {
    poco_assert(model);
    poco_assert(session);

    Poco::Mutex::ScopedLock lock(mutex_);

    if (model->LocalID() && !model->Dirty()) {
        return noError;
    }
    try {
        Poco::Logger &logger = Poco::Logger::get("database");
        if (model->LocalID()) {
            std::stringstream ss;
            ss << "Updating tag " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger.debug(ss.str());

            // FIXME: check how to property insert null :S
            if (model->GUID().empty()) {
                *session << "update tags set "
                    "id = :id, uid = :uid, name = :name, wid = :wid "
                    "where local_id = :local_id",
                    Poco::Data::use(model->ID()),
                    Poco::Data::use(model->UID()),
                    Poco::Data::use(model->Name()),
                    Poco::Data::use(model->WID()),
                    Poco::Data::use(model->LocalID()),
                    Poco::Data::now;
            } else {
                *session << "update tags set "
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
            error err = last_error();
            if (err != noError) {
              return err;
            }
            changes->push_back(ModelChange(
                "tag", "update", model->ID(), model->GUID()));

        } else {
            std::stringstream ss;
            ss << "Inserting tag " + model->String()
               << " in thread " << Poco::Thread::currentTid();
            logger.debug(ss.str());
            // FIXME: check how to property insert null :S
            if (model->GUID().empty()) {
                *session << "insert into tags(id, uid, name, wid) "
                    "values(:id, :uid, :name, :wid)",
                    Poco::Data::use(model->ID()),
                    Poco::Data::use(model->UID()),
                    Poco::Data::use(model->Name()),
                    Poco::Data::use(model->WID()),
                    Poco::Data::now;
            } else {
                *session << "insert into tags(id, uid, name, wid, guid) "
                    "values(:id, :uid, :name, :wid, :guid)",
                    Poco::Data::use(model->ID()),
                    Poco::Data::use(model->UID()),
                    Poco::Data::use(model->Name()),
                    Poco::Data::use(model->WID()),
                    Poco::Data::use(model->GUID()),
                    Poco::Data::now;
            }
            error err = last_error();
            if (err != noError) {
                return err;
            }
            Poco::Int64 local_id(0);
            *session << "select last_insert_rowid()",
                Poco::Data::into(local_id),
                Poco::Data::now;
            model->SetLocalID(local_id);
            changes->push_back(ModelChange(
              "tag", "insert", model->ID(), model->GUID()));
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
        User *model,
        const bool with_related_data,
        std::vector<ModelChange> *changes) {
    poco_assert(model);
    poco_assert(session);
    poco_assert(changes);

    Poco::Mutex::ScopedLock lock(mutex_);

    Poco::Logger &logger = Poco::Logger::get("database");

    {
        std::stringstream ss;
        ss << "Saving user in thread " << Poco::Thread::currentTid();
        logger.debug(ss.str());
    }

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    if (model->Email().empty()) {
        return error("Missing user e-mail, cannot save user");
    }
    if (model->APIToken().empty()) {
        return error("Missing user API token, cannot save user");
    }
    if (!model->ID()) {
        return error("Missing user ID, cannot save user");
    }

    session->begin();

    // Check if we really need to save model,
    // *but* do not return if we don't need to.
    // We might need to save related models, still.
    if (!model->LocalID() || model->Dirty()) {
        try {
            if (model->LocalID()) {
                std::stringstream ss;
                ss << "Updating user " + model->String()
                   << " in thread " << Poco::Thread::currentTid();
                logger.debug(ss.str());

                *session << "update users set "
                    "api_token = :api_token, default_wid = :default_wid, "
                    "since = :since, id = :id, fullname = :fullname, "
                    "email = :email, record_timeline = :record_timeline "
                    "where local_id = :local_id",
                    Poco::Data::use(model->APIToken()),
                    Poco::Data::use(model->DefaultWID()),
                    Poco::Data::use(model->Since()),
                    Poco::Data::use(model->ID()),
                    Poco::Data::use(model->Fullname()),
                    Poco::Data::use(model->Email()),
                    Poco::Data::use(model->RecordTimeline()),
                    Poco::Data::use(model->LocalID()),
                    Poco::Data::now;
                error err = last_error();
                if (err != noError) {
                    session->rollback();
                    return err;
                }
                changes->push_back(ModelChange(
                    "user", "update", model->ID(), ""));
            } else {
                std::stringstream ss;
                ss << "Inserting user " + model->String()
                   << " in thread " << Poco::Thread::currentTid();
                logger.debug(ss.str());
                *session << "insert into users("
                    "id, api_token, default_wid, since, fullname, email, "
                    "record_timeline"
                    ") values("
                    ":id, :api_token, :default_wid, :since, :fullname, :email, "
                    ":record_timeline"
                    ")",
                    Poco::Data::use(model->ID()),
                    Poco::Data::use(model->APIToken()),
                    Poco::Data::use(model->DefaultWID()),
                    Poco::Data::use(model->Since()),
                    Poco::Data::use(model->Fullname()),
                    Poco::Data::use(model->Email()),
                    Poco::Data::use(model->RecordTimeline()),
                    Poco::Data::now;
                error err = last_error();
                if (err != noError) {
                    session->rollback();
                    return err;
                }
                Poco::Int64 local_id(0);
                *session << "select last_insert_rowid()",
                    Poco::Data::into(local_id),
                    Poco::Data::now;
                model->SetLocalID(local_id);
                err = last_error();
                if (err != noError) {
                    session->rollback();
                    return err;
                }
                changes->push_back(ModelChange(
                    "user", "insert", model->ID(), ""));
            }
            model->ClearDirty();
        } catch(const Poco::Exception& exc) {
            session->rollback();
            return exc.displayText();
        } catch(const std::exception& ex) {
            session->rollback();
            return ex.what();
        } catch(const std::string& ex) {
            session->rollback();
            return ex;
        }
    }

    if (with_related_data) {
        error err = saveWorkspaces(model->ID(), &model->related.Workspaces,
            changes);
        if (err != noError) {
            session->rollback();
            return err;
        }
        err = saveClients(model->ID(), &model->related.Clients, changes);
        if (err != noError) {
            session->rollback();
            return err;
        }
        err = saveProjects(model->ID(), &model->related.Projects, changes);
        if (err != noError) {
            session->rollback();
            return err;
        }
        err = saveTasks(model->ID(), &model->related.Tasks, changes);
        if (err != noError) {
            session->rollback();
            return err;
        }
        err = saveTags(model->ID(), &model->related.Tags, changes);
        if (err != noError) {
            session->rollback();
            return err;
        }
        err = saveTimeEntries(model->ID(), &model->related.TimeEntries,
            changes);
        if (err != noError) {
            session->rollback();
            return err;
        }
    }

    session->commit();

    stopwatch.stop();

    {
        std::stringstream ss;
        ss  << "User with_related_data=" << with_related_data << " saved in "
            << stopwatch.elapsed() / 1000 << " ms in thread "
            << Poco::Thread::currentTid();
        logger.debug(ss.str());
    }

    return noError;
}

error Database::initialize_tables() {
    poco_assert(session);

    std::string table_name;
    // Check if we have migrations table
    *session <<
        "select name from sqlite_master "
        "where type='table' and name='kopsik_migrations'",
        Poco::Data::into(table_name),
        Poco::Data::limit(1),
        Poco::Data::now;

    if (table_name.length() == 0) {
        *session <<
            "create table kopsik_migrations(id integer primary key, "
            "name varchar not null)",
            Poco::Data::now;
        error err = last_error();
        if (err != noError) {
            return err;
        }
        *session <<
            "CREATE UNIQUE INDEX id_kopsik_migrations_name "
                "ON kopsik_migrations (name);",
            Poco::Data::now;
        err = last_error();
        if (err != noError) {
            return err;
        }
    }

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

    err = migrate("users.api_token",
        "CREATE UNIQUE INDEX id_users_api_token ON users (api_token);");
    if (err != noError) {
        return err;
    }

    err = migrate("workspaces",
        "create table workspaces("
        "local_id integer primary key,"
        "id integer not null, "
        "uid integer not null, "
        "name varchar not null,"
        "constraint fk_workspaces_uid foreign key (uid) "
        "   references users(id) on delete no action on update no action"
        "); ");
    if (err != noError) {
        return err;
    }

    err = migrate("workspaces.id",
        "CREATE UNIQUE INDEX id_workspaces_id ON workspaces (id);");
    if (err != noError) {
        return err;
    }

    err = migrate("clients",
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

    err = migrate("clients.guid",
        "CREATE UNIQUE INDEX id_clients_guid ON clients (uid, guid);");
    if (err != noError) {
        return err;
    }

    err = migrate("projects",
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

    err = migrate("projects.id",
        "CREATE UNIQUE INDEX id_projects_id ON projects (uid, id);");
    if (err != noError) {
        return err;
    }

    err = migrate("projects.guid",
        "CREATE UNIQUE INDEX id_projects_guid ON projects (uid, guid);");
    if (err != noError) {
        return err;
    }

    err = migrate("tasks",
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

    err = migrate("tasks.id",
        "CREATE UNIQUE INDEX id_tasks_id ON tasks (uid, id);");
    if (err != noError) {
        return err;
    }

    err = migrate("tags",
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

    err = migrate("tags.id",
        "CREATE UNIQUE INDEX id_tags_id ON tags (uid, id); ");
    if (err != noError) {
        return err;
    }

    err = migrate("tags.guid",
        "CREATE UNIQUE INDEX id_tags_guid ON tags (uid, guid); ");
    if (err != noError) {
        return err;
    }

    err = migrate("time_entries",
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
        "CREATE UNIQUE INDEX id_time_entries_id ON time_entries (uid, id); ");
    if (err != noError) {
      return err;
    }

    err = migrate("time_entries.guid",
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

    err = migrate("sessions.active",
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
        desktop_id_ = generateGUID();
        err = SaveDesktopID();
        if (err != noError) {
            return err;
        }
    }

    return noError;
}

error Database::CurrentAPIToken(std::string *token) {
    poco_assert(session);
    poco_assert(token);

    Poco::Mutex::ScopedLock lock(mutex_);

    *token = "";
    try {
        *session << "select api_token from sessions",
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
    return last_error();
}

error Database::ClearCurrentAPIToken() {
    poco_assert(session);

    Poco::Mutex::ScopedLock lock(mutex_);

    try {
        *session << "delete from sessions", Poco::Data::now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error();
}

error Database::SetCurrentAPIToken(const std::string &token) {
    poco_assert(session);

    Poco::Mutex::ScopedLock lock(mutex_);

    error err = ClearCurrentAPIToken();
    if (err != noError) {
        return err;
    }
    try {
        *session << "insert into sessions(api_token) values(:api_token)",
            Poco::Data::use(token),
            Poco::Data::now;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error();
}

error Database::SaveDesktopID() {
    poco_assert(session);

    Poco::Mutex::ScopedLock lock(mutex_);

    try {
        *session << "INSERT INTO timeline_installation(desktop_id) "
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
    return last_error();
}

error Database::migrate(
        const std::string name,
        const std::string sql) {
    poco_assert(session);
    poco_assert(!name.empty());
    poco_assert(!sql.empty());
    try {
        int count = 0;
        *session << "select count(*) from kopsik_migrations where name=:name",
            Poco::Data::into(count),
            Poco::Data::use(name),
            Poco::Data::now;

        if (count < 1) {
            *session << sql, Poco::Data::now;

            *session << "insert into kopsik_migrations(name) values(:name)",
                Poco::Data::use(name),
                Poco::Data::now;
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return last_error();
}

error Database::select_timeline_batch(
        const int user_id,
        std::vector<TimelineEvent> *timeline_events) {
    std::stringstream out;
    out << "select_batch, user_id = " << user_id;
    Poco::Logger &logger = Poco::Logger::get("database");
    logger.debug(out.str());

    poco_assert(user_id > 0);
    poco_assert(timeline_events->empty());
    if (!session) {
        logger.warning("select_batch database is not open, ignoring request");
        return noError;
    }
    Poco::Data::Statement select(*session);
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
            event.user_id = user_id;
            timeline_events->push_back(event);
            more = rs.moveNext();
        }
    }

    std::stringstream event_count;
    event_count << "select_batch found " << timeline_events->size()
        <<  " events.";
    logger.debug(event_count.str());

    return last_error();
}

error Database::insert_timeline_event(const TimelineEvent& event) {
    std::stringstream out;
    out << "insert " << event.start_time << ";" << event.end_time << ";"
        << event.filename << ";" << event.title;
    Poco::Logger &logger = Poco::Logger::get("database");
    logger.information(out.str());

    poco_assert(event.user_id > 0);
    poco_assert(event.start_time > 0);
    poco_assert(event.end_time > 0);
    if (!session) {
        logger.information("insert database is not open, ignoring request");
        return noError;
    }
    *session << "INSERT INTO timeline_events("
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
    return last_error();
}

error Database::delete_timeline_batch(
        const std::vector<TimelineEvent> &timeline_events) {
    std::stringstream out;
    out << "delete_batch " << timeline_events.size() << " events.";
    Poco::Logger &logger = Poco::Logger::get("database");
    logger.debug(out.str());

    poco_assert(!timeline_events.empty());
    if (!session) {
        logger.warning("delete_batch database is not open, ignoring request");
        return noError;
    }
    std::vector<int> ids;
    for (std::vector<TimelineEvent>::const_iterator i = timeline_events.begin();
            i != timeline_events.end();
            ++i) {
        const TimelineEvent &event = *i;
        ids.push_back(event.id);
    }
    *session << "DELETE FROM timeline_events WHERE id = :id",
        Poco::Data::use(ids),
        Poco::Data::now;
    return last_error();
}

void Database::handleTimelineEventNotification(
        TimelineEventNotification* notification) {
    Poco::Logger &logger = Poco::Logger::get("database");
    logger.debug("handleTimelineEventNotification");
    insert_timeline_event(notification->event);
}

void Database::handleCreateTimelineBatchNotification(
        CreateTimelineBatchNotification* notification) {
    Poco::Logger &logger = Poco::Logger::get("database");
    logger.debug("handleCreateTimelineBatchNotification");
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
    Poco::Logger &logger = Poco::Logger::get("database");
    logger.debug("handleDeleteTimelineBatchNotification");
    poco_assert(!notification->batch.empty());
    delete_timeline_batch(notification->batch);
}

}   // namespace kopsik
