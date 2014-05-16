
// Copyright 2014 Toggl Desktop developers.

#include "./ui.h"

#include <cstdlib>

namespace kopsik {

void UI::DisplayLogin(const _Bool open, const uint64_t user_id) {
    logger().debug("DisplayLogin");

    on_display_login_(open, user_id);
}

_Bool UI::DisplayError(const error err) {
    if (noError == err) {
        return true;
    }

    if (isNetworkingError(err)) {
        DisplayOnlineState(false);
        return false;
    }

    logger().debug("DisplayError");

    if (err.find("Request to server failed with status code: 403")
            != std::string::npos) {
        on_display_error_("Invalid e-mail or password!", true);
        return false;
    }

    on_display_error_(err.c_str(), isUserError(err));
    return false;
}

error UI::VerifyCallbacks() {
    logger().debug("VerifyCallbacks");
    error err = findMissingCallbacks();
    if (err != noError) {
        logger().error(err);
    }
    return err;
}

error UI::findMissingCallbacks() {
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
    if (!on_display_autocomplete_) {
        return error("!on_display_autocomplete_");
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
    return noError;
}

void UI::DisplayReminder() {
    logger().debug("DisplayReminder");

    on_display_reminder_("Reminder from Toggl Desktop",
                         "Don't forget to track your time!");
}

void UI::DisplayOnlineState(const _Bool is_online) {
    logger().debug("DisplayOnlineState");

    on_display_online_state_(is_online);
}

void UI::DisplayUpdate(const bool is_available,
                       const std::string url,
                       const std::string version) {
    logger().debug("DisplayUpdate");

    on_display_update_(is_available, url.c_str(), version.c_str());
}

void UI::DisplayAutocomplete(std::vector<kopsik::AutocompleteItem> *items) {
    logger().debug("DisplayAutocomplete");

    KopsikAutocompleteItem *first = 0;
    for (std::vector<kopsik::AutocompleteItem>::const_iterator it =
        items->begin(); it != items->end(); it++) {
        KopsikAutocompleteItem *item = autocomplete_item_init(*it);
        item->Next = first;
        first = item;
    }
    on_display_autocomplete_(first);
    autocomplete_item_clear(first);
}

void UI::DisplayTimeEntryList(const _Bool open,
                              KopsikTimeEntryViewItem* first) {
    logger().debug("DisplayTimeEntryList");
    on_display_time_entry_list_(open, first);
}

void UI::DisplayTags(std::vector<std::string> *tags) {
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

void UI::DisplayClientSelect(std::vector<kopsik::Client *> *clients) {
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

void UI::DisplayWorkspaceSelect(std::vector<kopsik::Workspace *> *list) {
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

void UI::DisplayTimeEntryEditor(const _Bool open,
                                KopsikTimeEntryViewItem *te,
                                const std::string focused_field_name) {
    logger().debug("DisplayTimeEntryEditor");
    on_display_time_entry_editor_(open, te, focused_field_name.c_str());
}

void UI::DisplayURL(const std::string URL) {
    logger().debug("DisplayURL");

    on_display_url_(URL.c_str());
}

void UI::DisplaySettings(const _Bool open,
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

void UI::DisplayTimerState(KopsikTimeEntryViewItem *te) {
    logger().debug("DisplayTimerState");
    on_display_timer_state_(te);
}

_Bool UI::isNetworkingError(const error err) const {
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

_Bool UI::isUserError(const error err) const {
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
    return false;
}

}  // namespace kopsik
