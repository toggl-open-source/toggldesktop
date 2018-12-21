// Copyright 2015 Toggl Desktop developers.

#include "../src/migrations.h"

#include "./const.h"
#include "./database.h"

#include "Poco/Random.h"

namespace toggl {

error Migrations::migrateObmActions() {
    return db_->Migrate(
        "obm_actions",
        "create table obm_actions("
        "local_id integer primary key,"
        "uid integer not null, "
        "experiment_id integer not null, "
        "key varchar not null, "
        "value varchar not null, "
        "constraint fk_obm_actions_uid foreign key (uid) "
        "   references users(id) on delete no action on update no action"
        "); ");
}

error Migrations::migrateObmExperiments() {
    error err = db_->Migrate(
        "obm_experiments",
        "create table obm_experiments("
        "local_id integer primary key,"
        "uid integer not null, "
        "nr integer not null, "
        "has_seen integer not null default 0, "
        "included integer not null default 0, "
        "actions varchar, "
        "constraint fk_obm_experiments_uid foreign key (uid) "
        "   references users(id) on delete no action on update no action"
        "); ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "obm_experiments.nr",
        "CREATE UNIQUE INDEX idx_obm_experiments_nr "
        "   ON obm_experiments (uid, nr);");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "drop obm_experiments.idx_obm_experiments_nr",
        "DROP INDEX IF EXISTS idx_obm_experiments_nr;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "correct obm_experiments.idx_obm_experiments_nr",
        "CREATE UNIQUE INDEX idx_obm_experiments_nr_uid "
        "   ON obm_experiments (uid, nr);");
    if (err != noError) {
        return err;
    }

    return err;
}

error Migrations::migrateAutotracker() {
    error err = db_->Migrate(
        "autotracker_settings",
        "create table autotracker_settings("
        "local_id integer primary key,"
        "uid integer not null, "
        "term varchar not null, "
        "pid integer not null, "
        "constraint fk_autotracker_settings_pid foreign key (pid) "
        "   references projects(id) on delete no action on update no action,"
        "constraint fk_autotracker_settings_uid foreign key (uid) "
        "   references users(id) on delete no action on update no action"
        "); ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "autotracker_settings.term",
        "CREATE UNIQUE INDEX autotracker_settings_term "
        "   ON autotracker_settings (uid, term);");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "autotracker_settings.tid",
        "alter table autotracker_settings"
        " add column tid integer references tasks (id);");
    if (err != noError) {
        return err;
    }

    return noError;
}

error Migrations::migrateClients() {
    error err = db_->Migrate(
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

    err = db_->Migrate(
        "clients.id",
        "CREATE UNIQUE INDEX id_clients_id ON clients (uid, id); ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "clients.guid",
        "CREATE UNIQUE INDEX id_clients_guid ON clients (uid, guid);");
    if (err != noError) {
        return err;
    }

    // Its perfectly fine to have multiple NULL client ID's in the db,
    // when user creates clients offline.
    err = db_->Migrate("drop clients.id_clients_id",
                       "drop index if exists id_clients_id");
    if (err != noError) {
        return err;
    }

    return err;
}

error Migrations::migrateTasks() {
    error err = db_->Migrate(
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

    err = db_->Migrate(
        "tasks.id",
        "CREATE UNIQUE INDEX id_tasks_id ON tasks (uid, id);");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "tasks.active",
        "alter table tasks add column active integer not null default 1;");
    if (err != noError) {
        return err;
    }

    return noError;
}

error Migrations::migrateTags() {
    error err = db_->Migrate(
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

    err = db_->Migrate(
        "tags.id",
        "CREATE UNIQUE INDEX id_tags_id ON tags (uid, id); ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "tags.guid",
        "CREATE UNIQUE INDEX id_tags_guid ON tags (uid, guid); ");
    if (err != noError) {
        return err;
    }

    return noError;
}

error Migrations::migrateSessions() {
    error err = db_->Migrate(
        "sessions",
        "create table sessions("
        "local_id integer primary key, "
        "api_token varchar not null, "
        "active integer not null default 1 "
        "); ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "sessions.active",
        "CREATE UNIQUE INDEX id_sessions_active ON sessions (active); ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "sessions.uid",
        "alter table sessions add column uid integer references users (id);");
    if (err != noError) {
        return err;
    }

    return err;
}

error Migrations::migrateWorkspaces() {
    error err = db_->Migrate(
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

    err = db_->Migrate(
        "workspaces.id",
        "CREATE UNIQUE INDEX id_workspaces_id ON workspaces (uid, id);");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "workspaces.premium",
        "alter table workspaces add column premium int default 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "workspaces.only_admins_may_create_projects",
        "alter table workspaces add column "
        "   only_admins_may_create_projects integer not null default 0; ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "workspaces.admin",
        "alter table workspaces add column "
        "   admin integer not null default 0; ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "workspaces.is_business",
        "alter table workspaces add column "
        "   is_business integer not null default 0; ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "workspaces.locked_date",
        "alter table workspaces add column "
        "   locked_time integer not null default 0; ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "workspaces.projects_billable_by_default",
        "alter table workspaces add column "
        "   projects_billable_by_default integer not null default 0; ");
    if (err != noError) {
        return err;
    }

    return err;
}

error Migrations::migrateProjects() {
    error err = db_->Migrate(
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

    err = db_->Migrate(
        "projects.billable",
        "ALTER TABLE projects ADD billable INT NOT NULL DEFAULT 0");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "projects.is_private",
        "ALTER TABLE projects ADD is_private INT NOT NULL DEFAULT 0");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "projects.client_guid",
        "ALTER TABLE projects "
        "ADD COLUMN client_guid VARCHAR;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "projects.id",
        "CREATE UNIQUE INDEX id_projects_id ON projects (uid, id);");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "projects.guid",
        "CREATE UNIQUE INDEX id_projects_guid ON projects (uid, guid);");
    if (err != noError) {
        return err;
    }

    return err;
}

