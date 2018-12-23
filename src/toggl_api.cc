// Copyright 2014 Toggl Desktop developers.

// No exceptions should be thrown from this library.

#include "./../src/toggl_api.h"
#include <fstream>
#include <cstring>
#include <set>

#include "./toggl_api_lua.h"

#include "./client.h"
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
    return new toggl::Context(to_string(app_name), to_string(app_version));
}

bool_t toggl_ui_start(void *context) {
    return toggl::noError == app(context)->StartEvents();
}

void toggl_password_forgot(void *context) {
    app(context)->UI()->DisplayURL(kLostPasswordURL);
}

void toggl_tos(void *context) {
    app(context)->UI()->DisplayURL(kTOSURL);
}

void toggl_privacy_policy(void *context) {
    app(context)->UI()->DisplayURL(kPrivacyPolicyURL);
}

void toggl_context_clear(void *context) {
    delete app(context);
}

bool_t toggl_set_settings_remind_days(
    void *context,
    const bool_t remind_mon,
    const bool_t remind_tue,
    const bool_t remind_wed,
    const bool_t remind_thu,
    const bool_t remind_fri,
    const bool_t remind_sat,
    const bool_t remind_sun) {

    return toggl::noError == app(context)->SetSettingsRemindDays(
        remind_mon,
        remind_tue,
        remind_wed,
        remind_thu,
        remind_fri,
        remind_sat,
        remind_sun);
}

bool_t toggl_set_settings_remind_times(
    void *context,
    const char_t *remind_starts,
    const char_t *remind_ends) {
    return toggl::noError == app(context)->SetSettingsRemindTimes(
        to_string(remind_starts),
        to_string(remind_ends));
}

bool_t toggl_set_settings_use_idle_detection(
    void *context,
    const bool_t use_idle_detection) {
    return toggl::noError == app(context)->
           SetSettingsUseIdleDetection(use_idle_detection);
}

bool_t toggl_set_settings_autotrack(
    void *context,
    const bool_t value) {
    return toggl::noError == app(context)->SetSettingsAutotrack(value);
}

bool_t toggl_set_settings_open_editor_on_shortcut(
    void *context,
    const bool_t value) {
    return toggl::noError ==
           app(context)->SetSettingsOpenEditorOnShortcut(value);
}

bool_t toggl_set_settings_autodetect_proxy(
    void *context,
    const bool_t autodetect_proxy) {
    return toggl::noError == app(context)->
           SetSettingsAutodetectProxy(autodetect_proxy);
}

bool_t toggl_set_settings_menubar_timer(
    void *context,
    const bool_t menubar_timer) {
    return toggl::noError == app(context)->
           SetSettingsMenubarTimer(menubar_timer);
}

bool_t toggl_set_settings_menubar_project(
    void *context,
    const bool_t menubar_project) {
    return toggl::noError == app(context)->
           SetSettingsMenubarProject(menubar_project);
}

bool_t toggl_set_settings_dock_icon(
    void *context,
    const bool_t dock_icon) {
    return toggl::noError == app(context)->SetSettingsDockIcon(dock_icon);
}

bool_t toggl_set_settings_on_top(
    void *context,
    const bool_t on_top) {
    return toggl::noError == app(context)->SetSettingsOnTop(on_top);
}

bool_t toggl_set_settings_reminder(
    void *context,
    const bool_t reminder) {
    return toggl::noError == app(context)->SetSettingsReminder(reminder);
}

bool_t toggl_set_settings_pomodoro(
    void *context,
    const bool_t pomodoro) {
    if (!pomodoro) {
        return (toggl::noError == app(context)->SetSettingsPomodoro(pomodoro)
                && toggl::noError ==
                app(context)->SetSettingsPomodoroBreak(pomodoro));
    }
    return toggl::noError == app(context)->SetSettingsPomodoro(pomodoro);
}

bool_t toggl_set_settings_pomodoro_break(
    void *context,
    const bool_t pomodoro_break) {
    return toggl::noError == app(context)->
           SetSettingsPomodoroBreak(pomodoro_break);
}

