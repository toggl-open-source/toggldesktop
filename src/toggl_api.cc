// Copyright 2014 Toggl Desktop developers.

// No exceptions should be thrown from this library.

#include "./../src/toggl_api.h"

#include <cstring>
#include <set>

#include "./toggl_api_lua.h"

#include "./const.h"
#include "./context.h"
#include "./custom_error_handler.h"
#include "./feedback.h"
#include "./formatter.h"
#include "./https_client.h"
#include "./project.h"
#include "./proxy.h"
#include "./time_entry.h"
#include "./timeline_uploader.h"
#include "./toggl_api_private.h"
#include "./user.h"
#include "./websocket_client.h"
#include "./window_change_recorder.h"

#include "Poco/Bugcheck.h"
#include "Poco/Path.h"
#include "Poco/Logger.h"
#include "Poco/UnicodeConverter.h"

void *toggl_context_init(
    const char_t *app_name,
    const char_t *app_version) {
    poco_check_ptr(app_name);
    poco_check_ptr(app_version);

    toggl::Context *ctx =
        new toggl::Context(to_string(app_name), to_string(app_version));

    return ctx;
}

_Bool toggl_ui_start(void *context) {
    return app(context)->StartEvents();
}

void toggl_password_forgot(void *context) {
    app(context)->UI()->DisplayURL(kLostPasswordURL);
}

void toggl_context_clear(void *context) {
    if (context) {
        app(context)->SetQuit();
        app(context)->Shutdown();
    }
    delete app(context);
}

_Bool toggl_set_settings_use_idle_detection(
    void *context,
    const _Bool use_idle_detection) {
    return app(context)->SetSettingsUseIdleDetection(use_idle_detection);
}

_Bool toggl_set_settings_autodetect_proxy(
    void *context,
    const _Bool autodetect_proxy) {
    return app(context)->SetSettingsAutodetectProxy(autodetect_proxy);
}

_Bool toggl_set_settings_menubar_timer(
    void *context,
    const _Bool menubar_timer) {
    return app(context)->SetSettingsMenubarTimer(menubar_timer);
}

_Bool toggl_set_settings_menubar_project(
    void *context,
    const _Bool menubar_project) {
    return app(context)->SetSettingsMenubarProject(menubar_project);
}

_Bool toggl_set_settings_dock_icon(
    void *context,
    const _Bool dock_icon) {
    return app(context)->SetSettingsDockIcon(dock_icon);
}

_Bool toggl_set_settings_on_top(
    void *context,
    const _Bool on_top) {
    return app(context)->SetSettingsOnTop(on_top);
}

_Bool toggl_set_settings_reminder(
    void *context,
    const _Bool reminder) {
    return app(context)->SetSettingsReminder(reminder);
}

_Bool toggl_set_settings_idle_minutes(
    void *context,
    const uint64_t idle_minutes) {
    return app(context)->SetSettingsIdleMinutes(idle_minutes);
}

_Bool toggl_set_settings_focus_on_shortcut(
    void *context,
    const _Bool focus_on_shortcut) {
    return app(context)->SetSettingsFocusOnShortcut(focus_on_shortcut);
}

_Bool toggl_set_settings_manual_mode(
    void *context,
    const _Bool manual_mode) {
    return app(context)->SetSettingsManualMode(manual_mode);
}

_Bool toggl_set_settings_reminder_minutes(
    void *context,
    const uint64_t reminder_minutes) {
    return app(context)->SetSettingsReminderMinutes(reminder_minutes);
}

_Bool toggl_set_window_settings(
    void *context,
    const int64_t window_x,
    const int64_t window_y,
    const int64_t window_height,
    const int64_t window_width) {

    return app(context)->SaveWindowSettings(
        window_x,
        window_y,
        window_height,
        window_width);
}

_Bool toggl_window_settings(
    void *context,
    int64_t *window_x,
    int64_t *window_y,
    int64_t *window_height,
    int64_t *window_width) {

    poco_check_ptr(context);

    poco_check_ptr(window_x);
    poco_check_ptr(window_y);
    poco_check_ptr(window_height);
    poco_check_ptr(window_width);

    return app(context)->LoadWindowSettings(
        window_x,
        window_y,
        window_height,
        window_width);
}

