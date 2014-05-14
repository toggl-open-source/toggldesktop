// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CONTEXT_H_
#define SRC_CONTEXT_H_

#include <string>
#include <vector>
#include <map>

#include "./types.h"
#include "./database.h"
#include "./websocket_client.h"
#include "./window_change_recorder.h"
#include "./timeline_uploader.h"
#include "./CustomErrorHandler.h"
#include "./autocomplete_item.h"
#include "./feedback.h"
#include "./kopsik_api.h"
#include "./ui.h"

#include "Poco/Timestamp.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Util/Timer.h"
#include "Poco/LocalDateTime.h"

namespace kopsik {

class Context {
 public:
    Context(
        const std::string app_name,
        const std::string app_version);
    ~Context();

    kopsik::UI *UI() {
        return &ui_;
    }

    // Check for logged in user etc, start up the app
    _Bool StartEvents();

    // Close connections and wait for tasks to finish
    void Shutdown();

    // Start tasks
    void FullSync();
    void FetchUpdates();
    void TimelineUpdateServerSettings();
    _Bool SendFeedback(Feedback);

    // Load model update from JSON string (from WebSocket)
    _Bool LoadUpdateFromJSONString(const std::string json);

    // Configure
    void SetAPIURL(const std::string value) {
        api_url_ = value;
    }
    void SetTimelineUploadURL(const std::string value) {
        timeline_upload_url_ = value;
    }
    void SetWebSocketClientURL(const std::string value);

    _Bool SetDBPath(const std::string path);

    _Bool Settings(kopsik::Settings *);

    _Bool SetSettings(const kopsik::Settings);

    _Bool ProxySettings(bool *use_proxy, Proxy *proxy);

    _Bool SetProxySettings(
        const bool use_proxy,
        const kopsik::Proxy proxy);

    // Session management
    _Bool CurrentAPIToken(std::string *token);

    _Bool SetCurrentAPIToken(const std::string token);

    _Bool loadCurrentUser();

    _Bool Login(
        const std::string email,
        const std::string password);

    _Bool Logout();

    _Bool SetLoggedInUserFromJSON(const std::string json);

    _Bool ClearCache();

    bool CanSeeBillable(const std::string GUID) const;
    bool CanAddProjects(const Poco::UInt64 workspace_id) const;

    Poco::UInt64 UsersDefaultWID() const;

    void CollectPushableTimeEntries(
        std::vector<kopsik::TimeEntry *> *models) const;

    _Bool Start(
        const std::string description,
        const std::string duration,
        const Poco::UInt64 task_id,
        const Poco::UInt64 project_id,
        kopsik::TimeEntry **);

    _Bool ContinueLatest(
        kopsik::TimeEntry **);

    _Bool Continue(
        const std::string GUID,
        kopsik::TimeEntry **);

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

    _Bool SetTimeEntryStartISO8601(
        const std::string GUID,
        const std::string value);

