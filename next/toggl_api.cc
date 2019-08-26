// Copyright 2019 Toggl Desktop developers.

#include "toggl_api.h"

#include "context.h"

#include <cstring>

using namespace toggl;

char *convert(const std::string &str) {
    return strdup(str.c_str());
}

void *toggl_context_init(const char *app_name, const char *app_version, bool production, bool check_for_updates) {
    if (!app_name || !app_version)
        return nullptr;
    return new Context(app_name, app_version, production, check_for_updates);
}

void toggl_context_clear(void *context) {
    if (context)
        delete reinterpret_cast<Context*>(context);
}

void toggl_set_cacert_path(void *context, const char *path) {
    reinterpret_cast<Context*>(context)->setCacertPath(path);
}

bool toggl_set_db_path(void *context, const char *path) {
    return reinterpret_cast<Context*>(context)->setDbPath(path);
}

void toggl_set_update_path(void *context, const char *path) {
    reinterpret_cast<Context*>(context)->setUpdatePath(path);
}

char *toggl_update_path(void *context) {
    return convert(reinterpret_cast<Context*>(context)->updatePath().c_str());
}

void toggl_set_log_path(void *context, const char *path) {
    reinterpret_cast<Context*>(context)->setLogPath(path);
}

void toggl_set_log_level(void *context, const char *level) {
    reinterpret_cast<Context*>(context)->setLogLevel(level);
}

void toggl_show_app(void *context) {
    reinterpret_cast<Context*>(context)->showApp();
}

void toggl_register_callbacks(void *context, TogglCallbacks callbacks) {
    reinterpret_cast<Context*>(context)->registerCallbacks(callbacks);
}

bool toggl_ui_start(void *context) {
    return reinterpret_cast<Context*>(context)->uiStart();
}

bool toggl_login(void *context, const char *email, const char *password) {
    return reinterpret_cast<Context*>(context)->login(email, password);
}

bool toggl_signup(void *context, const char *email, const char *password, uint64_t country_id) {
    return reinterpret_cast<Context*>(context)->signup(email, password, country_id);
}

bool toggl_google_login(void *context, const char *access_token) {
    return reinterpret_cast<Context*>(context)->googleLogin(access_token);
}

void toggl_password_forgot(void *context) {
    reinterpret_cast<Context*>(context)->passwordForgot();
}

void toggl_tos(void *context) {
    reinterpret_cast<Context*>(context)->tos();
}

void toggl_privacy_policy(void *context) {
    reinterpret_cast<Context*>(context)->privacyPolicy();
}

void toggl_open_in_browser(void *context) {
    reinterpret_cast<Context*>(context)->openInBrowser();
}

bool toggl_accept_tos(void *context) {
    return reinterpret_cast<Context*>(context)->acceptTos();
}

void toggl_get_support(void *context, int32_t type) {
    reinterpret_cast<Context*>(context)->getSupport(type);
}

bool toggl_feedback_send(void *context, const char *topic, const char *details, const char *filename) {
    return reinterpret_cast<Context*>(context)->feedbackSend(topic, details, filename);
}

void toggl_search_help_articles(void *context, const char *keywords) {
    reinterpret_cast<Context*>(context)->searchHelpArticles(keywords);
}

void toggl_view_time_entry_list(void *context) {
    reinterpret_cast<Context*>(context)->viewTimeEntryList();
}

void toggl_edit(void *context, const char *guid, bool edit_running_time_entry, const char *focused_field_name) {
    reinterpret_cast<Context*>(context)->edit(guid, edit_running_time_entry, focused_field_name);
}

void toggl_edit_preferences(void *context) {
    reinterpret_cast<Context*>(context)->editPreferences();
}

bool toggl_continue_time_entry(void *context, const char *guid) {
    return reinterpret_cast<Context*>(context)->continueTimeEntry(guid);
}

bool toggl_continue_latest_time_entry(void *context, bool prevent_on_app) {
    return reinterpret_cast<Context*>(context)->continueLatestTimeEntry(prevent_on_app);
}

bool toggl_delete_time_entry(void *context, const char *guid) {
    return reinterpret_cast<Context*>(context)->deleteTimeEntry(guid);
}

bool toggl_set_time_entry_duration(void *context, const char *guid, const char *value) {
    return reinterpret_cast<Context*>(context)->setTimeEntryDuration(guid, value);
}

