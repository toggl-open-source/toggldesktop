// Copyright 2013 Tanel Lebedev

#ifndef SRC_DATABASE_H_
#define SRC_DATABASE_H_

#include <string>
#include <vector>
#include <iostream> // NOLINT

#include "Poco/Logger.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/SQLite/Connector.h"

#include "./types.h"
#include "./toggl_api_client.h"

namespace kopsik {

class ModelChange {
    public:
        ModelChange(const std::string model_type,
                    const std::string change_type,
                    const Poco::UInt64 model_id,
                    const std::string GUID) :
          model_type_(model_type),
          change_type_(change_type),
          model_id_(model_id),
          GUID_(GUID) {}
        std::string const& GUID() { return GUID_; }
        std::string const& ModelType() { return model_type_; }
        Poco::UInt64 const& ModelID() { return model_id_; }
        std::string const& ChangeType() { return change_type_; }
    private:
        std::string model_type_;
        std::string change_type_;
        Poco::UInt64 model_id_;
        std::string GUID_;
};

class Database {
    public:
        explicit Database(std::string db_path) : session(0) {
            Poco::Data::SQLite::Connector::registerConnector();
            session = new Poco::Data::Session("SQLite", db_path);
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

        error LoadUserByID(Poco::UInt64 UID, User *user,
            bool with_related_data);
        error LoadUserByAPIToken(std::string api_token, User *user,
            bool with_related_data);
        error LoadCurrentUser(User *user, bool with_related_data);

        error UInt(std::string sql, Poco::UInt64 *result);

        error SaveUser(User *user, bool with_related_data,
            std::vector<ModelChange> *changes);
        error SaveWorkspace(Workspace *model);
        error SaveClient(Client *model);
        error SaveProject(Project *model);
        error SaveTask(Task *model);
        error SaveTag(Tag *model);
        error SaveTimeEntry(TimeEntry *model,
            std::vector<ModelChange> *changes);

        error LoadTimeEntriesForUpload(User *user);

        error CurrentAPIToken(std::string *token);
        error SetCurrentAPIToken(const std::string &token);
        error ClearCurrentAPIToken();

    private:
        error initialize_tables();
        error migrate(std::string name, std::string sql);
        error last_error();

        std::string generateGUID();

        error loadUsersRelatedData(User *user);

        error loadWorkspaces(Poco::UInt64 UID, std::vector<Workspace *> *list);
        error loadClients(Poco::UInt64 UID, std::vector<Client *> *list);
        error loadProjects(Poco::UInt64 UID, std::vector<Project *> *list);
        error loadTasks(Poco::UInt64 UID, std::vector<Task *> *list);
        error loadTags(Poco::UInt64 UID, std::vector<Tag *> *list);
        error loadTimeEntries(Poco::UInt64 UID, std::vector<TimeEntry *> *list);

        error loadTimeEntriesFromSQLStatement(Poco::Data::Statement *select,
            std::vector<TimeEntry *> *list);

        error saveWorkspaces(Poco::UInt64 UID,
            std::vector<Workspace *> *list);
        error saveClients(Poco::UInt64 UID,
            std::vector<Client *> *list);
        error saveProjects(Poco::UInt64 UID,
            std::vector<Project *> *list);
        error saveTasks(Poco::UInt64 UID,
            std::vector<Task *> *list);
        error saveTags(Poco::UInt64 UID,
            std::vector<Tag *> *list);
        error saveTimeEntries(Poco::UInt64 UID,
            std::vector<TimeEntry *> *list,
            std::vector<ModelChange> *changes);

        error deleteFromTable(std::string table_name, Poco::Int64 local_id);
        error deleteAllFromTableByUID(std::string table_name, Poco::Int64 UID);

        Poco::Data::Session *session;
};

}  // namespace kopsik

#endif  // SRC_DATABASE_H_
