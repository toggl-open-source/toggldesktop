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
#include "./model_change.h"
#include "./timeline_event.h"
#include "./timeline_notifications.h"
#include "./toggl_api.h"
#include "./types.h"
#include "./websocket_client.h"

#include "Poco/Activity.h"
#include "Poco/LocalDateTime.h"
#include "Poco/Timestamp.h"
#include "Poco/Util/Timer.h"

namespace toggl {

class AutotrackerRule;
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
    error StartEvents();

    // Close connections and wait for tasks to finish
    void Shutdown();

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

    error SetSettingsIdleMinutes(const Poco::UInt64 idle_minutes);

    error SetSettingsFocusOnShortcut(const bool focus_on_shortcut);

    error SetSettingsReminderMinutes(const Poco::UInt64 reminder_minutes);

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

    error Login(
        const std::string email,
        const std::string password);

    error Signup(
        const std::string email,
        const std::string password);

    error GoogleLogin(const std::string access_token);

    error Logout();

    error SetLoggedInUserFromJSON(const std::string user_data_json);

    error ClearCache();

    TimeEntry *Start(
        const std::string description,
        const std::string duration,
        const Poco::UInt64 task_id,
        const Poco::UInt64 project_id,
        const std::string project_guid);

    error ContinueLatest();

    error Continue(
        const std::string GUID);

    void DisplayTimeEntryList(const bool open);

    error DisplaySettings(const bool open = false);

    void Edit(const std::string GUID,
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

    error Stop();

    error DiscardTimeAt(
        const std::string GUID,
        const Poco::Int64 at,
        const bool split_into_new_entry);

    error DiscardTimeAndContinue(
        const std::string GUID,
        const Poco::Int64 at);

    TimeEntry * RunningTimeEntry() const;

    error ToggleTimelineRecording(
        const bool record_timeline);

    bool IsTimelineRecordingEnabled() {
        return user_ && user_->RecordTimeline();
    }

    error SaveUpdateChannel(
        const std::string channel);

    error UpdateChannel(
        std::string *update_channel);

    Project *CreateProject(
        const Poco::UInt64 workspace_id,
        const Poco::UInt64 client_id,
        const std::string project_name,
        const bool is_private);

    error CreateClient(
        const Poco::UInt64 workspace_id,
        const std::string client_name);

    void SetSleep();

    void SetWake();

    void SetOnline();

    error OpenReportsInBrowser();

    void SetIdleSeconds(const Poco::UInt64 idle_seconds) {
        idle_.SetIdleSeconds(idle_seconds, user_);
    }

    static void SetLogPath(const std::string path);

    void SetQuit() {
        quit_ = true;
    }

    error AddAutotrackerRule(
        const std::string term,
        const Poco::UInt64 pid);

    error DeleteAutotrackerRule(
        const Poco::Int64 id);

    std::string UserFullName() const;

    std::string UserEmail() const;

    // Timeline datasource
    error StartAutotrackerEvent(const TimelineEvent event);
    error CreateCompressedTimelineBatchForUpload(TimelineBatch *batch);
    error StartTimelineEvent(TimelineEvent *event);
    error MarkTimelineBatchAsUploaded(
        const std::vector<TimelineEvent> &events);

 protected:
    void uiUpdaterActivity();

 private:
    error updateURL(std::string *result);

    void trackSettingsUsage();

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
    void onTrackSettingsUsage(Poco::Util::TimerTask& task);  // NOLINT

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
    void displayTimeEntryEditor(const bool open,
                                TimeEntry *te,
                                const std::string focused_field_name);
    void displayTimeEntryAutocomplete();
    void displayMinitimerAutocomplete();
    void displayProjectAutocomplete();
    void displayWorkspaceSelect();
    void displayClientSelect();
    void displayTags();
    void displayAutotrackerRules();

    void displayReminder();

    Poco::Int64 totalDurationForDate(TimeEntry *te) const;

    void updateUI(std::vector<ModelChange> *changes);

    error displayError(const error err);

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

    AutotrackerRule *findAutotrackerRule(const TimelineEvent event) const;

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
    Poco::Timestamp next_analytics_at_;

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

    Settings settings_;

    std::set<std::string> autotracker_titles_;
};

void on_websocket_message(
    void *context,
    std::string json);

}  // namespace toggl

#endif  // SRC_CONTEXT_H_
