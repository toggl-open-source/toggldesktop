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

#include "Poco/Util/Timer.h"

namespace kopsik {

// FIXME: this is too low-level, we should not export it
typedef void (*ModelChangeCallback)(
  const ModelChange change);

typedef void (*ErrorCallback)(
  const error err);

typedef void (*CheckUpdateCallback)(
  const bool is_update_available,
  const std::string url,
  const std::string version);

// FIXME: add flag indicating online/offline state
typedef void (*OnlineCallback)();

class Context {
  public:
    Context(
      const std::string app_name,
      const std::string app_version);
    ~Context();

    // Close connections and wait for tasks to finish
    void Shutdown();

    // Start tasks
    void FullSync();
    void SwitchWebSocketOff();
    void SwitchWebSocketOn();
    void SwitchTimelineOff();
    void SwitchTimelineOn();
    void FetchUpdates();
    void TimelineUpdateServerSettings();
    kopsik::error SendFeedback(Feedback);

    // Load model update from JSON string (from WebSocket)
    void LoadUpdateFromJSONString(const std::string json);

    void SetModelChangeCallback(ModelChangeCallback cb) {
      on_model_change_callback_ = cb; }
    void SetOnErrorCallback(ErrorCallback cb) {
      on_error_callback_ = cb; }
    void SetCheckUpdateCallback(CheckUpdateCallback cb) {
      on_check_update_callback_ = cb; }
    void SetOnOnlineCallback(OnlineCallback cb) {
      on_online_callback_ = cb; }

    // Apply proxy settings
    kopsik::error ConfigureProxy();

    // Configure
    void SetAPIURL(const std::string value) { api_url_ = value; }
    void SetTimelineUploadURL(const std::string value) {
        timeline_upload_url_ = value; }
    void SetWebSocketClientURL(const std::string value);
    void SetDBPath(
      const std::string path);

    kopsik::error LoadSettings(
      bool *use_idle_settings,
      bool *menubar_timer,
      bool *dock_icon) const;

    kopsik::error SaveSettings(
      const bool use_idle_detection,
      const bool menubar_timer,
      const bool dock_icon);

    kopsik::error LoadProxySettings(
      bool *use_proxy,
      kopsik::Proxy *proxy) const;

    kopsik::error SaveProxySettings(
      const bool use_proxy,
      const kopsik::Proxy *proxy);

    // Session management
    kopsik::error CurrentAPIToken(std::string *token);
    kopsik::error SetCurrentAPIToken(const std::string token);
    kopsik::error CurrentUser(kopsik::User **result);
    kopsik::error Login(
      const std::string email,
      const std::string password);
    kopsik::error Logout();
    kopsik::error SetLoggedInUserFromJSON(const std::string json);
    kopsik::error ClearCache();

    bool UserHasPremiumWorkspaces() const;
    bool UserIsLoggedIn() const;
    Poco::UInt64 UsersDefaultWID() const;
    void CollectPushableTimeEntries(
      std::vector<kopsik::TimeEntry *> *models) const;
    std::vector<std::string> Tags() const;
    std::vector<kopsik::Workspace *> Workspaces() const;
    std::vector<kopsik::Client *> Clients(
      const Poco::UInt64 workspace_id) const;
    kopsik::TimeEntry *GetTimeEntryByGUID(const std::string GUID) const;

    kopsik::error Start(
      const std::string description,
      const std::string duration,
      const Poco::UInt64 task_id,
      const Poco::UInt64 project_id,
      kopsik::TimeEntry **);
    kopsik::error ContinueLatest(
      kopsik::TimeEntry **);
    kopsik::error Continue(
      const std::string GUID,
      kopsik::TimeEntry **);
    kopsik::error SetTimeEntryDuration(
      const std::string GUID,
      const std::string duration);
    kopsik::error DeleteTimeEntryByGUID(const std::string GUID);
    kopsik::error SetTimeEntryProject(
      const std::string GUID,
      const Poco::UInt64 task_id,
      const Poco::UInt64 project_id,
      const std::string project_guid);
    kopsik::error SetTimeEntryStartISO8601(
      const std::string GUID,
      const std::string value);
    kopsik::error SetTimeEntryEndISO8601(
      const std::string GUID,
      const std::string value);
    kopsik::error SetTimeEntryTags(
      const std::string GUID,
      const std::string value);
    kopsik::error SetTimeEntryBillable(
      const std::string GUID,
      const bool value);
    kopsik::error SetTimeEntryDescription(
      const std::string GUID,
      const std::string value);
    kopsik::error Stop(kopsik::TimeEntry **stopped_entry);
    kopsik::error SplitAt(
      const Poco::Int64 at,
      kopsik::TimeEntry **new_running_entry);
    kopsik::error StopAt(
      const Poco::Int64 at,
      kopsik::TimeEntry **stopped);
    kopsik::error RunningTimeEntry(
      kopsik::TimeEntry **running) const;
    kopsik::error ToggleTimelineRecording();
    kopsik::error TimeEntries(
      std::map<std::string, Poco::Int64> *date_durations,
      std::vector<kopsik::TimeEntry *> *visible) const;
    kopsik::error TrackedPerDateHeader(
      const std::string date_header,
      int *sum) const;
    bool RecordTimeline() const;
    kopsik::error SaveUpdateChannel(
      const std::string channel);
    kopsik::error LoadUpdateChannel(std::string *channel);
    void ProjectLabelAndColorCode(
      kopsik::TimeEntry *te,
      std::string *project_and_task_label,
      std::string *color_code) const;
    void AutocompleteItems(
      std::vector<AutocompleteItem> *list,
      const bool include_time_entries,
      const bool include_tasks,
      const bool include_projects) const;
    kopsik::error AddProject(
      const Poco::UInt64 workspace_id,
      const Poco::UInt64 client_id,
      const std::string project_name,
      Project **result);

  private:
    const std::string updateURL() const;

    static const std::string osName();

    Poco::Logger &logger() const { return Poco::Logger::get("context"); }

    void sync(const bool full_sync);

    kopsik::error save();

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

    void startPeriodicUpdateCheck();
    void executeUpdateCheck();

    void getTimeEntryAutocompleteItems(
      std::vector<AutocompleteItem> *list) const;
    void getTaskAutocompleteItems(
      std::vector<AutocompleteItem> *list) const;
    void getProjectAutocompleteItems(
      std::vector<AutocompleteItem> *list) const;

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

    ModelChangeCallback on_model_change_callback_;
    ErrorCallback on_error_callback_;
    CheckUpdateCallback on_check_update_callback_;
    OnlineCallback on_online_callback_;

    // Tasks are scheduled at:
    Poco::Timestamp next_full_sync_at_;
    Poco::Timestamp next_partial_sync_at_;
    Poco::Timestamp next_fetch_updates_at_;
    Poco::Timestamp next_update_timeline_settings_at_;

    // Schedule tasks using a timer:
    Poco::Mutex timer_m_;
    Poco::Util::Timer timer_;
};

}  // namespace kopsik

#endif  // SRC_CONTEXT_H_
