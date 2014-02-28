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

// FIXME: don't use C API from C++ class
#include "./kopsik_api.h"

#include "Poco/Util/Timer.h"

namespace kopsik {

class AutocompleteItem {
 public:
  AutocompleteItem()
    : Text("")
    , Description("")
    , ProjectAndTaskLabel("")
    , ProjectColor("")
    , TaskID(0)
    , ProjectID(0)
    , Type(0) {}
  ~AutocompleteItem() {}

  bool IsTimeEntry() const { return KOPSIK_AUTOCOMPLETE_TE == Type; }
  bool IsTask() const { return KOPSIK_AUTOCOMPLETE_TASK == Type; }
  bool IsProject() const { return KOPSIK_AUTOCOMPLETE_PROJECT == Type; }

  std::string Text;
  std::string Description;
  std::string ProjectAndTaskLabel;
  std::string ProjectColor;
  Poco::UInt64 TaskID;
  Poco::UInt64 ProjectID;
  Poco::UInt64 Type;
};

bool CompareAutocompleteItems(AutocompleteItem a, AutocompleteItem b);

// FIXME: rename
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
    kopsik::error SendFeedback(
      const std::string topic,
      const std::string details,
      const std::string filename);

    // Load model update from JSON string (from WebSocket)
    void LoadUpdateFromJSONString(const std::string json);

    // FIXME: dont use C callbacks in C++ class
    void SetChangeCallback(KopsikViewItemChangeCallback cb);
    void SetOnErrorCallback(KopsikErrorCallback cb);
    void SetCheckUpdatesCallback(KopsikCheckUpdateCallback cb);
    void SetOnOnlineCallback(KopsikOnOnlineCallback cb);

    // Apply proxy settings
    kopsik::error ConfigureProxy();

    // Configure
    void SetAPIURL(const std::string value);
    void SetTimelineUploadURL(const std::string value);
    void SetWebSocketClientURL(const std::string value);
    void SetDBPath(
      const std::string path);
    kopsik::error LoadSettings(
      bool *use_proxy,
      kopsik::Proxy *proxy,
      bool *use_idle_settings) const;
    kopsik::error SaveSettings(
      const bool use_proxy,
      const kopsik::Proxy *proxy,
      const bool use_idle_detection);

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
    Poco::UInt64 UsersDefaultWID() const;
    void CollectPushableTimeEntries(
      std::vector<kopsik::TimeEntry *> *models) const;
    std::vector<std::string> Tags() const;
    std::vector<kopsik::Workspace *> Workspaces() const;
    std::vector<kopsik::Client *> Clients(
      const Poco::UInt64 workspace_id) const;
    kopsik::TimeEntry *GetTimeEntryByGUID(const std::string GUID) const;

    kopsik::TimeEntry *Start(
      const std::string description,
      const std::string duration,
      const Poco::UInt64 task_id,
      const Poco::UInt64 project_id);
    kopsik::TimeEntry *ContinueLatest();
    kopsik::TimeEntry *Continue(const std::string GUID);
    kopsik::error SetTimeEntryDuration(
      const std::string GUID,
      const std::string duration);
    kopsik::error DeleteTimeEntryByGUID(const std::string GUID);
    kopsik::error SetTimeEntryProject(
      const std::string GUID,
      const Poco::UInt64 task_id,
      const Poco::UInt64 project_id);
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
      const std::string project_name);

  private:
    const std::string updateURL() const;
    static const std::string osName();
    Poco::Logger &logger() const { return Poco::Logger::get("context"); }
    const std::string feedbackJSON() const;
    const std::string feedback_filename() const;
    const std::string base64encode_attachment() const;
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
    void onTimelineUpdateServerSettings(Poco::Util::TimerTask& task);  // NOLINT
    void onSendFeedback(Poco::Util::TimerTask& task);  // NOLINT

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

    // FIXME: move into class
    std::string feedback_attachment_path_;
    std::string feedback_subject_;
    std::string feedback_details_;

    // FIXME: dont use C callbacks in C++ class
    KopsikViewItemChangeCallback change_callback_;
    KopsikErrorCallback on_error_callback_;
    KopsikCheckUpdateCallback check_updates_callback_;
    KopsikOnOnlineCallback on_online_callback_;

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
