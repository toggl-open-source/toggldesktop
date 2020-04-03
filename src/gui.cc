
// Copyright 2014 Toggl Desktop developers.

#include "gui.h"

#include <cstdlib>
#include <sstream>

#include "client.h"
#include "const.h"
#include "error.h"
#include "formatter.h"
#include "project.h"
#include "related_data.h"
#include "task.h"
#include "time_entry.h"
#include "user.h"
#include "workspace.h"

#include <Poco/Stopwatch.h>

namespace toggl {

namespace view {

bool TimeEntry::operator == (const TimeEntry&) const {
    return false;
}

void TimeEntry::Fill(toggl::TimeEntry * const model) {
    model->EnsureGUID();
    ID = model->ID();
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
    GroupName = model->GroupHash();
}

void TimeEntry::GenerateRoundedTimes() {
    int quarter = 900;
    int tmp_rounded;

    tmp_rounded = ((int)(Started / quarter) * quarter);
    // gets the percentage that is used to set margin from top
    RoundedStart = (Started - tmp_rounded) / 9;

    tmp_rounded = (((int)(Ended / quarter)) * quarter) + quarter;
    // gets the percentage that is used to set margin from bottom
    RoundedEnd = (tmp_rounded - Ended) / 9;
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
    std::vector<TogglCountryView> *items) {
    if (!on_display_countries_) {
        return;
    }
    TogglCountryView *first = country_list_init(items);
    on_display_countries_(first);
    country_list_clear(first);
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
    logger.debug("DisplayLogin open=", open, ", user_id=", user_id);

    on_display_login_(open, user_id);

    lastDisplayLoginOpen = open;
    lastDisplayLoginUserID = user_id;
}

error GUI::DisplayError(const error &err) {
    if (noError == err) {
        return noError;
    }

    logger.error(err);

    if (IsNetworkingError(err)) {
        logger.debug("You are offline (", err, ")");
        if (kBackendIsDownError == err) {
            DisplayOnlineState(kOnlineStateBackendDown);
        }
        else {
            DisplayOnlineState(kOnlineStateNoNetwork);
        }
        return err;
    }

    std::string actionable = MakeErrorActionable(err);
    bool is_user_error = IsUserError(err);

    logger.debug("DisplayError err=", err, " actionable=", actionable, " is_user_error=", is_user_error);

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
    logger.debug("VerifyCallbacks");
    error err = findMissingCallbacks();
    if (err != noError) {
        logger.error(err);
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
    if (!on_display_onboarding_) {
        return error("!on_display_onboarding_");
    }
    return noError;
}

void GUI::DisplayReminder() {
    logger.debug("DisplayReminder");

    char_t *s1 = copy_string("Reminder from Toggl Desktop");
    char_t *s2 = copy_string("Don't forget to track your time!");
    on_display_reminder_(s1, s2);
    free(s1);
    free(s2);
}

void GUI::DisplayPomodoro(const Poco::Int64 minutes) {
    logger.debug("DisplayPomodoro");
    char_t *s1 = copy_string("Toggl Desktop Pomodoro Timer");

    std::stringstream ss;
    ss << "You've been working for " << minutes << " min, time to take a break!";

    char_t *s2 = copy_string(ss.str());
    on_display_pomodoro_(s1, s2);
    free(s1);
    free(s2);
}

void GUI::DisplayPomodoroBreak(const Poco::Int64 minutes) {
    logger.debug("DisplayPomodoroBreak");
    char_t *s1 = copy_string("Toggl Desktop Pomodoro Break");

    std::stringstream ss;
    ss << "Hope you enjoyed your " << minutes << " min break, time for work!";

    char_t *s2 = copy_string(ss.str());
    on_display_pomodoro_break_(s1, s2);
    free(s1);
    free(s2);
}

void GUI::DisplayAutotrackerNotification(Project *const p, Task *const t) {
    poco_check_ptr(p);

    if (p) {
        logger.debug("DisplayAutotrackerNotification project ", p->Name(), ", ", p->ID(), ", ", p->GUID());
    }
    if (t) {
        logger.debug("DisplayAutotrackerNotification task ", t->Name(), ", ", t->ID());
    }

    if (!p && !t) {
        logger.error(
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
        logger.error("Invalid online state ", state);
        return;
    }

    switch (state) {
    case kOnlineStateOnline:
        logger.debug("DisplayOnlineState online");
        break;
    case kOnlineStateNoNetwork:
        logger.debug("DisplayOnlineState no network");
        break;
    case kOnlineStateBackendDown:
        logger.debug("DisplayOnlineState backend is down");
        break;
    }

    on_display_online_state_(state);

    lastOnlineState = state;
}

void GUI::DisplayTimeEntryAutocomplete(
    std::vector<toggl::view::Autocomplete> *items) {
    logger.debug("DisplayTimeEntryAutocomplete");

    TogglAutocompleteView *first = autocomplete_list_init(items);
    on_display_time_entry_autocomplete_(first);
    autocomplete_list_clear(first);
}

void GUI::DisplayHelpArticles(
    const std::vector<HelpArticle> &articles) {
    logger.debug("DisplayHelpArticles");

    if (!on_display_help_articles_) {
        return;
    }

    TogglHelpArticleView *first = help_article_list_init(articles);
    on_display_help_articles_(first);
    help_article_list_clear(first);
}

void GUI::DisplayMinitimerAutocomplete(
    std::vector<toggl::view::Autocomplete> *items) {
    logger.debug("DisplayMinitimerAutocomplete");

    TogglAutocompleteView *first = autocomplete_list_init(items);
    on_display_mini_timer_autocomplete_(first);
    autocomplete_list_clear(first);
}

void GUI::DisplayProjectAutocomplete(
    std::vector<toggl::view::Autocomplete> *items) {
    logger.debug("DisplayProjectAutocomplete");

    TogglAutocompleteView *first = autocomplete_list_init(items);
    on_display_project_autocomplete_(first);
    autocomplete_list_clear(first);
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
        logger.debug("DisplayTimeEntryList open=", open, ", has items=", renderList.size());
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

    on_display_time_entry_list_(open, first, show_load_more_button);

    time_entry_view_list_clear(first);

    stopwatch.stop();
    logger.debug("DisplayTimeEntryList done in ", stopwatch.elapsed() / 1000, " ms");
}

void GUI::DisplayTimeline(
    const bool open,
    const std::vector<TimelineEvent> list,
    const std::vector<view::TimeEntry> &entries_list) {

    if (!on_display_timeline_) {
        return;
    }

    TogglTimelineChunkView *first_chunk = nullptr;
    Poco::LocalDateTime datetime(
        TimelineDateAt().year(),
        TimelineDateAt().month(),
        TimelineDateAt().day());
    int tzd = datetime.tzd();

    // Get all entires in this day (no chunk, no overlap)
    TogglTimeEntryView *first_entry = nullptr;
    time_t start_day = datetime.timestamp().epochTime() - tzd;
    time_t end_day = start_day + 86400; // one day
    for (unsigned int i = 0; i < entries_list.size(); i++) {
        view::TimeEntry te = entries_list.at(i);
        TogglTimeEntryView *item = time_entry_view_item_init(te);
        time_t start_time_entry = Poco::Timestamp::fromEpochTime(item->Started).epochTime();

        if (start_time_entry >= start_day && start_time_entry <= end_day) {
            item->Next = first_entry;
            first_entry = item;
        } else {
            // Release
            time_entry_view_item_clear(item);
        }
    }

    // Get activity
    while (datetime.year() == TimelineDateAt().year()
            && datetime.month() == TimelineDateAt().month()
            && datetime.day() == TimelineDateAt().day()) {
        time_t epoch_time = datetime.timestamp().epochTime() - tzd;
        time_t epoch_time_end = epoch_time + 900;

        // Create new chunk
        TogglTimelineChunkView *chunk_view =
            timeline_chunk_view_init(epoch_time);

        // Attach matching events to chunk
        TogglTimelineEventView *first_event = nullptr;
        TogglTimelineEventView *ev = nullptr;
        for (std::vector<TimelineEvent>::const_iterator it = list.begin();
                it != list.end(); it++) {
            const TimelineEvent event = *it;

            // Calculate the start time of the chunk
            // that fits this timeline event
            time_t chunk_start_time =
                (event.Start() / kTimelineChunkSeconds)
                * kTimelineChunkSeconds;

            if (epoch_time != chunk_start_time) {
                // Skip event if does not match chunk
                continue;
            }

            // Grouping the items to parent-event and sub-events

            bool app_present = false;
            bool item_present = false;
            TogglTimelineEventView *event_app = first_event;
            while (event_app) {
                if (compare_string(event_app->Filename, to_char_t(event.Filename())) == 0) {
                    timeline_event_view_update_duration(event_app, event_app->Duration + event.Duration());
                    app_present = true;
                    item_present = false;
                    ev = reinterpret_cast<TogglTimelineEventView *>(event_app->Event);
                    while (ev) {
                        if (compare_string(ev->Title, to_char_t(event.Title())) == 0) {
                            timeline_event_view_update_duration(ev, ev->Duration + event.Duration());
                            item_present = true;
                        }
                        ev = reinterpret_cast<TogglTimelineEventView *>(ev->Next);
                    }

                    if (!item_present) {
                        TogglTimelineEventView *event_view = timeline_event_view_init(event);
                        event_view->Next = event_app->Event;
                        event_app->Event = event_view;
                    }
                }
                event_app = reinterpret_cast<TogglTimelineEventView *>(event_app->Next);
            }

            if (!app_present) {
                TogglTimelineEventView *app_event_view = timeline_event_view_init(event);
                if (event.Duration() > 0) {
                    app_event_view->Header = true;
                    if (app_event_view->Title) {
                        free(app_event_view->Title);
                        app_event_view->Title = nullptr;
                    }
                    app_event_view->Title = copy_string("");

                    TogglTimelineEventView *event_view = timeline_event_view_init(event);
                    app_event_view->Event = event_view;
                    app_event_view->Next = first_event;
                    first_event = app_event_view;
                }
            }
        }

        chunk_view->Ended = epoch_time_end;

        // Update endtime
        if (chunk_view->EndTimeString) {
            free(chunk_view->EndTimeString);
            chunk_view->EndTimeString = nullptr;
        }
        chunk_view->EndTimeString = copy_string(toggl::Formatter::FormatTimeForTimeEntryEditor(chunk_view->Ended));

        // Sort the list by duration descending
        if (first_event != NULL) {
            chunk_view->FirstEvent = SortList(first_event);
        }

        chunk_view->Next = first_chunk;
        first_chunk = chunk_view;
        datetime += Poco::Timespan(15 * Poco::Timespan::MINUTES);
    }

    std::string formatted_date = Formatter::FormatDateHeader(TimelineDateAt());
    char_t *date = copy_string(formatted_date.c_str());
    on_display_timeline_(open, date, first_chunk, first_entry, start_day, end_day);
    free(date);
    time_entry_view_list_clear(first_entry);
    timeline_chunk_view_list_clear(first_chunk);
}

TogglTimelineEventView* GUI::SortList(TogglTimelineEventView *head) {
    TogglTimelineEventView *top = nullptr;  // first Node we will return this value
    TogglTimelineEventView *current = nullptr;
    bool sorted = false;
    while (sorted == false) {
        // we are going to look for the lowest value in the list
        TogglTimelineEventView *parent = head;
        TogglTimelineEventView *lowparent = head;  // we need this because list is only linked forward
        TogglTimelineEventView *low = head;  // this will end up with the lowest Node
        sorted = true;
        while (parent->Next != nullptr) {
            // Sort sub events
            if (parent->Event != nullptr) {
                parent->Event = SortList(reinterpret_cast<TogglTimelineEventView *>(parent->Event));
            }
            // find the lowest valued event
            TogglTimelineEventView *next = reinterpret_cast<TogglTimelineEventView *>(parent->Next);
            if (parent->Duration < next->Duration) {
                lowparent = parent;
                low = next;
                sorted = false;
            }
            parent = reinterpret_cast<TogglTimelineEventView *>(parent->Next);
        }
        // Sort sub events
        if (parent->Event != nullptr) {
            parent->Event = SortList(reinterpret_cast<TogglTimelineEventView *>(parent->Event));
        }
        if (current != nullptr) {  // first time current == nullptr
            current->Next = low;
        }
        // remove the lowest item from the list and reconnect the list
        // we keep two lists, one with the sorted Nodes
        // and one with the remaining unsorted Nodes
        current = low;
        if (current == head) {
            head = reinterpret_cast<TogglTimelineEventView *>(current->Next);
        }
        lowparent->Next = low->Next;
        current->Next = nullptr;
        if (top == nullptr) {
            top = current;
        }
    }
    current->Next = head;
    return top;
}

void GUI::DisplayTags(const std::vector<view::Generic> list) {
    logger.debug("DisplayTags");

    TogglGenericView *first = generic_to_view_item_list(list);
    on_display_tags_(first);
    view_list_clear(first);
}

void GUI::DisplayAutotrackerRules(
    const std::vector<view::AutotrackerRule> &autotracker_rules,
    const std::vector<std::string> &titles) {

    if (!on_display_autotracker_rules_) {
        return;
    }

    // FIXME: dont re-render if cached items (models or view) are the same
    TogglAutotrackerRuleView *first = nullptr;
    for (std::vector<view::AutotrackerRule>::const_iterator
            it = autotracker_rules.begin();
            it != autotracker_rules.end();
            ++it) {
        TogglAutotrackerRuleView *item = autotracker_rule_to_view_item(*it);
        item->Next = first;
        first = item;
    }

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

    autotracker_view_list_clear(first);
}

void GUI::DisplayClientSelect(
    const std::vector<view::Generic> &list) {
    logger.debug("DisplayClientSelect");

    TogglGenericView *first = generic_to_view_item_list(list);
    on_display_client_select_(first);
    view_list_clear(first);
}

void GUI::DisplayWorkspaceSelect(
    const std::vector<view::Generic> &list) {
    logger.debug("DisplayWorkspaceSelect");

    TogglGenericView *first = generic_to_view_item_list(list);
    on_display_workspace_select_(first);
    view_list_clear(first);
}

void GUI::DisplayTimeEntryEditor(const bool open,
                                 const view::TimeEntry &te,
                                 const std::string &focused_field_name) {

    logger.debug(
        "DisplayTimeEntryEditor focused_field_name=" + focused_field_name);

    TogglTimeEntryView *view = time_entry_view_item_init(te);

    char_t *field_s = copy_string(focused_field_name);
    on_display_time_entry_editor_(open, view, field_s);
    free(field_s);

    time_entry_view_list_clear(view);
}

void GUI::DisplayURL(const std::string &URL) {
    logger.debug("DisplayURL " + URL);

    char_t *url = copy_string(URL);
    on_display_url_(url);
    free(url);
}

void GUI::DisplayUpdate(const std::string &URL) {
    logger.debug("DisplayUpdate " + URL);

    char_t *url = copy_string(URL);
    on_display_update_(url);
    free(url);
}

void GUI::DisplayUpdateDownloadState(
    const std::string &version,
    const Poco::Int64 download_state) {

    if (!CanDisplayUpdateDownloadState()) {
        logger.debug("Update download state display not supported by UI");
        return;
    }
    logger.debug("DisplayUpdateDownloadState version=", version, " state=", download_state);
    char_t *version_string = copy_string(version);
    on_display_update_download_state_(version_string, download_state);
    free(version_string);
}

void GUI::DisplayMessage(const std::string &title,
                         const std::string &text,
                         const std::string &button,
                         const std::string &url) {
    logger.debug("DisplayMessage: " + title);

    char_t *tmp_title = copy_string(title);
    char_t *tmp_text = copy_string(text);
    char_t *tmp_button = copy_string(button);
    char_t *tmp_url = copy_string(url);
    on_display_message_(
        tmp_title,
        tmp_text,
        tmp_button,
        tmp_url);

    free(tmp_title);
    free(tmp_text);
    free(tmp_button);
    free(tmp_url);
}


void GUI::DisplaySettings(const bool open,
                          const bool record_timeline,
                          const Settings &settings,
                          const bool use_proxy,
                          const Proxy &proxy) {
    logger.debug("DisplaySettings");

    TogglSettingsView *view = settings_view_item_init(
        record_timeline,
        settings,
        use_proxy,
        proxy);

    on_display_settings_(open, view);

    settings_view_item_clear(view);
}

void GUI::DisplayTimerState(
    const view::TimeEntry &te) {

    TogglTimeEntryView *view = time_entry_view_item_init(te);
    on_display_timer_state_(view);
    time_entry_view_list_clear(view);

    logger.debug("DisplayTimerState");
}

void GUI::DisplayEmptyTimerState() {
    on_display_timer_state_(nullptr);
    logger.debug("DisplayEmptyTimerState");
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

void GUI::DisplayOnboarding(const OnboardingType onboarding_type) {
    on_display_onboarding_(onboarding_type);
}

}  // namespace toggl
