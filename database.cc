// Copyright 2013 Tanel Lebedev

#include "./database.h"

#include <limits>
#include <string>
#include <vector>

#include "./toggl_api_client.h"

#include "Poco/Logger.h"
#include "Poco/Stopwatch.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/Binding.h"
#include "Poco/Data/SQLite/SessionImpl.h"
#include "Poco/Data/SQLite/Utility.h"

namespace kopsik {

error Database::Delete(User *model, bool with_related_data) {
    error err = deleteFromTable("users", model->LocalID);
    if (err != noError) {
        return err;
    }
    if (with_related_data) {
        err = deleteAllFromTableByUID("workspaces", model->ID);
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("clients", model->ID);
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("projects", model->ID);
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("tasks", model->ID);
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("tags", model->ID);
        if (err != noError) {
            return err;
        }
        err = deleteAllFromTableByUID("time_entries", model->ID);
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

error Database::Delete(TimeEntry *model) {
    return deleteFromTable("time_entries", model->LocalID);
}

error Database::Delete(Tag *model) {
    return deleteFromTable("tags", model->LocalID);
}

error Database::Delete(Workspace *model) {
    return deleteFromTable("workspaces", model->LocalID);
}

error Database::Delete(Task *model) {
    return deleteFromTable("tasks", model->LocalID);
}

error Database::Delete(Project *model) {
    return deleteFromTable("projects", model->LocalID);
}

error Database::Delete(Client *model) {
    return deleteFromTable("clients", model->LocalID);
}

error Database::deleteAllFromTableByUID(std::string table_name,
        Poco::Int64 UID) {
    poco_assert(session);
    poco_assert(UID > 0);
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

error Database::deleteFromTable(std::string table_name, Poco::Int64 local_id) {
    poco_assert(session);
    try {
        if (local_id != 0) {
            *session << "delete from " + table_name +
                " where local_id = :local_id",
                Poco::Data::use(local_id),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
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

error Database::Load(Poco::UInt64 UID, User *model, bool with_related_data) {
    poco_assert(session);

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    Poco::Logger &logger = Poco::Logger::get("database");

    try {
        *session << "select local_id, id, api_token, default_wid, since "
            "from users where id = :id limit 1",
            Poco::Data::into(model->LocalID),
            Poco::Data::into(model->ID),
            Poco::Data::into(model->APIToken),
            Poco::Data::into(model->DefaultWID),
            Poco::Data::into(model->Since),
            Poco::Data::use(UID),
            Poco::Data::now;
        error err = last_error();
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
    if (with_related_data) {
        error err = loadWorkspaces(model);
        if (err != noError) {
            return err;
        }
        err = loadClients(model);
        if (err != noError) {
            return err;
        }
        err = loadProjects(model);
        if (err != noError) {
            return err;
        }
        err = loadTasks(model);
        if (err != noError) {
            return err;
        }
        err = loadTags(model);
        if (err != noError) {
            return err;
        }
        err = loadTimeEntries(model);
        if (err != noError) {
            return err;
        }
    }

    stopwatch.stop();
    std::stringstream ss;
    ss << "User with_related_data=" << with_related_data << " loaded in "
        << stopwatch.elapsed() / 1000 << " ms" << std::endl;
    logger.debug(ss.str());

    return noError;
}


error Database::UInt(std::string sql, Poco::UInt64 *result) {
    poco_assert(session);
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
    return noError;
}

error Database::validate(User *user) {
    if (user->APIToken.empty()) {
        return error("Missing APIToken");
    }
    if (user->ID <= 0) {
        return error("Missing ID");
    }
    return noError;
}

error Database::loadWorkspaces(User *user) {
    poco_assert(user->ID > 0);
    user->ClearWorkspaces();
    Poco::Data::Statement select(*session);
    select << "SELECT local_id, id, uid, name "
        "FROM workspaces WHERE uid = :uid "
        "ORDER BY name",
        Poco::Data::use(user->ID);
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
            model->LocalID = rs[0].convert<Poco::Int64>();
            model->ID = rs[1].convert<Poco::UInt64>();
            model->UID = rs[2].convert<Poco::UInt64>();
            model->Name = rs[3].convert<std::string>();
            user->Workspaces.push_back(model);
            more = rs.moveNext();
        }
    }
    return noError;
}

error Database::loadClients(User *user) {
    poco_assert(user->ID > 0);
    user->ClearClients();
    Poco::Data::Statement select(*session);
    select << "SELECT local_id, id, uid, name, guid, wid "
        "FROM clients WHERE uid = :uid "
        "ORDER BY name",
        Poco::Data::use(user->ID);
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
            model->LocalID = rs[0].convert<Poco::Int64>();
            model->ID = rs[1].convert<Poco::UInt64>();
            model->UID = rs[2].convert<Poco::UInt64>();
            model->Name = rs[3].convert<std::string>();
            model->GUID = rs[4].convert<std::string>();
            model->WID = rs[5].convert<Poco::UInt64>();
            user->Clients.push_back(model);
            more = rs.moveNext();
        }
    }
    return noError;
}

error Database::loadProjects(User *user) {
    poco_assert(user->ID > 0);
    user->ClearProjects();
    Poco::Data::Statement select(*session);
    select << "SELECT local_id, id, uid, name, guid, wid "
        "FROM projects WHERE uid = :uid "
        "ORDER BY name",
        Poco::Data::use(user->ID);
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
            model->LocalID = rs[0].convert<Poco::Int64>();
            model->ID = rs[1].convert<Poco::UInt64>();
            model->UID = rs[2].convert<Poco::UInt64>();
            model->Name = rs[3].convert<std::string>();
            model->GUID = rs[4].convert<std::string>();
            model->WID = rs[5].convert<Poco::UInt64>();
            user->Projects.push_back(model);
            more = rs.moveNext();
        }
    }
    return noError;
}

error Database::loadTasks(User *user) {
    poco_assert(user->ID > 0);
    user->ClearTasks();
    Poco::Data::Statement select(*session);
    select << "SELECT local_id, id, uid, name, wid, pid "
        "FROM tasks WHERE uid = :uid "
        "ORDER BY name",
        Poco::Data::use(user->ID);
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
            model->LocalID = rs[0].convert<Poco::Int64>();
            model->ID = rs[1].convert<Poco::UInt64>();
            model->UID = rs[2].convert<Poco::UInt64>();
            model->Name = rs[3].convert<std::string>();
            model->WID = rs[4].convert<Poco::UInt64>();
            model->PID = rs[5].convert<Poco::UInt64>();
            user->Tasks.push_back(model);
            more = rs.moveNext();
        }
    }
    return noError;
}

