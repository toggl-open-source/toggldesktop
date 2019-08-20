// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_GUI_H_
#define SRC_GUI_H_

#include <set>
#include <string>
#include <vector>

#include "./https_client.h"
#include "./toggl_api.h"
#include "./toggl_api_private.h"

namespace Poco {
class Logger;
}

namespace toggl {

class Client;
class RelatedData;
class User;
class TimeEntry;
class Workspace;

class GUI : public SyncStateMonitor {
 public:
    GUI()
        : on_display_app_(nullptr)
    , on_display_error_(nullptr)
    , on_display_online_state_(nullptr)
    , on_display_login_(nullptr)
    , on_display_url_(nullptr)
    , on_display_reminder_(nullptr)
    , on_display_pomodoro_(nullptr)
    , on_display_pomodoro_break_(nullptr)
    , on_display_time_entry_list_(nullptr)
    , on_display_time_entry_autocomplete_(nullptr)
    , on_display_project_autocomplete_(nullptr)
    , on_display_workspace_select_(nullptr)
    , on_display_client_select_(nullptr)
    , on_display_tags_(nullptr)
    , on_display_time_entry_editor_(nullptr)
    , on_display_settings_(nullptr)
    , on_display_timer_state_(nullptr)
    , on_display_idle_notification_(nullptr)
    , on_display_mini_timer_autocomplete_(nullptr)
    , on_display_sync_state_(nullptr)
    , on_display_unsynced_items_(nullptr)
    , on_display_update_(nullptr)
    , on_display_update_download_state_(nullptr)
    , on_display_autotracker_rules_(nullptr)
    , on_display_autotracker_notification_(nullptr)
    , on_display_promotion_(nullptr)
    , on_display_help_articles_(nullptr)
    , on_display_project_colors_(nullptr)
    , on_display_obm_experiment_(nullptr)
    , lastSyncState(-1)
    , lastUnsyncedItemsCount(-1)
    , lastDisplayLoginOpen(false)
    , lastDisplayLoginUserID(0)
    , lastOnlineState(-1)
    , lastErr(noError)
    , isFirstLaunch(true) {}

    ~GUI() {}

    void DisplayApp();

    error DisplayError(const error);

    // Overlay screen triggers
    error DisplayWSError();
    error DisplayTosAccept();

    void DisplayHelpArticles(
        const std::vector<HelpArticle> articles);

    void DisplaySyncState(const Poco::Int64 state);

    void DisplayOnlineState(const Poco::Int64 state);

    void DisplayUnsyncedItems(const Poco::Int64 count);

    void DisplayReminder();

    void DisplayPomodoro(const Poco::Int64 minutes);

    void DisplayPomodoroBreak(const Poco::Int64 minutes);

    void DisplayAutotrackerNotification(
        toggl::Project *const p,
        toggl::Task *const t);

    void DisplayMinitimerAutocomplete(std::vector<toggl::view::Autocomplete> *);

    void DisplayTimeEntryAutocomplete(std::vector<toggl::view::Autocomplete> *);

    void DisplayProjectAutocomplete(std::vector<toggl::view::Autocomplete> *);

    void DisplayTimeEntryList(
        const bool open,
        const std::vector<view::TimeEntry> list,
        const bool show_load_more_button);

    void DisplayProjectColors();

    void DisplayCountries(
        std::vector<TogglCountryView> *items);

    void DisplayWorkspaceSelect(
        const std::vector<view::Generic> list);

    void DisplayClientSelect(
        const std::vector<view::Generic> list);

    void DisplayTags(
        const std::vector<view::Generic> list);

    void DisplayAutotrackerRules(
        const std::vector<view::AutotrackerRule> &autotracker_rules,
        const std::vector<std::string> &titles);

    void DisplayTimeEntryEditor(
        const bool open,
        const view::TimeEntry te,
        const std::string focused_field_name);

    void DisplayURL(const std::string);

