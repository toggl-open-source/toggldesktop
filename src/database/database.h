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

#include <Poco/Data/SQLite/Connector.h>

#include "model_change.h"
#include "model/timeline_event.h"
#include "types.h"
#include "util/logger.h"

namespace Poco {
namespace Data {
class Session;
class Statement;
}
}

namespace toggl {

class AutotrackerRule;
class Client;
class Project;
class Proxy;
class Settings;
class Tag;
class Task;
class TimeEntry;
class User;
class Workspace;
class OnboardingState;

class DatabaseError final : public ErrorBase {
public:
    enum Type {
        UNKNOWN_ERROR,
        DATABASE_BROKEN,
        SESSION_ERROR,
        INCONSISTENT_DATA,
        PROGRAMMING_ERROR
    };
    inline static const std::map<int, std::string> UserMessages {
        { UNKNOWN_ERROR, "Unexpected database error" },
        { DATABASE_BROKEN, "" },
        { SESSION_ERROR, "" },
        { INCONSISTENT_DATA, "" },
        { PROGRAMMING_ERROR, "" }
    };
    DatabaseError(enum Type type, const std::string &logMessage)
        : ErrorBase()
        , log_message_(logMessage)
        , type_(type)
    {}
    DatabaseError(DatabaseError &&o) = default;
    DatabaseError(const DatabaseError &o) = default;

    std::string Class() const override {
        return "DatabaseError";
    }
    int Type() const override {
        return type_;
    }
    std::string LogMessage() const override {
        return log_message_;
    }
    std::string UserMessage() const override {
        auto it = UserMessages.find(Type());
        if (it != UserMessages.end()) {
            return it->second;
        }
        return UserMessages.at(UNKNOWN_ERROR);
    }
private:
    std::string log_message_;
    enum Type type_;
};

class TOGGL_INTERNAL_EXPORT Database {
 public:
    explicit Database(const std::string &db_path);
    ~Database();

    Error DeleteFromTable(
        const std::string &table_name,
        const Poco::Int64 &local_id);

    Error DeleteUser(
        User *model,
        const bool with_related_data);

    Error LoadUserByID(
        const Poco::UInt64 &UID,
        User *user);

    Error LoadUserByEmail(
        const std::string &email,
        User *model);

    Error LoadCurrentUser(User *user);

    Error LoadSettings(Settings *settings);

    Error LoadWindowSettings(
        Poco::Int64 *window_x,
        Poco::Int64 *window_y,
        Poco::Int64 *window_height,
        Poco::Int64 *window_width);

    Error SaveWindowSettings(
        const Poco::Int64 window_x,
        const Poco::Int64 window_y,
        const Poco::Int64 window_height,
        const Poco::Int64 window_width);

    Error SetMiniTimerX(const Poco::Int64 x);
    Error GetMiniTimerX(Poco::Int64 *x);
    Error SetMiniTimerY(const Poco::Int64 y);
    Error GetMiniTimerY(Poco::Int64 *y);
    Error SetMiniTimerW(const Poco::Int64 w);
    Error GetMiniTimerW(Poco::Int64 *w);

    Error SetSettingsHasSeenBetaOffering(const bool &value);

    Error SetSettingsMessageSeen(const Poco::UInt64 message_id);

    Error SetSettingsUseIdleDetection(const bool &use_idle_detection);

    Error SetSettingsAutotrack(const bool &value);

    Error SetSettingsOpenEditorOnShortcut(const bool &value);

    Error SetSettingsMenubarTimer(const bool &menubar_timer);

    Error SetSettingsMenubarProject(const bool &menubar_project);

    Error SetSettingsDockIcon(const bool &dock_icon);

    Error SetSettingsOnTop(const bool &on_top);

    Error SetSettingsReminder(const bool &reminder);

    Error SetSettingsPomodoro(const bool &pomodoro);

    Error SetSettingsPomodoroBreak(const bool &pomodoro_break);

