
#include "database.h"

#include <limits>
#include <string>
#include <vector>
#include <iostream>

#include "Poco/Data/Common.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/Binding.h"

namespace kopsik {

error Database::open_database() {
	ses_ = new Poco::Data::Session("SQLite", "kopsik.db");
	return initialize_tables();
}

void Database::close_database() {
	if (ses_) {
		delete ses_;
		ses_ = 0;
	}
}

error Database::initialize_tables() {
	poco_assert(ses_);
	std::string table_name;
	// Check if we have migrations table
	*ses_ << "SELECT name FROM sqlite_master WHERE type='table' AND name='kopsik_migrations'",
		Poco::Data::into(table_name), Poco::Data::limit(1), Poco::Data::now;

	if (table_name.length() == 0) {
		*ses_ << "CREATE TABLE kopsik_migrations(id INTEGER PRIMARY KEY, name VARCHAR NOT NULL)", Poco::Data::now;
	}

	error err = migrate("users", "CREATE TABLE users(local_id INTEGER PRIMARY KEY, id INTEGER NOT NULL, api_token VARCHAR NOT NULL, "
		"default_wid INTEGER, since INTEGER)");
	if (err != noError) {
		return err;
	}

	err = migrate("workspaces", "CREATE TABLE workspaces(local_id INTEGER PRIMARY KEY, id INTEGER NOT NULL, name VARCHAR NOT NULL)");
	if (err != noError) {
		return err;
	}

	err = migrate("clients", "CREATE TABLE clients(local_id INTEGER PRIMARY KEY, id INTEGER NOT NULL, name VARCHAR NOT NULL, "
		"guid VARCHAR, wid INTEGER NOT NULL, "
		"CONSTRAINT fk_clients_wid FOREIGN KEY (wid) REFERENCES workpaces(id) ON DELETE NO ACTION ON UPDATE NO ACTION)");
	if (err != noError) {
		return err;
	}

	err = migrate("projects", "CREATE TABLE projects(local_id INTEGER PRIMARY KEY, id INTEGER NOT NULL, name VARCHAR NOT NULL, "
		"guid VARCHAR, wid INTEGER NOT NULL, "
		"CONSTRAINT fk_projects_wid FOREIGN KEY (wid) REFERENCES workpaces(id) ON DELETE NO ACTION ON UPDATE NO ACTION)");
	if (err != noError) {
		return err;
	}

	err = migrate("tasks", "CREATE TABLE tasks(local_id INTEGER PRIMARY KEY, id INTEGER NOT NULL, name VARCHAR NOT NULL, "
		"wid INTEGER NOT NULL, pid INTEGER, "
		"CONSTRAINT fk_tasks_wid FOREIGN KEY (wid) REFERENCES workpaces(id) ON DELETE NO ACTION ON UPDATE NO ACTION, "
		"CONSTRAINT fk_tasks_pid FOREIGN KEY (pid) REFERENCES projects(id) ON DELETE NO ACTION ON UPDATE NO ACTION)");
	if (err != noError) {
		return err;
	}

	err = migrate("tags", "CREATE TABLE tags(local_id INTEGER PRIMARY KEY, id INTEGER NOT NULL, name VARCHAR NOT NULL, "
		"wid INTEGER NOT NULL, guid VARCHAR, "
		"CONSTRAINT fk_tags_wid FOREIGN KEY (wid) REFERENCES tags(id) ON DELETE NO ACTION ON UPDATE NO ACTION)");
	if (err != noError) {
		return err;
	}

	err = migrate("time_entries", "CREATE TABLE time_entries(local_id INTEGER PRIMARY KEY, id INTEGER NOT NULL, description VARCHAR, "
		"wid INTEGER NOT NULL, guid VARCHAR, pid INTEGER, tid INTEGER, billable INTEGER NOT NULL DEFAULT 0,"
		"duronly INTEGER NOT NULL DEFAULT 0, ui_modified_at INTEGER, start INTEGER NOT NULL, stop INTEGER, duration INTEGER NOT NULL,"
		"tags TEXT,"
		"CONSTRAINT fk_time_entries_wid FOREIGN KEY (wid) REFERENCES workspaces(id) ON DELETE NO ACTION ON UPDATE NO ACTION,"
		"CONSTRAINT fk_time_entries_pid FOREIGN KEY (pid) REFERENCES projects(id) ON DELETE NO ACTION ON UPDATE NO ACTION,"
		"CONSTRAINT fk_time_entries_tid FOREIGN KEY (tid) REFERENCES tasks(id) ON DELETE NO ACTION ON UPDATE NO ACTION)");
	if (err != noError) {
		return err;
	}

	return noError;
}

error Database::migrate(std::string name, std::string sql) {
	int count = 0;
	*ses_ << "SELECT COUNT(*) FROM kopsik_migrations WHERE name=:name",
		Poco::Data::into(count), Poco::Data::use(name), Poco::Data::now;

	if (count < 1) {
		*ses_ << sql, Poco::Data::now;

		*ses_ << "INSERT INTO kopsik_migrations(name) VALUES(:name)",
			Poco::Data::use(name), Poco::Data::now;
	}

	return noError;
}

};