bool_t toggl_set_settings_stop_entry_on_shutdown_sleep(
    void *context,
    const bool_t stop_entry) {
    return toggl::noError == app(context)->
           SetSettingsStopEntryOnShutdownSleep(stop_entry);
}


bool_t toggl_set_settings_idle_minutes(
    void *context,
    const uint64_t idle_minutes) {
    return toggl::noError == app(context)->SetSettingsIdleMinutes(idle_minutes);
}

bool_t toggl_set_settings_focus_on_shortcut(
    void *context,
    const bool_t focus_on_shortcut) {
    return toggl::noError == app(context)->
           SetSettingsFocusOnShortcut(focus_on_shortcut);
}

bool_t toggl_set_settings_manual_mode(
    void *context,
    const bool_t manual_mode) {
    return toggl::noError == app(context)->SetSettingsManualMode(manual_mode);
}

bool_t toggl_set_settings_reminder_minutes(
    void *context,
    const uint64_t reminder_minutes) {
    return toggl::noError == app(context)->
           SetSettingsReminderMinutes(reminder_minutes);
}

bool_t toggl_set_settings_pomodoro_minutes(
    void *context,
    const uint64_t pomodoro_minutes) {
    return toggl::noError == app(context)->
           SetSettingsPomodoroMinutes(pomodoro_minutes);
}

bool_t toggl_set_settings_pomodoro_break_minutes(
    void *context,
    const uint64_t pomodoro_break_minutes) {
    return toggl::noError == app(context)->
           SetSettingsPomodoroBreakMinutes(pomodoro_break_minutes);
}

bool_t toggl_set_window_settings(
    void *context,
    const int64_t window_x,
    const int64_t window_y,
    const int64_t window_height,
    const int64_t window_width) {

    return toggl::noError == app(context)->SaveWindowSettings(
        window_x,
        window_y,
        window_height,
        window_width);
}

bool_t toggl_window_settings(
    void *context,
    int64_t *window_x,
    int64_t *window_y,
    int64_t *window_height,
    int64_t *window_width) {

    poco_check_ptr(window_x);
    poco_check_ptr(window_y);
    poco_check_ptr(window_height);
    poco_check_ptr(window_width);

    return toggl::noError == app(context)->LoadWindowSettings(
        window_x,
        window_y,
        window_height,
        window_width);
}

void toggl_set_window_maximized(
    void *context,
    const bool_t value) {
    app(context)->SetWindowMaximized(value);
}

bool_t toggl_get_window_maximized(
    void *context) {
    return app(context)->GetWindowMaximized();
}

void toggl_set_window_minimized(
    void *context,
    const bool_t value) {
    app(context)->SetWindowMinimized(value);
}

bool_t toggl_get_window_minimized(
    void *context) {
    return app(context)->GetWindowMinimized();
}

void toggl_set_window_edit_size_height(
    void *context,
    const int64_t value) {
    app(context)->SetWindowEditSizeHeight(value);
}

int64_t toggl_get_window_edit_size_height(
    void *context) {
    return app(context)->GetWindowEditSizeHeight();
}

void toggl_set_window_edit_size_width(
    void *context,
    const int64_t value) {
    app(context)->SetWindowEditSizeWidth(value);
}

int64_t toggl_get_window_edit_size_width(
    void *context) {
    return app(context)->GetWindowEditSizeWidth();
}

void toggl_set_key_start(
    void *context,
    const char_t *value) {
    app(context)->SetKeyStart(to_string(value));
}

char_t *toggl_get_key_start(
    void *context) {
    return copy_string(app(context)->GetKeyStart());
}

void toggl_set_key_show(
    void *context,
    const char_t *value) {
    app(context)->SetKeyShow(to_string(value));
}

char_t *toggl_get_key_show(
    void *context) {
    return copy_string(app(context)->GetKeyShow());
}

void toggl_set_key_modifier_show(
    void *context,
    const char_t *value) {
    app(context)->SetKeyModifierShow(to_string(value));
}

char_t *toggl_get_key_modifier_show(
    void *context) {
    return copy_string(app(context)->GetKeyModifierShow());
}

