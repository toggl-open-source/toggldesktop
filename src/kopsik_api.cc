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
    poco_check_ptr(context);

    return reinterpret_cast<kopsik::Context *>(context);
}

void *kopsik_context_init(
    const char *app_name,
    const char *app_version) {
    poco_check_ptr(app_name);
    poco_check_ptr(app_version);

    kopsik::Context *ctx =
        new kopsik::Context(std::string(app_name), std::string(app_version));

    ctx->SetAPIURL(kAPIURL);
    ctx->SetTimelineUploadURL(kTimelineUploadURL);
    ctx->SetWebSocketClientURL(kWebSocketURL);

    return ctx;
}

_Bool kopsik_context_start_events(void *context) {
    return app(context)->StartEvents();
}

void kopsik_password_forgot(void *context) {
    app(context)->UI()->DisplayURL(kLostPasswordURL);
}

void kopsik_context_clear(void *context) {
    if (context) {
        app(context)->Shutdown();
    }
    delete app(context);
}

_Bool kopsik_set_settings(
    void *context,
    const _Bool use_idle_detection,
    const _Bool menubar_timer,
    const _Bool dock_icon,
    const _Bool on_top,
    const _Bool reminder) {

    kopsik::Settings settings;
    settings.use_idle_detection = use_idle_detection;
    settings.menubar_timer = menubar_timer;
    settings.dock_icon = dock_icon;
    settings.on_top = on_top;
    settings.reminder = reminder;

    return app(context)->SetSettings(settings);
}

_Bool kopsik_set_proxy_settings(void *context,
                                const _Bool use_proxy,
                                const char *proxy_host,
                                const uint64_t proxy_port,
                                const char *proxy_username,
                                const char *proxy_password) {
    poco_check_ptr(proxy_host);
    poco_check_ptr(proxy_username);
    poco_check_ptr(proxy_password);

    kopsik::Proxy proxy;
    proxy.host = std::string(proxy_host);
    proxy.port = proxy_port;
    proxy.username = std::string(proxy_username);
    proxy.password = std::string(proxy_password);

    return app(context)->SetProxySettings(use_proxy, proxy);
}

_Bool kopsik_set_db_path(
    void *context,
    const char *path) {
    poco_check_ptr(path);

    std::stringstream ss;
    ss << "kopsik_set_db_path path=" << path;
    logger().debug(ss.str());

    return app(context)->SetDBPath(std::string(path));
}

void kopsik_set_log_path(const char *path) {
    poco_check_ptr(path);

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
    poco_check_ptr(level);

    rootLogger().setLevel(level);
}

void kopsik_set_api_url(
    void *context,
    const char *api_url) {
    poco_check_ptr(api_url);

    app(context)->SetAPIURL(std::string(api_url));
}

void kopsik_set_websocket_url(
    void *context,
    const char *websocket_url) {
    poco_check_ptr(websocket_url);

    app(context)->SetWebSocketClientURL(websocket_url);
}

_Bool kopsik_login(
    void *context,
    const char *email,
    const char *password) {

    poco_check_ptr(email);
    poco_check_ptr(password);

    return app(context)->Login(std::string(email),
                               std::string(password));
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

    poco_check_ptr(can_see);
    poco_check_ptr(guid);

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

    poco_check_ptr(can_add);

    *can_add = false;
    if (app(context)->CanAddProjects(workspace_id)) {
        *can_add = true;
    }

    return true;
}

_Bool kopsik_users_default_wid(
    void *context,
    uint64_t *default_wid) {

    poco_check_ptr(default_wid);

    *default_wid =
        static_cast<unsigned int>(app(context)->UsersDefaultWID());

    return true;
}

// Sync

void kopsik_sync(void *context) {
    logger().debug("kopsik_sync");
    app(context)->FullSync();
}

_Bool kopsik_add_project(
    void *context,
    const char *time_entry_guid,
    const uint64_t workspace_id,
    const uint64_t client_id,
    const char *project_name,
    const _Bool is_private) {

    poco_check_ptr(time_entry_guid);

    kopsik::Project *p = 0;
    if (!app(context)->AddProject(
        workspace_id,
        client_id,
        std::string(project_name),
        is_private,
        &p)) {
        return false;
    }

    poco_check_ptr(p);

    return kopsik_set_time_entry_project(
        context,
        time_entry_guid,
        0, /* no task ID */
        p->ID(),
        p->GUID().c_str());
}

_Bool kopsik_parse_time(
    const char *input,
    int *hours,
    int *minutes) {
    poco_check_ptr(hours);
    poco_check_ptr(minutes);

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

    poco_check_ptr(out_str);

    poco_assert(max_strlen);

    std::string formatted =
        kopsik::Formatter::FormatDurationInSecondsHHMMSS(duration_in_seconds);
    strncpy(out_str, formatted.c_str(), max_strlen);
}

void kopsik_format_duration_in_seconds_hhmm(
    const int64_t duration_in_seconds,
    char *out_str,
    const size_t max_strlen) {

    poco_check_ptr(out_str);

    poco_assert(max_strlen);

    std::string formatted = kopsik::Formatter::FormatDurationInSecondsHHMM(
        duration_in_seconds);
    strncpy(out_str, formatted.c_str(), max_strlen);
}

