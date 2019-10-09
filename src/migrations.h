// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_MIGRATIONS_H_
#define SRC_MIGRATIONS_H_

#include "./types.h"

namespace toggl {

class Database;

class TOGGL_INTERNAL_EXPORT Migrations {
 public:
    explicit Migrations(Database *db)
        : db_(db) {}
    virtual ~Migrations() {}

    error Run();

 private:
    Database *db_;

    error migrateAutotracker();
    error migrateClients();
    error migrateTasks();
    error migrateTags();
    error migrateSessions();
    error migrateWorkspaces();
    error migrateProjects();
    error migrateAnalytics();
    error migrateTimeline();
    error migrateUsers();
    error migrateTimeEntries();
    error migrateSettings();
    error migrateObmActions();
    error migrateObmExperiments();
};

}  // namespace toggl

#endif  // SRC_MIGRATIONS_H_