void toggl_set_key_modifier_start(
    void *context,
    const char_t *value) {
    app(context)->SetKeyModifierStart(to_string(value));
}

char_t *toggl_get_key_modifier_start(
    void *context) {
    return copy_string(app(context)->GetKeyModifierStart());
}

bool_t toggl_set_proxy_settings(void *context,
                                const bool_t use_proxy,
                                const char_t *proxy_host,
                                const uint64_t proxy_port,
                                const char_t *proxy_username,
                                const char_t *proxy_password) {
    toggl::Proxy proxy;
    proxy.SetHost(to_string(proxy_host));
    proxy.SetPort(proxy_port);
    proxy.SetUsername(to_string(proxy_username));
    proxy.SetPassword(to_string(proxy_password));

    return toggl::noError == app(context)->SetProxySettings(use_proxy, proxy);
}

void toggl_set_cacert_path(
    void *context,
    const char_t *path) {

    toggl::HTTPSClient::Config.CACertPath = to_string(path);
}

bool_t toggl_set_db_path(
    void *context,
    const char_t *path) {
    return toggl::noError == app(context)->SetDBPath(to_string(path));
}

void toggl_set_update_path(
    void *context,
    const char_t *path) {
    return app(context)->SetUpdatePath(to_string(path));
}

char_t *toggl_update_path(
    void *context) {
    return copy_string(app(context)->UpdatePath());
}

void toggl_set_environment(
    void *context,
    const char_t *environment) {
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
    toggl::Context::SetLogPath(to_string(path));
}

void toggl_set_log_level(const char_t *level) {
    Poco::Logger::get("").setLevel(to_string(level));
}

void toggl_show_app(
    void *context) {
    app(context)->UI()->DisplayApp();
}

bool_t toggl_login(
    void *context,
    const char_t *email,
    const char_t *password) {
    return toggl::noError == app(context)->Login(to_string(email),
            to_string(password));
}

bool_t toggl_signup(
    void *context,
    const char_t *email,
    const char_t *password,
    const uint64_t country_id) {
    return toggl::noError == app(context)->Signup(to_string(email),
            to_string(password), country_id);
}

bool_t toggl_google_login(
    void *context,
    const char_t *access_token) {
    return toggl::noError == app(context)->GoogleLogin(to_string(access_token));
}

bool_t toggl_logout(
    void *context) {

    logger().debug("toggl_logout");

    return toggl::noError == app(context)->Logout();
}

bool_t toggl_clear_cache(
    void *context) {

    logger().debug("toggl_clear_cache");

    return toggl::noError == app(context)->ClearCache();
}

void toggl_sync(void *context) {
    logger().debug("toggl_sync");
    app(context)->Sync();
}

void toggl_fullsync(void *context) {
    logger().debug("toggl_fullsync");
    app(context)->FullSync();
}

bool_t toggl_add_obm_action(
    void *context,
    const uint64_t experiment_id,
    const char_t *key,
    const char_t *value) {

    return toggl::noError == app(context)->AddObmAction(
        experiment_id,
        to_string(key),
        to_string(value));
}

void toggl_add_obm_experiment_nr(
    const uint64_t nr) {

    toggl::HTTPSClient::Config.OBMExperimentNrs.push_back(nr);
}

char_t *toggl_create_client(
    void *context,
    const uint64_t workspace_id,
    const char_t *client_name) {

    toggl::Client *c = app(context)->CreateClient(
        workspace_id,
        to_string(client_name));

    if (!c) {
        return nullptr;
    }

    poco_check_ptr(c);

    logger().debug("Created client " + c->String());

    poco_assert(!c->GUID().empty());

    return copy_string(c->GUID());
}