error Migrations::migrateAnalytics() {
    error err = db_->Migrate(
        "analytics_settings",
        "CREATE TABLE analytics_settings("
        "id INTEGER PRIMARY KEY, "
        "analytics_client_id VARCHAR NOT NULL"
        ")");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "analytics_settings.analytics_client_id",
        "CREATE UNIQUE INDEX id_analytics_settings_client_id "
        "ON analytics_settings(analytics_client_id);");
    if (err != noError) {
        return err;
    }

    err = db_->EnsureAnalyticsClientID();
    if (err != noError) {
        return err;
    }

    return noError;
}

error Migrations::migrateTimeline() {
    error err = db_->Migrate(
        "timeline_installation",
        "CREATE TABLE timeline_installation("
        "id INTEGER PRIMARY KEY, "
        "desktop_id VARCHAR NOT NULL"
        ")");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "timeline_installation.desktop_id",
        "CREATE UNIQUE INDEX id_timeline_installation_desktop_id "
        "ON timeline_installation(desktop_id);");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "timeline_events",
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

    err = db_->EnsureDesktopID();
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "timeline_events.chunked",
        "alter table timeline_events"
        "   add column chunked integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "timeline_events.uploaded",
        "alter table timeline_events"
        "   add column uploaded integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "timeline_events.local_id step #1",
        "ALTER TABLE timeline_events RENAME TO tmp_timeline_events");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "timeline_events.local_id step #2",
        "create table timeline_events("
        "   local_id integer primary key, "
        "   guid varchar, "
        "   title varchar, "
        "   filename varchar, "
        "   uid integer not null, "
        "   start_time INTEGER NOT NULL, "
        "   end_time INTEGER, "
        "   idle INTEGER NOT NULL, "
        "   uploaded integer not null default 0, "
        "   chunked integer not null default 0, "
        "   constraint fk_timeline_events_uid foreign key (uid) "
        "     references users(id) on delete no action on update no action"
        ")");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "timeline_events.local_id step #3",
        "insert into timeline_events"
        "   select id, null, title, filename, user_id, "
        "       start_time, end_time, idle, uploaded, chunked "
        "   from tmp_timeline_events");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "timeline_events.local_id step #4",
        "drop table tmp_timeline_events");
    if (err != noError) {
        return err;
    }

    err = db_->EnsureTimelineGUIDS();
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "timeline_events.guid",
        "CREATE UNIQUE INDEX idx_timeline_events_guid "
        "   ON timeline_events (guid);");
    if (err != noError) {
        return err;
    }

    return noError;
}

error Migrations::migrateUsers() {
    error err = db_->Migrate(
        "users",
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

    err = db_->Migrate(
        "users.store_start_and_stop_time",
        "ALTER TABLE users "
        "ADD COLUMN store_start_and_stop_time INT NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "users.timeofday_format",
        "ALTER TABLE users "
        "ADD COLUMN timeofday_format varchar NOT NULL DEFAULT 'HH:mm';");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "users.id",
        "CREATE UNIQUE INDEX id_users_id ON users (id);");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "users.duration_format",
        "alter table users "
        "add column duration_format varchar "
        "not null default 'classic';");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "drop users.email index",
        "DROP INDEX IF EXISTS id_users_email;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "users.api_token",
        "CREATE UNIQUE INDEX id_users_api_token ON users (api_token);");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "users.offline_data",
        "alter table users"
        " add column offline_data varchar");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "no api token step #1",
        "ALTER TABLE users RENAME TO tmp_users");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
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

    err = db_->Migrate(
        "no api token step #3",
        "insert into users"
        " select local_id, id, default_wid, since, fullname, email,"
        " record_timeline, store_start_and_stop_time, timeofday_format,"
        " duration_format, offline_data"
        " from tmp_users");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "no api token step #4",
        "drop table tmp_users");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "users.default_pid",
        "alter table users"
        " add column default_pid integer");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "users.default_tid",
        "alter table users"
        " add column default_tid integer references tasks (id);");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "users.collapse_entries",
        "alter table users"
        " add column collapse_entries integer not null default 0;");
    if (err != noError) {
        return err;
    }

    return err;
}

