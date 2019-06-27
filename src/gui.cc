
// Copyright 2014 Toggl Desktop developers.

#include "../src/gui.h"

#include <cstdlib>
#include <sstream>

#include "./related_data.h"
#include "./common/error.h"
#include "./common/const.h"
#include "./common/formatter.h"
#include "./model/client.h"
#include "./model/project.h"
#include "./model/task.h"
#include "./model/time_entry.h"
#include "./model/user.h"
#include "./model/workspace.h"

#include "Poco/Logger.h"
#include "Poco/Stopwatch.h"

namespace toggl {

namespace view {

bool TimeEntry::operator == (const TimeEntry&) const {
    return false;
}

void TimeEntry::Fill(toggl::TimeEntry * const model) {
    model->EnsureGUID();
    DurationInSeconds = model->DurationInSeconds();
    Description = model->Description();
    GUID = model->GUID();
    WID = model->WID();
    TID = model->TID();
    PID = model->PID();
    Started = model->Start();
    Ended = model->Stop();
    StartTimeString =
        toggl::Formatter::FormatTimeForTimeEntryEditor(
            model->Start());
    EndTimeString =
        toggl::Formatter::FormatTimeForTimeEntryEditor(
            model->Stop());
    Billable = model->Billable();
    Tags = model->Tags();
    UpdatedAt = model->UpdatedAt();
    DateHeader =
        toggl::Formatter::FormatDateHeader(model->Start());
    DurOnly = model->DurOnly();
    Error = model->ValidationError();
    Unsynced = model->Unsynced();

    std::stringstream ss;
    ss << DateHeader << model->Description() << model->PID()
       << model->TID() << model->Billable() << model->Tags();
    GroupName = ss.str();
}

bool Autocomplete::operator == (const Autocomplete&) const {
    return false;
}

bool Generic::operator == (const Generic&) const {
    return false;
}

bool AutotrackerRule::operator == (const AutotrackerRule&) const {
    return false;
}

bool TimelineEvent::operator == (const TimelineEvent&) const {
    return false;
}

}  // namespace view

void GUI::DisplayApp() {
    on_display_app_(true);
}

void GUI::DisplayProjectColors() {
    if (!on_display_project_colors_) {
        return;
    }
    uint64_t count = Project::ColorCodes.size();
    char_t **list = new char_t *[count];
    for (uint64_t i = 0; i < count; i++) {
        list[i] = copy_string(Project::ColorCodes[i]);
    }
    on_display_project_colors_(list, count);
    for (uint64_t i = 0; i < count; i++) {
        free(list[i]);
    }
    delete[] list;
}

void GUI::DisplayCountries(
    std::vector<toggl::view::Country> &items) {
    if (!on_display_countries_) {
        return;
    }
    //on_display_countries_(items->data());
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

error GUI::DisplayError(const error &err) {
    if (noError == err) {
        return noError;
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

error GUI::DisplayWSError() {
    on_display_overlay_(0);
    return noError;
}

error GUI::DisplayTosAccept() {
    on_display_overlay_(1);
    return noError;
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
    if (!on_display_pomodoro_) {
        return error("!on_display_pomodoro_");
    }
    if (!on_display_pomodoro_break_) {
        return error("!on_display_pomodoro_break_");
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

void GUI::DisplayPomodoro(const Poco::Int64 minutes) {
    logger().debug("DisplayPomodoro");
    char_t *s1 = copy_string("Pomodoro Timer");

    std::stringstream ss;
    ss << "You've been working for " << minutes << " minutes.";

    char_t *s2 = copy_string(ss.str());
    on_display_pomodoro_(s1, s2);
    free(s1);
    free(s2);
}

void GUI::DisplayPomodoroBreak(const Poco::Int64 minutes) {
    logger().debug("DisplayPomodoroBreak");
    char_t *s1 = copy_string("Pomodoro Break Timer");

    std::stringstream ss;
    ss << "Hope you enjoyed your " << minutes << "-minute break.";

    char_t *s2 = copy_string(ss.str());
    on_display_pomodoro_break_(s1, s2);
    free(s1);
    free(s2);
}

void GUI::DisplayAutotrackerNotification(Project *const p, Task *const t) {
    poco_check_ptr(p);

    std::stringstream ss;
    ss << "DisplayAutotrackerNotification ";
    if (p) {
        ss << "project " << p->Name() << ", " << p->ID() << ", " << p->GUID();
    }
    if (t) {
        ss << " task " << t->Name() << ", " << t->ID();
    }
    logger().debug(ss.str());

    if (!p && !t) {
        logger().error(
            "Need project ID or task ID for autotracker notification");
        return;
    }

    if (!on_display_autotracker_notification_) {
        return;
    }

    uint64_t pid(0);
    if (p) {
        pid = p->ID();
    }
    uint64_t tid(0);
    if (t) {
        tid = t->ID();
    }

    char_t *label = copy_string(Formatter::JoinTaskName(t, p));
    on_display_autotracker_notification_(label, pid, tid);
    free(label);
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

    on_display_time_entry_autocomplete_(reinterpret_cast<TogglAutocompleteView*>(items->data()));
}

void GUI::DisplayHelpArticles(
    const std::vector<HelpArticle> &articles) {
    logger().debug("DisplayHelpArticles");

    if (!on_display_help_articles_) {
        return;
    }

    TogglHelpArticleView *first = help_article_list_init(articles);
    on_display_help_articles_(first);
    help_article_clear(first);
}

void GUI::DisplayMinitimerAutocomplete(
    std::vector<toggl::view::Autocomplete> *items) {
    logger().debug("DisplayMinitimerAutocomplete");

    on_display_mini_timer_autocomplete_(reinterpret_cast<TogglAutocompleteView*>(items->data()));
}

void GUI::DisplayProjectAutocomplete(
    std::vector<toggl::view::Autocomplete> *items) {
    logger().debug("DisplayProjectAutocomplete");

    on_display_project_autocomplete_(reinterpret_cast<TogglAutocompleteView*>(items->data()));
}

void GUI::DisplayTimeEntryList(const bool open,
                               const std::vector<view::TimeEntry> &list,
                               const bool show_load_more_button) {
    Poco::Stopwatch stopwatch;
    auto renderList = std::vector<view::TimeEntry>();
    stopwatch.start();
    {
        if (this->isFirstLaunch) {
            this->isFirstLaunch = false;

            // Get render list from last 9 days at the first launch
            time_t last9Days = time(nullptr) - 9 * 86400;
            for (auto it = list.begin(); it != list.end(); it++) {
                auto timeEntry = *it;
                if (timeEntry.Started >= last9Days) {
                    renderList.push_back(timeEntry);
                }
            }
        } else {
            // Otherwise, just get from the list
            renderList = list;
        }
        std::stringstream ss;
        ss << "DisplayTimeEntryList open=" << open
           << ", has items=" << renderList.size();
        logger().debug(ss.str());
    }

    // Render

    view::TimeEntry *first = nullptr;
    for (unsigned int i = 0; i < renderList.size(); i++) {
        view::TimeEntry &te = renderList.at(i);
        te.Next = first;
        if (first && compare_string(te.DateHeader, first->DateHeader) != 0) {
            first->IsHeader = true;
        }
        first = &te;
        first->IsHeader = false;
    }

    if (first) {
        first->IsHeader = true;
    }

    on_display_time_entry_list_(open, reinterpret_cast<TogglTimeEntryView*>(first), show_load_more_button);

    stopwatch.stop();
    {
        std::stringstream ss;
        ss << "DisplayTimeEntryList done in "
           << stopwatch.elapsed() / 1000 << " ms";
        logger().debug(ss.str());
    }
}

void GUI::DisplayTags(std::vector<view::Generic> &list) {
    logger().debug("DisplayTags");

    on_display_tags_(reinterpret_cast<TogglGenericView*>(list.data()));
}

void GUI::DisplayAutotrackerRules(
    const std::vector<view::AutotrackerRule> &autotracker_rules,
    const std::vector<std::string> &titles) {

    if (!on_display_autotracker_rules_) {
        return;
    }

    // FIXME: dont re-render if cached items (models or view) are the same
    /*
    TogglAutotrackerRuleView *first = nullptr;
    for (std::vector<view::AutotrackerRule>::const_iterator
            it = autotracker_rules.begin();
            it != autotracker_rules.end();
            ++it) {
        TogglAutotrackerRuleView *item = autotracker_rule_to_view_item(*it);
        //item->Next = first;
        first = item;
    }
    */

    uint64_t title_count = titles.size();
    char_t **title_list = new char_t *[title_count];
    for (uint64_t i = 0; i < title_count; i++) {
        title_list[i] = copy_string(titles[i]);
    }
    on_display_autotracker_rules_(reinterpret_cast<const TogglAutotrackerRuleView*>(autotracker_rules.data()), title_count, title_list);
    for (uint64_t i = 0; i < title_count; i++) {
        free(title_list[i]);
    }
    delete[] title_list;
}

void GUI::DisplayClientSelect(
    const std::vector<view::Generic> &list) {
    logger().debug("DisplayClientSelect");

    on_display_client_select_(reinterpret_cast<const TogglGenericView*>(list.data()));
}

void GUI::DisplayWorkspaceSelect(
    const std::vector<view::Generic> &list) {
    logger().debug("DisplayWorkspaceSelect");

    on_display_workspace_select_(reinterpret_cast<const TogglGenericView*>(list.data()));
}

void GUI::DisplayTimeEntryEditor(const bool open,
    const view::TimeEntry &te,
    const std::string &focused_field_name) {

    logger().debug(
        "DisplayTimeEntryEditor focused_field_name=" + focused_field_name);

    char_t *field_s = copy_string(focused_field_name);
    on_display_time_entry_editor_(open, reinterpret_cast<const TogglTimeEntryView*>(&te), field_s);
    free(field_s);

}

void GUI::DisplayURL(const std::string &URL) {
    logger().debug("DisplayURL " + URL);

    char_t *url = copy_string(URL);
    on_display_url_(url);
    free(url);
}

void GUI::DisplayUpdate(const std::string &URL) {
    logger().debug("DisplayUpdate " + URL);

    char_t *url = copy_string(URL);
    on_display_update_(url);
    free(url);
}

void GUI::DisplayUpdateDownloadState(
    const std::string &version,
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
                          const view::Settings *settings) {
    logger().debug("DisplaySettings");

    on_display_settings_(open, reinterpret_cast<const TogglSettingsView*>(settings));
}

void GUI::DisplayTimerState(
    const view::TimeEntry &te) {

    on_display_timer_state_(reinterpret_cast<const TogglTimelineEventView*>(&te));

    logger().debug("DisplayTimerState");
}

void GUI::DisplayEmptyTimerState() {
    on_display_timer_state_(nullptr);
    logger().debug("DisplayEmptyTimerState");
}

void GUI::DisplayIdleNotification(const std::string &guid,
                                  const std::string &since,
                                  const std::string &duration,
                                  const int64_t started,
                                  const std::string &description) {
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
