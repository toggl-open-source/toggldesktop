// Copyright 2014 Toggl Desktop developers.

// No exceptions should be thrown from this library.

#include <cstring>
#include <set>

#include "./kopsik_api.h"
#include "./kopsik_api_private.h"
#include "./database.h"
#include "./user.h"
#include "./https_client.h"
#include "./websocket_client.h"
#include "./version.h"
#include "./timeline_uploader.h"
#include "./window_change_recorder.h"
#include "./CustomErrorHandler.h"
#include "./proxy.h"
#include "./context.h"
#include "./formatter.h"
#include "./feedback.h"

#include "Poco/Bugcheck.h"
#include "Poco/Path.h"
#include "Poco/Logger.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"

inline Poco::Logger &logger() {
    return Poco::Logger::get("kopsik_api");
}

inline Poco::Logger &rootLogger() {
    return Poco::Logger::get("");
}

inline kopsik::Context *app(void *context) {
    poco_assert(context);
    return reinterpret_cast<kopsik::Context *>(context);
}

// FIXME: move into Context
_Bool kopsik_is_networking_error(
    const char *error) {
    std::string value(error);
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
    return false;
}

// FIXME: delete
void kopsik_view_item_clear(
    KopsikViewItem *item) {
    if (!item) {
        return;
    }
    if (item->Name) {
        free(item->Name);
        item->Name = 0;
    }
    if (item->GUID) {
        free(item->GUID);
        item->GUID = 0;
    }
    if (item->Next) {
        KopsikViewItem *next = reinterpret_cast<KopsikViewItem *>(item->Next);
        kopsik_view_item_clear(next);
    }
    delete item;
    item = 0;
}

void *kopsik_context_init(
    const char *app_name,
    const char *app_version) {
    poco_assert(app_name);
    poco_assert(app_version);

    kopsik::Context *ctx =
        new kopsik::Context(std::string(app_name), std::string(app_version));

    ctx->SetAPIURL(kAPIURL);
    ctx->SetTimelineUploadURL(kTimelineUploadURL);
    ctx->SetWebSocketClientURL(kWebSocketURL);

    return ctx;
}

void kopsik_context_start_events(void *context) {
    app(context)->StartEvents();
}

void kopsik_password_forgot(void *context) {
    app(context)->PasswordForgot();
}

void kopsik_context_clear(void *context) {
    if (context) {
        app(context)->Shutdown();
    }
    delete app(context);
}

_Bool kopsik_get_settings(
    void *context,
    _Bool *out_use_idle_detection,
    _Bool *out_menubar_timer,
    _Bool *out_dock_icon,
    _Bool *out_on_top,
    _Bool *out_reminder) {

    poco_assert(out_use_idle_detection);
    poco_assert(out_menubar_timer);
    poco_assert(out_dock_icon);
    poco_assert(out_on_top);

    bool use_idle_detection(false);
    bool menubar_timer(false);
    bool dock_icon(false);
    bool on_top(false);
    bool reminder(false);

    if (!app(context)->LoadSettings(&use_idle_detection,
                                    &menubar_timer,
                                    &dock_icon,
                                    &on_top,
                                    &reminder)) {
        return false;
    }

    *out_use_idle_detection = false;
    if (use_idle_detection) {
        *out_use_idle_detection = true;
    }

    *out_menubar_timer = false;
    if (menubar_timer) {
        *out_menubar_timer = true;
    }

    *out_dock_icon = false;
    if (dock_icon) {
        *out_dock_icon = true;
    }

    *out_on_top = false;
    if (on_top) {
        *out_on_top = true;
    }

    *out_reminder = false;
    if (reminder) {
        *out_reminder = true;
    }
    return true;
}