error Migrations::migrateTimeEntries() {
    error err = db_->Migrate(
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

    err = db_->Migrate(
        "time_entries.id",
        "CREATE UNIQUE INDEX id_time_entries_id "
        "ON time_entries (uid, id); ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "time_entries.guid",
        "CREATE UNIQUE INDEX id_time_entries_guid "
        "ON time_entries (uid, guid); ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "time_entries.project_guid",
        "ALTER TABLE time_entries "
        "ADD COLUMN project_guid VARCHAR;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "time_entries.guid not null, step 1",
        "ALTER TABLE time_entries RENAME TO tmp_time_entries; ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
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

    err = db_->Migrate(
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

    err = db_->Migrate(
        "time_entries.guid not null, step 4",
        "drop table tmp_time_entries;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "time_entries.guid not null, step 5",
        "CREATE UNIQUE INDEX id_time_entries_id "
        "   ON time_entries (uid, id); ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "time_entries.guid not null, step 6",
        "CREATE UNIQUE INDEX id_time_entries_guid "
        "   ON time_entries (uid, guid); ");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "time_entries.validation_error",
        "ALTER TABLE time_entries "
        "ADD COLUMN validation_error VARCHAR;");
    if (err != noError) {
        return err;
    }

    return noError;
}

error Migrations::migrateSettings() {
    error err = db_->Migrate(
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

    err = db_->Migrate(
        "settings.update_channel",
        "ALTER TABLE settings "
        "ADD COLUMN update_channel varchar not null default 'stable';");
    if (err != noError) {
        return err;
    }

    // for 5% users, set the update channel to 'beta' instead of 'stable'
    Poco::UInt64 has_settings(0);
    err = db_->UInt("select count(1) from settings", &has_settings);
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
        err = db_->Migrate(
            "settings.default",
            "INSERT INTO settings(update_channel) VALUES('" + channel + "')");
        if (err != noError) {
            return err;
        }
    }

    err = db_->Migrate(
        "settings.menubar_timer",
        "ALTER TABLE settings "
        "ADD COLUMN menubar_timer integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.menubar_project",
        "ALTER TABLE settings "
        "ADD COLUMN menubar_project integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.dock_icon",
        "ALTER TABLE settings "
        "ADD COLUMN dock_icon INTEGER NOT NULL DEFAULT 1;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.on_top",
        "ALTER TABLE settings "
        "ADD COLUMN on_top INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.reminder",
        "ALTER TABLE settings "
        "ADD COLUMN reminder INTEGER NOT NULL DEFAULT 1;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.ignore_cert",
        "ALTER TABLE settings "
        "ADD COLUMN ignore_cert INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.idle_minutes",
        "ALTER TABLE settings "
        "ADD COLUMN idle_minutes INTEGER NOT NULL DEFAULT 5;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.focus_on_shortcut",
        "ALTER TABLE settings "
        "ADD COLUMN focus_on_shortcut INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.reminder_minutes",
        "ALTER TABLE settings "
        "ADD COLUMN reminder_minutes INTEGER NOT NULL DEFAULT 10;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.manual_mode",
        "ALTER TABLE settings "
        "ADD COLUMN manual_mode INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "focus on shortcut by default #1",
        "ALTER TABLE settings RENAME TO tmp_settings");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
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

    err = db_->Migrate(
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

    err = db_->Migrate(
        "focus on shortcut by default #4",
        "drop table tmp_settings");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "focus on shortcut by default #5",
        "update settings set focus_on_shortcut = 1");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.autodetect_proxy",
        "ALTER TABLE settings "
        "ADD COLUMN autodetect_proxy INTEGER NOT NULL DEFAULT 1;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.window_x",
        "ALTER TABLE settings "
        "ADD COLUMN window_x integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.window_y",
        "ALTER TABLE settings "
        "ADD COLUMN window_y integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.window_height",
        "ALTER TABLE settings "
        "ADD COLUMN window_height integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.window_width",
        "ALTER TABLE settings "
        "ADD COLUMN window_width integer not null default 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.remind_mon",
        "ALTER TABLE settings "
        "ADD COLUMN remind_mon integer not null default 1;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.remind_tue",
        "ALTER TABLE settings "
        "ADD COLUMN remind_tue integer not null default 1;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.remind_wed",
        "ALTER TABLE settings "
        "ADD COLUMN remind_wed integer not null default 1;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.remind_thu",
        "ALTER TABLE settings "
        "ADD COLUMN remind_thu integer not null default 1;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.remind_fri",
        "ALTER TABLE settings "
        "ADD COLUMN remind_fri integer not null default 1;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.remind_sat",
        "ALTER TABLE settings "
        "ADD COLUMN remind_sat integer not null default 1;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.remind_sun",
        "ALTER TABLE settings "
        "ADD COLUMN remind_sun integer not null default 1;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.remind_starts",
        "ALTER TABLE settings "
        "ADD COLUMN remind_starts varchar not null default '';");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.remind_ends",
        "ALTER TABLE settings "
        "ADD COLUMN remind_ends varchar not null default '';");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.autotrack",
        "ALTER TABLE settings "
        "ADD COLUMN autotrack INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.open_editor_on_shortcut",
        "ALTER TABLE settings "
        "ADD COLUMN open_editor_on_shortcut INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.has_seen_beta_offering",
        "ALTER TABLE settings "
        "ADD COLUMN has_seen_beta_offering INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.render_timeline",
        "ALTER TABLE settings "
        "ADD COLUMN render_timeline INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.window_maximized",
        "ALTER TABLE settings "
        "ADD COLUMN window_maximized INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.window_minimized",
        "ALTER TABLE settings "
        "ADD COLUMN window_minimized INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.window_edit_size_height",
        "ALTER TABLE settings "
        "ADD COLUMN window_edit_size_height INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.window_edit_size_width",
        "ALTER TABLE settings "
        "ADD COLUMN window_edit_size_width INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.key_start",
        "ALTER TABLE settings "
        "ADD COLUMN key_start varchar not null default ''");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.key_show",
        "ALTER TABLE settings "
        "ADD COLUMN key_show varchar not null default '';");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.key_modifier_show",
        "ALTER TABLE settings "
        "ADD COLUMN key_modifier_show varchar not null default '';");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.key_modifier_start",
        "ALTER TABLE settings "
        "ADD COLUMN key_modifier_start varchar not null default '';");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.compact_mode",
        "ALTER TABLE settings "
        "ADD COLUMN compact_mode INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.keep_end_time_fixed",
        "ALTER TABLE settings "
        "ADD COLUMN keep_end_time_fixed INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.mini_timer_x",
        "ALTER TABLE settings "
        "ADD COLUMN mini_timer_x INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.mini_timer_y",
        "ALTER TABLE settings "
        "ADD COLUMN mini_timer_y INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.mini_timer_w",
        "ALTER TABLE settings "
        "ADD COLUMN mini_timer_w INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.pomodoro",
        "ALTER TABLE settings "
        "ADD COLUMN pomodoro INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.pomodoro_break",
        "ALTER TABLE settings "
        "ADD COLUMN pomodoro_break INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.mini_timer_visible",
        "ALTER TABLE settings "
        "ADD COLUMN mini_timer_visible INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.pomodoro_minutes",
        "ALTER TABLE settings "
        "ADD COLUMN pomodoro_minutes INTEGER NOT NULL DEFAULT 25;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.pomodoro_break_minutes",
        "ALTER TABLE settings "
        "ADD COLUMN pomodoro_break_minutes INTEGER NOT NULL DEFAULT 5;");
    if (err != noError) {
        return err;
    }

    err = db_->Migrate(
        "settings.stop_entry_on_shutdown_sleep",
        "ALTER TABLE settings "
        "ADD COLUMN stop_entry_on_shutdown_sleep INTEGER NOT NULL DEFAULT 0;");
    if (err != noError) {
        return err;
    }

    return noError;
}

error Migrations::Run() {
    error err = noError;

    // FIXME: load known migrations before proceeding
    // FIXME: dont run db->Migrate directly, but consult existing list first

    if (noError == err) {
        err = migrateUsers();
    }
    if (noError == err) {
        err = migrateWorkspaces();
    }
    if (noError == err) {
        err = migrateClients();
    }
    if (noError == err) {
        err = migrateProjects();
    }
    if (noError == err) {
        err = migrateTasks();
    }
    if (noError == err) {
        err = migrateTags();
    }
    if (noError == err) {
        err = migrateTimeEntries();
    }
    if (noError == err) {
        err = migrateSessions();
    }
    if (noError == err) {
        err = migrateSettings();
    }
    if (noError == err) {
        err = migrateAnalytics();
    }
    if (noError == err) {
        err = migrateAutotracker();
    }
    if (noError == err) {
        err = migrateTimeline();
    }
    if (noError == err) {
        err = migrateObmActions();
    }
    if (noError == err) {
        err = migrateObmExperiments();
    }

    return err;
}

}   // namespace toggl
