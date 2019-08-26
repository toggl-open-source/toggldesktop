
// Copyright 2014 Toggl Desktop developers.

#include "views.h"
#include "gui.h"

#include <cstdlib>
#include <sstream>

#include "error.h"
#include "util.h"

#include "Poco/Logger.h"
#include "Poco/Stopwatch.h"

namespace toggl {

using namespace view;

void GUI::SetCallbacks(TogglCallbacks callbacks) {
    callbacks_ = callbacks;
}

void GUI::DisplayApp() {
    callbacks_.App(context_, true);
}

void GUI::DisplayProjectColors() {
    if (!callbacks_.ProjectColors) {
        return;
    }
    uint64_t count = Project::ColorCodes.size();
    char **list = new char *[count];
    for (uint64_t i = 0; i < count; i++) {
        list[i] = copy_string(Project::ColorCodes[i]);
    }
    callbacks_.ProjectColors(context_, list, count);
    for (uint64_t i = 0; i < count; i++) {
        free(list[i]);
    }
    delete[] list;
}

void GUI::DisplayCountries(
    std::vector<TogglCountryView> *items) {
    if (!callbacks_.Countries) {
        return;
    }
    TogglCountryView *first = country_list_init(items);
    callbacks_.Countries(context_, first);
    country_item_clear(first);
}

void GUI::DisplaySyncState(const Poco::Int64 state) {
    if (state == lastSyncState) {
        return;
    }
    if (callbacks_.SyncState) {
        callbacks_.SyncState(context_, state);
    }
    lastSyncState = state;
}

void GUI::DisplayUnsyncedItems(const Poco::Int64 count) {
    if (count == lastUnsyncedItemsCount) {
        return;
    }
    if (callbacks_.UnsyncedItems) {
        callbacks_.UnsyncedItems(context_, count);
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

    callbacks_.Login(context_, open, user_id);

    lastDisplayLoginOpen = open;
    lastDisplayLoginUserID = user_id;
}

error GUI::DisplayError(const error err) {
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

    char *err_s = copy_string(actionable);
    callbacks_.Error(context_, err_s, is_user_error);
    free(err_s);

    lastErr = err;

    return err;
}

error GUI::DisplayWSError() {
    callbacks_.Overlay(context_, 0);
    return noError;
}

error GUI::DisplayTosAccept() {
    callbacks_.Overlay(context_, 1);
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

void GUI::DisplayPromotion(const int64_t promotion_type) {
    if (callbacks_.Promotion) {
        callbacks_.Promotion(context_, promotion_type);
    }
}

void GUI::DisplayObmExperiment(const uint64_t nr, const bool included, const bool seen) {
    if (callbacks_.ObmExperiment) {
        callbacks_.ObmExperiment(context_, nr, included, seen);
    }
}

void GUI::resetFirstLaunch() {
    isFirstLaunch = true;
}

error GUI::findMissingCallbacks() {
    if (!callbacks_.App) {
        return error("!on_display_app_");
    }
    if (!callbacks_.Error) {
        return error("!on_display_error_");
    }
    if (!callbacks_.OnlineState) {
        return error("!on_display_online_state_");
    }
    if (!callbacks_.Login) {
        return error("!on_display_login_");
    }
    if (!callbacks_.URL) {
        return error("!on_display_url_");
    }
    if (!callbacks_.Reminder) {
        return error("!on_display_reminder_");
    }
    if (!callbacks_.TimeEntryList) {
        return error("!on_display_time_entry_list_");
    }
    if (!callbacks_.TimeEntryAutocomplete) {
        return error("!on_display_time_entry_autocomplete_");
    }
    if (!callbacks_.ProjectAutocomplete) {
        return error("!on_display_project_autocomplete_");
    }
    if (!callbacks_.WorkspaceSelect) {
        return error("!on_display_workspace_select_");
    }
    if (!callbacks_.ClientSelect) {
        return error("!on_display_client_select_");
    }
    if (!callbacks_.Tags) {
        return error("!on_display_tags_");
    }
    if (!callbacks_.TimeEntryEditor) {
        return error("!on_display_time_entry_editor_");
    }
    if (!callbacks_.Settings) {
        return error("!on_display_settings_");
    }
    if (!callbacks_.TimerState) {
        return error("!on_display_timer_state_");
    }
    if (!callbacks_.IdleNotification) {
        return error("!on_display_idle_notification_");
    }
    if (!callbacks_.MiniTimerAutocomplete) {
        return error("!on_display_mini_timer_autocomplete_");
    }
    if (!callbacks_.Pomodoro) {
        return error("!on_display_pomodoro_");
    }
    if (!callbacks_.PomodoroBreak) {
        return error("!on_display_pomodoro_break_");
    }
    return noError;
}

void GUI::DisplayReminder() {
    logger().debug("DisplayReminder");

    char *s1 = copy_string("Reminder from Toggl Desktop");
    char *s2 = copy_string("Don't forget to track your time!");
    callbacks_.Reminder(context_, s1, s2);
    free(s1);
    free(s2);
}

void GUI::DisplayPomodoro(const Poco::Int64 minutes) {
    logger().debug("DisplayPomodoro");
    char *s1 = copy_string("Pomodoro Timer");

    std::stringstream ss;
    ss << "You've been working for " << minutes << " minutes.";

    char *s2 = copy_string(ss.str());
    callbacks_.Pomodoro(context_, s1, s2);
    free(s1);
    free(s2);
}

void GUI::DisplayPomodoroBreak(const Poco::Int64 minutes) {
    logger().debug("DisplayPomodoroBreak");
    char *s1 = copy_string("Pomodoro Break Timer");

    std::stringstream ss;
    ss << "Hope you enjoyed your " << minutes << "-minute break.";

    char *s2 = copy_string(ss.str());
    callbacks_.PomodoroBreak(context_, s1, s2);
    free(s1);
    free(s2);
}

void GUI::DisplayAutotrackerNotification(locked<const Project> &p, locked<const Task> &t) {
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

    if (!callbacks_.AutotrackerNotification) {
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

    char *label = copy_string(Formatter::JoinTaskName(t, p));
    callbacks_.AutotrackerNotification(context_, label, pid, tid);
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

    callbacks_.OnlineState(context_, state);

    lastOnlineState = state;
}

void GUI::DisplayTimeEntryAutocomplete(
    std::vector<toggl::view::Autocomplete> *items) {
    logger().debug("DisplayTimeEntryAutocomplete");

    TogglAutocompleteView *first = autocomplete_list_init(items);
    callbacks_.TimeEntryAutocomplete(context_, first);
    autocomplete_item_clear(first);
}


void GUI::DisplayHelpArticles(locked<const std::vector<HelpArticle *>> &articles) {
    logger().debug("DisplayHelpArticles");

    if (!callbacks_.HelpArticles) {
        return;
    }

    TogglHelpArticleView *first = help_article_list_init(articles);
    callbacks_.HelpArticles(context_, first);
    help_article_clear(first);
}

void GUI::DisplayMinitimerAutocomplete(
    std::vector<toggl::view::Autocomplete> *items) {
    logger().debug("DisplayMinitimerAutocomplete");

    TogglAutocompleteView *first = autocomplete_list_init(items);
    callbacks_.MiniTimerAutocomplete(context_, first);
    autocomplete_item_clear(first);
}

void GUI::DisplayProjectAutocomplete(
    std::vector<toggl::view::Autocomplete> *items) {
    logger().debug("DisplayProjectAutocomplete");

    TogglAutocompleteView *first = autocomplete_list_init(items);
    callbacks_.ProjectAutocomplete(context_, first);
    autocomplete_item_clear(first);
}

void GUI::DisplayTimeEntryList(const bool open,
                               const std::vector<view::TimeEntry> list,
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
    TogglTimeEntryView *first = nullptr;
    for (unsigned int i = 0; i < renderList.size(); i++) {
        view::TimeEntry te = renderList.at(i);
        TogglTimeEntryView *item = time_entry_view_item_init(te);
        item->Next = first;
        if (first && compare_string(item->DateHeader, first->DateHeader) != 0) {
            first->IsHeader = true;
        }
        first = item;
    }

    if (first) {
        first->IsHeader = true;
    }

    callbacks_.TimeEntryList(context_, open, first, show_load_more_button);

    time_entry_view_item_clear(first);

    stopwatch.stop();
    {
        std::stringstream ss;
        ss << "DisplayTimeEntryList done in "
           << stopwatch.elapsed() / 1000 << " ms";
        logger().debug(ss.str());
    }
}

void GUI::DisplayTags(const std::vector<view::Generic> list) {
    logger().debug("DisplayTags");

    TogglGenericView *first = generic_to_view_item_list(list);
    callbacks_.Tags(context_, first);
    view_item_clear(first);
}

void GUI::DisplayAutotrackerRules(
    const std::vector<view::AutotrackerRule> &autotracker_rules,
    const std::vector<std::string> &titles) {

    if (!callbacks_.AutotrackerRules) {
        return;
    }

    // FIXME: dont re-render if cached items (models or view) are the same
    TogglAutotrackerRuleView *first = nullptr;
    for (std::vector<view::AutotrackerRule>::const_iterator
            it = autotracker_rules.begin();
            it != autotracker_rules.end();
            it++) {
        TogglAutotrackerRuleView *item = autotracker_rule_to_view_item(*it);
        item->Next = first;
        first = item;
    }

    uint64_t title_count = titles.size();
    char **title_list = new char *[title_count];
    for (uint64_t i = 0; i < title_count; i++) {
        title_list[i] = copy_string(titles[i]);
    }
    callbacks_.AutotrackerRules(context_, first, title_count, title_list);
    for (uint64_t i = 0; i < title_count; i++) {
        free(title_list[i]);
    }
    delete[] title_list;

    autotracker_view_item_clear(first);
}

void GUI::DisplayClientSelect(
    const std::vector<view::Generic> list) {
    logger().debug("DisplayClientSelect");

    TogglGenericView *first = generic_to_view_item_list(list);
    callbacks_.ClientSelect(context_, first);
    view_item_clear(first);
}

void GUI::DisplayWorkspaceSelect(
    const std::vector<view::Generic> list) {
    logger().debug("DisplayWorkspaceSelect");

    TogglGenericView *first = generic_to_view_item_list(list);
    callbacks_.WorkspaceSelect(context_, first);
    view_item_clear(first);
}

void GUI::DisplayTimeEntryEditor(
    const bool open,
    const view::TimeEntry te,
    const std::string focused_field_name) {

    logger().debug(
        "DisplayTimeEntryEditor focused_field_name=" + focused_field_name);

    TogglTimeEntryView *view = time_entry_view_item_init(te);

    char *field_s = copy_string(focused_field_name);
    callbacks_.TimeEntryEditor(context_, open, view, field_s);
    free(field_s);

    time_entry_view_item_clear(view);
}

void GUI::DisplayURL(const std::string URL) {
    logger().debug("DisplayURL " + URL);

    char *url = copy_string(URL);
    callbacks_.URL(context_, url);
    free(url);
}

void GUI::DisplayUpdate(const std::string URL) {
    logger().debug("DisplayUpdate " + URL);

    char *url = copy_string(URL);
    callbacks_.Update(context_, url);
    free(url);
}

void GUI::DisplayUpdateDownloadState(
    const std::string version,
    const Poco::Int64 download_state) {

    if (!callbacks_.UpdateDownloadState) {
        logger().debug("Update download state display not supported by UI");
        return;
    }
    {
        std::stringstream ss;
        ss << "DisplayUpdateDownloadState version=" << version
           << " state=" << download_state;
        logger().debug(ss.str());
    }
    char *version_string = copy_string(version);
    callbacks_.UpdateDownloadState(context_, version_string, download_state);
    free(version_string);
}

void GUI::DisplaySettings(const bool open,
                          const bool record_timeline,
                          const toggl::Settings &settings,
                          const bool use_proxy,
                          const Proxy proxy) {
    logger().debug("DisplaySettings");

    TogglSettingsView *view = settings_view_item_init(
        record_timeline,
        settings,
        use_proxy,
        proxy);

    callbacks_.Settings(context_, open, view);

    settings_view_item_clear(view);
}

void GUI::DisplayTimerState(
    const view::TimeEntry &te) {

    TogglTimeEntryView *view = time_entry_view_item_init(te);
    callbacks_.TimerState(context_, view);
    time_entry_view_item_clear(view);

    logger().debug("DisplayTimerState");
}

void GUI::DisplayEmptyTimerState() {
    callbacks_.TimerState(context_, nullptr);
    logger().debug("DisplayEmptyTimerState");
}

void GUI::DisplayIdleNotification(const std::string guid,
                                  const std::string since,
                                  const std::string duration,
                                  const int64_t started,
                                  const std::string description) {
    char *guid_s = copy_string(guid);
    char *since_s = copy_string(since);
    char *duration_s = copy_string(duration);
    char *description_s = copy_string(description);
    callbacks_.IdleNotification(context_,
                                guid_s,
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