_Bool kopsik_get_proxy_settings(
    void *context,
    _Bool *out_use_proxy,
    char **out_proxy_host,
    uint64_t *out_proxy_port,
    char **out_proxy_username,
    char **out_proxy_password) {

    poco_assert(out_use_proxy);
    poco_assert(out_proxy_host);
    poco_assert(out_proxy_port);
    poco_assert(out_proxy_username);
    poco_assert(out_proxy_password);

    bool use_proxy(false);
    kopsik::Proxy proxy;
    if (!app(context)->LoadProxySettings(
        &use_proxy,
        &proxy)) {
        return false;
    }

    *out_use_proxy = false;
    if (use_proxy) {
        *out_use_proxy = true;
    }

    *out_proxy_host = strdup(proxy.host.c_str());
    *out_proxy_port = proxy.port;
    *out_proxy_username = strdup(proxy.username.c_str());
    *out_proxy_password = strdup(proxy.password.c_str());

    return true;
}

_Bool kopsik_set_settings(
    void *context,
    const _Bool use_idle_detection,
    const _Bool menubar_timer,
    const _Bool dock_icon,
    const _Bool on_top,
    const _Bool reminder) {

    return app(context)->SaveSettings(
        use_idle_detection,
        menubar_timer,
        dock_icon,
        on_top,
        reminder);
}

_Bool kopsik_set_proxy_settings(void *context,
                                const _Bool use_proxy,
                                const char *proxy_host,
                                const uint64_t proxy_port,
                                const char *proxy_username,
                                const char *proxy_password) {
    poco_assert(proxy_host);
    poco_assert(proxy_username);
    poco_assert(proxy_password);

    kopsik::Proxy proxy;
    proxy.host = std::string(proxy_host);
    proxy.port = proxy_port;
    proxy.username = std::string(proxy_username);
    proxy.password = std::string(proxy_password);

    return app(context)->SaveProxySettings(use_proxy, &proxy);
}

_Bool kopsik_configure_proxy(
    void *context) {
    return app(context)->ConfigureProxy();
}

_Bool kopsik_set_db_path(
    void *context,
    const char *path) {
    poco_assert(path);

    std::stringstream ss;
    ss << "kopsik_set_db_path path=" << path;
    logger().debug(ss.str());

    return app(context)->SetDBPath(std::string(path));
}

void kopsik_set_log_path(const char *path) {
    poco_assert(path);

    Poco::AutoPtr<Poco::SimpleFileChannel> simpleFileChannel(
        new Poco::SimpleFileChannel);
    simpleFileChannel->setProperty("path", path);
    simpleFileChannel->setProperty("rotation", "1 M");

    Poco::AutoPtr<Poco::FormattingChannel> formattingChannel(
        new Poco::FormattingChannel(
            new Poco::PatternFormatter(
                "%Y-%m-%d %H:%M:%S.%i [%P %I]:%s:%q:%t")));
    formattingChannel->setChannel(simpleFileChannel);

    rootLogger().setChannel(formattingChannel);
    rootLogger().setLevel(Poco::Message::PRIO_DEBUG);
}

void kopsik_set_log_level(const char *level) {
    poco_assert(level);

    rootLogger().setLevel(level);
}

void kopsik_set_api_url(
    void *context,
    const char *api_url) {
    poco_assert(api_url);

    app(context)->SetAPIURL(std::string(api_url));
}

void kopsik_set_websocket_url(
    void *context,
    const char *websocket_url) {
    poco_assert(websocket_url);

    app(context)->SetWebSocketClientURL(websocket_url);
}

_Bool kopsik_set_api_token(
    void *context,
    const char *api_token) {
    poco_assert(api_token);

    std::stringstream ss;
    ss << "kopsik_set_api_token api_token=" << api_token;
    logger().debug(ss.str());

    return app(context)->SetCurrentAPIToken(api_token);
}

_Bool kopsik_get_api_token(
    void *context,
    char *str,
    const size_t max_strlen) {

    poco_assert(str);
    poco_assert(max_strlen);

    std::string token("");
    if (!app(context)->CurrentAPIToken(&token)) {
        return false;
    }

    strncpy(str, token.c_str(), max_strlen);

    return true;
}

_Bool kopsik_set_logged_in_user(
    void *context,
    const char *json) {
    poco_assert(json);

    logger().debug("kopsik_set_logged_in_user");

    return app(context)->SetLoggedInUserFromJSON(std::string(json));
}

