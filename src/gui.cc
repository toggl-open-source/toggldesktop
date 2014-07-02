
// Copyright 2014 Toggl Desktop developers.

#include "./gui.h"

#include <cstdlib>
#include <sstream>

namespace kopsik {

void GUI::DisplayApp() {
    on_display_app_(true);
}

void GUI::DisplayLogin(const _Bool open, const uint64_t user_id) {
    logger().debug("DisplayLogin");

    on_display_login_(open, user_id);
}

_Bool GUI::DisplayError(const error err) {
    if (noError == err) {
        return true;
    }

    logger().error(err);

    if (isNetworkingError(err)) {
        DisplayOnlineState(false, err);
        return false;
    }

    logger().debug("DisplayError");

    on_display_error_(err.c_str(), isUserError(err));
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
    if (!on_display_update_) {
        return error("!on_display_update_");
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
    return noError;
}

void GUI::DisplayReminder() {
    logger().debug("DisplayReminder");

    on_display_reminder_("Reminder from Toggl Desktop",
                         "Don't forget to track your time!");
}

void GUI::DisplayOnlineState(const _Bool is_online, const std::string reason) {
    logger().debug("DisplayOnlineState");

    on_display_online_state_(is_online, reason.c_str());
}

void GUI::DisplayUpdate(const _Bool open,
                        const std::string update_channel,
                        const _Bool is_checking,
                        const _Bool is_available,
                        const std::string url,
                        const std::string version) {
    logger().debug("DisplayUpdate");

    KopsikUpdateViewItem view;
    view.UpdateChannel = strdup(update_channel.c_str());
    view.IsChecking = is_checking;
    view.IsUpdateAvailable = is_available;
    view.URL = strdup(url.c_str());
    view.Version = strdup(version.c_str());

    on_display_update_(open, &view);

    free(view.UpdateChannel);
    free(view.URL);
    free(view.Version);
}

void GUI::DisplayTimeEntryAutocomplete(
    std::vector<kopsik::AutocompleteItem> *items) {
    logger().debug("DisplayTimeEntryAutocomplete");

    KopsikAutocompleteItem *first = 0;
    for (std::vector<kopsik::AutocompleteItem>::const_reverse_iterator it =
        items->rbegin(); it != items->rend(); it++) {
        KopsikAutocompleteItem *item = autocomplete_item_init(*it);
        item->Next = first;
        first = item;
    }
    on_display_time_entry_autocomplete_(first);
    autocomplete_item_clear(first);
}

void GUI::DisplayProjectAutocomplete(
    std::vector<kopsik::AutocompleteItem> *items) {
    logger().debug("DisplayProjectAutocomplete");

    KopsikAutocompleteItem *first = 0;
    for (std::vector<kopsik::AutocompleteItem>::const_reverse_iterator it =
        items->rbegin(); it != items->rend(); it++) {
        KopsikAutocompleteItem *item = autocomplete_item_init(*it);
        item->Next = first;
        first = item;
    }
    on_display_project_autocomplete_(first);
    autocomplete_item_clear(first);
}

void GUI::DisplayTimeEntryList(const _Bool open,
                               KopsikTimeEntryViewItem* first) {
    logger().debug("DisplayTimeEntryList");
    on_display_time_entry_list_(open, first);
}

void GUI::DisplayTags(std::vector<std::string> *tags) {
    logger().debug("DisplayTags");

    KopsikViewItem *first = 0;
    for (std::vector<std::string>::const_iterator it = tags->begin();
            it != tags->end(); it++) {
        std::string name = *it;
        KopsikViewItem *item = tag_to_view_item(name);
        item->Next = first;
        first = item;
    }
    on_display_tags_(first);
    view_item_clear(first);
}

void GUI::DisplayClientSelect(std::vector<kopsik::Client *> *clients) {
    logger().debug("DisplayClientSelect");

    KopsikViewItem *first = 0;
    for (std::vector<kopsik::Client *>::const_iterator it = clients->begin();
            it != clients->end(); it++) {
        KopsikViewItem *item = client_to_view_item(*it);
        item->Next = first;
        first = item;
    }
    on_display_client_select_(first);
    view_item_clear(first);
}

void GUI::DisplayWorkspaceSelect(std::vector<kopsik::Workspace *> *list) {
    logger().debug("DisplayWorkspaceSelect");

    KopsikViewItem *first = 0;
    for (std::vector<kopsik::Workspace *>::const_iterator it =
        list->begin(); it != list->end(); it++) {
        KopsikViewItem *item = workspace_to_view_item(*it);
        item->Next = first;
        first = item;
    }
    on_display_workspace_select_(first);
    view_item_clear(first);
}

void GUI::DisplayTimeEntryEditor(const _Bool open,
                                 KopsikTimeEntryViewItem *te,
                                 const std::string focused_field_name) {
    logger().debug("DisplayTimeEntryEditor");
    on_display_time_entry_editor_(open, te, focused_field_name.c_str());
}

void GUI::DisplayURL(const std::string URL) {
    logger().debug("DisplayURL");

    on_display_url_(URL.c_str());
}

void GUI::DisplaySettings(const _Bool open,
                          const _Bool record_timeline,
                          const Settings settings,
                          const _Bool use_proxy,
                          const Proxy proxy) {
    logger().debug("DisplaySettings");

    KopsikSettingsViewItem *view = settings_view_item_init(
        record_timeline,
        settings,
        use_proxy,
        proxy);

    on_display_settings_(open, view);

    settings_view_item_clear(view);
}

void GUI::DisplayTimerState(KopsikTimeEntryViewItem *te) {
    logger().debug("DisplayTimerState");
    on_display_timer_state_(te);
}

void GUI::DisplayIdleNotification(const std::string guid,
                                  const std::string since,
                                  const std::string duration,
                                  const uint64_t started) {
    on_display_idle_notification_(guid.c_str(),
                                  since.c_str(),
                                  duration.c_str(),
                                  started);
}

_Bool GUI::isNetworkingError(const error err) const {
    std::string value(err);
    if (value.find("Host not found") != std::string::npos) {
        return true;
    }
    if (value.find("Cannot upgrade to WebSocket connection")
            != std::string::npos) { // NOLINT
        return true;
    }
    if (value.find("No message received") != std::string::npos) {
        return true;
    }
    if (value.find("Connection refused") != std::string::npos) {
        return true;
    }
    if (value.find("Connection timed out") != std::string::npos) {
        return true;
    }
    if (value.find("connect timed out") != std::string::npos) {
        return true;
    }
    if (value.find("SSL connection unexpectedly closed") != std::string::npos) {
        return true;
    }
    if (value.find("Network is down") != std::string::npos) {
        return true;
    }
    if (value.find("Network is unreachable") != std::string::npos) {
        return true;
    }
    if (value.find("Host is down") != std::string::npos) {
        return true;
    }
    if (value.find("No route to host") != std::string::npos) {
        return true;
    }
    if ((value.find("I/O error: 1") != std::string::npos)
            && (value.find(":443") != std::string::npos)) {
        return true;
    }
    if (value.find("The request timed out") != std::string::npos) {
        return true;
    }
    if (value.find("Could not connect to the server") != std::string::npos) {
        return true;
    }
    if (value.find("Connection reset by peer") != std::string::npos) {
        return true;
    }
    if (value.find("The Internet connection appears to be offline")
            != std::string::npos) {
        return true;
    }
    return false;
}

_Bool GUI::isUserError(const error err) const {
    if (noError == err) {
        return false;
    }
    std::string value(err);
    if (value.find("is suspended") != std::string::npos) {
        return true;
    }
    if (value.find("Request to server failed with status code: 403")
            != std::string::npos) {
        return true;
    }
    if (value.find("Stop time must be after start time")
            != std::string::npos) {
        return true;
    }
    if (value.find("Invalid e-mail or password") != std::string::npos) {
        return true;
    }
    return false;
}

}  // namespace kopsik