    Error SetSettingsStopEntryOnShutdownSleep(const bool &stop_entry);

    Error SetSettingsIdleMinutes(const Poco::UInt64 idle_minutes);

    Error SetSettingsFocusOnShortcut(const bool &focus_on_shortcut);

    Error SetSettingsReminderMinutes(const Poco::UInt64 reminder_minutes);

    Error SetSettingsPomodoroMinutes(const Poco::UInt64 pomodoro_minutes);

    Error SetSettingsPomodoroBreakMinutes(
        const Poco::UInt64 pomodoro_break_minutes);

    Error SetSettingsManualMode(const bool &manual_mode);

    Error SetSettingsAutodetectProxy(const bool &autodetect_proxy);

    Error SetSettingsShowTouchBar(const bool &show_touch_bar);

    Error SetSettingsStartAutotrackerWithoutSuggestions(const bool &start_autotracker_without_suggestions);

    Error SetSettingsActiveTab(const uint8_t &active_tab);

    Error SetSettingsColorTheme(const uint8_t &color_theme);

    Error SetSettingsForceIgnoreCert(const bool& force_ignore_cert);

    Error SetSettingsRemindTimes(
        const std::string &remind_starts,
        const std::string &remind_ends);

    Error SetSettingsRemindDays(
        const bool &remind_mon,
        const bool &remind_tue,
        const bool &remind_wed,
        const bool &remind_thu,
        const bool &remind_fri,
        const bool &remind_sat,
        const bool &remind_sun);

    Error LoadMigrations(
        std::vector<std::string> *);

    Error SetMiniTimerVisible(
        const bool);

    Error GetMiniTimerVisible(
        bool *);

    Error SetKeepEndTimeFixed(
        const bool);

    Error GetKeepEndTimeFixed(
        bool *);

    Error GetShowTouchBar(bool *result);

    Error GetActiveTab(uint8_t *result);

    Error SetWindowMaximized(
        const bool value);

    Error GetWindowMaximized(bool *result);

    Error SetWindowMinimized(
        const bool value);

    Error GetWindowMinimized(bool *result);

    Error SetWindowEditSizeHeight(
        const Poco::Int64 value);

    Error GetWindowEditSizeHeight(Poco::Int64 *result);

    Error SetWindowEditSizeWidth(
        const Poco::Int64 value);

    Error GetWindowEditSizeWidth(Poco::Int64 *result);

    Error SetKeyStart(
        const std::string &value);

    Error GetKeyStart(std::string *result);

    Error SetKeyShow(
        const std::string &value);

    Error GetKeyShow(std::string *result);

    Error SetKeyModifierShow(
        const std::string &value);

    Error GetKeyModifierShow(std::string *result);

    Error SetKeyModifierStart(
        const std::string &value);

    Error GetKeyModifierStart(std::string *result);

    Error GetMessageSeen(Poco::Int64 *result);

    Error LoadProxySettings(
        bool *use_proxy,
        Proxy *proxy);

    Error SaveProxySettings(
        const bool &use_proxy,
        const Proxy &proxy);

    Error LoadUpdateChannel(
        std::string *update_channel);

    Error SaveUpdateChannel(
        const std::string &update_channel);

    Error UInt(
        const std::string &sql,
        Poco::UInt64 *result);

    Error String(
        const std::string &sql,
        std::string *result);

    Error SaveUser(User *user, bool with_related_data,
                   std::vector<ModelChange> *changes);

    Error LoadTimeEntriesForUpload(User *user);

    Error CurrentAPIToken(
        std::string *token,
        Poco::UInt64 *uid);
    Error SetCurrentAPIToken(
        const std::string &token,
        const Poco::UInt64 &uid);
    Error ClearCurrentAPIToken();

    static std::string GenerateGUID();

    std::string DesktopID() const {
        return desktop_id_;
    }
    Error EnsureDesktopID();