_Bool kopsik_login(
    void *context,
    const char *in_email,
    const char *in_password) {

    poco_assert(in_email);
    poco_assert(in_password);

    std::stringstream ss;
    ss << "kopik_login email=" << in_email;
    logger().debug(ss.str());

    std::string email(in_email);
    std::string password(in_password);

    return app(context)->Login(email, password);
}

_Bool kopsik_logout(
    void *context) {

    logger().debug("kopsik_logout");

    return app(context)->Logout();
}

_Bool kopsik_clear_cache(
    void *context) {

    logger().debug("kopsik_clear_cache");

    return app(context)->ClearCache();
}

_Bool kopsik_user_can_see_billable_flag(
    void *context,
    const char *guid,
    _Bool *can_see) {

    poco_assert(can_see);
    poco_assert(guid);

    *can_see = false;
    if (app(context)->CanSeeBillable(std::string(guid))) {
        *can_see = true;
    }

    return true;
}

_Bool kopsik_user_can_add_projects(
    void *context,
    const uint64_t workspace_id,
    _Bool *can_add) {

    poco_assert(can_add);

    *can_add = false;
    if (app(context)->CanAddProjects(workspace_id)) {
        *can_add = true;
    }

    return true;
}

_Bool kopsik_user_is_logged_in(
    void *context,
    _Bool *is_logged_in) {

    poco_assert(is_logged_in);

    *is_logged_in = false;
    if (app(context)->UserIsLoggedIn()) {
        *is_logged_in = true;
    }

    return true;
}

_Bool kopsik_users_default_wid(
    void *context,
    uint64_t *default_wid) {

    poco_assert(default_wid);

    *default_wid =
        static_cast<unsigned int>(app(context)->UsersDefaultWID());

    return true;
}

// Sync

void kopsik_sync(void *context) {
    logger().debug("kopsik_sync");
    app(context)->FullSync();
}

void kopsik_autocomplete_item_clear(
    KopsikAutocompleteItem *item) {
    if (!item) {
        return;
    }
    if (item->Text) {
        free(item->Text);
        item->Text = 0;
    }
    if (item->ProjectAndTaskLabel) {
        free(item->ProjectAndTaskLabel);
        item->ProjectAndTaskLabel = 0;
    }
    if (item->Description) {
        free(item->Description);
        item->Description = 0;
    }
    if (item->ProjectColor) {
        free(item->ProjectColor);
        item->ProjectColor = 0;
    }
    if (item->Next) {
        KopsikAutocompleteItem *next =
            reinterpret_cast<KopsikAutocompleteItem *>(item->Next);
        kopsik_autocomplete_item_clear(next);
        item->Next = 0;
    }
    delete item;
}

_Bool kopsik_autocomplete_items(
    void *context,
    KopsikAutocompleteItem **first,
    const _Bool include_time_entries,
    const _Bool include_tasks,
    const _Bool include_projects) {

    poco_assert(first);

    logger().debug("kopsik_autocomplete_items");

    *first = 0;

    std::vector<kopsik::AutocompleteItem> items;
    app(context)->AutocompleteItems(&items,
                                    include_time_entries,
                                    include_tasks,
                                    include_projects);

    KopsikAutocompleteItem *previous = 0;
    for (std::vector<kopsik::AutocompleteItem>::iterator it =
        items.begin();
            it != items.end();
            it++) {
        kopsik::AutocompleteItem &item = *it;

        KopsikAutocompleteItem *autocomplete_item =
            autocomplete_item_init();
        if (!*first) {
            *first = autocomplete_item;
        }
        if (previous) {
            previous->Next = autocomplete_item;
        }

        autocomplete_item->Description = strdup(item.Description.c_str());
        autocomplete_item->Text = strdup(item.Text.c_str());
        autocomplete_item->ProjectAndTaskLabel =
            strdup(item.ProjectAndTaskLabel.c_str());
        autocomplete_item->ProjectColor =
            strdup(item.ProjectColor.c_str());
        autocomplete_item->ProjectID =
            static_cast<unsigned int>(item.ProjectID);
        autocomplete_item->TaskID =
            static_cast<unsigned int>(item.TaskID);
        autocomplete_item->Type =
            static_cast<unsigned int>(item.Type);

        previous = autocomplete_item;
    }

    return true;
}