bool toggl_set_time_entry_project(void *context, const char *guid, uint64_t task_id, uint64_t project_id, const char *project_guid) {
    return reinterpret_cast<Context*>(context)->setTimeEntryProject(guid, task_id, project_id, project_guid);
}

bool toggl_set_time_entry_date(void *context, const char *guid, int64_t unix_timestamp) {
    return reinterpret_cast<Context*>(context)->setTimeEntryDate(guid, unix_timestamp);
}

bool toggl_set_time_entry_start(void *context, const char *guid, const char *value) {
    return reinterpret_cast<Context*>(context)->setTimeEntryStart(guid, value);
}

bool toggl_set_time_entry_end(void *context, const char *guid, const char *value) {
    return reinterpret_cast<Context*>(context)->setTimeEntryEnd(guid, value);
}

bool toggl_set_time_entry_tags(void *context, const char *guid, const char *value) {
    return reinterpret_cast<Context*>(context)->setTimeEntryTags(guid, value);
}

bool toggl_set_time_entry_billable(void *context, const char *guid, bool value) {
    return reinterpret_cast<Context*>(context)->setTimeEntryBillable(guid, value);
}

bool toggl_set_time_entry_description(void *context, const char *guid, const char *value) {
    return reinterpret_cast<Context*>(context)->setTimeEntryDescription(guid, value);
}

bool toggl_stop(void *context, bool prevent_on_app) {
    return reinterpret_cast<Context*>(context)->stop(prevent_on_app);
}

bool toggl_discard_time_at(void *context, const char *guid, int64_t at, bool split_into_new_entry) {
    return reinterpret_cast<Context*>(context)->discardTimeAt(guid, at, split_into_new_entry);
}

bool toggl_discard_time_and_continue(void *context, const char *guid, int64_t at) {
    return reinterpret_cast<Context*>(context)->discardTimeAndContinue(guid, at);
}

bool toggl_set_settings_remind_days(void *context, bool remind_mon, bool remind_tue, bool remind_wed, bool remind_thu, bool remind_fri, bool remind_sat, bool remind_sun) {
    return reinterpret_cast<Context*>(context)->setSettingsRemindDays(remind_mon, remind_tue, remind_wed, remind_thu, remind_fri, remind_sat, remind_sun);
}

bool toggl_set_settings_remind_times(void *context, const char *remind_starts, const char *remind_ends) {
    return reinterpret_cast<Context*>(context)->setSettingsRemindTimes(remind_starts, remind_ends);
}

bool toggl_set_settings_use_idle_detection(void *context, bool use_idle_detection) {
    return reinterpret_cast<Context*>(context)->setSettingsUseIdleDetection(use_idle_detection);
}

bool toggl_set_settings_autotrack(void *context, bool value) {
    return reinterpret_cast<Context*>(context)->setSettingsAutotrack(value);
}

bool toggl_set_settings_open_editor_on_shortcut(void *context, bool value) {
    return reinterpret_cast<Context*>(context)->setSettingsOpenEditorOnShortcut(value);
}

bool toggl_set_settings_autodetect_proxy(void *context, bool autodetect_proxy) {
    return reinterpret_cast<Context*>(context)->setSettingsAutodetectProxy(autodetect_proxy);
}

bool toggl_set_settings_menubar_timer(void *context, bool menubar_timer) {
    return reinterpret_cast<Context*>(context)->setSettingsMenubarTimer(menubar_timer);
}

bool toggl_set_settings_menubar_project(void *context, bool menubar_project) {
    return reinterpret_cast<Context*>(context)->setSettingsMenubarProject(menubar_project);
}

bool toggl_set_settings_dock_icon(void *context, bool dock_icon) {
    return reinterpret_cast<Context*>(context)->setSettingsDockIcon(dock_icon);
}

bool toggl_set_settings_on_top(void *context, bool on_top) {
    return reinterpret_cast<Context*>(context)->setSettingsOnTop(on_top);
}

bool toggl_set_settings_reminder(void *context, bool reminder) {
    return reinterpret_cast<Context*>(context)->setSettingsReminder(reminder);
}

bool toggl_set_settings_pomodoro(void *context, bool pomodoro) {
    return reinterpret_cast<Context*>(context)->setSettingsPomodoro(pomodoro);
}

bool toggl_set_settings_pomodoro_break(void *context, bool pomodoro_break) {
    return reinterpret_cast<Context*>(context)->setSettingsPomodoroBreak(pomodoro_break);
}