    _Bool SetTimeEntryEndISO8601(
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

    _Bool Stop(kopsik::TimeEntry **stopped_entry);

    _Bool StopAt(
        const Poco::Int64 at,
        kopsik::TimeEntry **result);

    _Bool RunningTimeEntry(
        kopsik::TimeEntry **running) const;

    _Bool ToggleTimelineRecording();

    _Bool SaveUpdateChannel(
        const std::string channel);

    _Bool LoadUpdateChannel(std::string *channel);

    _Bool AddProject(
        const Poco::UInt64 workspace_id,
        const Poco::UInt64 client_id,
        const std::string project_name,
        const _Bool is_private,
        Project **result);

    void SetSleep();
    void SetWake();

 protected:
    kopsik::HTTPSClient https_client() const;

 private:
    const std::string updateURL() const;

    static const std::string osName();

    Poco::Logger &logger() const {
        return Poco::Logger::get("context");
    }

    void sync(const bool full_sync);

    error save(const bool push_changes = true);

    void partialSync();

    // timer_ callbacks
    void onFullSync(Poco::Util::TimerTask& task);  // NOLINT
    void onPartialSync(Poco::Util::TimerTask& task);  // NOLINT
    void onSwitchWebSocketOff(Poco::Util::TimerTask& task);  // NOLINT
    void onSwitchWebSocketOn(Poco::Util::TimerTask& task);  // NOLINT
    void onSwitchTimelineOff(Poco::Util::TimerTask& task);  // NOLINT
    void onSwitchTimelineOn(Poco::Util::TimerTask& task);  // NOLINT
    void onFetchUpdates(Poco::Util::TimerTask& task);  // NOLINT
    void onPeriodicUpdateCheck(Poco::Util::TimerTask& task);  // NOLINT
    void onTimelineUpdateServerSettings(Poco::Util::TimerTask& task);  // NOLINT
    void onSendFeedback(Poco::Util::TimerTask& task);  // NOLINT
    void onRemind(Poco::Util::TimerTask&);  // NOLINT

    void startPeriodicUpdateCheck();
    void executeUpdateCheck();

    void timeEntryAutocompleteItems(
        std::vector<AutocompleteItem> *list) const;
    void taskAutocompleteItems(
        std::vector<AutocompleteItem> *list) const;
    void projectAutocompleteItems(
        std::vector<AutocompleteItem> *list) const;

    bool isPostponed(
        const Poco::Timestamp value,
        const Poco::Timestamp::TimeDiff
        throttleMicros = kRequestThrottleMicros) const;
    static Poco::Timestamp postpone(
        const Poco::Timestamp::TimeDiff
        throttleMicros = kRequestThrottleMicros);

    void setUser(User *value);

    void switchWebSocketOff();
    void switchWebSocketOn();
    void switchTimelineOff();
    void switchTimelineOn();

    Database *db() const;

    std::vector<AutocompleteItem> autocompleteItems(
        const bool include_time_entries,
        const bool include_tasks,
        const bool include_projects) const;

    std::vector<kopsik::TimeEntry *> timeEntries() const;

    void projectLabelAndColorCode(
        kopsik::TimeEntry *te,
        std::string *project_and_task_label,
        std::string *color_code) const;

    std::vector<std::string> tags() const;
    std::vector<kopsik::Workspace *> workspaces() const;
    std::vector<kopsik::Client *> clients() const;

    KopsikTimeEntryViewItem *timeEntryViewItem(TimeEntry *te);

    void displayTimerState();

    void displayTimeEntryEditor(const _Bool open,
                                TimeEntry *te,
                                const std::string focused_field_name);

    Poco::Int64 totalDurationForDate(TimeEntry *te) const;

    Poco::Mutex db_m_;
    kopsik::Database *db_;

    Poco::Mutex user_m_;
    kopsik::User *user_;

    Poco::Mutex ws_client_m_;
    kopsik::WebSocketClient *ws_client_;

    Poco::Mutex timeline_uploader_m_;
    kopsik::TimelineUploader *timeline_uploader_;

    Poco::Mutex window_change_recorder_m_;
    kopsik::WindowChangeRecorder *window_change_recorder_;

    std::string app_name_;
    std::string app_version_;

    std::string api_url_;
    std::string timeline_upload_url_;

    CustomErrorHandler error_handler_;

    std::string update_channel_;

    Feedback feedback_;

    // Tasks are scheduled at:
    Poco::Timestamp next_full_sync_at_;
    Poco::Timestamp next_partial_sync_at_;
    Poco::Timestamp next_fetch_updates_at_;
    Poco::Timestamp next_update_timeline_settings_at_;
    Poco::Timestamp next_reminder_at_;

    // Schedule tasks using a timer:
    Poco::Mutex timer_m_;
    Poco::Util::Timer timer_;

    kopsik::UI ui_;

    std::string time_entry_editor_guid_;
};

}  // namespace kopsik

#endif  // SRC_CONTEXT_H_