_Bool kopsik_tags(
    void *context,
    KopsikViewItem **first) {

    poco_assert(first);
    poco_assert(!*first);

    std::vector<std::string> tags = app(context)->Tags();

    *first = 0;
    for (std::vector<std::string>::const_iterator it = tags.begin();
            it != tags.end();
            it++) {
        std::string name = *it;
        KopsikViewItem *item = tag_to_view_item(name);
        item->Next = *first;
        *first = item;
    }

    return true;
}

_Bool kopsik_workspaces(
    void *context,
    KopsikViewItem **first) {

    poco_assert(first);
    poco_assert(!*first);

    std::vector<kopsik::Workspace *> workspaces = app(context)->Workspaces();

    *first = 0;
    for (std::vector<kopsik::Workspace *>::const_iterator it =
        workspaces.begin();
            it != workspaces.end();
            it++) {
        KopsikViewItem *item = workspace_to_view_item(*it);
        item->Next = *first;
        *first = item;
    }

    return true;
}

_Bool kopsik_clients(
    void *context,
    const uint64_t workspace_id,
    KopsikViewItem **first) {

    poco_assert(first);
    poco_assert(!*first);

    std::vector<kopsik::Client *> clients = app(context)->Clients(workspace_id);

    *first = 0;
    for (std::vector<kopsik::Client *>::const_iterator it = clients.begin();
            it != clients.end();
            it++) {
        KopsikViewItem *item = client_to_view_item(*it);
        item->Next = *first;
        *first = item;
    }

    return true;
}

_Bool kopsik_add_project(
    void *context,
    const char *time_entry_guid,
    const uint64_t workspace_id,
    const uint64_t client_id,
    const char *project_name,
    const _Bool is_private) {

    poco_assert(time_entry_guid);

    kopsik::Project *p = 0;
    if (!app(context)->AddProject(
        workspace_id,
        client_id,
        std::string(project_name),
        is_private,
        &p)) {
        return false;
    }

    poco_assert(p);

    return kopsik_set_time_entry_project(
        context,
        time_entry_guid,
        0, /* no task ID */
        p->ID(),
        p->GUID().c_str());
}

KopsikTimeEntryViewItem *kopsik_time_entry_view_item_init() {
    KopsikTimeEntryViewItem *item = new KopsikTimeEntryViewItem();
    item->DurationInSeconds = 0;
    item->Description = 0;
    item->ProjectAndTaskLabel = 0;
    item->WID = 0;
    item->PID = 0;
    item->TID = 0;
    item->Duration = 0;
    item->Color = 0;
    item->GUID = 0;
    item->Billable = false;
    item->Tags = 0;
    item->Started = 0;
    item->Ended = 0;
    item->UpdatedAt = 0;
    item->DateHeader = 0;
    item->DurOnly = false;
    item->Next = 0;
    return item;
}

void kopsik_time_entry_view_item_clear(
    KopsikTimeEntryViewItem *item) {
    if (!item) {
        return;
    }
    if (item->Description) {
        free(item->Description);
        item->Description = 0;
    }
    if (item->ProjectAndTaskLabel) {
        free(item->ProjectAndTaskLabel);
        item->ProjectAndTaskLabel = 0;
    }
    if (item->Duration) {
        free(item->Duration);
        item->Duration = 0;
    }
    if (item->Color) {
        free(item->Color);
        item->Color = 0;
    }
    if (item->GUID) {
        free(item->GUID);
        item->GUID = 0;
    }
    if (item->Tags) {
        free(item->Tags);
        item->Tags = 0;
    }
    if (item->DateHeader) {
        free(item->DateHeader);
        item->DateHeader = 0;
    }
    if (item->DateDuration) {
        free(item->DateDuration);
        item->DateDuration = 0;
    }
    if (item->Next) {
        KopsikTimeEntryViewItem *next =
            reinterpret_cast<KopsikTimeEntryViewItem *>(item->Next);
        kopsik_time_entry_view_item_clear(next);
        item->Next = 0;
    }
    delete item;
}

