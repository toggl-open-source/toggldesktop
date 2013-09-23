// Copyright 2013 Tanel Lebedev

#ifndef DATABASE_H_
#define DATABASE_H_

#include <string>
#include <vector>

#include "Poco/Logger.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/SQLite/Connector.h"

#include "./types.h"
#include "./toggl_api_client.h"

namespace kopsik {

class Database {
    public:
        explicit Database(std::string dbname) : session(0) {
            Poco::Data::SQLite::Connector::registerConnector();
            session = new Poco::Data::Session("SQLite", dbname);
            error err = initialize_tables();
            if (err != noError) {
                Poco::Logger &logger = Poco::Logger::get("database");
                logger.error(err);
            }
            poco_assert(err == noError);
        }

        ~Database() {
            if (session) {
                delete session;
                session = 0;
            }
            Poco::Data::SQLite::Connector::unregisterConnector();
        }

        error DeleteUser(User *model, bool with_related_data);
        error DeleteWorkspace(Workspace *model);
        error DeleteClient(Client *model);
        error DeleteProject(Project *model);
        error DeleteTask(Task *model);
        error DeleteTag(Tag *model);
        error DeleteTimeEntry(TimeEntry *model);

        error LoadUserByID(Poco::UInt64 UID, User *user,
            bool with_related_data);
        error LoadUserByAPIToken(std::string api_token, User *user,
            bool with_related_data);
        error loadUsersRelatedData(User *user);

        error UInt(std::string sql, Poco::UInt64 *result);

        error SaveUser(User *user, bool with_related_data);
        error SaveWorkspace(Workspace *model);
        error SaveClient(Client *model);
        error SaveProject(Project *model);
        error SaveTask(Task *model);
        error SaveTag(Tag *model);
        error SaveTimeEntry(TimeEntry *model);

        error LoadTimeEntriesForUpload(User *user);

        error CurrentAPIToken(std::string *token);

    private:
        error initialize_tables();
        error migrate(std::string name, std::string sql);
        error validate(User *user);
        error last_error();

        error loadWorkspaces(Poco::UInt64 UID, std::vector<Workspace *> *list);
        error loadClients(Poco::UInt64 UID, std::vector<Client *> *list);
        error loadProjects(Poco::UInt64 UID, std::vector<Project *> *list);
        error loadTasks(Poco::UInt64 UID, std::vector<Task *> *list);
        error loadTags(Poco::UInt64 UID, std::vector<Tag *> *list);
        error loadTimeEntries(Poco::UInt64 UID, std::vector<TimeEntry *> *list);

        error loadTimeEntriesFromSQLStatement(Poco::Data::Statement *select,
            std::vector<TimeEntry *> *list);

        error saveWorkspaces(Poco::UInt64 UID, std::vector<Workspace *> *list);
        error saveClients(Poco::UInt64 UID, std::vector<Client *> *list);
        error saveProjects(Poco::UInt64 UID, std::vector<Project *> *list);
        error saveTasks(Poco::UInt64 UID, std::vector<Task *> *list);
        error saveTags(Poco::UInt64 UID, std::vector<Tag *> *list);
        error saveTimeEntries(Poco::UInt64 UID, std::vector<TimeEntry *> *list);

        error deleteFromTable(std::string table_name, Poco::Int64 local_id);
        error deleteAllFromTableByUID(std::string table_name, Poco::Int64 UID);

        Poco::Data::Session *session;
};

}  // namespace kopsik

#endif  // DATABASE_H_
