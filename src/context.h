// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CONTEXT_H_
#define SRC_CONTEXT_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <iostream> // NOLINT

#include "./analytics.h"
#include "./custom_error_handler.h"
#include "./feedback.h"
#include "./gui.h"
#include "./help_article.h"
#include "./idle.h"
#include "./model_change.h"
#include "./timeline_event.h"
#include "./timeline_notifications.h"
#include "./types.h"
#include "./websocket_client.h"

#include "Poco/Activity.h"
#include "Poco/LocalDateTime.h"
#include "Poco/Timestamp.h"
#include "Poco/Util/Timer.h"

namespace toggl {

class Database;
class TimelineUploader;
class WindowChangeRecorder;

class UIElements {
 public:
    UIElements()
        : first_load(false)
    , display_time_entries(false)
    , display_time_entry_autocomplete(false)
    , display_mini_timer_autocomplete(false)
    , display_project_autocomplete(false)
    , display_client_select(false)
    , display_workspace_select(false)
    , display_timer_state(false)
    , display_time_entry_editor(false)
    , open_settings(false)
    , open_time_entry_list(false)
    , open_time_entry_editor(false)
    , display_autotracker_rules(false)
    , display_settings(false)
    , time_entry_editor_guid("")
    , time_entry_editor_field("")
    , display_unsynced_items(false) {}

    static UIElements Reset();

    std::string String() const;

    void ApplyChanges(
        const std::string editor_guid,
        const std::vector<ModelChange> &changes);

    bool first_load;
    bool display_time_entries;
    bool display_time_entry_autocomplete;
    bool display_mini_timer_autocomplete;
    bool display_project_autocomplete;
    bool display_client_select;
    bool display_workspace_select;
    bool display_timer_state;
    bool display_time_entry_editor;
    bool open_settings;
    bool open_time_entry_list;
    bool open_time_entry_editor;
    bool display_autotracker_rules;
    bool display_settings;
    std::string time_entry_editor_guid;
    std::string time_entry_editor_field;
    bool display_unsynced_items;
};

class Context : public TimelineDatasource {
 public:
    Context(
        const std::string app_name,
        const std::string app_version);
    virtual ~Context();

    GUI *UI() {
        return &ui_;
    }

    // Check for logged in user etc, start up the app
    error StartEvents();

    // Close connections and wait for tasks to finish
    void Shutdown();

    void FullSync();
    void Sync();
    void TimelineUpdateServerSettings();
    error SendFeedback(Feedback);

    // Load model update from JSON string (from WebSocket)
    error LoadUpdateFromJSONString(const std::string json);

    void SetWebSocketClientURL(const std::string value);

    error SetDBPath(const std::string path);

    void SetUpdatePath(const std::string path) {
        update_path_ = path;
    }
    const std::string &UpdatePath() {
        return update_path_;
    }

    void SetEnvironment(const std::string environment);
    std::string Environment() const {
        return environment_;
    }

    void DisableUpdateCheck() {
        update_check_disabled_ = true;
    }

    error SetSettingsUseIdleDetection(const bool use_idle_detection);

    error SetSettingsAutotrack(const bool value);

    error SetSettingsOpenEditorOnShortcut(const bool value);

    error SetSettingsMenubarTimer(const bool menubar_timer);

    error SetSettingsMenubarProject(const bool menubar_project);

    error SetSettingsDockIcon(const bool dock_icon);

    error SetSettingsOnTop(const bool on_top);

    error SetSettingsReminder(const bool reminder);

    error SetSettingsPomodoro(const bool pomodoro);

    error SetSettingsPomodoroBreak(const bool pomodoro_break);

    error SetSettingsStopEntryOnShutdownSleep(const bool stop_entry);

    error SetSettingsIdleMinutes(const Poco::UInt64 idle_minutes);

    error SetSettingsFocusOnShortcut(const bool focus_on_shortcut);

    error SetSettingsReminderMinutes(const Poco::UInt64 reminder_minutes);

    error SetSettingsPomodoroMinutes(const Poco::UInt64 pomodoro_minutes);

    error SetSettingsPomodoroBreakMinutes(
        const Poco::UInt64 pomodoro_break_minutes);

    error SetSettingsManualMode(const bool manual_mode);

    error SetSettingsAutodetectProxy(const bool autodetect_proxy);

    error SetSettingsRemindTimes(
        const std::string remind_starts,
        const std::string remind_ends);

    error SetSettingsRemindDays(
        const bool remind_mon,
        const bool remind_tue,
        const bool remind_wed,
        const bool remind_thu,
        const bool remind_fri,
        const bool remind_sat,
        const bool remind_sun);