_Bool kopsik_parse_time(
    const char *input,
    int *hours,
    int *minutes) {
    poco_assert(hours);
    poco_assert(minutes);
    *hours = 0;
    *minutes = 0;

    if (!input) {
        return true;
    }

    return kopsik::Formatter::ParseTimeInput(
        std::string(input), hours, minutes);
}

void kopsik_format_duration_in_seconds_hhmmss(
    const int64_t duration_in_seconds,
    char *out_str,
    const size_t max_strlen) {
    poco_assert(out_str);
    poco_assert(max_strlen);
    std::string formatted =
        kopsik::Formatter::FormatDurationInSecondsHHMMSS(duration_in_seconds);
    strncpy(out_str, formatted.c_str(), max_strlen);
}

void kopsik_format_duration_in_seconds_hhmm(
    const int64_t duration_in_seconds,
    char *out_str,
    const size_t max_strlen) {
    poco_assert(out_str);
    poco_assert(max_strlen);
    std::string formatted = kopsik::Formatter::FormatDurationInSecondsHHMM(
        duration_in_seconds);
    strncpy(out_str, formatted.c_str(), max_strlen);
}

_Bool kopsik_start(
    void *context,
    const char *description,
    const char *duration,
    const uint64_t task_id,
    const uint64_t project_id) {

    logger().debug("kopsik_start");

    std::string desc("");
    if (description) {
        desc = std::string(description);
    }

    std::string dur("");
    if (duration) {
        dur = std::string(duration);
    }

    kopsik::TimeEntry *te = 0;
    return app(context)->Start(desc, dur, task_id, project_id, &te);
}

_Bool kopsik_time_entry_view_item_by_guid(
    void *context,
    const char *guid,
    KopsikTimeEntryViewItem *view_item,
    _Bool *was_found) {

    poco_assert(guid);
    poco_assert(view_item);
    poco_assert(was_found);

    std::stringstream ss;
    ss << "kopsik_time_entry_view_item_by_guid guid=" << guid;
    logger().trace(ss.str());

    std::string GUID(guid);
    poco_assert(!GUID.empty());

    kopsik::TimeEntry *te = app(context)->GetTimeEntryByGUID(GUID);
    if (!te) {
        *was_found = false;
        return true;
    }

    *was_found = true;

    std::string project_label("");
    std::string color_code("");
    app(context)->ProjectLabelAndColorCode(te, &project_label, &color_code);

    time_entry_to_view_item(te, project_label, color_code, view_item, "");

    return true;
}

_Bool kopsik_continue(
    void *context,
    const char *guid) {

    poco_assert(guid);

    std::stringstream ss;
    ss << "kopsik_continue guid=" << guid;
    logger().debug(ss.str());

    std::string GUID(guid);


    kopsik::TimeEntry *te = 0;
    return app(context)->Continue(GUID, &te);
}

_Bool kopsik_continue_latest(
    void *context) {

    logger().debug("kopsik_continue_latest");

    kopsik::TimeEntry *te = 0;
    return app(context)->ContinueLatest(&te);
}

_Bool kopsik_delete_time_entry(
    void *context,
    const char *guid) {

    poco_assert(guid);

    std::stringstream ss;
    ss << "kopsik_delete_time_entry guid=" << guid;
    logger().debug(ss.str());

    std::string GUID(guid);

    return app(context)->DeleteTimeEntryByGUID(GUID);
}

_Bool kopsik_set_time_entry_duration(
    void *context,
    const char *guid,
    const char *value) {

    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_duration guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryDuration(
        std::string(guid),
        std::string(value));
}

