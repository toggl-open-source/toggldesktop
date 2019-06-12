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

#include "Poco/Data/SQLite/Connector.h"

#include "./model_change.h"
#include "./timeline_event.h"
#include "./types.h"

namespace Poco {
class Logger;

namespace Data {
class Session;
class Statement;
}
}

namespace toggl {

class AutotrackerRule;
class Client;
class ObmAction;
class ObmExperiment;
class Project;
class Proxy;
class Settings;
class Tag;
class Task;
class TimeEntry;
class User;
class Workspace;

class Database {
 public:
    explicit Database(const std::string &db_path);
    ~Database();

    error DeleteFromTable(
        const std::string &table_name,
        const Poco::Int64 &local_id);

    error DeleteUser(
        User *model,
        const bool with_related_data);

    error LoadUserByID(
        const Poco::UInt64 &UID,
        User *user);

    error LoadUserByEmail(
        const std::string &email,
        User *model);

    error LoadCurrentUser(User *user);

    error LoadSettings(Settings *settings);

    error LoadWindowSettings(
        Poco::Int64 *window_x,
        Poco::Int64 *window_y,
        Poco::Int64 *window_height,
        Poco::Int64 *window_width);

    error SaveWindowSettings(
        const Poco::Int64 window_x,
        const Poco::Int64 window_y,
        const Poco::Int64 window_height,
        const Poco::Int64 window_width);

    error SetMiniTimerX(const Poco::Int64 x);
    error GetMiniTimerX(Poco::Int64 *x);
    error SetMiniTimerY(const Poco::Int64 y);
    error GetMiniTimerY(Poco::Int64 *y);
    error SetMiniTimerW(const Poco::Int64 w);
    error GetMiniTimerW(Poco::Int64 *w);

    error SetSettingsHasSeenBetaOffering(const bool &value);

    error SetSettingsUseIdleDetection(const bool &use_idle_detection);

    error SetSettingsAutotrack(const bool &value);

    error SetSettingsOpenEditorOnShortcut(const bool &value);

    error SetSettingsMenubarTimer(const bool &menubar_timer);

    error SetSettingsMenubarProject(const bool &menubar_project);

    error SetSettingsDockIcon(const bool &dock_icon);

    error SetSettingsOnTop(const bool &on_top);

    error SetSettingsReminder(const bool &reminder);

    error SetSettingsPomodoro(const bool &pomodoro);

    error SetSettingsPomodoroBreak(const bool &pomodoro_break);

    error SetSettingsStopEntryOnShutdownSleep(const bool &stop_entry);

    error SetSettingsIdleMinutes(const Poco::UInt64 idle_minutes);

    error SetSettingsFocusOnShortcut(const bool &focus_on_shortcut);

    error SetSettingsReminderMinutes(const Poco::UInt64 reminder_minutes);

    error SetSettingsPomodoroMinutes(const Poco::UInt64 pomodoro_minutes);

    error SetSettingsPomodoroBreakMinutes(
        const Poco::UInt64 pomodoro_break_minutes);

    error SetSettingsManualMode(const bool &manual_mode);

    error SetSettingsAutodetectProxy(const bool &autodetect_proxy);

    error SetSettingsRemindTimes(
        const std::string &remind_starts,
        const std::string &remind_ends);

    error SetSettingsRemindDays(
        const bool &remind_mon,
        const bool &remind_tue,
        const bool &remind_wed,
        const bool &remind_thu,
        const bool &remind_fri,
        const bool &remind_sat,
        const bool &remind_sun);

    error LoadMigrations(
        std::vector<std::string> *);

    error SetCompactMode(
        const bool);

    error GetCompactMode(
        bool *);

    error SetMiniTimerVisible(
        const bool);

    error GetMiniTimerVisible(
        bool *);

    error SetKeepEndTimeFixed(
        const bool);

    error GetKeepEndTimeFixed(
        bool *);

    error SetWindowMaximized(
        const bool value);

    error GetWindowMaximized(bool *result);

    error SetWindowMinimized(
        const bool value);

    error GetWindowMinimized(bool *result);

    error SetWindowEditSizeHeight(
        const Poco::Int64 value);

    error GetWindowEditSizeHeight(Poco::Int64 *result);

    error SetWindowEditSizeWidth(
        const Poco::Int64 value);

    error GetWindowEditSizeWidth(Poco::Int64 *result);

    error SetKeyStart(
        const std::string &value);

    error GetKeyStart(std::string *result);

    error SetKeyShow(
        const std::string &value);

    error GetKeyShow(std::string *result);

    error SetKeyModifierShow(
        const std::string &value);

