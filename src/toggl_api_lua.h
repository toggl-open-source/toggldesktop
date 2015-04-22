// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TOGGL_API_LUA_H_
#define SRC_TOGGL_API_LUA_H_

#include "./toggl_api.h"

#include <lua.hpp>

#ifdef _WIN32
#include <ustring.h>
#endif

#include <cstdlib>

static void *toggl_app_instance_ = 0;

void pushstring(lua_State *L, char_t *str) {
#ifdef _WIN32
    push_utf8_string(L, str);
#else
    lua_pushstring(L, str);
#endif
}

const char_t *checkstring(lua_State *L, int pos) {
#ifdef _WIN32
    return check_utf8_string(L, pos);
#else
    return luaL_checkstring(L, pos);
#endif
}

static int l_toggl_environment(lua_State *L) {
    char_t *str = toggl_environment(toggl_app_instance_);
    pushstring(L, str);
    free(str);
    return 1;
}

static int l_toggl_set_environment(lua_State *L) {
    toggl_set_environment(toggl_app_instance_,
                          checkstring(L, -1));
    return 0;
}

static int l_toggl_disable_update_check(lua_State *L) {
    toggl_disable_update_check(toggl_app_instance_);
    return 0;
}

static int l_toggl_set_cacert_path(lua_State *L) {
    toggl_set_cacert_path(toggl_app_instance_,
                          checkstring(L, -1));
    return 0;
}

static int l_toggl_set_db_path(lua_State *L) {
    toggl_set_db_path(toggl_app_instance_,
                      checkstring(L, -1));
    return 0;
}

static int l_toggl_set_log_path(lua_State *L) {
    toggl_set_log_path(
        checkstring(L, -1));
    return 0;
}

static int l_toggl_set_log_level(lua_State *L) {
    toggl_set_log_level(
        checkstring(L, -1));
    return 0;
}

static int l_toggl_show_app(lua_State *L) {
    toggl_show_app(toggl_app_instance_);
    return 0;
}