void kopsik_format_duration_in_seconds_pretty_hhmm(
    const int64_t duration_in_seconds,
    char *out_str,
    const size_t max_strlen) {

    poco_check_ptr(out_str);

    poco_assert(max_strlen);

    std::string formatted =
        kopsik::Formatter::FormatDurationInSecondsPrettyHHMM(
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

    return app(context)->Start(desc, dur, task_id, project_id);
}

_Bool kopsik_continue(
    void *context,
    const char *guid) {

    poco_check_ptr(guid);

    std::stringstream ss;
    ss << "kopsik_continue guid=" << guid;
    logger().debug(ss.str());

    std::string GUID(guid);

    return app(context)->Continue(GUID);
}

void kopsik_view_time_entry_list(void *context) {
    app(context)->DisplayTimeEntryList(true);
}

void kopsik_edit(
    void *context,
    const char *guid,
    const _Bool edit_running_entry,
    const char *focused_field_name) {

    poco_check_ptr(guid);
    poco_check_ptr(focused_field_name);

    std::stringstream ss;
    ss << "kopsik_edit guid=" << guid
       << ", edit_running_entry = " << edit_running_entry
       << ", focused_field_name = " << focused_field_name;
    logger().debug(ss.str());

    app(context)->Edit(std::string(guid),
                       edit_running_entry,
                       std::string(focused_field_name));
}

void kopsik_edit_preferences(void *context) {
    app(context)->DisplaySettings(true);
}

_Bool kopsik_continue_latest(
    void *context) {

    logger().debug("kopsik_continue_latest");

    return app(context)->ContinueLatest();
}

_Bool kopsik_delete_time_entry(
    void *context,
    const char *guid) {

    poco_check_ptr(guid);

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

    poco_check_ptr(guid);
    poco_check_ptr(value);

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

    poco_check_ptr(guid);
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

    poco_check_ptr(guid);
    poco_check_ptr(value);

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

    poco_check_ptr(guid);
    poco_check_ptr(value);

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

    poco_check_ptr(guid);
    poco_check_ptr(value);

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

    poco_check_ptr(guid);

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
    poco_check_ptr(guid);
    poco_check_ptr(value);

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

    return app(context)->Stop();
}

_Bool kopsik_stop_running_time_entry_at(
    void *context,
    const uint64_t at) {
    poco_assert(at);

    logger().debug("kopsik_stop");

    return app(context)->StopAt(at);
}

void kopsik_timeline_toggle_recording(
    void *context) {
    logger().debug("kopsik_timeline_toggle_recording");
    app(context)->ToggleTimelineRecording();
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

    poco_check_ptr(update_channel);

    return app(context)->SaveUpdateChannel(std::string(update_channel));
}

_Bool kopsik_get_update_channel(
    void *context,
    char *update_channel,
    const size_t update_channel_len) {

    poco_check_ptr(update_channel);

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

void kopsik_on_error(
    void *context,
    KopsikDisplayError cb) {

    app(context)->UI()->OnDisplayError(cb);
}

void kopsik_on_update(
    void *context,
    KopsikDisplayUpdate cb) {

    app(context)->UI()->OnDisplayUpdate(cb);
}

void kopsik_on_online_state(
    void *context,
    KopsikDisplayOnlineState cb) {

    app(context)->UI()->OnDisplayOnlineState(cb);
}

void kopsik_on_url(
    void *context,
    KopsikDisplayURL cb) {

    app(context)->UI()->OnDisplayURL(cb);
}

void kopsik_on_login(
    void *context,
    KopsikDisplayLogin cb) {

    app(context)->UI()->OnDisplayLogin(cb);
}

void kopsik_on_reminder(
    void *context,
    KopsikDisplayReminder cb) {

    app(context)->UI()->OnDisplayReminder(cb);
}

void kopsik_set_sleep(void *context) {
    app(context)->SetSleep();
}

void kopsik_set_wake(void *context) {
    app(context)->SetWake();
}

void kopsik_open_in_browser(void *context) {
    app(context)->UI()->DisplayURL(kTogglWebsiteURL);
}

void kopsik_get_support(void *context) {
    app(context)->UI()->DisplayURL(kSupportURL);
}

void kopsik_on_workspace_select(
    void *context,
    KopsikDisplayViewItems cb) {
    app(context)->UI()->OnDisplayWorkspaceSelect(cb);
}

void kopsik_on_client_select(
    void *context,
    KopsikDisplayViewItems cb) {
    app(context)->UI()->OnDisplayClientSelect(cb);
}

void kopsik_on_tags(
    void *context,
    KopsikDisplayViewItems cb) {
    app(context)->UI()->OnDisplayTags(cb);
}

void kopsik_on_time_entry_list(
    void *context,
    KopsikDisplayTimeEntryList cb) {
    app(context)->UI()->OnDisplayTimeEntryList(cb);
}

void kopsik_on_autocomplete(
    void *context,
    KopsikDisplayAutocomplete cb) {
    app(context)->UI()->OnDisplayAutocomplete(cb);
}

void kopsik_on_time_entry_editor(
    void *context,
    KopsikDisplayTimeEntryEditor cb) {
    app(context)->UI()->OnDisplayTimeEntryEditor(cb);
}

void kopsik_on_settings(
    void *context,
    KopsikDisplaySettings cb) {
    app(context)->UI()->OnDisplaySettings(cb);
}

void kopsik_on_timer_state(
    void *context,
    KopsikDisplayTimerState cb) {
    app(context)->UI()->OnDisplayTimerState(cb);
}

void kopsik_debug(
    void *context,
    const char *text) {
    logger().debug(text);
}