    std::string AnalyticsClientID() const {
        return analytics_client_id_;
    }
    Error EnsureAnalyticsClientID();

    Error Migrate(
        const std::string &name,
        const std::string &sql);

    Error EnsureTimelineGUIDS();

    Error Trim(const std::string &text, std::string *result);

    Error ResetWindow();
    Error LoadOnboardingState(const Poco::UInt64 &UID, OnboardingState *state);
    Error SetOnboardingState(const Poco::UInt64 &UID, OnboardingState *state);
    
 private:
    Error vacuum();

    Error initialize_tables();

    Error ensureMigrationTable();

    template<typename T>
    Error setSettingsValue(
        const std::string &field_name,
        const T &value);

    template<typename T>
    Error getSettingsValue(
        const std::string &field_name,
        T *value);

    Error execute(
        const std::string &sql);

    Error last_error(
        const std::string &was_doing);

    Error journalMode(std::string *);
    Error setJournalMode(const std::string &);

    Error loadUsersRelatedData(User *user);

    Error loadWorkspaces(
        const Poco::UInt64 &UID,
        std::vector<Workspace *> *list);

    Error loadClients(
        const Poco::UInt64 &UID,
        std::vector<Client *> *list);

    Error loadProjects(
        const Poco::UInt64 &UID,
        std::vector<Project *> *list);

    Error loadTasks(
        const Poco::UInt64 &UID,
        std::vector<Task *> *list);

    Error loadTags(
        const Poco::UInt64 &UID,
        std::vector<Tag *> *list);

    Error loadAutotrackerRules(
        const Poco::UInt64 &UID,
        std::vector<AutotrackerRule *> *list);

    Error loadTimeEntries(
        const Poco::UInt64 &UID,
        std::vector<TimeEntry *> *list);

    Error loadTimelineEvents(
        const Poco::UInt64 &UID,
        std::vector<TimelineEvent *> *list);

    Error loadTimeEntriesFromSQLStatement(
        Poco::Data::Statement *select,
        std::vector<TimeEntry *> *list);

    template <typename T>
    Error saveRelatedModels(
        const Poco::UInt64 UID,
        const std::string &table_name,
        std::vector<T *> *list,
        std::vector<ModelChange> *changes);

    Error deleteAllFromTableByDate(
        const std::string &table_name,
        const Poco::Timestamp &time);

    Error deleteAllSyncedTimelineEventsByDate(
        const Poco::Timestamp &time);

    Error deleteAllFromTableByUID(
        const std::string &table_name,
        const Poco::UInt64 &UID);

    Error saveModel(
        AutotrackerRule *model,
        std::vector<ModelChange> *changes);

    Error saveModel(
        Workspace *model,
        std::vector<ModelChange> *changes);

    Error saveModel(
        Client *model,
        std::vector<ModelChange> *changes);

    Error saveModel(
        Project *model,
        std::vector<ModelChange> *changes);

    Error saveModel(
        Task *model,
        std::vector<ModelChange> *changes);

    Error saveModel(
        Tag *model,
        std::vector<ModelChange> *changes);

    Error saveModel(
        TimeEntry *model,
        std::vector<ModelChange> *changes);

    Error saveModel(
        TimelineEvent *model,
        std::vector<ModelChange> *changes);

    Error saveDesktopID();
    Error saveAnalyticsClientID();

    Error deleteTooOldTimeline(
        const Poco::UInt64 &UID);

    Error deleteUserTimeline(
        const Poco::UInt64 &UID);

    Error selectCompressedTimelineBatchForUpload(
        const Poco::UInt64 &user_id,
        std::vector<TimelineEvent> *timeline_events);

    Logger logger { "database" };

    Poco::Mutex session_m_;
    Poco::Data::Session *session_;

    std::string desktop_id_;
    std::string analytics_client_id_;
};

}  // namespace toggl

#endif  // SRC_DATABASE_H_