static int l_toggl_login(lua_State *L) {
    _Bool res = toggl_login(toggl_app_instance_,
                            checkstring(L, 1),
                            checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_signup(lua_State *L) {
    _Bool res = toggl_signup(toggl_app_instance_,
                             checkstring(L, 1),
                             checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_google_login(lua_State *L) {
    _Bool res = toggl_google_login(toggl_app_instance_,
                                   checkstring(L, -1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_password_forgot(lua_State *L) {
    toggl_password_forgot(toggl_app_instance_);
    return 0;
}

static int l_toggl_open_in_browser(lua_State *L) {
    toggl_open_in_browser(toggl_app_instance_);
    return 0;
}

static int l_toggl_get_support(lua_State *L) {
    toggl_get_support(toggl_app_instance_);
    return 0;
}

static int l_toggl_feedback_send(lua_State *L) {
    _Bool res = toggl_feedback_send(toggl_app_instance_,
                                    checkstring(L, 1),
                                    checkstring(L, 2),
                                    checkstring(L, 3));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_view_time_entry_list(lua_State *L) {
    toggl_view_time_entry_list(toggl_app_instance_);
    return 0;
}

static int l_toggl_edit(lua_State *L) {
    toggl_edit(toggl_app_instance_,
               checkstring(L, 1),
               lua_toboolean(L, 2),
               checkstring(L, 3));
    return 0;
}

static int l_toggl_edit_preferences(lua_State *L) {
    toggl_edit_preferences(toggl_app_instance_);
    return 0;
}

static int l_toggl_continue(lua_State *L) {
    _Bool res = toggl_continue(toggl_app_instance_,
                               checkstring(L, -1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_continue_latest(lua_State *L) {
    _Bool res = toggl_continue_latest(toggl_app_instance_);
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_delete_time_entry(lua_State *L) {
    _Bool res = toggl_delete_time_entry(toggl_app_instance_,
                                        checkstring(L, -1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_duration(lua_State *L) {
    _Bool res = toggl_set_time_entry_duration(toggl_app_instance_,
                checkstring(L, 1),
                checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_project(lua_State *L) {
    _Bool res = toggl_set_time_entry_project(toggl_app_instance_,
                checkstring(L, 1),
                lua_tointeger(L, 2),
                lua_tointeger(L, 3),
                checkstring(L, 4));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_date(lua_State *L) {
    _Bool res = toggl_set_time_entry_date(toggl_app_instance_,
                                          checkstring(L, 1),
                                          lua_tointeger(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_start(lua_State *L) {
    _Bool res = toggl_set_time_entry_start(toggl_app_instance_,
                                           checkstring(L, 1),
                                           checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_end(lua_State *L) {
    _Bool res = toggl_set_time_entry_end(toggl_app_instance_,
                                         checkstring(L, 1),
                                         checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_tags(lua_State *L) {
    _Bool res = toggl_set_time_entry_tags(toggl_app_instance_,
                                          checkstring(L, 1),
                                          checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_billable(lua_State *L) {
    _Bool res = toggl_set_time_entry_billable(toggl_app_instance_,
                checkstring(L, 1),
                lua_toboolean(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_description(lua_State *L) {
    _Bool res = toggl_set_time_entry_description(toggl_app_instance_,
                checkstring(L, 1),
                checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_stop(lua_State *L) {
    _Bool res = toggl_stop(toggl_app_instance_);
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_discard_time_at(lua_State *L) {
    _Bool res = toggl_discard_time_at(toggl_app_instance_,
                                      checkstring(L, 1),
                                      lua_tointeger(L, 2),
                                      lua_toboolean(L, 3));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_settings_use_idle_detection(lua_State *L) {
    _Bool res = toggl_set_settings_use_idle_detection(toggl_app_instance_,
                lua_toboolean(L, 1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_settings_menubar_timer(lua_State *L) {
    _Bool res = toggl_set_settings_menubar_timer(toggl_app_instance_,
                lua_toboolean(L, 1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_settings_dock_icon(lua_State *L) {
    _Bool res = toggl_set_settings_dock_icon(toggl_app_instance_,
                lua_toboolean(L, 1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_settings_on_top(lua_State *L) {
    _Bool res = toggl_set_settings_on_top(toggl_app_instance_,
                                          lua_toboolean(L, 1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_settings_reminder(lua_State *L) {
    _Bool res = toggl_set_settings_reminder(toggl_app_instance_,
                                            lua_toboolean(L, 1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_settings_idle_minutes(lua_State *L) {
    _Bool res = toggl_set_settings_idle_minutes(toggl_app_instance_,
                lua_toboolean(L, 1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_settings_focus_on_shortcut(lua_State *L) {
    _Bool res = toggl_set_settings_focus_on_shortcut(toggl_app_instance_,
                lua_toboolean(L, 1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_settings_reminder_minutes(lua_State *L) {
    _Bool res = toggl_set_settings_reminder_minutes(toggl_app_instance_,
                lua_toboolean(L, 1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_proxy_settings(lua_State *L) {
    _Bool res = toggl_set_proxy_settings(toggl_app_instance_,
                                         lua_toboolean(L, 1),
                                         checkstring(L, 2),
                                         lua_tointeger(L, 3),
                                         checkstring(L, 4),
                                         checkstring(L, 5));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_logout(lua_State *L) {
    _Bool res = toggl_logout(toggl_app_instance_);
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_clear_cache(lua_State *L) {
    _Bool res = toggl_clear_cache(toggl_app_instance_);
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_start(lua_State *L) {
    char_t *guid = toggl_start(toggl_app_instance_,
                               checkstring(L, 1),
                               checkstring(L, 2),
                               lua_tointeger(L, 3),
                               lua_tointeger(L, 4));
    pushstring(L, guid);
    free(guid);
    return 1;
}

static int l_toggl_add_project(lua_State *L) {
    _Bool res = toggl_add_project(toggl_app_instance_,
                                  checkstring(L, 1),
                                  lua_tointeger(L, 2),
                                  lua_tointeger(L, 3),
                                  checkstring(L, 4),
                                  lua_toboolean(L, 5));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_create_project(lua_State *L) {
    _Bool res = toggl_create_project(toggl_app_instance_,
                                     lua_tointeger(L, 1),
                                     lua_tointeger(L, 2),
                                     checkstring(L, 3),
                                     lua_toboolean(L, 4));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_create_client(lua_State *L) {
    _Bool res = toggl_create_client(toggl_app_instance_,
                                    lua_tointeger(L, 1),
                                    checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_update_channel(lua_State *L) {
    _Bool res = toggl_set_update_channel(toggl_app_instance_,
                                         checkstring(L, -1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_get_update_channel(lua_State *L) {
    char_t *str = toggl_get_update_channel(toggl_app_instance_);
    pushstring(L, str);
    free(str);
    return 1;
}

static int l_toggl_get_user_fullname(lua_State *L) {
    char_t *str = toggl_get_user_fullname(toggl_app_instance_);
    pushstring(L, str);
    free(str);
    return 1;
}

static int l_toggl_get_user_email(lua_State *L) {
    char_t *str = toggl_get_user_email(toggl_app_instance_);
    pushstring(L, str);
    free(str);
    return 1;
}

static int l_toggl_sync(lua_State *L) {
    toggl_sync(toggl_app_instance_);
    return 0;
}

static int l_toggl_timeline_toggle_recording(lua_State *L) {
    _Bool res = toggl_timeline_toggle_recording(toggl_app_instance_,
                lua_toboolean(L, 1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_timeline_is_recording_enabled(lua_State *L) {
    _Bool res = toggl_timeline_is_recording_enabled(toggl_app_instance_);
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_sleep(lua_State *L) {
    toggl_set_sleep(toggl_app_instance_);
    return 0;
}

static int l_toggl_set_wake(lua_State *L) {
    toggl_set_wake(toggl_app_instance_);
    return 0;
}

static int l_toggl_set_online(lua_State *L) {
    toggl_set_online(toggl_app_instance_);
    return 0;
}

static int l_toggl_set_idle_seconds(lua_State *L) {
    toggl_set_idle_seconds(toggl_app_instance_,
                           lua_tointeger(L, -1));
    return 0;
}

static int l_toggl_format_tracking_time_duration(lua_State *L) {
    char_t *str = toggl_format_tracking_time_duration(
        lua_tointeger(L, -1));
    pushstring(L, str);
    free(str);
    return 1;
}

static int l_toggl_format_tracked_time_duration(lua_State *L) {
    char_t *str = toggl_format_tracked_time_duration(
        lua_tointeger(L, -1));
    pushstring(L, str);
    free(str);
    return 1;
}

static int l_toggl_debug(lua_State *L) {
    toggl_debug(
        checkstring(L, -1));
    return 0;
}

static int l_testing_sleep(lua_State *L) {
    testing_sleep(
        static_cast<int>(lua_tointeger(L, -1)));
    return 0;
}

static int l_testing_set_logged_in_user(lua_State *L) {
    _Bool res = testing_set_logged_in_user(toggl_app_instance_,
                                           luaL_checkstring(L, -1));
    lua_pushboolean(L, res);
    return 1;
}

static const struct luaL_Reg toggl_f[] = {
    {"set_environment", l_toggl_set_environment},
    {"environment", l_toggl_environment},
    {"disable_update_check", l_toggl_disable_update_check},
    {"set_cacert_path", l_toggl_set_cacert_path},
    {"set_db_path", l_toggl_set_db_path},
    {"set_log_path", l_toggl_set_log_path},
    {"set_log_level", l_toggl_set_log_level},
    {"show_app", l_toggl_show_app},
    {"login", l_toggl_login},
    {"signup", l_toggl_signup},
    {"google_login", l_toggl_google_login},
    {"password_forgot", l_toggl_password_forgot},
    {"open_in_browser", l_toggl_open_in_browser},
    {"get_support", l_toggl_get_support},
    {"feedback_send", l_toggl_feedback_send},
    {"view_time_entry_list", l_toggl_view_time_entry_list},
    {"edit", l_toggl_edit},
    {"edit_preferences", l_toggl_edit_preferences},
    {"continue", l_toggl_continue},
    {"continue_latest", l_toggl_continue_latest},
    {"delete_time_entry", l_toggl_delete_time_entry},
    {"set_time_entry_duration", l_toggl_set_time_entry_duration},
    {"set_time_entry_project", l_toggl_set_time_entry_project},
    {"set_time_entry_date", l_toggl_set_time_entry_date},
    {"set_time_entry_start", l_toggl_set_time_entry_start},
    {"set_time_entry_end", l_toggl_set_time_entry_end},
    {"set_time_entry_tags", l_toggl_set_time_entry_tags},
    {"set_time_entry_billable", l_toggl_set_time_entry_billable},
    {"set_time_entry_description", l_toggl_set_time_entry_description},
    {"stop", l_toggl_stop},
    {"discard_time_at", l_toggl_discard_time_at},
    {   "set_settings_use_idle_detection",
        l_toggl_set_settings_use_idle_detection
    },
    {"set_settings_menubar_timer", l_toggl_set_settings_menubar_timer},
    {"set_settings_dock_icon", l_toggl_set_settings_dock_icon},
    {"set_settings_on_top", l_toggl_set_settings_on_top},
    {"set_settings_reminder", l_toggl_set_settings_reminder},
    {"set_settings_idle_minutes", l_toggl_set_settings_idle_minutes},
    {"set_settings_focus_on_shortcut", l_toggl_set_settings_focus_on_shortcut},
    {"set_settings_reminder_minutes", l_toggl_set_settings_reminder_minutes},
    {"set_proxy_settings", l_toggl_set_proxy_settings},
    {"logout", l_toggl_logout},
    {"clear_cache", l_toggl_clear_cache},
    {"start", l_toggl_start},
    {"add_project", l_toggl_add_project},
    {"create_project", l_toggl_create_project},
    {"create_client", l_toggl_create_client},
    {"set_update_channel", l_toggl_set_update_channel},
    {"update_channel", l_toggl_get_update_channel},
    {"user_fullname", l_toggl_get_user_fullname},
    {"user_email", l_toggl_get_user_email},
    {"sync", l_toggl_sync},
    {"timeline_toggle_recording", l_toggl_timeline_toggle_recording},
    {"timeline_is_recording_enabled", l_toggl_timeline_is_recording_enabled},
    {"set_sleep", l_toggl_set_sleep},
    {"set_wake", l_toggl_set_wake},
    {"set_online", l_toggl_set_online},
    {"set_idle_seconds", l_toggl_set_idle_seconds},
    {   "toggl_format_tracking_time_duration",
        l_toggl_format_tracking_time_duration
    },
    {   "toggl_format_tracked_time_duration",
        l_toggl_format_tracked_time_duration
    },
    {"debug", l_toggl_debug},
    {"sleep", l_testing_sleep},
    {"set_logged_in_user", l_testing_set_logged_in_user},
    {NULL, NULL}
};

static int luaopen_toggl(lua_State *L) {
    luaL_newlibtable(L, toggl_f);
    luaL_setfuncs(L, toggl_f, 0);
    return 1;
}

static int toggl_register_lua(void *ctx, lua_State *L) {
    toggl_app_instance_ = ctx;

    luaL_requiref(L, "toggl", luaopen_toggl, 1);
    return 1;
}

#endif  // SRC_TOGGL_API_LUA_H_
