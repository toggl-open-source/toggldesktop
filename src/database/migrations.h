// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_MIGRATIONS_H_
#define SRC_MIGRATIONS_H_

#include "types.h"

namespace toggl {

class Database;

class TOGGL_INTERNAL_EXPORT Migrations {
 public:
    explicit Migrations(Database *db)
        : db_(db) {}
    virtual ~Migrations() {}

    Error Run();

 private:
    Database *db_;

    Error migrateAutotracker();
    Error migrateClients();
    Error migrateTasks();
    Error migrateTags();
    Error migrateSessions();
    Error migrateWorkspaces();
    Error migrateProjects();
    Error migrateAnalytics();
    Error migrateUsers();
    Error migrateTimeline();
    Error migrateTimeEntries();
    Error migrateSettings();
    Error migrateObmActions();
    Error migrateObmExperiments();
    Error migrateOnboardingStates();
};

}  // namespace toggl

#endif  // SRC_MIGRATIONS_H_
