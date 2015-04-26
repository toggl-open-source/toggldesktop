// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_GUI_H_
#define SRC_GUI_H_

#include <string>
#include <vector>

#include "./toggl_api.h"
#include "./types.h"
#include "./autocomplete_item.h"
#include "./settings.h"
#include "./proxy.h"
#include "./https_client.h"

namespace Poco {
class Logger;
}

namespace toggl {

class Client;
class Workspace;

class GUI : public SyncStateMonitor {
 public:
    GUI() : on_display_app_(0)
    , on_display_error_(0)
    , on_display_online_state_(0)
    , on_display_login_(0)
    , on_display_url_(0)
    , on_display_reminder_(0)
    , on_display_time_entry_list_(0)
    , on_display_time_entry_autocomplete_(0)
    , on_display_project_autocomplete_(0)
    , on_display_workspace_select_(0)
    , on_display_client_select_(0)
    , on_display_tags_(0)
    , on_display_time_entry_editor_(0)
    , on_display_settings_(0)
    , on_display_timer_state_(0)
    , on_display_idle_notification_(0)
    , on_display_mini_timer_autocomplete_(0)
    , on_display_sync_state_(0)
    , on_display_unsynced_items_(0)
    , on_display_update_(0) {}

    ~GUI() {}

    void DisplayApp();

    _Bool DisplayError(const error);

    void DisplaySyncState(const Poco::Int64 state);

    void DisplayOnlineState(const Poco::Int64 state);

    void DisplayUnsyncedItems(const Poco::Int64 count);

    void DisplayReminder();

    void DisplayMinitimerAutocomplete(std::vector<toggl::AutocompleteItem> *);

    void DisplayTimeEntryAutocomplete(std::vector<toggl::AutocompleteItem> *);

    void DisplayProjectAutocomplete(std::vector<toggl::AutocompleteItem> *);

    void DisplayTimeEntryList(
        const _Bool open,
        TogglTimeEntryView *first);

    void DisplayWorkspaceSelect(std::vector<toggl::Workspace *> *list);

    void DisplayClientSelect(std::vector<toggl::Client *> *clients);

    void DisplayTags(std::vector<std::string> *tags);

    void DisplayTimeEntryEditor(
        const _Bool open,
        TogglTimeEntryView *te,
        const std::string focused_field_name);

    void DisplayURL(const std::string);

    void DisplayLogin(const _Bool open, const uint64_t user_id);

    void DisplaySettings(
        const _Bool open,
        const _Bool record_timeline,
        const Settings settings,
        const _Bool use_proxy,
        const Proxy proxy);

    void DisplayTimerState(TogglTimeEntryView *te);

    void DisplayIdleNotification(const std::string guid,
                                 const std::string since,
                                 const std::string duration,
                                 const uint64_t started,
                                 const std::string description);

    void DisplayUpdate(const std::string URL);

    error VerifyCallbacks();

    void OnDisplayUpdate(TogglDisplayUpdate cb) {
        on_display_update_ = cb;
    }

    void OnDisplayApp(TogglDisplayApp cb) {
        on_display_app_ = cb;
    }

    void OnDisplayError(TogglDisplayError cb) {
        on_display_error_ = cb;
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

    bool CanDisplayUpdate() const {
        return !!on_display_update_;
    }

 private:
    error findMissingCallbacks();

    TogglDisplayApp on_display_app_;
    TogglDisplayError on_display_error_;
    TogglDisplayOnlineState on_display_online_state_;
    TogglDisplayLogin on_display_login_;
    TogglDisplayURL on_display_url_;
    TogglDisplayReminder on_display_reminder_;
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

    Poco::Logger &logger() const;
};

}  // namespace toggl

#endif  // SRC_GUI_H_