    void DisplayLogin(const bool open, const uint64_t user_id);

    void DisplaySettings(
        const bool open,
        const bool record_timeline,
        const Settings settings,
        const bool use_proxy,
        const Proxy proxy);

    void DisplayTimerState(
        const view::TimeEntry &te);

    void DisplayEmptyTimerState();

    void DisplayIdleNotification(const std::string guid,
                                 const std::string since,
                                 const std::string duration,
                                 const int64_t started,
                                 const std::string description);

    void DisplayUpdate(const std::string URL);

    void DisplayUpdateDownloadState(
        const std::string version,
        const Poco::Int64 download_state);

    error VerifyCallbacks();

    void OnDisplayUpdate(TogglDisplayUpdate cb) {
        on_display_update_ = cb;
    }

    void OnDisplayHelpArticles(TogglDisplayHelpArticles cb) {
        on_display_help_articles_ = cb;
    }

    void OnDisplayProjectColors(TogglDisplayProjectColors cb) {
        on_display_project_colors_ = cb;
    }

    void OnDisplayCountries(TogglDisplayCountries cb) {
        on_display_countries_ = cb;
    }

    void OnDisplayUpdateDownloadState(TogglDisplayUpdateDownloadState cb) {
        on_display_update_download_state_ = cb;
    }

    void OnDisplayApp(TogglDisplayApp cb) {
        on_display_app_ = cb;
    }

    void OnDisplayError(TogglDisplayError cb) {
        on_display_error_ = cb;
    }

    void OnDisplayOverlay(TogglDisplayOverlay cb) {
        on_display_overlay_ = cb;
    }

    void OnDisplayOnlineState(TogglDisplayOnlineState cb) {
        on_display_online_state_ = cb;
    }

    void OnDisplayLogin(TogglDisplayLogin cb) {
        on_display_login_ = cb;
    }

    void OnDisplayURL(TogglDisplayURL cb) {
        on_display_url_ = cb;
    }

    void OnDisplayReminder(TogglDisplayReminder cb) {
        on_display_reminder_ = cb;
    }

    void OnDisplayPomodoro(TogglDisplayPomodoro cb) {
        on_display_pomodoro_ = cb;
    }

    void OnDisplayPomodoroBreak(TogglDisplayPomodoroBreak cb) {
        on_display_pomodoro_break_ = cb;
    }

    void OnDisplayAutotrackerNotification(
        TogglDisplayAutotrackerNotification cb) {
        on_display_autotracker_notification_ = cb;
    }

    void OnDisplayTimeEntryList(TogglDisplayTimeEntryList cb) {
        on_display_time_entry_list_ = cb;
    }

    void OnDisplayWorkspaceSelect(TogglDisplayViewItems cb) {
        on_display_workspace_select_ = cb;
    }

    void OnDisplayClientSelect(TogglDisplayViewItems cb) {
        on_display_client_select_ = cb;
    }

    void OnDisplayTags(TogglDisplayViewItems cb) {
        on_display_tags_ = cb;
    }

    void OnDisplayTimeEntryEditor(TogglDisplayTimeEntryEditor cb) {
        on_display_time_entry_editor_ = cb;
    }

    void OnDisplayTimeEntryAutocomplete(TogglDisplayAutocomplete cb) {
        on_display_time_entry_autocomplete_ = cb;
    }

    void OnDisplayProjectAutocomplete(TogglDisplayAutocomplete cb) {
        on_display_project_autocomplete_ = cb;
    }

    void OnDisplaySettings(TogglDisplaySettings cb) {
        on_display_settings_ = cb;
    }

    void OnDisplayTimerState(TogglDisplayTimerState cb) {
        on_display_timer_state_ = cb;
    }

    void OnDisplayIdleNotification(TogglDisplayIdleNotification cb) {
        on_display_idle_notification_  = cb;
    }

    void OnDisplayMinitimerAutocomplete(TogglDisplayAutocomplete cb) {
        on_display_mini_timer_autocomplete_ = cb;
    }