_Bool kopsik_set_time_entry_project(
    void *context,
    const char *guid,
    const uint64_t task_id,
    const uint64_t project_id,
    const char *project_guid) {

    poco_assert(guid);
    std::string pguid("");
    if (project_guid) {
        pguid = std::string(project_guid);
    }
    return app(context)->SetTimeEntryProject(std::string(guid),
            task_id,
            project_id,
            pguid);
}

_Bool kopsik_set_time_entry_start_iso_8601(
    void *context,
    const char *guid,
    const char *value) {

    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_start_iso_8601 guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryStartISO8601(std::string(guid),
            std::string(value));
}

_Bool kopsik_set_time_entry_end_iso_8601(
    void *context,
    const char *guid,
    const char *value) {

    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_end_iso_8601 guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryEndISO8601(
        std::string(guid),
        std::string(value));
}

_Bool kopsik_set_time_entry_tags(
    void *context,
    const char *guid,
    const char *value) {

    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_tags guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryTags(std::string(guid),
                                          std::string(value));
}

_Bool kopsik_set_time_entry_billable(
    void *context,
    const char *guid,
    const _Bool value) {

    poco_assert(guid);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_billable guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryBillable(std::string(guid), value);
}

_Bool kopsik_set_time_entry_description(
    void *context,
    const char *guid,
    const char *value) {
    poco_assert(guid);
    poco_assert(value);

    std::stringstream ss;
    ss  << "kopsik_set_time_entry_description guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryDescription(std::string(guid),
            std::string(value));
}

_Bool kopsik_stop(
    void *context) {
    logger().debug("kopsik_stop");

    kopsik::TimeEntry *te = 0;
    return app(context)->Stop(&te);
}

_Bool kopsik_stop_running_time_entry_at(
    void *context,
    const uint64_t at) {
    poco_assert(at);

    logger().debug("kopsik_stop");

    kopsik::TimeEntry *te = 0;
    return app(context)->StopAt(at, &te);
}

_Bool kopsik_running_time_entry_view_item(
    void *context,
    KopsikTimeEntryViewItem *out_item,
    _Bool *out_is_tracking) {

    poco_assert(out_item);
    poco_assert(out_is_tracking);

    logger().debug("kopsik_running_time_entry_view_item");

    *out_is_tracking = false;
    kopsik::TimeEntry *te = 0;
    if (!app(context)->RunningTimeEntry(&te)) {
        return false;
    }
    if (te) {
        *out_is_tracking = true;
        std::string project_label("");
        std::string color_code("");
        app(context)->ProjectLabelAndColorCode(te,
                                               &project_label,
                                               &color_code);
        time_entry_to_view_item(te,
                                project_label,
                                color_code,
                                out_item,
                                "");
    }
    return true;
}

_Bool kopsik_time_entry_view_items(
    void *context,
    KopsikTimeEntryViewItem **first) {

    poco_assert(first);

    logger().debug("kopsik_time_entry_view_items");

    std::map<std::string, Poco::Int64> date_durations;
    std::vector<kopsik::TimeEntry *> visible;

    if (!app(context)->TimeEntries(&date_durations,
                                   &visible)) {
        return false;
    }

    if (visible.empty()) {
        return true;
    }

    *first = 0;
    KopsikTimeEntryViewItem *previous = 0;
    for (unsigned int i = 0; i < visible.size(); i++) {
        kopsik::TimeEntry *te = visible[i];
        KopsikTimeEntryViewItem *view_item =
            kopsik_time_entry_view_item_init();
        if (previous) {
            previous->Next = view_item;
        }
        if (!*first) {
            *first = view_item;
        }

        Poco::Int64 duration = date_durations[te->DateHeaderString()];
        std::string formatted =
            kopsik::Formatter::FormatDurationInSecondsHHMM(duration);

        std::string project_label("");
        std::string color_code("");
        app(context)->ProjectLabelAndColorCode(te,
                                               &project_label,
                                               &color_code);
        time_entry_to_view_item(te,
                                project_label,
                                color_code,
                                view_item,
                                formatted);
        previous = view_item;
    }
    return true;
}

