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

        error Delete(User *model, bool with_related_data);
        error Delete(Workspace *model);
        error Delete(Client *model);
        error Delete(Project *model);
        error Delete(Task *model);
        error Delete(Tag *model);
        error Delete(TimeEntry *model);

        error Load(Poco::UInt64 UID, User *model, bool with_related_data);
        error UInt(std::string sql, Poco::UInt64 *result);

        error Save(User *user, bool with_related_data);
        error Save(Workspace *model);
        error Save(Client *model);
        error Save(Project *model);
        error Save(Task *model);
        error Save(Tag *model);
        error Save(TimeEntry *model);

    private:
        error initialize_tables();
        error migrate(std::string name, std::string sql);
        error validate(User *user);
        error last_error();

        error loadWorkspaces(User *user);
        error loadClients(User *user);
        error loadProjects(User *user);
        error loadTasks(User *user);
        error loadTags(User *user);
        error loadTimeEntries(User *user);

        error saveList(Poco::UInt64 UID, std::vector<Workspace *> *list);
        error saveList(Poco::UInt64 UID, std::vector<Client *> *list);
        error saveList(Poco::UInt64 UID, std::vector<Project *> *list);
        error saveList(Poco::UInt64 UID, std::vector<Task *> *list);
        error saveList(Poco::UInt64 UID, std::vector<Tag *> *list);
        error saveList(Poco::UInt64 UID, std::vector<TimeEntry *> *list);

        error deleteFromTable(std::string table_name, Poco::Int64 local_id);
        error deleteAllFromTableByUID(std::string table_name, Poco::Int64 UID);

        Poco::Data::Session *session;
};

}  // namespace kopsik

#endif  // DATABASE_H_