_Bool toggl_set_proxy_settings(void *context,
                               const _Bool use_proxy,
                               const char_t *proxy_host,
                               const uint64_t proxy_port,
                               const char_t *proxy_username,
                               const char_t *proxy_password) {
    poco_check_ptr(proxy_host);
    poco_check_ptr(proxy_username);
    poco_check_ptr(proxy_password);

    toggl::Proxy proxy;
    proxy.SetHost(to_string(proxy_host));
    proxy.SetPort(proxy_port);
    proxy.SetUsername(to_string(proxy_username));
    proxy.SetPassword(to_string(proxy_password));

    return app(context)->SetProxySettings(use_proxy, proxy);
}

void toggl_set_cacert_path(
    void *context,
    const char_t *path) {
    poco_check_ptr(path);

    toggl::HTTPSClient::Config.CACertPath = to_string(path);
}

_Bool toggl_set_db_path(
    void *context,
    const char_t *path) {
    poco_check_ptr(path);

    return app(context)->SetDBPath(to_string(path));
}

void toggl_set_update_path(
    void *context,
    const char_t *path) {
    poco_check_ptr(path);

    return app(context)->SetUpdatePath(to_string(path));
}

char_t *toggl_update_path(
    void *context) {
    return copy_string(app(context)->UpdatePath());
}


void toggl_set_environment(
    void *context,
    const char_t *environment) {
    poco_check_ptr(environment);

    return app(context)->SetEnvironment(to_string(environment));
}

char_t *toggl_environment(
    void *context) {
    return copy_string(app(context)->Environment());
}

void toggl_disable_update_check(
    void *context) {

    app(context)->DisableUpdateCheck();
}

void toggl_set_log_path(const char_t *path) {
    poco_check_ptr(path);

    toggl::Context::SetLogPath(to_string(path));
}

void toggl_set_log_level(const char_t *level) {
    poco_check_ptr(level);

    Poco::Logger::get("").setLevel(to_string(level));
}

void toggl_show_app(
    void *context) {
    app(context)->UI()->DisplayApp();
}

_Bool toggl_login(
    void *context,
    const char_t *email,
    const char_t *password) {

    poco_check_ptr(email);
    poco_check_ptr(password);

    return app(context)->Login(to_string(email),
                               to_string(password));
}

_Bool toggl_signup(
    void *context,
    const char_t *email,
    const char_t *password) {

    poco_check_ptr(email);
    poco_check_ptr(password);

    return app(context)->Signup(to_string(email),
                                to_string(password));
}

_Bool toggl_google_login(
    void *context,
    const char_t *access_token) {

    poco_check_ptr(access_token);

    return app(context)->GoogleLogin(to_string(access_token));
}

_Bool toggl_logout(
    void *context) {

    logger().debug("toggl_logout");

    return app(context)->Logout();
}

_Bool toggl_clear_cache(
    void *context) {

    logger().debug("toggl_clear_cache");

    return app(context)->ClearCache();
}

void toggl_sync(void *context) {
    logger().debug("toggl_sync");
    app(context)->Sync();
}

_Bool toggl_create_project(
    void *context,
    const uint64_t workspace_id,
    const uint64_t client_id,
    const char_t *project_name,
    const _Bool is_private) {

    toggl::Project *p = 0;
    return app(context)->CreateProject(
        workspace_id,
        client_id,
        to_string(project_name),
        is_private,
        &p);
}

_Bool toggl_create_client(
    void *context,
    const uint64_t workspace_id,
    const char_t *client_name) {

    return app(context)->CreateClient(
        workspace_id,
        to_string(client_name));
}

_Bool toggl_add_project(
    void *context,
    const char_t *time_entry_guid,
    const uint64_t workspace_id,
    const uint64_t client_id,
    const char_t *project_name,
    const _Bool is_private) {

    poco_check_ptr(time_entry_guid);

    toggl::Project *p = 0;
    if (!app(context)->CreateProject(
        workspace_id,
        client_id,
        to_string(project_name),
        is_private,
        &p)) {
        return false;
    }

    poco_check_ptr(p);

    char_t *guid = copy_string(p->GUID());
    _Bool res = toggl_set_time_entry_project(
        context,
        time_entry_guid,
        0, /* no task ID */
        p->ID(),
        guid);
    free(guid);
    return res;
}

char_t *toggl_format_tracking_time_duration(
    const int64_t duration_in_seconds) {

    std::string formatted = toggl::Formatter::FormatDuration(
        duration_in_seconds, toggl::Format::Classic);

    return copy_string(formatted);
}

char_t *toggl_format_tracked_time_duration(
    const int64_t duration_in_seconds) {

    std::string formatted = toggl::Formatter::FormatDuration(
        duration_in_seconds, toggl::Format::Improved, false);

    return copy_string(formatted);
}

