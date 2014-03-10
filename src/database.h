// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_DATABASE_H_
#define SRC_DATABASE_H_

#if defined(POCO_UNBUNDLED)
#include <sqlite3.h>
#else
#include "sqlite3.h" // NOLINT
#endif

#include <string>
#include <vector>

#include "Poco/Logger.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/NotificationCenter.h"
#include "Poco/Observer.h"

#include "./types.h"
#include "./proxy.h"
#include "./user.h"
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
        std::string GUID() const { return GUID_; }
        std::string ModelType() const { return model_type_; }
        Poco::UInt64 ModelID() const { return model_id_; }
        std::string ChangeType() const { return change_type_; }
    private:
        std::string model_type_;
        std::string change_type_;
        Poco::UInt64 model_id_;
        std::string GUID_;
};

class Database {
    public:
        explicit Database(const std::string db_path);
        ~Database();

        error DeleteUser(
            User *model,
            const bool with_related_data);

        error LoadUserByID(
            const Poco::UInt64 UID,
            User *user,
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
            bool *use_idle_settings,
            bool *menubar_timer);
        error SaveSettings(
            const bool use_proxy,
            const Proxy *proxy,
            const bool use_idle_detection,
            const bool menubar_timer);

        error LoadUpdateChannel(
            std::string *update_channel);
        error SaveUpdateChannel(
            const std::string update_channel);

        error UInt(
            const std::string sql,
            Poco::UInt64 *result);
        error String(
            const std::string sql,
            std::string *result);

        error SaveUser(User *user, bool with_related_data,
            std::vector<ModelChange> *changes);

        error LoadTimeEntriesForUpload(User *user);

        error CurrentAPIToken(std::string *token);
        error SetCurrentAPIToken(const std::string &token);
        error ClearCurrentAPIToken();

        error SaveDesktopID();

        static std::string GenerateGUID();

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
        error last_error(
            const std::string was_doing);

        error journalMode(std::string *);
        error setJournalMode(const std::string);

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
            const Poco::UInt64 user_id,
            std::vector<TimelineEvent> *timeline_events);
        error delete_timeline_batch(
            const std::vector<TimelineEvent> &timeline_events);

        error saveWorkspace(
            Workspace *model,
            std::vector<ModelChange> *changes);
        error saveClient(
            Client *model,
            std::vector<ModelChange> *changes);
        error saveProject(
            Project *model,
            std::vector<ModelChange> *changes);
        error saveTask(
            Task *model,
            std::vector<ModelChange> *changes);
        error saveTag(
            Tag *model,
            std::vector<ModelChange> *changes);
        error saveTimeEntry(
            TimeEntry *model,
            std::vector<ModelChange> *changes);

        Poco::Logger &logger() const;

        Poco::Data::Session *session;
        std::string desktop_id_;

        Poco::Mutex mutex_;
};

}  // namespace kopsik

#endif  // SRC_DATABASE_H_
