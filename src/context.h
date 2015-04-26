// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CONTEXT_H_
#define SRC_CONTEXT_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream> // NOLINT

#include "./analytics.h"
#include "./custom_error_handler.h"
#include "./feedback.h"
#include "./gui.h"
#include "./idle.h"
#include "./toggl_api.h"
#include "./model_change.h"
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
    _Bool StartEvents();

    // Close connections and wait for tasks to finish
    void Shutdown();

    void Sync();
    void TimelineUpdateServerSettings();
    _Bool SendFeedback(Feedback);

    // Load model update from JSON string (from WebSocket)
    _Bool LoadUpdateFromJSONString(const std::string json);

    void SetWebSocketClientURL(const std::string value);

    _Bool SetDBPath(const std::string path);

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

    _Bool LoadSettings(Settings *settings);

    _Bool SetSettingsUseIdleDetection(const _Bool use_idle_detection);

    _Bool SetSettingsMenubarTimer(const _Bool menubar_timer);

    _Bool SetSettingsMenubarProject(const _Bool menubar_project);

    _Bool SetSettingsDockIcon(const _Bool dock_icon);

    _Bool SetSettingsOnTop(const _Bool on_top);

    _Bool SetSettingsReminder(const _Bool reminder);

    _Bool SetSettingsIdleMinutes(const Poco::UInt64 idle_minutes);

    _Bool SetSettingsFocusOnShortcut(const _Bool focus_on_shortcut);

    _Bool SetSettingsReminderMinutes(const Poco::UInt64 reminder_minutes);

    _Bool SetSettingsManualMode(const _Bool manual_mode);

    _Bool SetSettingsAutodetectProxy(const _Bool autodetect_proxy);

    _Bool ProxySettings(bool *use_proxy, Proxy *proxy);

    _Bool SetProxySettings(
        const bool use_proxy,
        const Proxy proxy);

    _Bool LoadWindowSettings(
        int64_t *window_x,
        int64_t *window_y,
        int64_t *window_height,
        int64_t *window_width);

    _Bool SaveWindowSettings(
        const int64_t window_x,
        const int64_t window_y,
        const int64_t window_height,
        const int64_t window_width);

    _Bool Login(
        const std::string email,
        const std::string password);

    _Bool Signup(
        const std::string email,
        const std::string password);

    _Bool GoogleLogin(const std::string access_token);

    _Bool Logout();

    _Bool SetLoggedInUserFromJSON(const std::string user_data_json);

    _Bool ClearCache();

    TimeEntry *Start(
        const std::string description,
        const std::string duration,
        const Poco::UInt64 task_id,
        const Poco::UInt64 project_id);

    _Bool ContinueLatest();

    _Bool Continue(
        const std::string GUID);

    void DisplayTimeEntryList(const _Bool open);

    _Bool DisplaySettings(const _Bool open);

    void Edit(const std::string GUID,
              const _Bool edit_running_entry,
              const std::string focused_field_name);

    _Bool SetTimeEntryDuration(
        const std::string GUID,
        const std::string duration);

    _Bool DeleteTimeEntryByGUID(const std::string GUID);

    _Bool SetTimeEntryProject(
        const std::string GUID,
        const Poco::UInt64 task_id,
        const Poco::UInt64 project_id,
        const std::string project_guid);

    _Bool SetTimeEntryDate(
        const std::string GUID,
        const Poco::Int64 unix_timestamp);

    _Bool SetTimeEntryStart(
        const std::string GUID,
        const std::string value);

    _Bool SetTimeEntryStop(
        const std::string GUID,
        const std::string value);

    _Bool SetTimeEntryTags(
        const std::string GUID,
        const std::string value);

    _Bool SetTimeEntryBillable(
        const std::string GUID,
        const bool value);

    _Bool SetTimeEntryDescription(
        const std::string GUID,
        const std::string value);

    _Bool Stop();

    _Bool DiscardTimeAt(
        const std::string GUID,
        const Poco::Int64 at,
        const bool split_into_new_entry);

    _Bool RunningTimeEntry(
        TimeEntry **running) const;

    _Bool ToggleTimelineRecording(
        const _Bool record_timeline);

    _Bool IsTimelineRecordingEnabled() {
        return user_ && user_->RecordTimeline();
    }

    _Bool SaveUpdateChannel(
        const std::string channel);

    _Bool UpdateChannel(
        std::string *update_channel);

    _Bool CreateProject(
        const Poco::UInt64 workspace_id,
        const Poco::UInt64 client_id,
        const std::string project_name,
        const _Bool is_private,
        Project **result);

    _Bool CreateClient(
        const Poco::UInt64 workspace_id,
        const std::string client_name);

    void SetSleep();

    void SetWake();

    void SetOnline();

    _Bool OpenReportsInBrowser();

    void SetIdleSeconds(const Poco::UInt64 idle_seconds) {
        idle_.SetIdleSeconds(idle_seconds, user_);
    }

    static void SetLogPath(const std::string path);

    void SetQuit() {
        quit_ = true;
    }

    std::string UserFullName() const;

    std::string UserEmail() const;

    // Timeline datasource
    error CreateTimelineBatch(TimelineBatch *batch);
    error SaveTimelineEvent(TimelineEvent *event);
    error DeleteTimelineBatch(const std::vector<TimelineEvent> &events);

 protected:
    void uiUpdaterActivity();

 private:
    error updateURL(std::string *result);

    static const std::string installerPlatform();
    static const std::string linuxPlatformName();

    Poco::Logger &logger() const;

    void sync(const bool full_sync);

    error save(const bool push_changes = true);

    void pushChanges();

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
    void onRemind(Poco::Util::TimerTask&);  // NOLINT
    void onPeriodicSync(Poco::Util::TimerTask& task);  // NOLINT

    void startPeriodicUpdateCheck();
    void executeUpdateCheck();

    void startPeriodicSync();

    void setUser(User *value, const bool user_logged_in = false);

    void displayUI();

    void switchWebSocketOff();
    void switchWebSocketOn();
    void switchTimelineOff();
    void switchTimelineOn();

    Database *db() const;

    std::vector<TimeEntry *> timeEntries(const bool including_running) const;

    TogglTimeEntryView *timeEntryViewItem(TimeEntry *te);

    void displayTimerState();
    void displayTimeEntryEditor(const _Bool open,
                                TimeEntry *te,
                                const std::string focused_field_name);
    void displayTimeEntryAutocomplete();
    void displayMinitimerAutocomplete();
    void displayProjectAutocomplete();
    void displayWorkspaceSelect();
    void displayClientSelect();
    void displayTags();

    Poco::Int64 totalDurationForDate(TimeEntry *te) const;

    void updateUI(std::vector<ModelChange> *changes);

    _Bool displayError(const error err);

    bool canSeeBillable(Workspace *workspace) const;

    void scheduleSync();

    void setOnline(const std::string reason);

    void remindToTrackTime();

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
    Poco::Timestamp next_reminder_at_;

    // Schedule tasks using a timer:
    Poco::Mutex timer_m_;
    Poco::Util::Timer timer_;

    class GUI ui_;

    std::string time_entry_editor_guid_;

    std::string environment_;

    Idle idle_;

    Poco::UInt64 last_sync_started_;
    Poco::Int64 sync_interval_seconds_;

    bool update_check_disabled_;

    Poco::LocalDateTime last_time_entry_list_render_at_;

    bool quit_;

    Poco::Mutex ui_updater_m_;
    Poco::Activity<Context> ui_updater_;

    Analytics analytics_;

    std::string update_path_;

    bool im_a_teapot_;

    static std::string log_path_;
};

void on_websocket_message(
    void *context,
    std::string json);

}  // namespace toggl

#endif  // SRC_CONTEXT_H_