    error GetKeyModifierShow(std::string *result);

    error SetKeyModifierStart(
        const std::string &value);

    error GetKeyModifierStart(std::string *result);

    error LoadProxySettings(
        bool *use_proxy,
        Proxy *proxy);

    error SaveProxySettings(
        const bool &use_proxy,
        const Proxy &proxy);

    error LoadUpdateChannel(
        std::string *update_channel);

    error SaveUpdateChannel(
        const std::string &update_channel);

    error UInt(
        const std::string &sql,
        Poco::UInt64 *result);

    error String(
        const std::string &sql,
        std::string *result);

    error SaveUser(User *user, bool with_related_data,
                   std::vector<ModelChange> *changes);

    error LoadTimeEntriesForUpload(User *user);

    error CurrentAPIToken(
        std::string *token,
        Poco::UInt64 *uid);
    error SetCurrentAPIToken(
        const std::string &token,
        const Poco::UInt64 &uid);
    error ClearCurrentAPIToken();

    static std::string GenerateGUID();

    std::string DesktopID() const {
        return desktop_id_;
    }
    error EnsureDesktopID();

    std::string AnalyticsClientID() const {
        return analytics_client_id_;
    }
    error EnsureAnalyticsClientID();

    error Migrate(
        const std::string &name,
        const std::string &sql);

    error EnsureTimelineGUIDS();

    error Trim(const std::string &text, std::string *result);

    error ResetWindow();

 private:
    error vacuum();

    error initialize_tables();

    error ensureMigrationTable();

    template<typename T>
    error setSettingsValue(
        const std::string &field_name,
        const T &value);

    template<typename T>
    error getSettingsValue(
        const std::string &field_name,
        T *value);

    error execute(
        const std::string &sql);

    error last_error(
        const std::string &was_doing);

    error journalMode(std::string *);
    error setJournalMode(const std::string &);

    error loadUsersRelatedData(User *user);

    error loadWorkspaces(
        const Poco::UInt64 &UID,
        std::vector<Workspace *> *list);

    error loadClients(
        const Poco::UInt64 &UID,
        std::vector<Client *> *list);

    error loadProjects(
        const Poco::UInt64 &UID,
        std::vector<Project *> *list);

    error loadTasks(
        const Poco::UInt64 &UID,
        std::vector<Task *> *list);

    error loadTags(
        const Poco::UInt64 &UID,
        std::vector<Tag *> *list);

    error loadAutotrackerRules(
        const Poco::UInt64 &UID,
        std::vector<AutotrackerRule *> *list);

    error loadObmActions(
        const Poco::UInt64 &UID,
        std::vector<ObmAction *> *list);

    error loadObmExperiments(
        const Poco::UInt64 &UID,
        std::vector<ObmExperiment *> *list);

    error loadTimeEntries(
        const Poco::UInt64 &UID,
        std::vector<TimeEntry *> *list);

    error loadTimelineEvents(
        const Poco::UInt64 &UID,
        std::vector<TimelineEvent *> *list);

    error loadTimeEntriesFromSQLStatement(
        Poco::Data::Statement *select,
        std::vector<TimeEntry *> *list);

    template <typename T>
    error saveRelatedModels(
        const Poco::UInt64 UID,
        const std::string &table_name,
        std::vector<T *> *list,
        std::vector<ModelChange> *changes);

    error deleteAllFromTableByDate(
        const std::string &table_name,
        const Poco::Timestamp &time);

    error deleteAllSyncedTimelineEventsByDate(
        const Poco::Timestamp &time);

    error deleteAllFromTableByUID(
        const std::string &table_name,
        const Poco::UInt64 &UID);

    error saveModel(
        ObmAction *model,
        std::vector<ModelChange> *changes);

    error saveModel(
        ObmExperiment *model,
        std::vector<ModelChange> *changes);

    error saveModel(
        AutotrackerRule *model,
        std::vector<ModelChange> *changes);

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

    error saveModel(
        TimelineEvent *model,
        std::vector<ModelChange> *changes);

    error saveDesktopID();
    error saveAnalyticsClientID();

    error deleteTooOldTimeline(
        const Poco::UInt64 &UID);

    error deleteUserTimeline(
        const Poco::UInt64 &UID);

    error selectCompressedTimelineBatchForUpload(
        const Poco::UInt64 &user_id,
        std::vector<TimelineEvent> *timeline_events);

    Poco::Logger &logger() const;

    Poco::Mutex session_m_;
    Poco::Data::Session *session_;

    std::string desktop_id_;
    std::string analytics_client_id_;
};

}  // namespace toggl

#endif  // SRC_DATABASE_H_