_Bool kopsik_duration_for_date_header(
    void *context,
    const char *date,
    char *duration,
    const size_t duration_len) {

    poco_assert(duration);
    poco_assert(duration_len);
    poco_assert(date);

    logger().debug("kopsik_duration_for_date_header");

    int sum(0);
    if (!app(context)->TrackedPerDateHeader(std::string(date), &sum)) {
        return false;
    }

    kopsik_format_duration_in_seconds_hhmm(sum, duration, duration_len);
    return true;
}

void kopsik_websocket_switch(
    void *context,
    const _Bool on) {
    std::stringstream ss;
    ss << "kopsik_websocket_switch on=" << on;
    logger().debug(ss.str());

    if (on) {
        app(context)->SwitchWebSocketOn();
        return;
    }
    app(context)->SwitchWebSocketOff();
}

void kopsik_timeline_switch(
    void *context,
    const _Bool on) {
    std::stringstream ss;
    ss << "kopsik_timeline_switch on=" << on;
    logger().debug(ss.str());

    if (on) {
        app(context)->SwitchTimelineOn();
        return;
    }

    app(context)->SwitchTimelineOff();
}

void kopsik_timeline_toggle_recording(
    void *context) {
    logger().debug("kopsik_timeline_toggle_recording");
    app(context)->ToggleTimelineRecording();
}

_Bool kopsik_timeline_is_recording_enabled(
    void *context) {
    return app(context)->RecordTimeline();
}

_Bool kopsik_feedback_send(
    void *context,
    const char *topic,
    const char *details,
    const char *filename) {
    std::stringstream ss;
    ss << "kopsik_feedback_send topic=" << topic << " details=" << details;
    logger().debug(ss.str());

    kopsik::Feedback feedback(topic, details, filename);

    return app(context)->SendFeedback(feedback);
}

void kopsik_check_for_updates(
    void *context) {
    logger().debug("kopsik_check_for_updates");

    app(context)->FetchUpdates();
}

_Bool kopsik_set_update_channel(
    void *context,
    const char *update_channel) {

    poco_assert(update_channel);

    return app(context)->SaveUpdateChannel(std::string(update_channel));
}

_Bool kopsik_get_update_channel(
    void *context,
    char *update_channel,
    const size_t update_channel_len) {

    poco_assert(update_channel);
    poco_assert(update_channel_len);

    std::string s("");
    if (!app(context)->LoadUpdateChannel(&s)) {
        return false;
    }

    strncpy(update_channel, s.c_str(), update_channel_len);

    return true;
}

int64_t kopsik_parse_duration_string_into_seconds(const char *duration_string) {
    if (!duration_string) {
        return 0;
    }
    return kopsik::Formatter::ParseDurationString(std::string(duration_string));
}

// FIXME: delete
void kopsik_context_set_view_item_change_callback(
    void *context,
    KopsikViewItemChangeCallback cb) {

    app(context)->SetModelChangeCallback(cb);
}

void kopsik_context_set_error_callback(
    void *context,
    KopsikErrorCallback cb) {

    app(context)->SetOnErrorCallback(cb);
}

void kopsik_context_set_check_update_callback(
    void *context,
    KopsikCheckUpdateCallback cb) {

    app(context)->SetCheckUpdateCallback(cb);
}

void kopsik_context_set_online_callback(
    void *context,
    KopsikOnOnlineCallback cb) {

    app(context)->SetOnOnlineCallback(cb);
}

void kopsik_set_open_url_callback(
    void *context,
    KopsikOpenURLCallback cb) {

    app(context)->SetOpenURLCallback(cb);
}

void kopsik_context_set_user_login_callback(
    void *context,
    KopsikUserLoginCallback cb) {

    app(context)->SetUserLoginCallback(cb);
}

void kopsik_set_remind_callback(
    void *context,
    KopsikRemindCallback cb) {

    app(context)->SetRemindCallback(cb);
}

void kopsik_set_sleep(void *context) {
    app(context)->SetSleep();
}

void kopsik_set_wake(void *context) {
    app(context)->SetWake();
}
