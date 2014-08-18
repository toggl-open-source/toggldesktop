// Copyright 2014 Toggl Desktop developers.

// No exceptions should be thrown from this library.

#include <cstring>
#include <set>

#include "./lib/include/kopsik_api.h"
#include "./kopsik_api_private.h"
#include "./database.h"
#include "./user.h"
#include "./https_client.h"
#include "./websocket_client.h"
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
#include "Poco/UnicodeConverter.h"

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
        app(context)->SetQuit(true);
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

void kopsik_set_cacert_path(
    void *context,
    const char *path) {
    poco_check_ptr(path);

    kopsik::HTTPSClient::CACertPath = std::string(path);
}

void kopsik_set_cacert_path_utf16(
    void *context,
    const wchar_t *path) {
    poco_check_ptr(path);

    std::string utf8("");
    Poco::UnicodeConverter::toUTF8(path, utf8);

    kopsik::HTTPSClient::CACertPath = utf8;
}

_Bool kopsik_set_db_path(
    void *context,
    const char *path) {
    poco_check_ptr(path);

    return app(context)->SetDBPath(std::string(path));
}

_Bool kopsik_set_db_path_utf16(
    void *context,
    const wchar_t *path) {
    poco_check_ptr(path);

    std::string utf8("");
    Poco::UnicodeConverter::toUTF8(path, utf8);

    return app(context)->SetDBPath(utf8);
}

void kopsik_set_environment(
    void *context,
    const char *environment) {
    poco_check_ptr(environment);

    return app(context)->SetEnvironment(environment);
}

void kopsik_disable_update_check(
    void *context) {

    app(context)->DisableUpdateCheck();
}

void kopsik_set_log_path(const char *path) {
    poco_check_ptr(path);

    kopsik::Context::SetLogPath(std::string(path));
}

void kopsik_set_log_path_utf16(const wchar_t *path) {
    poco_check_ptr(path);

    std::string utf8("");
    Poco::UnicodeConverter::toUTF8(path, utf8);

    kopsik::Context::SetLogPath(utf8);
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

_Bool kopsik_google_login(
    void *context,
    const char *access_token) {

    poco_check_ptr(access_token);

    return app(context)->GoogleLogin(std::string(access_token));
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

// Sync

void kopsik_sync(void *context) {
    logger().debug("kopsik_sync");
    app(context)->Sync();
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

void kopsik_about(void *context) {
    app(context)->About();
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

_Bool kopsik_discard_time_at(
    void *context,
    const char *guid,
    const uint64_t at) {
    poco_check_ptr(guid);
    poco_assert(at);

    logger().debug("kopsik_discard_time_at");

    return app(context)->DiscardTimeAt(guid, at);
}

_Bool kopsik_timeline_toggle_recording(
    void *context,
    const _Bool record_timeline) {
    logger().debug("kopsik_timeline_toggle_recording");
    return app(context)->ToggleTimelineRecording(record_timeline);
}

_Bool kopsik_timeline_is_recording_enabled(
    void *context) {
    return app(context)->IsTimelineRecordingEnabled();
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

_Bool kopsik_set_update_channel(
    void *context,
    const char *update_channel) {

    poco_check_ptr(update_channel);

    return app(context)->SaveUpdateChannel(std::string(update_channel));
}

int64_t kopsik_parse_duration_string_into_seconds(const char *duration_string) {
    if (!duration_string) {
        return 0;
    }
    return kopsik::Formatter::ParseDurationString(std::string(duration_string));
}

void kopsik_on_app(
    void *context,
    KopsikDisplayApp cb) {

    app(context)->UI()->OnDisplayApp(cb);
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
    if (!context) {
        return;
    }
    app(context)->SetWake();
}

void kopsik_set_online(void *context) {
    if (!context) {
        return;
    }
    app(context)->SetOnline();
}

void kopsik_open_in_browser(void *context) {
    app(context)->OpenReportsInBrowser();
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

void kopsik_on_time_entry_autocomplete(
    void *context,
    KopsikDisplayAutocomplete cb) {
    app(context)->UI()->OnDisplayTimeEntryAutocomplete(cb);
}

void kopsik_on_project_autocomplete(
    void *context,
    KopsikDisplayAutocomplete cb) {
    app(context)->UI()->OnDisplayProjectAutocomplete(cb);
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

void kopsik_on_idle_notification(
    void *context,
    KopsikDisplayIdleNotification cb) {
    app(context)->UI()->OnDisplayIdleNotification(cb);
}

void kopsik_debug(const char *text) {
    logger().debug(text);
}

void kopsik_check_view_item_size(
    const int time_entry_view_item_size,
    const int autocomplete_view_item_size,
    const int view_item_size,
    const int settings_size,
    const int update_view_item_size) {
    size_t size = sizeof(KopsikTimeEntryViewItem);
    if (time_entry_view_item_size != size) {
        logger().error("Invalid time entry view item struct size");
        poco_assert(false);
    }
    size = sizeof(KopsikAutocompleteItem);
    if (autocomplete_view_item_size != size) {
        logger().error("Invalid autocomplete view item struct size");
        poco_assert(false);
    }
    size = sizeof(KopsikViewItem);
    if (view_item_size != size) {
        logger().error("Invalid view item struct size");
        poco_assert(false);
    }
    size = sizeof(KopsikSettingsViewItem);
    if (settings_size != size) {
        logger().error("Invalid settings view item struct size");
        poco_assert(false);
    }
    size = sizeof(KopsikUpdateViewItem);
    if (update_view_item_size != size) {
        logger().error("Invalid update view item size");
        poco_assert(false);
    }
}

void kopsik_set_idle_seconds(
    void *context,
    const uint64_t idle_seconds) {
    app(context)->SetIdleSeconds(idle_seconds);
}