bool toggl_set_settings_stop_entry_on_shutdown_sleep(void *context, bool stop_entry) {
    return reinterpret_cast<Context*>(context)->setSettingsStopEntryOnShutdownSleep(stop_entry);
}

bool toggl_set_settings_idle_minutes(void *context, uint64_t idle_minutes) {
    return reinterpret_cast<Context*>(context)->setSettingsIdleMinutes(idle_minutes);
}

bool toggl_set_settings_focus_on_shortcut(void *context, bool focus_on_shortcut) {
    return reinterpret_cast<Context*>(context)->setSettingsFocusOnShortcut(focus_on_shortcut);
}

bool toggl_set_settings_reminder_minutes(void *context, uint64_t reminder_minutes) {
    return reinterpret_cast<Context*>(context)->setSettingsReminderMinutes(reminder_minutes);
}

bool toggl_set_settings_pomodoro_minutes(void *context, uint64_t pomodoro_minutes) {
    return reinterpret_cast<Context*>(context)->setSettingsPomodoroMinutes(pomodoro_minutes);
}

bool toggl_set_settings_pomodoro_break_minutes(void *context, uint64_t pomodoro_break_minutes) {
    return reinterpret_cast<Context*>(context)->setSettingsPomodoroBreakMinutes(pomodoro_break_minutes);
}

bool toggl_set_settings_manual_mode(void *context, bool manual_mode) {
    return reinterpret_cast<Context*>(context)->setSettingsManualMode(manual_mode);
}

bool toggl_set_proxy_settings(void *context, bool use_proxy, const char *proxy_host, uint64_t proxy_port, const char *proxy_username, const char *proxy_password) {
    return reinterpret_cast<Context*>(context)->setProxySettings(use_proxy, proxy_host, proxy_port, proxy_username, proxy_password);
}

bool toggl_set_window_settings(void *context, int64_t window_x, int64_t window_y, int64_t window_height, int64_t window_width) {
    return reinterpret_cast<Context*>(context)->setWindowSettings(window_x, window_y, window_height, window_width);
}

bool toggl_window_settings(void *context, int64_t *window_x, int64_t *window_y, int64_t *window_height, int64_t *window_width) {
    return reinterpret_cast<Context*>(context)->windowSettings(window_x, window_y, window_height, window_width);
}

void toggl_set_window_maximized(void *context, bool value) {
    reinterpret_cast<Context*>(context)->setWindowMaximized(value);
}

bool toggl_get_window_maximized(void *context) {
    return reinterpret_cast<Context*>(context)->getWindowMaximized();
}

void toggl_set_window_minimized(void *context, bool value) {
    reinterpret_cast<Context*>(context)->setWindowMinimized(value);
}

bool toggl_get_window_minimized(void *context) {
    return reinterpret_cast<Context*>(context)->getWindowMinimized();
}

void toggl_set_window_edit_size_height(void *context, int64_t value) {
    reinterpret_cast<Context*>(context)->setWindowEditSizeHeight(value);
}

int64_t toggl_get_window_edit_size_height(void *context) {
    return reinterpret_cast<Context*>(context)->getWindowEditSizeHeight();
}

void toggl_set_window_edit_size_width(void *context, int64_t value) {
    reinterpret_cast<Context*>(context)->setWindowEditSizeWidth(value);
}

int64_t toggl_get_window_edit_size_width(void *context) {
    return reinterpret_cast<Context*>(context)->getWindowEditSizeWidth();
}

void toggl_set_key_start(void *context, const char *value) {
    reinterpret_cast<Context*>(context)->setKeyStart(value);
}

char *toggl_get_key_start(void *context) {
    return convert(reinterpret_cast<Context*>(context)->getKeyStart());
}

void toggl_set_key_show(void *context, const char *value) {
    reinterpret_cast<Context*>(context)->setKeyShow(value);
}

char *toggl_get_key_show(void *context) {
    return convert(reinterpret_cast<Context*>(context)->getKeyShow());
}

void toggl_set_key_modifier_show(void *context, const char *value) {
    reinterpret_cast<Context*>(context)->setKeyModifierShow(value);
}

char *toggl_get_key_modifier_show(void *context) {
    return convert(reinterpret_cast<Context*>(context)->getKeyModifierShow());
}