char_t *toggl_start(
    void *context,
    const char_t *description,
    const char_t *duration,
    const uint64_t task_id,
    const uint64_t project_id) {

    logger().debug("toggl_start");

    std::string desc("");
    if (description) {
        desc = to_string(description);
    }

    std::string dur("");
    if (duration) {
        dur = to_string(duration);
    }

    toggl::TimeEntry *te = app(context)->Start(desc, dur, task_id, project_id);
    if (te) {
        return copy_string(te->GUID());
    }

    return 0;
}

_Bool toggl_continue(
    void *context,
    const char_t *guid) {

    poco_check_ptr(guid);

    std::stringstream ss;
    ss << "toggl_continue guid=" << guid;
    logger().debug(ss.str());

    return app(context)->Continue(to_string(guid));
}

void toggl_view_time_entry_list(void *context) {
    app(context)->DisplayTimeEntryList(true);
}

void toggl_edit(
    void *context,
    const char_t *guid,
    const _Bool edit_running_entry,
    const char_t *focused_field_name) {

    poco_check_ptr(guid);
    poco_check_ptr(focused_field_name);

    std::stringstream ss;
    ss << "toggl_edit guid=" << guid
       << ", edit_running_entry = " << edit_running_entry
       << ", focused_field_name = " << focused_field_name;
    logger().debug(ss.str());

    app(context)->Edit(to_string(guid),
                       edit_running_entry,
                       to_string(focused_field_name));
}

void toggl_edit_preferences(void *context) {
    app(context)->DisplaySettings(true);
}

_Bool toggl_continue_latest(
    void *context) {

    logger().debug("toggl_continue_latest");

    return app(context)->ContinueLatest();
}

_Bool toggl_delete_time_entry(
    void *context,
    const char_t *guid) {

    poco_check_ptr(guid);

    std::stringstream ss;
    ss << "toggl_delete_time_entry guid=" << guid;
    logger().debug(ss.str());

    return app(context)->DeleteTimeEntryByGUID(to_string(guid));
}

_Bool toggl_set_time_entry_duration(
    void *context,
    const char_t *guid,
    const char_t *value) {

    poco_check_ptr(guid);
    poco_check_ptr(value);

    std::stringstream ss;
    ss  << "toggl_set_time_entry_duration guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryDuration(
        to_string(guid),
        to_string(value));
}

_Bool toggl_set_time_entry_project(
    void *context,
    const char_t *guid,
    const uint64_t task_id,
    const uint64_t project_id,
    const char_t *project_guid) {

    poco_check_ptr(guid);
    std::string pguid("");
    if (project_guid) {
        pguid = to_string(project_guid);
    }
    return app(context)->SetTimeEntryProject(to_string(guid),
            task_id,
            project_id,
            pguid);
}

_Bool toggl_set_time_entry_date(
    void *context,
    const char_t *guid,
    const int64_t unix_timestamp) {

    poco_check_ptr(guid);

    std::stringstream ss;
    ss  << "toggl_set_time_entry_date guid=" << guid
        << ", unix_timestamp=" << unix_timestamp;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryDate(to_string(guid), unix_timestamp);
}

_Bool toggl_set_time_entry_start(
    void *context,
    const char_t *guid,
    const char_t *value) {
    poco_check_ptr(guid);
    poco_check_ptr(value);

    std::stringstream ss;
    ss  << "toggl_set_time_entry_start guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryStart(to_string(guid), to_string(value));
}

_Bool toggl_set_time_entry_end(
    void *context,
    const char_t *guid,
    const char_t *value) {
    poco_check_ptr(guid);
    poco_check_ptr(value);

    std::stringstream ss;
    ss  << "toggl_set_time_entry_end guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryStop(to_string(guid), to_string(value));
}

_Bool toggl_set_time_entry_tags(
    void *context,
    const char_t *guid,
    const char_t *value) {

    poco_check_ptr(guid);
    poco_check_ptr(value);

    std::stringstream ss;
    ss  << "toggl_set_time_entry_tags guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryTags(to_string(guid),
                                          to_string(value));
}

_Bool toggl_set_time_entry_billable(
    void *context,
    const char_t *guid,
    const _Bool value) {

    poco_check_ptr(guid);

    std::stringstream ss;
    ss  << "toggl_set_time_entry_billable guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryBillable(to_string(guid), value);
}