error Database::loadTags(User *user) {
    poco_assert(user->ID > 0);
    user->ClearTags();
    Poco::Data::Statement select(*session);
    select << "SELECT local_id, id, uid, name, wid, guid "
        "FROM tags WHERE uid = :uid "
        "ORDER BY name",
        Poco::Data::use(user->ID);
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
            model->LocalID = rs[0].convert<Poco::Int64>();
            model->ID = rs[1].convert<Poco::UInt64>();
            model->UID = rs[2].convert<Poco::UInt64>();
            model->Name = rs[3].convert<std::string>();
            model->WID = rs[4].convert<Poco::UInt64>();
            model->GUID = rs[5].convert<std::string>();
            user->Tags.push_back(model);
            more = rs.moveNext();
        }
    }
    return noError;
}

error Database::loadTimeEntries(User *user) {
    poco_assert(user->ID > 0);
    user->ClearTimeEntries();
    Poco::Data::Statement select(*session);
    select << "SELECT local_id, id, uid, description, wid, guid, pid, "
        "tid, billable, duronly, ui_modified_at, start, stop, "
        "duration, tags "
        "FROM time_entries WHERE uid = :uid "
        "ORDER BY start DESC",
        Poco::Data::use(user->ID);
    error err = last_error();
    if (err != noError) {
        return err;
    }
    Poco::Data::RecordSet rs(select);
    while (!select.done()) {
        select.execute();
        bool more = rs.moveFirst();
        while (more) {
            TimeEntry *model = new TimeEntry();
            model->LocalID = rs[0].convert<Poco::Int64>();
            model->ID = rs[1].convert<Poco::UInt64>();
            model->UID = rs[2].convert<Poco::UInt64>();
            model->Description = rs[3].convert<std::string>();
            model->WID = rs[4].convert<Poco::UInt64>();
            model->GUID = rs[5].convert<std::string>();
            model->PID = rs[6].convert<Poco::UInt64>();
            model->TID = rs[7].convert<Poco::UInt64>();
            model->Billable = rs[8].convert<bool>();
            model->DurOnly = rs[9].convert<bool>();
            model->UIModifiedAt = rs[10].convert<Poco::UInt64>();
            model->Start = rs[11].convert<std::string>();
            model->Stop = rs[12].convert<std::string>();
            model->DurationInSeconds = rs[13].convert<Poco::Int64>();
            model->SetTags(rs[14].convert<std::string>());
            user->TimeEntries.push_back(model);
            more = rs.moveNext();
        }
    }
    return noError;
}