    void OnDisplaySyncState(TogglDisplaySyncState cb) {
        on_display_sync_state_ = cb;
    }

    void OnDisplayUnsyncedItems(TogglDisplayUnsyncedItems cb) {
        on_display_unsynced_items_ = cb;
    }

    void OnDisplayAutotrackerRules(TogglDisplayAutotrackerRules cb) {
        on_display_autotracker_rules_ = cb;
    }

    void OnDisplayPromotion(TogglDisplayPromotion cb) {
        on_display_promotion_ = cb;
    }

    void OnDisplayObmExperiment(TogglDisplayObmExperiment cb) {
        on_display_obm_experiment_ = cb;
    }

    bool CanDisplayUpdate() const {
        return !!on_display_update_;
    }

    bool CanDisplayUpdateDownloadState() const {
        return !!on_display_update_download_state_;
    }

    bool CanDisplayAutotrackerRules() const {
        return !!on_display_autotracker_rules_;
    }

    bool CanDisplayPromotion() const {
        return !!on_display_promotion_;
    }

    void DisplayPromotion(const int64_t promotion_type) {
        if (on_display_promotion_) {
            on_display_promotion_(promotion_type);
        }
    }

    void DisplayObmExperiment(
        const uint64_t nr,
        const bool_t included,
        const bool_t seen) {
        if (on_display_obm_experiment_) {
            on_display_obm_experiment_(nr, included, seen);
        }
    }

    void resetFirstLaunch() {
        isFirstLaunch = true;
    }

 private:
    error findMissingCallbacks();

    TogglDisplayApp on_display_app_;
    TogglDisplayError on_display_error_;
    TogglDisplayOverlay on_display_overlay_;
    TogglDisplayOnlineState on_display_online_state_;
    TogglDisplayLogin on_display_login_;
    TogglDisplayURL on_display_url_;
    TogglDisplayReminder on_display_reminder_;
    TogglDisplayPomodoro on_display_pomodoro_;
    TogglDisplayPomodoroBreak on_display_pomodoro_break_;
    TogglDisplayTimeEntryList on_display_time_entry_list_;
    TogglDisplayAutocomplete on_display_time_entry_autocomplete_;
    TogglDisplayAutocomplete on_display_project_autocomplete_;
    TogglDisplayViewItems on_display_workspace_select_;
    TogglDisplayViewItems on_display_client_select_;
    TogglDisplayViewItems on_display_tags_;
    TogglDisplayTimeEntryEditor on_display_time_entry_editor_;
    TogglDisplaySettings on_display_settings_;
    TogglDisplayTimerState on_display_timer_state_;
    TogglDisplayIdleNotification on_display_idle_notification_;
    TogglDisplayAutocomplete on_display_mini_timer_autocomplete_;
    TogglDisplaySyncState on_display_sync_state_;
    TogglDisplayUnsyncedItems on_display_unsynced_items_;
    TogglDisplayUpdate on_display_update_;
    TogglDisplayUpdateDownloadState on_display_update_download_state_;
    TogglDisplayAutotrackerRules on_display_autotracker_rules_;
    TogglDisplayAutotrackerNotification on_display_autotracker_notification_;
    TogglDisplayPromotion on_display_promotion_;
    TogglDisplayHelpArticles on_display_help_articles_;
    TogglDisplayProjectColors on_display_project_colors_;
    TogglDisplayCountries on_display_countries_;
    TogglDisplayObmExperiment on_display_obm_experiment_;

    // Cached views
    Poco::Int64 lastSyncState;
    Poco::Int64 lastUnsyncedItemsCount;
    bool lastDisplayLoginOpen;
    uint64_t lastDisplayLoginUserID;
    Poco::Int64 lastOnlineState;
    error lastErr;
    bool isFirstLaunch;
    Poco::Logger &logger() const;
};

}  // namespace toggl

#endif  // SRC_GUI_H_
