// Copyright 2013 Tanel Lebedev

#ifndef SRC_DATABASE_H_
#define SRC_DATABASE_H_

#include <string>
#include <vector>
#include <iostream> // NOLINT

#include "Poco/Logger.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/NotificationCenter.h"
#include "Poco/Observer.h"

#include "./types.h"
#include "./proxy.h"
#include "./toggl_api_client.h"
#include "./timeline_notifications.h"

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
        explicit Database(std::string db_path) :
                session(0), desktop_id_("") {
            Poco::Data::SQLite::Connector::registerConnector();
            session = new Poco::Data::Session("SQLite", db_path);
            error err = initialize_tables();
            if (err != noError) {
                Poco::Logger &logger = Poco::Logger::get("database");
                logger.error(err);
            }
            poco_assert(err == noError);

            Poco::NotificationCenter& nc =
            Poco::NotificationCenter::defaultCenter();

            Poco::Observer<Database, TimelineEventNotification>
              observeCreate(*this,
            &Database::handleTimelineEventNotification);
            nc.addObserver(observeCreate);

            Poco::Observer<Database, CreateTimelineBatchNotification>
                observeSelect(*this,
            &Database::handleCreateTimelineBatchNotification);
            nc.addObserver(observeSelect);

            Poco::Observer<Database, DeleteTimelineBatchNotification>
                observeDelete(*this,
            &Database::handleDeleteTimelineBatchNotification);
            nc.addObserver(observeDelete);
        }

        ~Database() {
            if (session) {
                delete session;
                session = 0;
            }
            Poco::Data::SQLite::Connector::unregisterConnector();
        }

        error DeleteUser(User *model, bool with_related_data);

        error LoadUserByID(const Poco::UInt64 UID, User *user,
            const bool with_related_data);
        error LoadUserByAPIToken(
            const std::string api_token,
            User *user,
            const bool with_related_data);
        error LoadCurrentUser(
            User *user,
            const bool with_related_data);

        error LoadSettings(
            bool *use_proxy,
            Proxy *proxy,
            bool *use_idle_settings);
        error SaveSettings(
            const bool use_proxy,
            const Proxy *proxy,
            const bool use_idle_detection);

        error UInt(
            const std::string sql,
            Poco::UInt64 *result);
        error String(
            const std::string sql,
            std::string *result);

        error SaveUser(User *user, bool with_related_data,
            std::vector<ModelChange> *changes);
        error SaveWorkspace(Workspace *model,
                            std::vector<ModelChange> *changes);
        error SaveClient(Client *model,
                         std::vector<ModelChange> *changes);
        error SaveProject(Project *model,
                          std::vector<ModelChange> *changes);
        error SaveTask(Task *model,
                       std::vector<ModelChange> *changes);
        error SaveTag(Tag *model,
                      std::vector<ModelChange> *changes);
        error SaveTimeEntry(TimeEntry *model,
                            std::vector<ModelChange> *changes);

        error LoadTimeEntriesForUpload(User *user);

        error CurrentAPIToken(std::string *token);
        error SetCurrentAPIToken(const std::string &token);
        error ClearCurrentAPIToken();

        error SaveDesktopID();

     protected:
        void handleTimelineEventNotification(
            TimelineEventNotification* notification);
        void handleCreateTimelineBatchNotification(
            CreateTimelineBatchNotification *notification);
        void handleDeleteTimelineBatchNotification(
            DeleteTimelineBatchNotification *notification);

    private:
        error initialize_tables();
        error migrate(
            const std::string name,
            const std::string sql);
        error last_error();

        static std::string generateGUID();

        error loadUsersRelatedData(User *user);

        error loadWorkspaces(
            const Poco::UInt64 UID,
            std::vector<Workspace *> *list);
        error loadClients(
            const Poco::UInt64 UID,
            std::vector<Client *> *list);
        error loadProjects(
            const Poco::UInt64 UID,
            std::vector<Project *> *list);
        error loadTasks(
            const Poco::UInt64 UID,
            std::vector<Task *> *list);
        error loadTags(
            const Poco::UInt64 UID,
            std::vector<Tag *> *list);
        error loadTimeEntries(
            const Poco::UInt64 UID,
            std::vector<TimeEntry *> *list);

        error loadTimeEntriesFromSQLStatement(
            Poco::Data::Statement *select,
            std::vector<TimeEntry *> *list);

        error saveWorkspaces(
            const Poco::UInt64 UID,
            std::vector<Workspace *> *list,
            std::vector<ModelChange> *changes);
        error saveClients(
            const Poco::UInt64 UID,
            std::vector<Client *> *list,
            std::vector<ModelChange> *changes);
        error saveProjects(
            const Poco::UInt64 UID,
            std::vector<Project *> *list,
            std::vector<ModelChange> *changes);
        error saveTasks(
            const Poco::UInt64 UID,
            std::vector<Task *> *list,
            std::vector<ModelChange> *changes);
        error saveTags(
            const Poco::UInt64 UID,
            std::vector<Tag *> *list,
            std::vector<ModelChange> *changes);
        error saveTimeEntries(
            const Poco::UInt64 UID,
            std::vector<TimeEntry *> *list,
            std::vector<ModelChange> *changes);

        error deleteFromTable(
            const std::string table_name,
            const Poco::Int64 local_id);
        error deleteAllFromTableByUID(
            const std::string table_name,
            const Poco::Int64 UID);

        error insert_timeline_event(const TimelineEvent& info);
        error select_timeline_batch(
            const int user_id,
            std::vector<TimelineEvent> *timeline_events);
        error delete_timeline_batch(
            const std::vector<TimelineEvent> &timeline_events);

        Poco::Data::Session *session;
        std::string desktop_id_;

        Poco::Mutex mutex_;
};

}  // namespace kopsik

#endif  // SRC_DATABASE_H_