error Database::saveList(Poco::UInt64 UID, std::vector<Workspace *> *list) {
    for (std::vector<Workspace *>::iterator it = list->begin();
            it != list->end(); ++it) {
        Workspace *model = *it;
        model->UID = UID;
        error err = Save(model);
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

error Database::saveList(Poco::UInt64 UID, std::vector<Client *> *list) {
    for (std::vector<Client *>::iterator it = list->begin();
            it != list->end(); ++it) {
        Client *model = *it;
        model->UID = UID;
        error err = Save(model);
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

error Database::saveList(Poco::UInt64 UID, std::vector<Project *> *list) {
    for (std::vector<Project *>::iterator it = list->begin();
            it != list->end(); ++it) {
        Project *model = *it;
        model->UID = UID;
        error err = Save(model);
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

error Database::saveList(Poco::UInt64 UID, std::vector<Task *> *list) {
    for (std::vector<Task *>::iterator it = list->begin();
            it != list->end(); ++it) {
        Task *model = *it;
        model->UID = UID;
        error err = Save(model);
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

error Database::saveList(Poco::UInt64 UID, std::vector<Tag *> *list) {
    for (std::vector<Tag *>::iterator it = list->begin();
            it != list->end(); ++it) {
        Tag *model = *it;
        model->UID = UID;
        error err = Save(model);
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

error Database::saveList(Poco::UInt64 UID, std::vector<TimeEntry *> *list) {
    for (std::vector<TimeEntry *>::iterator it = list->begin();
            it != list->end(); ++it) {
        TimeEntry *model = *it;
        model->UID = UID;
        error err = Save(model);
        if (err != noError) {
            return err;
        }
    }
    return noError;
}

error Database::Save(TimeEntry *model) {
    poco_assert(session);
    try {
        if (model->ID > 0) {
            *session << "select local_id from time_entries where id = :id",
                Poco::Data::into(model->LocalID),
                Poco::Data::use(model->ID),
                Poco::Data::limit(1),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
        }
        Poco::Logger &logger = Poco::Logger::get("database");
        if (model->LocalID != 0) {
            logger.debug("Updating time entry " + model->String());
            *session << "update time_entries set "
                "id = :id, uid = :uid, description = :description, wid = :wid, "
                "guid = :guid, pid = :pid, tid = :tid, billable = :billable, "
                "duronly = :duronly, ui_modified_at = :ui_modified_at, "
                "start = :start, stop = :stop, duration = :duration, "
                "tags = :tags "
                "where local_id = :local_id",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->UID),
                Poco::Data::use(model->Description),
                Poco::Data::use(model->WID),
                Poco::Data::use(model->GUID),
                Poco::Data::use(model->PID),
                Poco::Data::use(model->TID),
                Poco::Data::use(model->Billable),
                Poco::Data::use(model->DurOnly),
                Poco::Data::use(model->UIModifiedAt),
                Poco::Data::use(model->Start),
                Poco::Data::use(model->Stop),
                Poco::Data::use(model->DurationInSeconds),
                Poco::Data::use(model->Tags()),
                Poco::Data::use(model->LocalID),
                Poco::Data::now;
        } else {
            logger.debug("Inserting time entry " + model->String());
            *session << "insert into time_entries(id, uid, description, wid, "
                "guid, pid, tid, billable, "
                "duronly, ui_modified_at, "
                "start, stop, duration, "
                "tags) "
                "values(:id, :uid, :description, :wid, "
                ":guid, :pid, :tid, :billable, "
                ":duronly, :ui_modified_at, "
                ":start, :stop, :duration, "
                ":tags)",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->UID),
                Poco::Data::use(model->Description),
                Poco::Data::use(model->WID),
                Poco::Data::use(model->GUID),
                Poco::Data::use(model->PID),
                Poco::Data::use(model->TID),
                Poco::Data::use(model->Billable),
                Poco::Data::use(model->DurOnly),
                Poco::Data::use(model->UIModifiedAt),
                Poco::Data::use(model->Start),
                Poco::Data::use(model->Stop),
                Poco::Data::use(model->DurationInSeconds),
                Poco::Data::use(model->Tags()),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
            *session << "select last_insert_rowid()",
                Poco::Data::into(model->LocalID),
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

error Database::Save(Workspace *model) {
    poco_assert(session);
    try {
        if (model->ID > 0) {
            *session << "select local_id from workspaces where id = :id",
                Poco::Data::into(model->LocalID),
                Poco::Data::use(model->ID),
                Poco::Data::limit(1),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
        }
        Poco::Logger &logger = Poco::Logger::get("database");
        if (model->LocalID != 0) {
            logger.debug("Updating workspace " + model->String());
            *session << "update workspaces set "
                "id = :id, uid = :uid, name = :name "
                "where local_id = :local_id",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->UID),
                Poco::Data::use(model->Name),
                Poco::Data::use(model->LocalID),
                Poco::Data::now;
        } else {
            logger.debug("Inserting workspace " + model->String());
            *session << "insert into workspaces(id, uid, name) "
                "values(:id, :uid, :name)",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->UID),
                Poco::Data::use(model->Name),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
            *session << "select last_insert_rowid()",
                Poco::Data::into(model->LocalID),
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

error Database::Save(Client *model) {
    poco_assert(session);
    try {
        if (model->ID > 0) {
            *session << "select local_id from clients where id = :id",
                Poco::Data::into(model->LocalID),
                Poco::Data::use(model->ID),
                Poco::Data::limit(1),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
        }
        Poco::Logger &logger = Poco::Logger::get("database");
        if (model->LocalID != 0) {
            logger.debug("Updating client " + model->String());
            *session << "update clients set "
                "id = :id, uid = :uid, name = :name, guid = :guid, wid = :wid "
                "where local_id = :local_id",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->UID),
                Poco::Data::use(model->Name),
                Poco::Data::use(model->GUID),
                Poco::Data::use(model->WID),
                Poco::Data::use(model->LocalID),
                Poco::Data::now;
        } else {
            logger.debug("Inserting client " + model->String());
            *session << "insert into clients(id, uid, name, guid, wid) "
                "values(:id, :uid, :name, :guid, :wid)",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->UID),
                Poco::Data::use(model->Name),
                Poco::Data::use(model->GUID),
                Poco::Data::use(model->WID),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
            *session << "select last_insert_rowid()",
                Poco::Data::into(model->LocalID),
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

error Database::Save(Project *model) {
    poco_assert(session);
    try {
        if (model->ID > 0) {
            *session << "select local_id from projects where id = :id",
                Poco::Data::into(model->LocalID),
                Poco::Data::use(model->ID),
                Poco::Data::limit(1),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
        }
        Poco::Logger &logger = Poco::Logger::get("database");
        if (model->LocalID != 0) {
            logger.debug("Updating project " + model->String());
            *session << "update projects set "
                "id = :id, uid = :uid, name = :name, guid = :guid, wid = :wid "
                "where local_id = :local_id",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->UID),
                Poco::Data::use(model->Name),
                Poco::Data::use(model->GUID),
                Poco::Data::use(model->WID),
                Poco::Data::use(model->LocalID),
                Poco::Data::now;
        } else {
            logger.debug("Inserting project " + model->String());
            *session << "insert into projects(id, uid, name, guid, wid) "
                "values(:id, :uid, :name, :guid, :wid)",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->UID),
                Poco::Data::use(model->Name),
                Poco::Data::use(model->GUID),
                Poco::Data::use(model->WID),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
            *session << "select last_insert_rowid()",
                Poco::Data::into(model->LocalID),
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

error Database::Save(Task *model) {
    poco_assert(session);
    try {
        if (model->ID > 0) {
            *session << "select local_id from tasks where id = :id",
                Poco::Data::into(model->LocalID),
                Poco::Data::use(model->ID),
                Poco::Data::limit(1),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
        }
        Poco::Logger &logger = Poco::Logger::get("database");
        if (model->LocalID != 0) {
            logger.debug("Updating task " + model->String());
            *session << "update tasks set "
                "id = :id, uid = :uid, name = :name, wid = :wid, pid = :pid "
                "where local_id = :local_id",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->UID),
                Poco::Data::use(model->Name),
                Poco::Data::use(model->WID),
                Poco::Data::use(model->PID),
                Poco::Data::use(model->LocalID),
                Poco::Data::now;
        } else {
            logger.debug("Inserting task " + model->String());
            *session << "insert into tasks(id, uid, name, wid, pid) "
                "values(:id, :uid, :name, :wid, :pid)",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->UID),
                Poco::Data::use(model->Name),
                Poco::Data::use(model->WID),
                Poco::Data::use(model->PID),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
            *session << "select last_insert_rowid()",
                Poco::Data::into(model->LocalID),
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

error Database::Save(Tag *model) {
    poco_assert(session);
    try {
        if (model->ID > 0) {
            *session << "select local_id from tags where id = :id",
                Poco::Data::into(model->LocalID),
                Poco::Data::use(model->ID),
                Poco::Data::limit(1),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
        }
        Poco::Logger &logger = Poco::Logger::get("database");
        if (model->LocalID != 0) {
            logger.debug("Updating tag " + model->String());
            *session << "update tags set "
                "id = :id, uid = :uid, name = :name, wid = :wid, guid = :guid "
                "where local_id = :local_id",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->UID),
                Poco::Data::use(model->Name),
                Poco::Data::use(model->WID),
                Poco::Data::use(model->GUID),
                Poco::Data::use(model->LocalID),
                Poco::Data::now;
        } else {
            logger.debug("Inserting tag " + model->String());
            *session << "insert into tags(id, uid, name, wid, guid) "
                "values(:id, :uid, :name, :wid, :guid)",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->UID),
                Poco::Data::use(model->Name),
                Poco::Data::use(model->WID),
                Poco::Data::use(model->GUID),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
            *session << "select last_insert_rowid()",
                Poco::Data::into(model->LocalID),
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

error Database::Save(User *model, bool with_related_data) {
    poco_assert(session);
    error err = validate(model);
    if (err != noError) {
        return err;
    }

    Poco::Stopwatch stopwatch;
    stopwatch.start();

    Poco::Logger &logger = Poco::Logger::get("database");

    try {
        if (model->ID > 0) {
            *session << "select local_id from users where id = :id",
                Poco::Data::into(model->LocalID),
                Poco::Data::use(model->ID),
                Poco::Data::limit(1),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
        }
        if (model->LocalID != 0) {
            logger.debug("Updating user " + model->String());
            *session << "update users set "
                "api_token = :api_token, default_wid = :default_wid, "
                "since = :since, id = :id "
                "where local_id = :local_id",
                Poco::Data::use(model->APIToken),
                Poco::Data::use(model->DefaultWID),
                Poco::Data::use(model->Since),
                Poco::Data::use(model->ID),
                Poco::Data::use(model->LocalID),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
        } else {
            logger.debug("Inserting user " + model->String());
            *session << "insert into users(id, api_token, default_wid, since) "
                "values(:id, :api_token, :default_wid, :since)",
                Poco::Data::use(model->ID),
                Poco::Data::use(model->APIToken),
                Poco::Data::use(model->DefaultWID),
                Poco::Data::use(model->Since),
                Poco::Data::now;
            error err = last_error();
            if (err != noError) {
                return err;
            }
            *session << "select last_insert_rowid()",
                Poco::Data::into(model->LocalID),
                Poco::Data::now;
            err = last_error();
            if (err != noError) {
                return err;
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }

    if (with_related_data) {
        err = saveList(model->ID, &model->Workspaces);
        if (err != noError) {
            return err;
        }
        err = saveList(model->ID, &model->Clients);
        if (err != noError) {
            return err;
        }
        err = saveList(model->ID, &model->Projects);
        if (err != noError) {
            return err;
        }
        err = saveList(model->ID, &model->Tasks);
        if (err != noError) {
            return err;
        }
        err = saveList(model->ID, &model->Tags);
        if (err != noError) {
            return err;
        }
        err = saveList(model->ID, &model->TimeEntries);
        if (err != noError) {
            return err;
        }
    }

    stopwatch.stop();
    std::stringstream ss;
    ss << "User with_related_data=" << with_related_data << " saved in "
        << stopwatch.elapsed() / 1000 << " ms" << std::endl;
    logger.debug(ss.str());

    return noError;
}

error Database::initialize_tables() {
    poco_assert(session);

    std::string table_name;
    // Check if we have migrations table
    *session <<
        "select name from sqlite_master "
        "where type='table' and name='kopsik_migrations'",
        Poco::Data::into(table_name), Poco::Data::limit(1), Poco::Data::now;

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
        "since integer"
        "); "
        "CREATE UNIQUE INDEX id_users_id ON users (id);");
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
        "references users(id) on delete no action on update no action"
        "); "
        "CREATE UNIQUE INDEX id_workspaces_id ON workspaces (id);");
    if (err != noError) {
        return err;
    }

    err = migrate("clients",
        "create table clients("
        "local_id integer primary key,"
        "id integer not null, "
        "uid integer not null, "
        "name varchar not null, "
        "guid varchar, "
        "wid integer not null, "
        "constraint fk_clients_wid foreign key (wid) "
        "references workpaces(id) on delete no action on update no action,"
        "constraint fk_clients_uid foreign key (uid) "
        "references users(id) on delete no action on update no action"
        "); "
        "CREATE UNIQUE INDEX id_clients_id ON clients (id); "
        "CREATE UNIQUE INDEX id_clients_guid ON clients (guid);");
    if (err != noError) {
        return err;
    }

    err = migrate("projects",
        "create table projects("
        "local_id integer primary key, "
        "id integer not null, "
        "uid integer not null, "
        "name varchar not null, "
        "guid varchar, "
        "wid integer not null, "
        "constraint fk_projects_wid foreign key (wid) "
        "references workpaces(id) on delete no action on update no action,"
        "constraint fk_projects_uid foreign key (uid) "
        "referENCES users(id) ON DELETE NO ACTION ON UPDATE NO ACTION"
        "); "
        "CREATE UNIQUE INDEX id_projects_id ON projects (id) "
        "CREATE UNIQUE INDEX id_projects_guid ON projects (guid) ");
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
        "references workpaces(id) on delete no action on update no action, "
        "constraint fk_tasks_pid foreign key (pid) "
        "references projects(id) on delete no action on update no action, "
        "constraint fk_tasks_uid foreign key (uid) "
        "references users(id) on delete no action on update no action "
        "); "
        "CREATE UNIQUE INDEX id_tasks_id ON tasks (id);");
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
        "references workspaces(id) on delete no action on update no action,"
        "constraint fk_tags_uid foreign key (uid) "
        "references users(id) on delete no action on update no action"
        "); "
        "CREATE UNIQUE INDEX id_tags_id ON tags (id); "
        "CREATE UNIQUE INDEX id_tags_guid ON tags (guid); ");
    if (err != noError) {
        return err;
    }

    err = migrate("time_entries",
        "create table time_entries("
        "local_id integer primary key, "
        "id integer not null, "
        "uid integer not null, "
        "description varchar, "
        "wid integer not null, "
        "guid varchar, "
        "pid integer, "
        "tid integer, "
        "billable integer not null default 0,"
        "duronly integer not null default 0, "
        "ui_modified_at integer, "
        "start varchar not null, "
        "stop varchar, "
        "duration integer not null,"
        "tags text,"
        "constraint fk_time_entries_wid foreign key (wid) "
        "references workspaces(id) on delete no action on update no action, "
        "constraint fk_time_entries_pid foreign key (pid) "
        "references projects(id) on delete no action on update no action, "
        "constraint fk_time_entries_tid foreign key (tid) "
        "references tasks(id) on delete no action on update no action, "
        "constraint fk_time_entries_uid foreign key (uid) "
        "references users(id) on delete no action on update no action"
        "); "
        "CREATE UNIQUE INDEX id_time_entries_id ON time_entries (id); "
        "CREATE UNIQUE INDEX id_time_entries_guid ON time_entries (guid); ");
    return last_error();
}

error Database::migrate(std::string name, std::string sql) {
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

    return noError;
}

}   // namespace kopsik