void toggl_set_key_modifier_start(void *context, const char *value) {
    reinterpret_cast<Context*>(context)->setKeyModifierStart(value);
}

char *toggl_get_key_modifier_start(void *context) {
    return convert(reinterpret_cast<Context*>(context)->getKeyModifierStart());
}

bool toggl_logout(void *context) {
    return reinterpret_cast<Context*>(context)->logout();
}

bool toggl_clear_cache(void *context) {
    return reinterpret_cast<Context*>(context)->clearCache();
}

char *toggl_start(void *context, const char *description, const char *duration, uint64_t task_id, uint64_t project_id, const char *project_guid, const char *tags, bool prevent_on_app) {
    return convert(reinterpret_cast<Context*>(context)->start(description, duration, task_id, project_id, project_guid, tags, prevent_on_app));
}

char *toggl_add_project(void *context, const char *time_entry_guid, uint64_t workspace_id, uint64_t client_id, const char *client_guid, const char *project_name, bool is_private, const char *project_color) {
    return convert(reinterpret_cast<Context*>(context)->addProject(time_entry_guid, workspace_id, client_id, client_guid, project_name, is_private, project_color));
}

char *toggl_create_client(void *context, uint64_t workspace_id, const char *client_name) {
    return convert(reinterpret_cast<Context*>(context)->createClient(workspace_id, client_name));
}

bool toggl_add_obm_action(void *context, uint64_t experiment_id, const char *key, const char *value) {
    return reinterpret_cast<Context*>(context)->addObmAction(experiment_id, key, value);
}

void toggl_add_obm_experiment_nr(void *context, uint64_t nr) {
    reinterpret_cast<Context*>(context)->addObmExperimentNr(nr);
}

bool toggl_set_default_project(void *context, uint64_t pid, uint64_t tid) {
    return reinterpret_cast<Context*>(context)->setDefaultProject(pid, tid);
}

void toggl_get_project_colors(void *context) {
    reinterpret_cast<Context*>(context)->getProjectColors();
}

void toggl_get_countries(void *context) {
    reinterpret_cast<Context*>(context)->getCountries();
}

char *toggl_get_default_project_name(void *context) {
    return convert(reinterpret_cast<Context*>(context)->getDefaultProjectName());
}

uint64_t toggl_get_default_project_id(void *context) {
    return reinterpret_cast<Context*>(context)->getDefaultProjectId();
}

uint64_t toggl_get_default_task_id(void *context) {
    return reinterpret_cast<Context*>(context)->getDefaultTaskId();
}

bool toggl_set_update_channel(void *context, const char *update_channel) {
    return reinterpret_cast<Context*>(context)->setUpdateChannel(update_channel);
}

char *toggl_get_update_channel(void *context) {
    return convert(reinterpret_cast<Context*>(context)->getUpdateChannel());
}

char *toggl_get_user_fullname(void *context) {
    return convert(reinterpret_cast<Context*>(context)->getUserFullname());
}

char *toggl_get_user_email(void *context) {
    return convert(reinterpret_cast<Context*>(context)->getUserEmail());
}

void toggl_sync(void *context) {
    reinterpret_cast<Context*>(context)->sync();
}

void toggl_fullsync(void *context) {
    reinterpret_cast<Context*>(context)->fullsync();
}

bool toggl_timeline_toggle_recording(void *context, bool record_timeline) {
    return reinterpret_cast<Context*>(context)->timelineToggleRecording(record_timeline);
}

bool toggl_timeline_is_recording_enabled(void *context) {
    return reinterpret_cast<Context*>(context)->timelineIsRecordingEnabled();
}

void toggl_set_sleep(void *context) {
    reinterpret_cast<Context*>(context)->setSleep();
}

void toggl_set_wake(void *context) {
    reinterpret_cast<Context*>(context)->setWake();
}

void toggl_set_locked(void *context) {
    reinterpret_cast<Context*>(context)->setLocked();
}

void toggl_set_unlocked(void *context) {
    reinterpret_cast<Context*>(context)->setUnlocked();
}

void toggl_os_shutdown(void *context) {
    reinterpret_cast<Context*>(context)->osShutdown();
}

void toggl_set_online(void *context) {
    reinterpret_cast<Context*>(context)->setOnline();
}

void toggl_set_idle_seconds(void *context, uint64_t idle_seconds) {
    reinterpret_cast<Context*>(context)->setIdleSeconds(idle_seconds);
}

