// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_GUI_H_
#define SRC_GUI_H_

#include <string>
#include <vector>

#include "./libkopsik/include/kopsik_api.h"
#include "./types.h"
#include "./autocomplete_item.h"
#include "./time_entry.h"
#include "./workspace.h"
#include "./client.h"
#include "./project.h"
#include "./settings.h"
#include "./proxy.h"
#include "./kopsik_api_private.h"

namespace kopsik {

class GUI {
 public:
    GUI() : on_display_app_(0)
    , on_display_error_(0)
    , on_display_update_(0)
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
    , on_display_idle_notification_(0) {}

    ~GUI() {}

    void DisplayApp();
    _Bool DisplayError(const error);
    void DisplayOnlineState(const _Bool online, const std::string reason);
    void DisplayReminder();
    void DisplayUpdate(const _Bool open,
                       const std::string update_channel,
                       const _Bool is_checking,
                       const _Bool is_available,
                       const std::string url,
                       const std::string version);
    void DisplayTimeEntryAutocomplete(std::vector<kopsik::AutocompleteItem> *);
    void DisplayProjectAutocomplete(std::vector<kopsik::AutocompleteItem> *);
    void DisplayTimeEntryList(const _Bool open,
                              KopsikTimeEntryViewItem *first);
    void DisplayWorkspaceSelect(std::vector<kopsik::Workspace *> *list);
    void DisplayClientSelect(std::vector<kopsik::Client *> *clients);
    void DisplayTags(std::vector<std::string> *tags);
    void DisplayTimeEntryEditor(
        const _Bool open,
        KopsikTimeEntryViewItem *te,
        const std::string focused_field_name);
    void DisplayURL(const std::string);
    void DisplayLogin(const _Bool open, const uint64_t user_id);
    void DisplaySettings(const _Bool open,
                         const _Bool record_timeline,
                         const Settings settings,
                         const _Bool use_proxy,
                         const Proxy proxy);
    void DisplayTimerState(KopsikTimeEntryViewItem *te);
    void DisplayIdleNotification(const std::string guid,
                                 const std::string since,
                                 const std::string duration,
                                 const uint64_t started);

    error VerifyCallbacks();

    void OnDisplayApp(KopsikDisplayApp cb) {
        on_display_app_ = cb;
    }

    void OnDisplayError(KopsikDisplayError cb) {
        on_display_error_ = cb;
    }

    void OnDisplayUpdate(KopsikDisplayUpdate cb) {
        on_display_update_ = cb;
    }

    void OnDisplayOnlineState(KopsikDisplayOnlineState cb) {
        on_display_online_state_ = cb;
    }

    void OnDisplayLogin(KopsikDisplayLogin cb) {
        on_display_login_ = cb;
    }

    void OnDisplayURL(KopsikDisplayURL cb) {
        on_display_url_ = cb;
    }

    void OnDisplayReminder(KopsikDisplayReminder cb) {
        on_display_reminder_ = cb;
    }

    void OnDisplayTimeEntryList(KopsikDisplayTimeEntryList cb) {
        on_display_time_entry_list_ = cb;
    }

    void OnDisplayWorkspaceSelect(KopsikDisplayViewItems cb) {
        on_display_workspace_select_ = cb;
    }

    void OnDisplayClientSelect(KopsikDisplayViewItems cb) {
        on_display_client_select_ = cb;
    };

    void OnDisplayTags(KopsikDisplayViewItems cb) {
        on_display_tags_ = cb;
    }

    void OnDisplayTimeEntryEditor(KopsikDisplayTimeEntryEditor cb) {
        on_display_time_entry_editor_ = cb;
    }

    void OnDisplayTimeEntryAutocomplete(KopsikDisplayAutocomplete cb) {
        on_display_time_entry_autocomplete_ = cb;
    }

    void OnDisplayProjectAutocomplete(KopsikDisplayAutocomplete cb) {
        on_display_project_autocomplete_ = cb;
    }

    void OnDisplaySettings(KopsikDisplaySettings cb) {
        on_display_settings_ = cb;
    }

    void OnDisplayTimerState(KopsikDisplayTimerState cb) {
        on_display_timer_state_ = cb;
    }

    void OnDisplayIdleNotification(KopsikDisplayIdleNotification cb) {
        on_display_idle_notification_  = cb;
    }

 private:
    _Bool isNetworkingError(const error) const;
    _Bool isUserError(const error) const;
    error findMissingCallbacks();

    KopsikDisplayApp on_display_app_;
    KopsikDisplayError on_display_error_;
    KopsikDisplayUpdate on_display_update_;
    KopsikDisplayOnlineState on_display_online_state_;
    KopsikDisplayLogin on_display_login_;
    KopsikDisplayURL on_display_url_;
    KopsikDisplayReminder on_display_reminder_;
    KopsikDisplayTimeEntryList on_display_time_entry_list_;
    KopsikDisplayAutocomplete on_display_time_entry_autocomplete_;
    KopsikDisplayAutocomplete on_display_project_autocomplete_;
    KopsikDisplayViewItems on_display_workspace_select_;
    KopsikDisplayViewItems on_display_client_select_;
    KopsikDisplayViewItems on_display_tags_;
    KopsikDisplayTimeEntryEditor on_display_time_entry_editor_;
    KopsikDisplaySettings on_display_settings_;
    KopsikDisplayTimerState on_display_timer_state_;
    KopsikDisplayIdleNotification on_display_idle_notification_;

    Poco::Logger &logger() const {
        return Poco::Logger::get("ui");
    }
};

}  // namespace kopsik

#endif  // SRC_GUI_H_
