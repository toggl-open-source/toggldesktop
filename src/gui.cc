
// Copyright 2014 Toggl Desktop developers.

#include "../src/gui.h"

#include <cstdlib>
#include <sstream>

#include "./const.h"
#include "./error.h"
#include "./toggl_api_private.h"

#include "Poco/Logger.h"
#include "Poco/Stopwatch.h"

namespace toggl {

void GUI::DisplayApp() {
    on_display_app_(true);
}

void GUI::DisplaySyncState(const Poco::Int64 state) {
    if (on_display_sync_state_) {
        on_display_sync_state_(state);
    }
}

void GUI::DisplayUnsyncedItems(const Poco::Int64 count) {
    if (on_display_unsynced_items_) {
        on_display_unsynced_items_(count);
    }
}

void GUI::DisplayLogin(const _Bool open, const uint64_t user_id) {
    std::stringstream ss;
    ss << "DisplayLogin open=" << open << ", user_id=" << user_id;
    logger().debug(ss.str());

    on_display_login_(open, user_id);
}

_Bool GUI::DisplayError(const error err) {
    if (noError == err) {
        return true;
    }

    logger().error(err);

    if (IsNetworkingError(err)) {
        std::stringstream ss;
        ss << "You are offline (" << err << ")";
        if (kBackendIsDownError == err) {
            DisplayOnlineState(kOnlineStateBackendDown);
        } else {
            DisplayOnlineState(kOnlineStateNoNetwork);
        }
        return false;
    }

    std::string actionable = MakeErrorActionable(err);
    bool is_user_error = IsUserError(err);

    {
        std::stringstream ss;
        ss << "DisplayError err=" << err
           << " actionable=" << actionable
           << " is_user_error=" << is_user_error;
        logger().debug(ss.str());
    }

    char_t *err_s = copy_string(actionable);
    on_display_error_(err_s, is_user_error);
    free(err_s);

    return false;
}

error GUI::VerifyCallbacks() {
    logger().debug("VerifyCallbacks");
    error err = findMissingCallbacks();
    if (err != noError) {
        logger().error(err);
    }
    return err;
}

error GUI::findMissingCallbacks() {
    if (!on_display_app_) {
        return error("!on_display_app_");
    }
    if (!on_display_error_) {
        return error("!on_display_error_");
    }
    if (!on_display_online_state_) {
        return error("!on_display_online_state_");
    }
    if (!on_display_login_) {
        return error("!on_display_login_");
    }
    if (!on_display_url_) {
        return error("!on_display_url_");
    }
    if (!on_display_reminder_) {
        return error("!on_display_reminder_");
    }
    if (!on_display_time_entry_list_) {
        return error("!on_display_time_entry_list_");
    }
    if (!on_display_time_entry_autocomplete_) {
        return error("!on_display_time_entry_autocomplete_");
    }
    if (!on_display_project_autocomplete_) {
        return error("!on_display_project_autocomplete_");
    }
    if (!on_display_workspace_select_) {
        return error("!on_display_workspace_select_");
    }
    if (!on_display_client_select_) {
        return error("!on_display_client_select_");
    }
    if (!on_display_tags_) {
        return error("!on_display_tags_");
    }
    if (!on_display_time_entry_editor_) {
        return error("!on_display_time_entry_editor_");
    }
    if (!on_display_settings_) {
        return error("!on_display_settings_");
    }
    if (!on_display_timer_state_) {
        return error("!on_display_timer_state_");
    }
    if (!on_display_idle_notification_) {
        return error("!on_display_idle_notification_");
    }
    if (!on_display_mini_timer_autocomplete_) {
        return error("!on_display_mini_timer_autocomplete_");
    }
    return noError;
}

void GUI::DisplayReminder() {
    logger().debug("DisplayReminder");

    char_t *s1 = copy_string("Reminder from Toggl Desktop");
    char_t *s2 = copy_string("Don't forget to track your time!");
    on_display_reminder_(s1, s2);
    free(s1);
    free(s2);
}

void GUI::DisplayOnlineState(const Poco::Int64 state) {
    if (!(kOnlineStateOnline == state
            || kOnlineStateNoNetwork == state
            || kOnlineStateBackendDown == state)) {
        std::stringstream ss;
        ss << "Invalid online state " << state;
        logger().error(ss.str());
        return;
    }

    std::stringstream ss;
    ss << "DisplayOnlineState ";

    switch (state) {
    case kOnlineStateOnline:
        ss << "online";
        break;
    case kOnlineStateNoNetwork:
        ss << "no network";
        break;
    case kOnlineStateBackendDown:
        ss << "backend is down";
        break;
    }
    logger().debug(ss.str());

    on_display_online_state_(state);
}

void GUI::DisplayTimeEntryAutocomplete(
    std::vector<toggl::AutocompleteItem> *items) {
    logger().debug("DisplayTimeEntryAutocomplete");

    TogglAutocompleteView *first = autocomplete_list_init(items);
    on_display_time_entry_autocomplete_(first);
    autocomplete_item_clear(first);
}

void GUI::DisplayMinitimerAutocomplete(
    std::vector<toggl::AutocompleteItem> *items) {
    logger().debug("DisplayMinitimerAutocomplete");

    TogglAutocompleteView *first = autocomplete_list_init(items);
    on_display_mini_timer_autocomplete_(first);
    autocomplete_item_clear(first);
}

void GUI::DisplayProjectAutocomplete(
    std::vector<toggl::AutocompleteItem> *items) {
    logger().debug("DisplayProjectAutocomplete");

    TogglAutocompleteView *first = autocomplete_list_init(items);
    on_display_project_autocomplete_(first);
    autocomplete_item_clear(first);
}

void GUI::DisplayTimeEntryList(const _Bool open,
                               TogglTimeEntryView* first) {
    Poco::Stopwatch stopwatch;
    stopwatch.start();
    {
        std::stringstream ss;
        bool has_items = first ? true : false;
        ss << "DisplayTimeEntryList open=" << open
           << ", has items=" << has_items;
        logger().debug(ss.str());
    }
    on_display_time_entry_list_(open, first);
    stopwatch.stop();
    {
        std::stringstream ss;
        ss << "DisplayTimeEntryList done in "
           << stopwatch.elapsed() / 1000 << " ms";
        logger().debug(ss.str());
    }
}

void GUI::DisplayTags(std::vector<std::string> *tags) {
    logger().debug("DisplayTags");

    TogglGenericView *first = 0;
    for (std::vector<std::string>::const_iterator it = tags->begin();
            it != tags->end(); it++) {
        std::string name = *it;
        TogglGenericView *item = tag_to_view_item(name);
        item->Next = first;
        first = item;
    }
    on_display_tags_(first);
    view_item_clear(first);
}

void GUI::DisplayClientSelect(std::vector<toggl::Client *> *clients) {
    logger().debug("DisplayClientSelect");

    TogglGenericView *first = 0;
    for (std::vector<toggl::Client *>::const_iterator it = clients->begin();
            it != clients->end(); it++) {
        TogglGenericView *item = client_to_view_item(*it);
        item->Next = first;
        first = item;
    }
    on_display_client_select_(first);
    view_item_clear(first);
}

void GUI::DisplayWorkspaceSelect(std::vector<toggl::Workspace *> *list) {
    logger().debug("DisplayWorkspaceSelect");

    TogglGenericView *first = 0;
    for (std::vector<toggl::Workspace *>::const_iterator it =
        list->begin(); it != list->end(); it++) {
        TogglGenericView *item = workspace_to_view_item(*it);
        item->Next = first;
        first = item;
    }
    on_display_workspace_select_(first);
    view_item_clear(first);
}

void GUI::DisplayTimeEntryEditor(const _Bool open,
                                 TogglTimeEntryView *te,
                                 const std::string focused_field_name) {
    logger().debug("DisplayTimeEntryEditor");
    char_t *field_s = copy_string(focused_field_name);
    on_display_time_entry_editor_(open, te, field_s);
    free(field_s);
}

void GUI::DisplayURL(const std::string URL) {
    logger().debug("DisplayURL " + URL);

    char_t *url = copy_string(URL);
    on_display_url_(url);
    free(url);
}

void GUI::DisplayUpdate(const std::string URL) {
    logger().debug("DisplayUpdate " + URL);

    char_t *url = copy_string(URL);
    on_display_update_(url);
    free(url);
}

void GUI::DisplaySettings(const _Bool open,
                          const _Bool record_timeline,
                          const Settings settings,
                          const _Bool use_proxy,
                          const Proxy proxy) {
    logger().debug("DisplaySettings");

    TogglSettingsView *view = settings_view_item_init(
        record_timeline,
        settings,
        use_proxy,
        proxy);

    on_display_settings_(open, view);

    settings_view_item_clear(view);
}

void GUI::DisplayTimerState(TogglTimeEntryView *te) {
    logger().debug("DisplayTimerState");
    on_display_timer_state_(te);
}

void GUI::DisplayIdleNotification(const std::string guid,
                                  const std::string since,
                                  const std::string duration,
                                  const uint64_t started,
                                  const std::string description) {
    char_t *guid_s = copy_string(guid);
    char_t *since_s = copy_string(since);
    char_t *duration_s = copy_string(duration);
    char_t *description_s = copy_string(description);
    on_display_idle_notification_(guid_s,
                                  since_s,
                                  duration_s,
                                  started,
                                  description_s);
    free(guid_s);
    free(since_s);
    free(duration_s);
    free(description_s);
}

Poco::Logger &GUI::logger() const {
    return Poco::Logger::get("ui");
}

}  // namespace toggl
