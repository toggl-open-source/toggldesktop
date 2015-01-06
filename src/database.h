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
#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"

#include "./types.h"
#include "./proxy.h"
#include "./user.h"
#include "./model_change.h"
#include "./settings.h"
#include "./timeline_event.h"

namespace toggl {

class Database {
 public:
    explicit Database(const std::string db_path);
    ~Database();

    error DeleteUser(
        User *model,
        const bool with_related_data);

    error LoadUserByID(
        const Poco::UInt64 UID,
        User *user);

    error LoadUserByAPIToken(
        const std::string api_token,
        User *user);

    error LoadCurrentUser(User *user);

    error LoadSettings(Settings *settings);

    error SetSettingsUseIdleDetection(const bool &use_idle_detection);

    error SetSettingsMenubarTimer(const bool menubar_timer);

    error SetSettingsDockIcon(const bool dock_icon);

    error SetSettingsOnTop(const bool on_top);

    error SetSettingsReminder(const bool reminder);

    error SetSettingsIdleMinutes(const Poco::UInt64 idle_minutes);

    error SetSettingsFocusOnShortcut(const bool focus_on_shortcut);

    error SetSettingsReminderMinutes(const Poco::UInt64 reminder_minutes);

    error LoadProxySettings(
        bool *use_proxy,
        Proxy *proxy);

    error SaveProxySettings(
        const bool &use_proxy,
        const Proxy &proxy);

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

    error SelectTimelineBatch(const Poco::UInt64 user_id,
                              std::vector<TimelineEvent> *timeline_events);

    static std::string GenerateGUID();

    std::string DesktopID() const {
        return desktop_id_;
    }

    error InsertTimelineEvent(TimelineEvent *info);

    error DeleteTimelineBatch(
        const std::vector<TimelineEvent> &timeline_events);

 private:
    error initialize_tables();

    error ensureMigrationTable();

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
        const Poco::Int64 &UID);

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

    error saveDesktopID();

    Poco::Logger &logger() const;

    Poco::Mutex session_m_;
    Poco::Data::Session *session_;

    std::string desktop_id_;
};

}  // namespace toggl

#endif  // SRC_DATABASE_H_
