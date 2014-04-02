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
#include "./model_change.h"

namespace kopsik {

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
        bool *use_idle_settings,
        bool *menubar_timer,
        bool *dock_icon,
        bool *on_top);

    error SaveSettings(
        const bool use_idle_detection,
        const bool menubar_timer,
        const bool dock_icon,
        const bool on_top);

    error LoadProxySettings(
        bool *use_proxy,
        Proxy *proxy);

    error SaveProxySettings(
        const bool use_proxy,
        const Proxy *proxy);

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

    error execute(
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

    template <typename T>
    error saveRelatedModels(
        const Poco::UInt64 UID,
        const std::string table_name,
        std::vector<T *> *list,
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

    error saveModel(
        Workspace *model,
        std::vector<ModelChange> *changes);

    error saveModel(
        Client *model,
        std::vector<ModelChange> *changes);

    error saveModel(
        Project *model,
        std::vector<ModelChange> *changes);

    error saveModel(
        Task *model,
        std::vector<ModelChange> *changes);

    error saveModel(
        Tag *model,
        std::vector<ModelChange> *changes);

    error saveModel(
        TimeEntry *model,
        std::vector<ModelChange> *changes);

    Poco::Logger &logger() const;

    Poco::Data::Session *session;
    std::string desktop_id_;

    Poco::Mutex mutex_;
};

}  // namespace kopsik

#endif  // SRC_DATABASE_H_
