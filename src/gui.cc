
// Copyright 2014 Toggl Desktop developers.

#include "../src/gui.h"

#include <cstdlib>
#include <sstream>

#include "./client.h"
#include "./const.h"
#include "./error.h"
#include "./formatter.h"
#include "./project.h"
#include "./related_data.h"
#include "./time_entry.h"
#include "./user.h"
#include "./workspace.h"

#include "Poco/Logger.h"
#include "Poco/Stopwatch.h"

namespace toggl {

namespace view {

bool TimeEntry::operator == (const TimeEntry& a) const {
    return false;
}

bool Autocomplete::operator == (const Autocomplete& a) const {
    return false;
}

bool Generic::operator == (const Generic& a) const {
    return false;
}

bool AutotrackerRule::operator == (const AutotrackerRule& a) const {
    return false;
}

bool TimelineEvent::operator == (const TimelineEvent& a) const {
    return false;
}

}  // namespace view

void GUI::DisplayApp() {
    on_display_app_(true);
}

void GUI::DisplaySyncState(const Poco::Int64 state) {
    if (state == lastSyncState) {
        return;
    }
    if (on_display_sync_state_) {
        on_display_sync_state_(state);
    }
    lastSyncState = state;
}

void GUI::DisplayUnsyncedItems(const Poco::Int64 count) {
    if (count == lastUnsyncedItemsCount) {
        return;
    }
    if (on_display_unsynced_items_) {
        on_display_unsynced_items_(count);
    }
    lastUnsyncedItemsCount = count;
}

void GUI::DisplayLogin(const bool open, const uint64_t user_id) {
    if (open == lastDisplayLoginOpen && user_id == lastDisplayLoginUserID) {
        return;
    }
    std::stringstream ss;
    ss << "DisplayLogin open=" << open << ", user_id=" << user_id;
    logger().debug(ss.str());

    on_display_login_(open, user_id);

    lastDisplayLoginOpen = open;
    lastDisplayLoginUserID = user_id;
}

error GUI::DisplayError(const error err) {
    if (noError == err) {
        return noError;
    }

    if (err == lastErr) {
        return err;
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
        return err;
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

    lastErr = err;

    return err;
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

void GUI::DisplayAutotrackerNotification(Project *p) {
    poco_check_ptr(p);

    std::stringstream ss;
    ss << "DisplayAutotrackerNotification "
       << p->Name() << ", " << p->ID() << ", " << p->GUID();

    logger().debug(ss.str());

    if (!on_display_autotracker_notification_) {
        return;
    }

    char_t *project_name_s = copy_string(p->Name());

    on_display_autotracker_notification_(project_name_s, p->ID());

    free(project_name_s);
}


void GUI::DisplayOnlineState(const Poco::Int64 state) {
    if (state == lastOnlineState) {
        return;
    }

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

    lastOnlineState = state;
}

void GUI::DisplayTimeEntryAutocomplete(
    std::vector<toggl::view::Autocomplete> *items) {
    logger().debug("DisplayTimeEntryAutocomplete");

    TogglAutocompleteView *first = autocomplete_list_init(items);
    on_display_time_entry_autocomplete_(first);
    autocomplete_item_clear(first);
}

void GUI::DisplayMinitimerAutocomplete(
    std::vector<toggl::view::Autocomplete> *items) {
    logger().debug("DisplayMinitimerAutocomplete");

    TogglAutocompleteView *first = autocomplete_list_init(items);
    on_display_mini_timer_autocomplete_(first);
    autocomplete_item_clear(first);
}

void GUI::DisplayProjectAutocomplete(
    std::vector<toggl::view::Autocomplete> *items) {
    logger().debug("DisplayProjectAutocomplete");

    TogglAutocompleteView *first = autocomplete_list_init(items);
    on_display_project_autocomplete_(first);
    autocomplete_item_clear(first);
}

void GUI::DisplayTimeEntryList(const bool open,
                               const RelatedData &related,
                               const std::vector<TimeEntry *> list) {
    time_entry_editor_guid_ = "";

    Poco::Stopwatch stopwatch;
    stopwatch.start();
    {
        std::stringstream ss;
        ss << "DisplayTimeEntryList open=" << open
           << ", has items=" << list.size();
        logger().debug(ss.str());
    }

    // Calculate total duration for each date: will be displayed in date header
    std::map<std::string, Poco::Int64> date_durations;
    for (unsigned int i = 0; i < list.size(); i++) {
        TimeEntry *te = list.at(i);
        std::string date_header = Formatter::FormatDateHeader(te->Start());
        Poco::Int64 duration = date_durations[date_header];
        duration += Formatter::AbsDuration(te->Duration());
        date_durations[date_header] = duration;
    }

    // Render
    TogglTimeEntryView *first = nullptr;
    for (unsigned int i = 0; i < list.size(); i++) {
        TimeEntry *te = list.at(i);

        if (te->Duration() < 0) {
            // Don't display running entries
            continue;
        }

        Poco::Int64 duration =
            date_durations[Formatter::FormatDateHeader(te->Start())];
        std::string date_duration =
            Formatter::FormatDurationForDateHeader(duration);

        TogglTimeEntryView *item = nullptr;

        std::string workspace_name("");
        std::string project_and_task_label("");
        std::string task_label("");
        std::string project_label("");
        std::string client_label("");
        std::string color("");
        related.ProjectLabelAndColorCode(te,
                                         &workspace_name,
                                         &project_and_task_label,
                                         &task_label,
                                         &project_label,
                                         &client_label,
                                         &color);

        item = time_entry_view_item_init(te,
                                         workspace_name,
                                         project_and_task_label,
                                         task_label,
                                         project_label,
                                         client_label,
                                         color,
                                         date_duration,
                                         false);

        item->Next = first;

        if (first && compare_string(item->DateHeader, first->DateHeader) != 0) {
            first->IsHeader = true;
        }
        first = item;
    }

    if (first) {
        first->IsHeader = true;
    }

    on_display_time_entry_list_(open, first);

    time_entry_view_item_clear(first);

    stopwatch.stop();
    {
        std::stringstream ss;
        ss << "DisplayTimeEntryList done in "
           << stopwatch.elapsed() / 1000 << " ms";
        logger().debug(ss.str());
    }
}

void GUI::DisplayTimeline(
    const bool open,
    const std::vector<TimelineEvent> list) {

    if (!on_display_timeline_) {
        return;
    }

    std::string formatted_date = Formatter::FormatDateHeader(TimelineDateAt());

    TogglTimelineView *first = nullptr;
    for (std::vector<TimelineEvent>::const_iterator it = list.begin();
            it != list.end(); it++) {
        TogglTimelineView *view = timeline_view_init(*it);
        view->Next = first;
        first = view;
    }
    on_display_timeline_(open, formatted_date.c_str(), first);
    timeline_view_clear(first);
}

void GUI::DisplayTags(std::vector<std::string> *tags) {
    logger().debug("DisplayTags");

    TogglGenericView *first = nullptr;
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

void GUI::DisplayAutotrackerRules(
    const RelatedData &related,
    const std::set<std::string> &autotracker_titles) {

    if (!on_display_autotracker_rules_) {
        return;
    }

    // FIXME: dont re-render if cached items (models or view) are the same
    TogglAutotrackerRuleView *first = nullptr;
    for (std::vector<toggl::AutotrackerRule *>::const_iterator it =
        related.AutotrackerRules.begin();
            it != related.AutotrackerRules.end();
            it++) {
        AutotrackerRule *rule = *it;
        Project *p = related.ProjectByID(rule->PID());
        std::string project_name("");
        if (p) {
            project_name = p->Name();
        }
        TogglAutotrackerRuleView *item =
            autotracker_rule_to_view_item(*it, project_name);
        item->Next = first;
        first = item;
    }

    std::vector<std::string> titles;
    for (std::set<std::string>::const_iterator
            it = autotracker_titles.begin();
            it != autotracker_titles.end();
            ++it) {
        titles.push_back(*it);
    }
    std::sort(titles.begin(), titles.end(), CompareAutotrackerTitles);

    uint64_t title_count = titles.size();
    char_t **title_list = new char_t *[title_count];
    for (uint64_t i = 0; i < title_count; i++) {
        title_list[i] = copy_string(titles[i]);
    }
    on_display_autotracker_rules_(first, title_count, title_list);
    for (uint64_t i = 0; i < title_count; i++) {
        free(title_list[i]);
    }
    delete[] title_list;

    autotracker_view_item_clear(first);
}

void GUI::DisplayClientSelect(
    const RelatedData &related,
    std::vector<toggl::Client *> *clients) {
    logger().debug("DisplayClientSelect");

    TogglGenericView *first = nullptr;
    for (std::vector<toggl::Client *>::const_iterator it = clients->begin();
            it != clients->end(); it++) {
        Client *c = *it;
        Workspace *ws = nullptr;
        if (c->WID()) {
            ws = related.WorkspaceByID(c->WID());
        }
        TogglGenericView *item = client_to_view_item(c, ws);
        item->Next = first;
        first = item;
    }
    on_display_client_select_(first);
    view_item_clear(first);
}

void GUI::DisplayWorkspaceSelect(std::vector<toggl::Workspace *> *list) {
    logger().debug("DisplayWorkspaceSelect");

    TogglGenericView *first = nullptr;
    for (std::vector<toggl::Workspace *>::const_iterator it =
        list->begin(); it != list->end(); it++) {
        TogglGenericView *item = workspace_to_view_item(*it);
        item->Next = first;
        first = item;
    }
    on_display_workspace_select_(first);
    view_item_clear(first);
}

void GUI::DisplayTimeEntryEditor(
    const bool open,
    const RelatedData &related,
    const TimeEntry *te,
    const std::string focused_field_name,
    const Poco::Int64 total_duration_for_date,
    const User *user) {

    time_entry_editor_guid_ = te->GUID();

    logger().debug(
        "DisplayTimeEntryEditor focused_field_name=" + focused_field_name);

    TogglTimeEntryView *view =
        timeEntryViewItem(related, te, total_duration_for_date);

    Workspace *ws = nullptr;
    if (te->WID()) {
        ws = related.WorkspaceByID(te->WID());
    }
    view->CanSeeBillable = user->CanSeeBillable(ws);
    view->DefaultWID = user->DefaultWID();
    if (ws) {
        view->CanAddProjects = ws->Admin() ||
                               !ws->OnlyAdminsMayCreateProjects();
    } else {
        view->CanAddProjects = user->CanAddProjects();
    }

    char_t *field_s = copy_string(focused_field_name);
    on_display_time_entry_editor_(open, view, field_s);
    free(field_s);

    time_entry_view_item_clear(view);
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

void GUI::DisplayUpdateDownloadState(
    const std::string version,
    const Poco::Int64 download_state) {

    if (!CanDisplayUpdateDownloadState()) {
        logger().debug("Update download state display not supported by UI");
        return;
    }
    {
        std::stringstream ss;
        ss << "DisplayUpdateDownloadState version=" << version
           << " state=" << download_state;
        logger().debug(ss.str());
    }
    char_t *version_string = copy_string(version);
    on_display_update_download_state_(version_string, download_state);
    free(version_string);
}

void GUI::DisplaySettings(const bool open,
                          const bool record_timeline,
                          const Settings settings,
                          const bool use_proxy,
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

void GUI::DisplayTimerState(
    const RelatedData &related,
    const TimeEntry *te,
    const Poco::Int64 total_duration_for_date) {

    TogglTimeEntryView *view =
        timeEntryViewItem(related, te, total_duration_for_date);
    on_display_timer_state_(view);
    time_entry_view_item_clear(view);

    logger().debug("DisplayTimerState");
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

TogglTimeEntryView *timeEntryViewItem(
    const RelatedData &related,
    const TimeEntry *te,
    const Poco::Int64 total_duration_for_date) {

    if (!te) {
        return nullptr;
    }

    std::string workspace_name("");
    std::string project_and_task_label("");
    std::string task_label("");
    std::string project_label("");
    std::string client_label("");
    std::string color("");
    related.ProjectLabelAndColorCode(te,
                                     &workspace_name,
                                     &project_and_task_label,
                                     &task_label,
                                     &project_label,
                                     &client_label,
                                     &color);

    std::string date_duration =
        Formatter::FormatDurationForDateHeader(
            total_duration_for_date);

    return time_entry_view_item_init(te,
                                     workspace_name,
                                     project_and_task_label,
                                     task_label,
                                     project_label,
                                     client_label,
                                     color,
                                     date_duration,
                                     true);
}

}  // namespace toggl