char_t *toggl_add_project(
    void *context,
    const char_t *time_entry_guid,
    const uint64_t workspace_id,
    const uint64_t client_id,
    const char_t *client_guid,
    const char_t *project_name,
    const bool_t is_private,
    const char_t *project_color) {

    toggl::Project *p = app(context)->CreateProject(
        workspace_id,
        client_id,
        to_string(client_guid),
        to_string(project_name),
        is_private,
        to_string(project_color));
    if (!p) {
        return nullptr;
    }

    poco_check_ptr(p);

    char_t *guid = copy_string(p->GUID());
    toggl_set_time_entry_project(
        context,
        time_entry_guid,
        0, /* no task ID */
        p->ID(),
        guid);

    // Update billable if new project is billable
    if (p->Billable()) {
        toggl_set_time_entry_billable(
            context,
            time_entry_guid,
            p->Billable());
    }

    return guid;
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
    const uint64_t project_id,
    const char_t *project_guid,
    const char_t *tags,
    const bool_t prevent_on_app) {

    logger().debug("toggl_start");

    std::string desc("");
    if (description) {
        desc = to_string(description);
    }

    std::string dur("");
    if (duration) {
        dur = to_string(duration);
    }

    std::string p_guid("");
    if (project_guid) {
        p_guid = to_string(project_guid);
    }

    std::string tag_list("");
    if (tags) {
        tag_list = to_string(tags);
    }

    toggl::TimeEntry *te = app(context)->Start(
        desc,
        dur,
        task_id,
        project_id,
        p_guid,
        tag_list,
        prevent_on_app);
    if (te) {
        return copy_string(te->GUID());
    }
    return nullptr;
}

bool_t toggl_continue(
    void *context,
    const char_t *guid) {

    std::stringstream ss;
    ss << "toggl_continue guid=" << guid;
    logger().debug(ss.str());

    toggl::TimeEntry *result = app(context)->Continue(to_string(guid));
    if (!result) {
        return false;
    }

    return true;
}

void toggl_view_time_entry_list(void *context) {
    app(context)->OpenTimeEntryList();
}

void toggl_edit(
    void *context,
    const char_t *guid,
    const bool_t edit_running_entry,
    const char_t *focused_field_name) {

    std::stringstream ss;
    ss << "toggl_edit guid=" << guid
       << ", edit_running_entry = " << edit_running_entry
       << ", focused_field_name = " << focused_field_name;
    logger().debug(ss.str());

    app(context)->OpenTimeEntryEditor(
        to_string(guid),
        edit_running_entry,
        to_string(focused_field_name));
}

void toggl_edit_preferences(void *context) {
    app(context)->OpenSettings();
}

bool_t toggl_continue_latest(
    void *context,
    const bool_t prevent_on_app) {

    logger().debug("toggl_continue_latest");

    toggl::TimeEntry *result = app(context)->ContinueLatest(prevent_on_app);
    if (!result) {
        return false;
    }
    return true;
}

bool_t toggl_delete_time_entry(
    void *context,
    const char_t *guid) {

    std::stringstream ss;
    ss << "toggl_delete_time_entry guid=" << guid;
    logger().debug(ss.str());

    return toggl::noError == app(context)->
           DeleteTimeEntryByGUID(to_string(guid));
}

bool_t toggl_set_time_entry_duration(
    void *context,
    const char_t *guid,
    const char_t *value) {

    std::stringstream ss;
    ss  << "toggl_set_time_entry_duration guid=" << guid
        << ", value=" << value;
    logger().debug(ss.str());

    return toggl::noError == app(context)->SetTimeEntryDuration(
        to_string(guid),
        to_string(value));
}

bool_t toggl_set_time_entry_project(
    void *context,
    const char_t *guid,
    const uint64_t task_id,
    const uint64_t project_id,
    const char_t *project_guid) {
    return toggl::noError == app(context)->SetTimeEntryProject(
        to_string(guid),
        task_id,
        project_id,
        to_string(project_guid));
}

bool_t toggl_set_time_entry_date(
    void *context,
    const char_t *guid,
    const int64_t unix_timestamp) {
    return toggl::noError == app(context)->
           SetTimeEntryDate(to_string(guid), unix_timestamp);
}

bool_t toggl_set_time_entry_start(
    void *context,
    const char_t *guid,
    const char_t *value) {
    return toggl::noError == app(context)->
           SetTimeEntryStart(to_string(guid), to_string(value));
}

