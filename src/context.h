// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CONTEXT_H_
#define SRC_CONTEXT_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream> // NOLINT

#include "./types.h"
#include "./database.h"
#include "./websocket_client.h"
#include "./window_change_recorder.h"
#include "./timeline_uploader.h"
#include "./custom_error_handler.h"
#include "./autocomplete_item.h"
#include "./feedback.h"
#include "./lib/include/toggl_api.h"
#include "./gui.h"

#include "Poco/Timestamp.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Util/Timer.h"
#include "Poco/LocalDateTime.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"

namespace toggl {

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

    void SetAPIURL(const std::string value) {
        HTTPSClientConfig::APIURL = value;
    }
    void SetTimelineUploadURL(const std::string value) {
        timeline_upload_url_ = value;
    }
    void SetWebSocketClientURL(const std::string value);

    _Bool SetDBPath(const std::string path);

    void SetEnvironment(const std::string environment);
    std::string Environment() const {
        return environment_;
    }

    void DisableUpdateCheck() {
        update_check_disabled_ = true;
    }

    _Bool LoadSettings(Settings *settings);

    _Bool SetSettings(const Settings);

    _Bool ProxySettings(bool *use_proxy, Proxy *proxy);

    _Bool SetProxySettings(
        const bool use_proxy,
        const Proxy proxy);

    _Bool Login(
        const std::string email,
        const std::string password);

    _Bool GoogleLogin(const std::string access_token);

    _Bool Logout();

    _Bool SetLoggedInUserFromJSON(const std::string json);

    _Bool ClearCache();

    void CollectPushableTimeEntries(
        std::vector<TimeEntry *> *models) const;

    _Bool Start(
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

    void About();

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

    _Bool Stop();

    _Bool DiscardTimeAt(
        const std::string GUID,
        const Poco::Int64 at);

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

    _Bool AddProject(
        const Poco::UInt64 workspace_id,
        const Poco::UInt64 client_id,
        const std::string project_name,
        const _Bool is_private,
        Project **result);

    void SetSleep();
    void SetWake();
    void SetOnline();

    _Bool OpenReportsInBrowser();

    void SetIdleSeconds(const Poco::UInt64 idle_seconds);

    static void SetLogPath(const std::string path) {
        Poco::AutoPtr<Poco::SimpleFileChannel> simpleFileChannel(
            new Poco::SimpleFileChannel);
        simpleFileChannel->setProperty("path", path);
        simpleFileChannel->setProperty("rotation", "1 M");

        Poco::AutoPtr<Poco::FormattingChannel> formattingChannel(
            new Poco::FormattingChannel(
                new Poco::PatternFormatter(
                    "%Y-%m-%d %H:%M:%S.%i [%P %I]:%s:%q:%t")));
        formattingChannel->setChannel(simpleFileChannel);

        Poco::Logger::get("").setChannel(formattingChannel);
    }

    void SetQuit() {
        quit_ = true;
    }

    // Timeline datasource
    error CreateTimelineBatch(TimelineBatch *batch);
    error SaveTimelineEvent(TimelineEvent *event);
    error DeleteTimelineBatch(const std::vector<TimelineEvent> &events);

 private:
    const std::string updateURL();

    static const std::string installerPlatform();
    static const std::string linuxPlatformName();

    Poco::Logger &logger() const {
        return Poco::Logger::get("context");
    }

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

    bool isPostponed(
        const Poco::Timestamp value,
        const Poco::Timestamp::TimeDiff throttleMicros) const;
    static Poco::Timestamp postpone(
        const Poco::Timestamp::TimeDiff throttleMicros);

    void setUser(User *value, const bool user_logged_in = false);

    void displayUI();

    void switchWebSocketOff();
    void switchWebSocketOn();
    void switchTimelineOff();
    void switchTimelineOn();

    Database *db() const;

    std::vector<TimeEntry *> timeEntries(const bool including_running) const;

    void projectLabelAndColorCode(
        TimeEntry *te,
        std::string *project_and_task_label,
        std::string *task_label,
        std::string *project_label,
        std::string *client_label,
        std::string *color_code) const;

    std::vector<std::string> tags() const;
    std::vector<Workspace *> workspaces() const;
    std::vector<Client *> clients() const;

    TogglTimeEntryView *timeEntryViewItem(TimeEntry *te);

    void displayTimerState();
    void displayTimeEntryEditor(const _Bool open,
                                TimeEntry *te,
                                const std::string focused_field_name);
    void displayTimeEntryAutocomplete();
    void displayProjectAutocomplete();
    void displayWorkspaceSelect();
    void displayClientSelect();
    void displayTags();

    Poco::Int64 totalDurationForDate(TimeEntry *te) const;

    void updateUI(std::vector<ModelChange> *changes);

    _Bool displayError(const error err, const std::string calling_method);

    std::string timeOfDayFormat() const;

    bool canSeeBillable(
        TimeEntry *time_entry,
        Workspace *workspace) const;

    void scheduleSync();
    void displayOnlineState(const std::string reason);
    void remindToTrackTime();

    Poco::Mutex db_m_;
    Database *db_;

    Poco::Mutex user_m_;
    User *user_;

    Poco::Mutex ws_client_m_;
    WebSocketClient *ws_client_;

    Poco::Mutex timeline_uploader_m_;
    TimelineUploader *timeline_uploader_;

    Poco::Mutex window_change_recorder_m_;
    WindowChangeRecorder *window_change_recorder_;

    std::string timeline_upload_url_;

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

    Poco::UInt64 last_idle_seconds_reading_;
    Poco::UInt64 last_idle_started_;

    Poco::UInt64 last_sync_started_;

    bool update_check_disabled_;

    Poco::LocalDateTime last_time_entry_list_render_at_;

    bool quit_;
};

}  // namespace toggl

#endif  // SRC_CONTEXT_H_
