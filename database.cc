// Copyright 2013 Tanel Lebedev

#include "./database.h"

#include <limits>
#include <string>
#include <vector>

#include "Poco/Data/Common.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/Binding.h"

namespace kopsik {

error Database::Open() {
    session = new Poco::Data::Session("SQLite", "kopsik.db");
    return initialize_tables();
}

void Database::Close() {
    if (session) {
        delete session;
        session = 0;
    }
}

error Database::Save(const User &user) {
    poco_assert(session);

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
    }

    error err = migrate("users",
        "create table users("
        "local_id integer primary key, "
        "id integer not null, "
        "api_token varchar not null, "
        "default_wid integer, "
        "since integer"
        ")");
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
        ")");
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
        ")");
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
        ")");
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
        ")");
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
        ")");
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
        "start integer not null, "
        "stop integer, "
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
        ")");
    if (err != noError) {
        return err;
    }

    return noError;
}

error Database::migrate(std::string name, std::string sql) {
    int count = 0;
    *session << "select count(*) from kopsik_migrations where name=:name",
        Poco::Data::into(count), Poco::Data::use(name), Poco::Data::now;

    if (count < 1) {
        *session << sql, Poco::Data::now;

        *session << "insert into kopsik_migrations(name) values(:name)",
            Poco::Data::use(name), Poco::Data::now;
    }

    return noError;
}

}   // namespace kopsik