    void SetCompactMode(
        const bool);

    bool GetCompactMode();

    void SetMiniTimerVisible(
        const bool);

    bool GetMiniTimerVisible();

    void SetKeepEndTimeFixed(
        const bool);

    bool GetKeepEndTimeFixed();

    void SetWindowMaximized(
        const bool value);

    bool GetWindowMaximized();

    void SetWindowMinimized(
        const bool_t value);

    bool GetWindowMinimized();

    void SetWindowEditSizeHeight(
        const int64_t value);

    int64_t GetWindowEditSizeHeight();

    void SetWindowEditSizeWidth(
        const int64_t value);

    int64_t GetWindowEditSizeWidth();

    void SetKeyStart(
        const std::string value);

    std::string GetKeyStart();

    void SetKeyShow(
        const std::string value);

    std::string GetKeyShow();

    void SetKeyModifierShow(
        const std::string value);

    std::string GetKeyModifierShow();

    void SetKeyModifierStart(
        const std::string value);

    std::string GetKeyModifierStart();

    error ProxySettings(bool *use_proxy, Proxy *proxy);

    error SetProxySettings(
        const bool use_proxy,
        const Proxy proxy);

    error LoadWindowSettings(
        int64_t *window_x,
        int64_t *window_y,
        int64_t *window_height,
        int64_t *window_width);

    error SaveWindowSettings(
        const int64_t window_x,
        const int64_t window_y,
        const int64_t window_height,
        const int64_t window_width);

    Poco::Int64 GetMiniTimerX();

    void SetMiniTimerX(
        const int64_t x);

    Poco::Int64 GetMiniTimerY();

    void SetMiniTimerY(
        const int64_t y);

    Poco::Int64 GetMiniTimerW();

    void SetMiniTimerW(
        const int64_t w);


    error Login(
        const std::string email,
        const std::string password);

    error Signup(
        const std::string email,
        const std::string password,
        const uint64_t country_id);

    error GoogleLogin(const std::string access_token);

    error Logout();

    error SetLoggedInUserFromJSON(const std::string json);

    error ClearCache();

    TimeEntry *Start(
        const std::string description,
        const std::string duration,
        const Poco::UInt64 task_id,
        const Poco::UInt64 project_id,
        const std::string project_guid,
        const std::string tags,
        const bool prevent_on_app);

    TimeEntry *ContinueLatest(const bool prevent_on_app);

    TimeEntry *Continue(
        const std::string GUID);

    void OpenTimeEntryList();

    void OpenSettings();

    void OpenTimeEntryEditor(
        const std::string GUID,
        const bool edit_running_entry = false,
        const std::string focused_field_name = "");

    error SetTimeEntryDuration(
        const std::string GUID,
        const std::string duration);

    error DeleteTimeEntryByGUID(const std::string GUID);

    error SetTimeEntryProject(
        const std::string GUID,
        const Poco::UInt64 task_id,
        const Poco::UInt64 project_id,
        const std::string project_guid);

    error SetTimeEntryDate(
        const std::string GUID,
        const Poco::Int64 unix_timestamp);

    error SetTimeEntryStart(
        const std::string GUID,
        const std::string value);

    error SetTimeEntryStop(
        const std::string GUID,
        const std::string value);

    error SetTimeEntryTags(
        const std::string GUID,
        const std::string value);

    error SetTimeEntryBillable(
        const std::string GUID,
        const bool value);

    error SetTimeEntryDescription(
        const std::string GUID,
        const std::string value);

    error Stop(const bool prevent_on_app);

    error DiscardTimeAt(
        const std::string GUID,
        const Poco::Int64 at,
        const bool split_into_new_entry);

    TimeEntry *DiscardTimeAndContinue(
        const std::string GUID,
        const Poco::Int64 at);

    TimeEntry *RunningTimeEntry();

    error ToggleTimelineRecording(
        const bool record_timeline);

    bool IsTimelineRecordingEnabled() const {
        return user_ && user_->RecordTimeline();
    }

    error SetDefaultProject(
        const Poco::UInt64 pid,
        const Poco::UInt64 tid);
    error DefaultProjectName(std::string *name);
    error DefaultPID(Poco::UInt64 *result);
    error DefaultTID(Poco::UInt64 *result);

    void SearchHelpArticles(
        const std::string keywords);

    error SetUpdateChannel(
        const std::string channel);

    error UpdateChannel(
        std::string *update_channel);