_Bool toggl_set_time_entry_description(
    void *context,
    const char_t *guid,
    const char_t *value) {
    poco_check_ptr(guid);
    poco_check_ptr(value);

    std::stringstream ss;
    ss  << "toggl_set_time_entry_description guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return app(context)->SetTimeEntryDescription(to_string(guid),
            to_string(value));
}

_Bool toggl_stop(
    void *context) {
    logger().debug("toggl_stop");

    return app(context)->Stop();
}

_Bool toggl_discard_time_at(
    void *context,
    const char_t *guid,
    const uint64_t at,
    const _Bool split_into_new_entry) {

    if (!guid) {
        logger().error("Cannot discard time without GUID");
        return false;
    }

    if (!at) {
        logger().error("Cannot discard time without a timestamp");
        return false;
    }

    logger().debug("toggl_discard_time_at");

    return app(context)->DiscardTimeAt(
        to_string(guid),
        at,
        split_into_new_entry);
}

_Bool toggl_timeline_toggle_recording(
    void *context,
    const _Bool record_timeline) {
    logger().debug("toggl_timeline_toggle_recording");
    return app(context)->ToggleTimelineRecording(record_timeline);
}

_Bool toggl_timeline_is_recording_enabled(
    void *context) {
    return app(context)->IsTimelineRecordingEnabled();
}

_Bool toggl_feedback_send(
    void *context,
    const char_t *topic,
    const char_t *details,
    const char_t *filename) {
    std::stringstream ss;
    ss << "toggl_feedback_send topic=" << topic << " details=" << details;
    logger().debug(ss.str());

    toggl::Feedback feedback(to_string(topic),
                             to_string(details),
                             to_string(filename));

    return app(context)->SendFeedback(feedback);
}

_Bool toggl_set_update_channel(
    void *context,
    const char_t *update_channel) {

    poco_check_ptr(update_channel);

    return app(context)->SaveUpdateChannel(to_string(update_channel));
}

char_t *toggl_get_update_channel(
    void *context) {

    std::string update_channel("");
    app(context)->UpdateChannel(&update_channel);
    return copy_string(update_channel);
}

char_t *toggl_get_user_fullname(
    void *context) {

    std::string fullname = app(context)->UserFullName();
    return copy_string(fullname);
}

char_t *toggl_get_user_email(
    void *context) {

    std::string email = app(context)->UserEmail();
    return copy_string(email);
}

int64_t toggl_parse_duration_string_into_seconds(
    const char_t *duration_string) {
    if (!duration_string) {
        return 0;
    }
    return toggl::Formatter::ParseDurationString(to_string(duration_string));
}

void toggl_on_show_app(
    void *context,
    TogglDisplayApp cb) {

    app(context)->UI()->OnDisplayApp(cb);
}

void toggl_on_sync_state(
    void *context,
    TogglDisplaySyncState cb) {

    app(context)->UI()->OnDisplaySyncState(cb);
}

void toggl_on_unsynced_items(
    void *context,
    TogglDisplayUnsyncedItems cb) {

    app(context)->UI()->OnDisplayUnsyncedItems(cb);
}

void toggl_on_error(
    void *context,
    TogglDisplayError cb) {

    app(context)->UI()->OnDisplayError(cb);
}

void toggl_on_online_state(
    void *context,
    TogglDisplayOnlineState cb) {

    app(context)->UI()->OnDisplayOnlineState(cb);
}

void toggl_on_update(
    void *context,
    TogglDisplayUpdate cb) {

    app(context)->UI()->OnDisplayUpdate(cb);
}

void toggl_on_url(
    void *context,
    TogglDisplayURL cb) {

    app(context)->UI()->OnDisplayURL(cb);
}

void toggl_on_login(
    void *context,
    TogglDisplayLogin cb) {

    app(context)->UI()->OnDisplayLogin(cb);
}

void toggl_on_reminder(
    void *context,
    TogglDisplayReminder cb) {

    app(context)->UI()->OnDisplayReminder(cb);
}

void toggl_set_sleep(void *context) {
    app(context)->SetSleep();
}

void toggl_set_wake(void *context) {
    if (!context) {
        return;
    }
    app(context)->SetWake();
}

void toggl_set_online(void *context) {
    if (!context) {
        return;
    }
    app(context)->SetOnline();
}

void toggl_open_in_browser(void *context) {
    app(context)->OpenReportsInBrowser();
}