bool_t toggl_set_time_entry_end(
    void *context,
    const char_t *guid,
    const char_t *value) {
    return toggl::noError == app(context)->
           SetTimeEntryStop(to_string(guid), to_string(value));
}

bool_t toggl_set_time_entry_tags(
    void *context,
    const char_t *guid,
    const char_t *value) {
    return toggl::noError == app(context)->SetTimeEntryTags(to_string(guid),
            to_string(value));
}

bool_t toggl_set_time_entry_billable(
    void *context,
    const char_t *guid,
    const bool_t value) {
    return toggl::noError == app(context)->
           SetTimeEntryBillable(to_string(guid), value);
}

bool_t toggl_set_time_entry_description(
    void *context,
    const char_t *guid,
    const char_t *value) {
    return toggl::noError == app(context)->
           SetTimeEntryDescription(to_string(guid), to_string(value));
}

bool_t toggl_stop(
    void *context,
    const bool_t prevent_on_app) {
    return toggl::noError == app(context)->Stop(prevent_on_app);
}

bool_t toggl_discard_time_at(
    void *context,
    const char_t *guid,
    const uint64_t at,
    const bool_t split_into_new_entry) {

    if (!guid) {
        logger().error("Cannot discard time without GUID");
        return false;
    }

    if (!at) {
        logger().error("Cannot discard time without a timestamp");
        return false;
    }

    logger().debug("toggl_discard_time_at");

    return toggl::noError == app(context)->DiscardTimeAt(
        to_string(guid),
        at,
        split_into_new_entry);
}

bool_t toggl_discard_time_and_continue(
    void *context,
    const char_t *guid,
    const uint64_t at) {

    if (!at) {
        logger().error("Cannot discard time without a timestamp");
        return false;
    }

    toggl::TimeEntry *result = app(context)->DiscardTimeAndContinue(
        to_string(guid),
        at);

    if (!result) {
        return false;
    }

    return true;
}

bool_t toggl_timeline_toggle_recording(
    void *context,
    const bool_t record_timeline) {
    return toggl::noError == app(context)->
           ToggleTimelineRecording(record_timeline);
}

bool_t toggl_timeline_is_recording_enabled(
    void *context) {
    return app(context)->IsTimelineRecordingEnabled();
}

bool_t toggl_feedback_send(
    void *context,
    const char_t *topic,
    const char_t *details,
    const char_t *filename) {

    toggl::Feedback feedback;
    feedback.SetSubject(to_string(topic));
    feedback.SetDetails(to_string(details));

    if (filename != NULL) {
        // Check image size (max 5mb)
        std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);

        if(file.is_open())
        {
            long long size = file.tellg();
            file.close();

            if (size > kMaxFileSize) {
                // Filesize too big
                return false;
            }
            feedback.SetAttachmentPath(to_string(filename));
        }
    }

    return toggl::noError == app(context)->SendFeedback(feedback);
}

void toggl_search_help_articles(
    void *context,
    const char_t *keywords) {
    app(context)->SearchHelpArticles(to_string(keywords));
}

bool_t toggl_set_default_project(
    void *context,
    const uint64_t pid,
    const uint64_t tid) {
    return toggl::noError == app(context)->SetDefaultProject(pid, tid);
}

// result is passed by toggl_on_project_colors
void toggl_get_project_colors(
    void *context) {
    app(context)->UI()->DisplayProjectColors();
}

void toggl_get_countries(void *context) {
    app(context)->PullCountries();
}

// Close/Open Entries Group
void toggl_toggle_entries_group(void *context,
                                const char_t *name) {
    app(context)->ToggleEntriesGroup(to_string(name));
}

char_t *toggl_get_default_project_name(
    void *context) {
    std::string name("");
    app(context)->DefaultProjectName(&name);
    if (name.empty()) {
        return nullptr;
    }
    return copy_string(name);
}

uint64_t toggl_get_default_project_id(
    void *context) {
    Poco::UInt64 ret(0);
    app(context)->DefaultPID(&ret);
    return ret;
}