    Project *CreateProject(
        const Poco::UInt64 workspace_id,
        const Poco::UInt64 client_id,
        const std::string client_guid,
        const std::string project_name,
        const bool is_private,
        const std::string project_color);

    Client *CreateClient(
        const Poco::UInt64 workspace_id,
        const std::string client_name);

    error AddObmAction(
        const Poco::UInt64 experiment_id,
        const std::string key,
        const std::string value);

    void SetSleep();

    void SetWake();

    void osShutdown();

    void SetOnline();

    error OpenReportsInBrowser();

    error ToSAccept();

    void SetIdleSeconds(const Poco::UInt64 idle_seconds) {
        idle_.SetIdleSeconds(idle_seconds, user_);
    }

    void LoadMore();

    static void SetLogPath(const std::string path);

    void SetQuit() {
        quit_ = true;
    }

    error AddAutotrackerRule(
        const std::string term,
        const Poco::UInt64 pid,
        const Poco::UInt64 tid,
        Poco::Int64 *rule_id);

    error DeleteAutotrackerRule(
        const Poco::Int64 id);

    std::string UserFullName();

    std::string UserEmail();

    // Timeline datasource
    error StartAutotrackerEvent(const TimelineEvent event);
    error CreateCompressedTimelineBatchForUpload(TimelineBatch *batch);
    error StartTimelineEvent(TimelineEvent *event);
    error MarkTimelineBatchAsUploaded(
        const std::vector<TimelineEvent> &events);

    error SetPromotionResponse(
        const int64_t promotion_type,
        const int64_t promotion_response);

    error ToggleEntriesGroup(
        std::string name);

    error PullCountries();

    void TrackWindowSize(const Poco::Int64 width,
                         const Poco::Int64 height);

 protected:
    void uiUpdaterActivity();
    void checkReminders();
    void reminderActivity();
    void syncerActivity();

 private:
    error updateURL(std::string *result);

    static const std::string installerPlatform();
    static const std::string linuxPlatformName();
    static const std::string shortOSName();

    Poco::Logger &logger() const;

    void sync(const bool full_sync);

    error save(const bool push_changes = true);

    void fetchUpdates();

    // timer_ callbacks
    void onSync(Poco::Util::TimerTask& task);  // NOLINT
    void onPushChanges(Poco::Util::TimerTask& task);  // NOLINT
    void onSwitchWebSocketOff(Poco::Util::TimerTask& task);  // NOLINT
    void onSwitchWebSocketOn(Poco::Util::TimerTask& task);  // NOLINT
    void onSwitchTimelineOff(Poco::Util::TimerTask& task);  // NOLINT
    void onSwitchTimelineOn(Poco::Util::TimerTask& task);  // NOLINT
    void onFetchUpdates(Poco::Util::TimerTask& task);  // NOLINT
    void onPeriodicUpdateCheck(Poco::Util::TimerTask& task);  // NOLINT
    void onTimelineUpdateServerSettings(Poco::Util::TimerTask& task);  // NOLINT
    void onSendFeedback(Poco::Util::TimerTask& task);  // NOLINT
    void onPeriodicSync(Poco::Util::TimerTask& task);  // NOLINT
    void onTrackSettingsUsage(Poco::Util::TimerTask& task);  // NOLINT
    void onWake(Poco::Util::TimerTask& task);  // NOLINT
    void onLoadMore(Poco::Util::TimerTask& task); // NOLINT

    void onTimeEntryAutocompletes(Poco::Util::TimerTask& task);  // NOLINT
    void onMiniTimerAutocompletes(Poco::Util::TimerTask& task);  // NOLINT
    void onProjectAutocompletes(Poco::Util::TimerTask& task);  // NOLINT

    void startPeriodicUpdateCheck();
    void executeUpdateCheck();

    void startPeriodicSync();

    void setUser(User *value, const bool user_logged_in = false);

    void switchWebSocketOff();
    void switchWebSocketOn();
    void switchTimelineOff();
    void switchTimelineOn();

    Database *db() const;

    void displayTimeEntryEditor(const bool open,
                                TimeEntry *te,
                                const std::string focused_field_name);
    void displayReminder();
    void resetLastTrackingReminderTime();

    void displayPomodoro();

    void displayPomodoroBreak();

    void updateUI(const UIElements &elements);

    error displayError(const error err);

    void scheduleSync();

    void setOnline(const std::string reason);

    int nextSyncIntervalSeconds() const;

    bool isPostponed(
        const Poco::Timestamp value,
        const Poco::Timestamp::TimeDiff throttleMicros) const;

