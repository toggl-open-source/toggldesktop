// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_H_
#define SRC_UI_H_

#include <string>
#include <vector>

#include "./kopsik_api.h"
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

class UI {
 public:
    UI()
        : on_display_error_(0)
    , on_display_update_(0)
    , on_display_online_state_(0)
    , on_display_login_(0)
    , on_display_url_(0)
    , on_display_reminder_(0)
    , on_display_time_entry_list_(0)
    , on_display_autocomplete_(0)
    , on_display_workspace_select_(0)
    , on_display_client_select_(0)
    , on_display_tags_(0)
    , on_display_time_entry_editor_(0)
    , on_display_settings_(0)
    , on_display_timer_state_(0) {}

    ~UI() {}

    _Bool DisplayError(const error);
    void DisplayOnlineState(const _Bool);
    void DisplayReminder();
    void DisplayUpdate(
        const bool is_available,
        const std::string url,
        const std::string version);
    void DisplayAutocomplete(std::vector<kopsik::AutocompleteItem> *items);
    void DisplayTimeEntryList(const _Bool open,
                              std::vector<kopsik::TimeEntry *> *visible);
    void DisplayWorkspaceSelect(std::vector<kopsik::Workspace *> *list);
    void DisplayClientSelect(std::vector<kopsik::Client *> *clients);
    void DisplayTags(std::vector<std::string> *tags);
    void DisplayTimeEntryEditor(const _Bool open,
                                kopsik::TimeEntry *te,
                                const std::string focused_field_name);
    void DisplayURL(const std::string);
    void DisplayLogin(const _Bool open, const uint64_t user_id);
    void DisplaySettings(const _Bool open,
                         const _Bool record_timeline,
                         const Settings settings,
                         const _Bool use_proxy,
                         const Proxy proxy);
    void DisplayTimerState(kopsik::TimeEntry *te,
                           const std::string project_and_task_label,
                           const std::string color,
                           const std::string start_time_string,
                           const std::string end_time_string,
                           const std::string date_duration);

    error VerifyCallbacks();

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

    void OnDisplayAutocomplete(KopsikDisplayAutocomplete cb) {
        on_display_autocomplete_ = cb;
    }

    void OnDisplaySettings(KopsikDisplaySettings cb) {
        on_display_settings_ = cb;
    }

    void OnDisplayTimerState(KopsikDisplayTimerState cb) {
        on_display_timer_state_ = cb;
    }

 protected:
    _Bool isNetworkingError(const error) const;
    _Bool isUserError(const error) const;

    KopsikDisplayError on_display_error_;
    KopsikDisplayUpdate on_display_update_;
    KopsikDisplayOnlineState on_display_online_state_;
    KopsikDisplayLogin on_display_login_;
    KopsikDisplayURL on_display_url_;
    KopsikDisplayReminder on_display_reminder_;
    KopsikDisplayTimeEntryList on_display_time_entry_list_;
    KopsikDisplayAutocomplete on_display_autocomplete_;
    KopsikDisplayViewItems on_display_workspace_select_;
    KopsikDisplayViewItems on_display_client_select_;
    KopsikDisplayViewItems on_display_tags_;
    KopsikDisplayTimeEntryEditor on_display_time_entry_editor_;
    KopsikDisplaySettings on_display_settings_;
    KopsikDisplayTimerState on_display_timer_state_;

 private:
    Poco::Logger &logger() const {
        return Poco::Logger::get("ui");
    }
};

}  // namespace kopsik

#endif  // SRC_UI_H_