uint64_t toggl_get_default_task_id(
    void *context) {
    Poco::UInt64 ret(0);
    app(context)->DefaultTID(&ret);
    return ret;
}

bool_t toggl_set_update_channel(
    void *context,
    const char_t *update_channel) {
    return toggl::noError == app(context)->
           SetUpdateChannel(to_string(update_channel));
}

char_t *toggl_get_update_channel(
    void *context) {
    std::string update_channel("");
    app(context)->UpdateChannel(&update_channel);
    return copy_string(update_channel);
}

char_t *toggl_get_user_fullname(
    void *context) {
    return copy_string(app(context)->UserFullName());
}

char_t *toggl_get_user_email(
    void *context) {
    return copy_string(app(context)->UserEmail());
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

void toggl_on_help_articles(
    void *context,
    TogglDisplayHelpArticles cb) {
    app(context)->UI()->OnDisplayHelpArticles(cb);
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

void toggl_on_overlay(
    void *context,
    TogglDisplayOverlay cb) {
    app(context)->UI()->OnDisplayOverlay(cb);
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

void toggl_on_update_download_state(
    void *context,
    TogglDisplayUpdateDownloadState cb) {
    app(context)->UI()->OnDisplayUpdateDownloadState(cb);
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

void toggl_on_pomodoro(
    void *context,
    TogglDisplayPomodoro cb) {
    app(context)->UI()->OnDisplayPomodoro(cb);
}

void toggl_on_pomodoro_break(
    void *context,
    TogglDisplayPomodoroBreak cb) {
    app(context)->UI()->OnDisplayPomodoroBreak(cb);
}

void toggl_on_autotracker_notification(
    void *context,
    TogglDisplayAutotrackerNotification cb) {
    app(context)->UI()->OnDisplayAutotrackerNotification(cb);
}

void toggl_on_promotion(
    void *context,
    TogglDisplayPromotion cb) {
    app(context)->UI()->OnDisplayPromotion(cb);
}

void toggl_on_obm_experiment(
    void *context,
    TogglDisplayObmExperiment cb) {
    app(context)->UI()->OnDisplayObmExperiment(cb);
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

void toggl_os_shutdown(void *context) {
    if (!context) {
        return;
    }
    app(context)->osShutdown();
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

bool_t toggl_accept_tos(void *context) {
    return toggl::noError == app(context)->ToSAccept();
}

void toggl_get_support(void *context, const int type) {
    if (type == 1) {
        app(context)->UI()->DisplayURL(kMacSupportURL);
    } else if (type == 2) {
        app(context)->UI()->DisplayURL(kLinuxSupportURL);
    } else {
        app(context)->UI()->DisplayURL(kGeneralSupportURL);
    }
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

void toggl_on_autotracker_rules(
    void *context,
    TogglDisplayAutotrackerRules cb) {
    app(context)->UI()->OnDisplayAutotrackerRules(cb);
}

void toggl_on_project_colors(
    void *context,
    TogglDisplayProjectColors cb) {
    app(context)->UI()->OnDisplayProjectColors(cb);
}

void toggl_on_countries(
    void *context,
    TogglDisplayCountries cb) {
    app(context)->UI()->OnDisplayCountries(cb);
}

void toggl_debug(const char_t *text) {
    std::cout << to_string(text) << std::endl;
    logger().debug(to_string(text));
}

char_t *toggl_check_view_struct_size(
    const int time_entry_view_item_size,
    const int autocomplete_view_item_size,
    const int view_item_size,
    const int settings_size,
    const int autotracker_view_item_size) {
    int size = sizeof(TogglTimeEntryView);
    if (time_entry_view_item_size != size) {
        std::stringstream ss;
        ss << "TogglTimeEntryView expected=" << size
           <<" actual=" << time_entry_view_item_size;
        return copy_string(ss.str());
    }
    size = sizeof(TogglAutocompleteView);
    if (autocomplete_view_item_size != size) {
        std::stringstream ss;
        ss << "TogglAutocompleteView expected=" << size
           <<" actual=" << autocomplete_view_item_size;
        return copy_string(ss.str());
    }
    size = sizeof(TogglGenericView);
    if (view_item_size != size) {
        std::stringstream ss;
        ss << "TogglGenericView expected=" << size
           << " actual=" << view_item_size;
        return copy_string(ss.str());
    }
    size = sizeof(TogglSettingsView);
    if (settings_size != size) {
        std::stringstream ss;
        ss << "TogglSettingsView expected=" << size
           << " actual=" << settings_size;
        return copy_string(ss.str());
    }
    size = sizeof(TogglAutotrackerRuleView);
    if (autotracker_view_item_size != size) {
        std::stringstream ss;
        ss << "TogglAutotrackerRuleView expected=" << size
           << " actual=" << autotracker_view_item_size;
        return copy_string(ss.str());
    }
    return nullptr;
}

void toggl_set_idle_seconds(
    void *context,
    const uint64_t idle_seconds) {
    if (context) {
        app(context)->SetIdleSeconds(idle_seconds);
    }
}

bool_t toggl_set_promotion_response(
    void *context,
    const int64_t promotion_type,
    const int64_t promotion_response) {
    return toggl::noError == app(context)->SetPromotionResponse(
        promotion_type, promotion_response);
}

char_t *toggl_run_script(
    void *context,
    const char_t* script,
    int64_t *err) {

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    toggl_register_lua(context, L);
    lua_settop(L, 0);

    *err = luaL_loadstring(L, to_string(script).c_str());
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

int64_t toggl_autotracker_add_rule(
    void *context,
    const char_t *term,
    const uint64_t project_id,
    const uint64_t task_id) {
    Poco::Int64 rule_id(0);
    app(context)->AddAutotrackerRule(
        to_string(term),
        project_id,
        task_id,
        &rule_id);
    return rule_id;
}

bool_t toggl_autotracker_delete_rule(
    void *context,
    const int64_t id) {
    return toggl::noError == app(context)->DeleteAutotrackerRule(id);
}

void testing_sleep(
    const int seconds) {
    Poco::Thread::sleep(seconds*1000);
}

bool_t testing_set_logged_in_user(
    void *context,
    const char *json) {
    toggl::Context *ctx = reinterpret_cast<toggl::Context *>(context);
    return toggl::noError == ctx->SetLoggedInUserFromJSON(std::string(json));
}

void toggl_set_compact_mode(
    void *context,
    const bool_t value) {
    app(context)->SetCompactMode(value);
}

bool_t toggl_get_compact_mode(
    void *context) {
    return app(context)->GetCompactMode();
}

void toggl_set_keep_end_time_fixed(
    void *context,
    const bool_t value) {
    app(context)->SetKeepEndTimeFixed(value);
}

bool_t toggl_get_keep_end_time_fixed(
    void *context) {
    return app(context)->GetKeepEndTimeFixed();
}


void toggl_set_mini_timer_x(
    void *context,
    const int64_t value) {
    app(context)->SetMiniTimerX(value);
}

int64_t toggl_get_mini_timer_x(
    void *context) {
    return app(context)->GetMiniTimerX();
}

void toggl_set_mini_timer_y(
    void *context,
    const int64_t value) {
    app(context)->SetMiniTimerY(value);
}

int64_t toggl_get_mini_timer_y(
    void *context) {
    return app(context)->GetMiniTimerY();
}

void toggl_set_mini_timer_w(
    void *context,
    const int64_t value) {
    app(context)->SetMiniTimerW(value);
}

int64_t toggl_get_mini_timer_w(
    void *context) {
    return app(context)->GetMiniTimerW();
}

void toggl_set_mini_timer_visible(
    void *context,
    const bool_t value) {
    app(context)->SetMiniTimerVisible(value);
}

bool_t toggl_get_mini_timer_visible(
    void *context) {
    return app(context)->GetMiniTimerVisible();
}

void toggl_load_more(void* context) {
    app(context)->LoadMore();
}

void track_window_size(void *context,
                       const uint64_t width,
                       const uint64_t height) {
    if (!context) {
        return;
    }
    app(context)->TrackWindowSize(width, height);
}