void toggl_get_support(void *context) {
    app(context)->UI()->DisplayURL(kSupportURL);
}

void toggl_on_workspace_select(
    void *context,
    TogglDisplayViewItems cb) {
    app(context)->UI()->OnDisplayWorkspaceSelect(cb);
}

void toggl_on_client_select(
    void *context,
    TogglDisplayViewItems cb) {
    app(context)->UI()->OnDisplayClientSelect(cb);
}

void toggl_on_tags(
    void *context,
    TogglDisplayViewItems cb) {
    app(context)->UI()->OnDisplayTags(cb);
}

void toggl_on_time_entry_list(
    void *context,
    TogglDisplayTimeEntryList cb) {
    app(context)->UI()->OnDisplayTimeEntryList(cb);
}

void toggl_on_mini_timer_autocomplete(
    void *context,
    TogglDisplayAutocomplete cb) {
    app(context)->UI()->OnDisplayMinitimerAutocomplete(cb);
}

void toggl_on_time_entry_autocomplete(
    void *context,
    TogglDisplayAutocomplete cb) {
    app(context)->UI()->OnDisplayTimeEntryAutocomplete(cb);
}

void toggl_on_project_autocomplete(
    void *context,
    TogglDisplayAutocomplete cb) {
    app(context)->UI()->OnDisplayProjectAutocomplete(cb);
}

void toggl_on_time_entry_editor(
    void *context,
    TogglDisplayTimeEntryEditor cb) {
    app(context)->UI()->OnDisplayTimeEntryEditor(cb);
}

void toggl_on_settings(
    void *context,
    TogglDisplaySettings cb) {
    app(context)->UI()->OnDisplaySettings(cb);
}

void toggl_on_timer_state(
    void *context,
    TogglDisplayTimerState cb) {
    app(context)->UI()->OnDisplayTimerState(cb);
}

void toggl_on_idle_notification(
    void *context,
    TogglDisplayIdleNotification cb) {
    app(context)->UI()->OnDisplayIdleNotification(cb);
}

void toggl_debug(const char_t *text) {
    logger().debug(to_string(text));
}

_Bool toggl_check_view_struct_size(
    const int time_entry_view_item_size,
    const int autocomplete_view_item_size,
    const int view_item_size,
    const int settings_size) {
    int size = sizeof(TogglTimeEntryView);
    if (time_entry_view_item_size != size) {
        logger().error("Invalid time entry view item struct size");
        return false;
    }
    size = sizeof(TogglAutocompleteView);
    if (autocomplete_view_item_size != size) {
        logger().error("Invalid autocomplete view item struct size");
        return false;
    }
    size = sizeof(TogglGenericView);
    if (view_item_size != size) {
        logger().error("Invalid view item struct size");
        return false;
    }
    size = sizeof(TogglSettingsView);
    if (settings_size != size) {
        logger().error("Invalid settings view item struct size");
        return false;
    }
    return true;
}

void toggl_set_idle_seconds(
    void *context,
    const uint64_t idle_seconds) {
    if (context) {
        app(context)->SetIdleSeconds(idle_seconds);
    }
}

char_t *toggl_run_script(
    void *context,
    const char* script,
    int64_t *err) {

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    toggl_register_lua(context, L);
    lua_settop(L, 0);

    *err = luaL_loadstring(L, script);
    if (*err) {
        return copy_string(lua_tostring(L, -1));
    }

    *err = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (*err) {
        return copy_string(lua_tostring(L, -1));
    }

    int argc = lua_gettop(L);

    std::stringstream ss;
    ss << argc << " value(s) returned" << std::endl;

    for (int i = 0; i < argc; i++) {
        if (lua_isstring(L, -1)) {
            ss << lua_tostring(L, -1);
        } else if (lua_isnumber(L, -1)) {
            ss << lua_tointeger(L, -1);
        } else if (lua_isboolean(L, -1)) {
            ss << lua_toboolean(L, -1);
        } else {
            ss << "ok";
        }
        lua_pop(L, -1);
    }
    ss << std::endl << std::endl;

    lua_close(L);

    return copy_string(ss.str());
}

void testing_sleep(
    const int seconds) {
    Poco::Thread::sleep(seconds*1000);
}

_Bool testing_set_logged_in_user(
    void *context,
    const char *json) {
    poco_check_ptr(json);

    toggl::Context *ctx = reinterpret_cast<toggl::Context *>(context);
    return ctx->SetLoggedInUserFromJSON(std::string(json));
}