    Poco::Timestamp postpone(
        const Poco::Timestamp::TimeDiff throttleMicros) const;

    error attemptOfflineLogin(const std::string email,
                              const std::string password);

    error downloadUpdate();

    void stopActivities();

    error offerBetaChannel(bool *did_offer);

    error runObmExperiments();

    error compressTimeline();

    error applySettingsSaveResultToUI(const error err);

    error pullAllUserData(TogglClient *https_client);
    error pullChanges(TogglClient *https_client);
    error pullUserPreferences(
        TogglClient* toggl_client);

    error pushChanges(
        TogglClient *https_client,
        bool *had_something_to_push);
    error pushClients(
        std::vector<Client *> clients,
        std::string api_token,
        TogglClient toggl_client);
    error pushProjects(
        std::vector<Project *> projects,
        std::vector<Client *> clients,
        std::string api_token,
        TogglClient toggl_client);
    error pushEntries(
        std::map<std::string, BaseModel *> models,
        std::vector<TimeEntry *> time_entries,
        std::string api_token,
        TogglClient toggl_client);
    error updateEntryProjects(
        std::vector<Project *> projects,
        std::vector<TimeEntry *> time_entries);
    static error signup(
        TogglClient *https_client,
        const std::string email,
        const std::string password,
        std::string *user_data_json,
        const uint64_t country_id);
    static error me(
        TogglClient *https_client,
        const std::string email,
        const std::string password,
        std::string *user_data,
        const Poco::UInt64 since);

    bool isTimeEntryLocked(TimeEntry* te);
    bool isTimeLockedInWorkspace(time_t t, Workspace* ws);
    bool canChangeStartTimeTo(TimeEntry* te, time_t t);
    bool canChangeProjectTo(TimeEntry* te, Project* p);

    error logAndDisplayUserTriedEditingLockedEntry();

    error pullWorkspaces(TogglClient* toggl_client);

    error pullWorkspacePreferences(TogglClient* https_client);
    error pullWorkspacePreferences(TogglClient* https_client,
                                   Workspace *workspace, std::string* json);

    error pushObmAction();

    error pullObmExperiments();

    template<typename T>
    void collectPushableModels(
        const std::vector<T *> list,
        std::vector<T *> *result,
        std::map<std::string, BaseModel *> *models = nullptr) const;

    Poco::Mutex db_m_;
    Database *db_;

    Poco::Mutex user_m_;
    User *user_;

    Poco::Mutex ws_client_m_;
    WebSocketClient ws_client_;

    Poco::Mutex timeline_uploader_m_;
    TimelineUploader *timeline_uploader_;

    Poco::Mutex window_change_recorder_m_;
    WindowChangeRecorder *window_change_recorder_;

    custom_error_handler error_handler_;

    Feedback feedback_;

    // Tasks are scheduled at:
    Poco::Timestamp next_sync_at_;
    Poco::Timestamp next_push_changes_at_;
    Poco::Timestamp next_fetch_updates_at_;
    Poco::Timestamp next_update_timeline_settings_at_;
    Poco::Timestamp next_wake_at_;

    // Schedule tasks using a timer:
    Poco::Mutex timer_m_;
    Poco::Util::Timer timer_;

    class GUI ui_;

    std::string time_entry_editor_guid_;

    std::string environment_;

    Idle idle_;

    Poco::UInt64 last_sync_started_;
    Poco::Int64 sync_interval_seconds_;
    Poco::UInt64 last_tracking_reminder_time_;
    Poco::UInt64 last_pomodoro_reminder_time_;
    Poco::UInt64 last_pomodoro_break_reminder_time_;

    bool update_check_disabled_;

    bool trigger_sync_;
    bool trigger_push_;

    Poco::LocalDateTime last_time_entry_list_render_at_;

    bool quit_;

    Poco::Mutex ui_updater_m_;
    Poco::Activity<Context> ui_updater_;

    Poco::Mutex reminder_m_;
    Poco::Activity<Context> reminder_;

    Poco::Mutex syncer_m_;
    Poco::Activity<Context> syncer_;

    Analytics analytics_;

    std::string update_path_;

    static std::string log_path_;

    Settings settings_;

    std::set<std::string> autotracker_titles_;

    HelpDatabase help_database_;

    TimeEntry *pomodoro_break_entry_;

    // To cache grouped entries open/close status
    std::map<std::string, bool_t> entry_groups;

    bool overlay_visible_;
};

void on_websocket_message(
    void *context,
    std::string json);

}  // namespace toggl

#endif  // SRC_CONTEXT_H_