bool toggl_set_promotion_response(void *context, int64_t promotion_type, int64_t promotion_response) {
    return reinterpret_cast<Context*>(context)->setPromotionResponse(promotion_type, promotion_response);
}

char *toggl_format_tracking_time_duration(int64_t duration_in_seconds) {
    return convert(Context::formatTrackingTimeDuration(duration_in_seconds));
}

char *toggl_format_tracked_time_duration(int64_t duration_in_seconds) {
    return convert(Context::formatTrackedTimeDuration(duration_in_seconds));
}

int64_t toggl_parse_duration_string_into_seconds(void *context, const char *duration_string) {
    return reinterpret_cast<Context*>(context)->parseDurationStringIntoSeconds(duration_string);
}

void toggl_debug(void *context, const char *text) {
    reinterpret_cast<Context*>(context)->debug(text);
}

char *toggl_check_view_struct_size(void *context, int32_t time_entry_view_item_size, int32_t autocomplete_view_item_size, int32_t view_item_size, int32_t settings_size, int32_t autotracker_view_item_size) {
    return convert(reinterpret_cast<Context*>(context)->checkViewStructSize(time_entry_view_item_size, autocomplete_view_item_size, view_item_size, settings_size, autotracker_view_item_size));
}

char *toggl_run_script(void *context, const char *script, int64_t *err) {
    return convert(reinterpret_cast<Context*>(context)->runScript(script, err));
}

int64_t toggl_autotracker_add_rule(void *context, const char *term, uint64_t project_id, uint64_t task_id) {
    return reinterpret_cast<Context*>(context)->autotrackerAddRule(term, project_id, task_id);
}

bool toggl_autotracker_delete_rule(void *context, int64_t id) {
    return reinterpret_cast<Context*>(context)->autotrackerDeleteRule(id);
}

void testing_sleep(void *context, int32_t seconds) {
    reinterpret_cast<Context*>(context)->testingSleep(seconds);
}

bool testing_set_logged_in_user(void *context, const char *json) {
    return reinterpret_cast<Context*>(context)->testingSetLoggedInUser(json);
}

void toggl_set_compact_mode(void *context, bool value) {
    reinterpret_cast<Context*>(context)->setCompactMode(value);
}

bool toggl_get_compact_mode(void *context) {
    return reinterpret_cast<Context*>(context)->getCompactMode();
}

void toggl_set_keep_end_time_fixed(void *context, bool value) {
    reinterpret_cast<Context*>(context)->setKeepEndTimeFixed(value);
}

bool toggl_get_keep_end_time_fixed(void *context) {
    return reinterpret_cast<Context*>(context)->getKeepEndTimeFixed();
}

void toggl_set_mini_timer_x(void *context, int64_t value) {
    reinterpret_cast<Context*>(context)->setMiniTimerX(value);
}

int64_t toggl_get_mini_timer_x(void *context) {
    return reinterpret_cast<Context*>(context)->getMiniTimerX();
}

void toggl_set_mini_timer_y(void *context, int64_t value) {
    reinterpret_cast<Context*>(context)->setMiniTimerY(value);
}

int64_t toggl_get_mini_timer_y(void *context) {
    return reinterpret_cast<Context*>(context)->getMiniTimerY();
}

void toggl_set_mini_timer_w(void *context, int64_t value) {
    reinterpret_cast<Context*>(context)->setMiniTimerW(value);
}

int64_t toggl_get_mini_timer_w(void *context) {
    return reinterpret_cast<Context*>(context)->getMiniTimerW();
}

void toggl_set_mini_timer_visible(void *context, bool value) {
    reinterpret_cast<Context*>(context)->setMiniTimerVisible(value);
}

bool toggl_get_mini_timer_visible(void *context) {
    return reinterpret_cast<Context*>(context)->getMiniTimerVisible();
}

void toggl_load_more(void *context) {
    reinterpret_cast<Context*>(context)->loadMore();
}

void track_window_size(void *context, uint64_t width, uint64_t height) {
    reinterpret_cast<Context*>(context)->trackWindowSize(width, height);
}

void track_edit_size(void *context, uint64_t width, uint64_t height) {
    reinterpret_cast<Context*>(context)->trackEditSize(width, height);
}


void toggl_toggle_entries_group(void *context, const char *name) {
    reinterpret_cast<Context*>(context)->toggleEntriesGroup(name);
}
