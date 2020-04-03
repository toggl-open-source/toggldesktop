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

#include "util/logger.h"
#include "model/timeline_event.h"
#include "model_change.h"
#include "types.h"

namespace Poco {
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
struct Compare;

class TOGGL_INTERNAL_EXPORT Database {
 public:
    explicit Database(const std::string &db_path);
    ~Database();

    error DeleteFromTable(
        const std::string &table_name,
        const Poco::Int64 &local_id);

    error DeleteUser(locked<User> &model,
        const bool with_related_data);

    error LoadUserByID(const Poco::UInt64 &UID,
        locked<User> &user);

    error LoadUserByEmail(const std::string &email,
        locked<User> &model);

    error LoadCurrentUser(locked<User> &user);

    error LoadSettings(locked<Settings> &settings);

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

    error SetSettingsMessageSeen(const Poco::UInt64 message_id);

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

    error SetSettingsShowTouchBar(const bool &show_touch_bar);

    error SetSettingsActiveTab(const uint8_t &active_tab);

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

    error SetMiniTimerVisible(
        const bool);

    error GetMiniTimerVisible(
        bool *);

    error SetKeepEndTimeFixed(
        const bool);

    error GetKeepEndTimeFixed(
        bool *);

    error GetShowTouchBar(bool *result);

    error GetActiveTab(uint8_t *result);

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

    error GetMessageSeen(Poco::Int64 *result);

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

    error SaveUser(locked<User> &user, bool with_related_data,
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

    error loadUsersRelatedData(locked<User> &user);

    template <typename T>
    error loadModels(
        const Poco::UInt64 &UID,
        ProtectedContainer<T> &list);

    template <typename T>
    error saveRelatedModels(
        const Poco::UInt64 UID,
        const std::string &table_name,
        ProtectedContainer<T> &list,
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
        locked<ObmAction> &model,
        std::vector<ModelChange> *changes);

    error saveModel(
        locked<ObmExperiment> &model,
        std::vector<ModelChange> *changes);

    error saveModel(
        locked<AutotrackerRule> &model,
        std::vector<ModelChange> *changes);

    error saveModel(
        locked<Workspace> &model,
        std::vector<ModelChange> *changes);

    error saveModel(
        locked<Client> &model,
        std::vector<ModelChange> *changes);

    error saveModel(
        locked<Project> &model,
        std::vector<ModelChange> *changes);

    error saveModel(
        locked<Task> &model,
        std::vector<ModelChange> *changes);

    error saveModel(
        locked<Tag> &model,
        std::vector<ModelChange> *changes);

    error saveModel(
        locked<TimeEntry> &model,
        std::vector<ModelChange> *changes);

    error saveModel(
        locked<TimelineEvent> &model,
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

    Logger logger { "database" };

    Poco::Mutex session_m_;
    Poco::Data::Session *session_;

    std::string desktop_id_;
    std::string analytics_client_id_;
};

}  // namespace toggl

#endif  // SRC_DATABASE_H_
