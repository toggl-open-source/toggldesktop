// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_LIB_INCLUDE_TOGGL_API_LUA_H_
#define SRC_LIB_INCLUDE_TOGGL_API_LUA_H_

#include "lua.h"  // NOLINT
#include "lualib.h"  // NOLINT
#include "lauxlib.h"  // NOLINT

#include "./toggl_api.h"

// FIXME: cant we keep it somehow better, in lua state, perhaps?
static void *app = 0;

static int l_toggl_environment(lua_State *L) {
    char *str = toggl_environment(app);
    lua_pushstring(L, str);
    free(str);
    return 1;
}

static int l_toggl_set_environment(lua_State *L) {
    toggl_set_environment(app,
                          luaL_checkstring(L, -1));
    return 0;
}

static int l_toggl_disable_update_check(lua_State *L) {
    toggl_disable_update_check(app);
    return 0;
}

static int l_toggl_set_cacert_path(lua_State *L) {
    toggl_set_cacert_path(app,
                          luaL_checkstring(L, -1));
    return 0;
}

static int l_toggl_set_db_path(lua_State *L) {
    toggl_set_db_path(app,
                      luaL_checkstring(L, -1));
    return 0;
}

static int l_toggl_set_log_path(lua_State *L) {
    toggl_set_log_path(
        luaL_checkstring(L, -1));
    return 0;
}

static int l_toggl_set_log_level(lua_State *L) {
    toggl_set_log_level(
        luaL_checkstring(L, -1));
    return 0;
}

static int l_toggl_set_api_url(lua_State *L) {
    toggl_set_api_url(app,
                      luaL_checkstring(L, -1));
    return 0;
}

static int l_toggl_set_websocket_url(lua_State *L) {
    toggl_set_websocket_url(app,
                            luaL_checkstring(L, -1));
    return 0;
}

static int l_toggl_show_app(lua_State *L) {
    toggl_show_app(app);
    return 0;
}

static int l_toggl_login(lua_State *L) {
    _Bool res = toggl_login(app,
                            luaL_checkstring(L, 1),
                            luaL_checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_google_login(lua_State *L) {
    _Bool res = toggl_google_login(app,
                                   luaL_checkstring(L, -1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_password_forgot(lua_State *L) {
    toggl_password_forgot(app);
    return 0;
}

static int l_toggl_open_in_browser(lua_State *L) {
    toggl_open_in_browser(app);
    return 0;
}

static int l_toggl_get_support(lua_State *L) {
    toggl_get_support(app);
    return 0;
}

static int l_toggl_feedback_send(lua_State *L) {
    _Bool res = toggl_feedback_send(app,
                                    luaL_checkstring(L, 1),
                                    luaL_checkstring(L, 2),
                                    luaL_checkstring(L, 3));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_about(lua_State *L) {
    toggl_about(app);
    return 0;
}

static int l_toggl_view_time_entry_list(lua_State *L) {
    toggl_view_time_entry_list(app);
    return 0;
}

static int l_toggl_edit(lua_State *L) {
    toggl_edit(app,
               luaL_checkstring(L, 1),
               lua_toboolean(L, 2),
               luaL_checkstring(L, 3));
    return 0;
}

static int l_toggl_edit_preferences(lua_State *L) {
    toggl_edit_preferences(app);
    return 0;
}

static int l_toggl_continue(lua_State *L) {
    _Bool res = toggl_continue(app,
                               luaL_checkstring(L, -1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_continue_latest(lua_State *L) {
    _Bool res = toggl_continue_latest(app);
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_delete_time_entry(lua_State *L) {
    _Bool res = toggl_delete_time_entry(app,
                                        luaL_checkstring(L, -1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_duration(lua_State *L) {
    _Bool res = toggl_set_time_entry_duration(app,
                luaL_checkstring(L, 1),
                luaL_checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_project(lua_State *L) {
    _Bool res = toggl_set_time_entry_project(app,
                luaL_checkstring(L, 1),
                lua_tointeger(L, 2),
                lua_tointeger(L, 3),
                luaL_checkstring(L, 4));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_date(lua_State *L) {
    _Bool res = toggl_set_time_entry_date(app,
                                          luaL_checkstring(L, 1),
                                          lua_tointeger(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_start(lua_State *L) {
    _Bool res = toggl_set_time_entry_start(app,
                                           luaL_checkstring(L, 1),
                                           luaL_checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_end(lua_State *L) {
    _Bool res = toggl_set_time_entry_end(app,
                                         luaL_checkstring(L, 1),
                                         luaL_checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_tags(lua_State *L) {
    _Bool res = toggl_set_time_entry_tags(app,
                                          luaL_checkstring(L, 1),
                                          luaL_checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_billable(lua_State *L) {
    _Bool res = toggl_set_time_entry_billable(app,
                luaL_checkstring(L, 1),
                lua_toboolean(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_time_entry_description(lua_State *L) {
    _Bool res = toggl_set_time_entry_description(app,
                luaL_checkstring(L, 1),
                luaL_checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_stop(lua_State *L) {
    _Bool res = toggl_stop(app);
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_discard_time_at(lua_State *L) {
    _Bool res = toggl_discard_time_at(app,
                                      luaL_checkstring(L, 1),
                                      lua_tointeger(L, 2),
                                      lua_toboolean(L, 3));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_settings(lua_State *L) {
    _Bool res = toggl_set_settings(app,
                                   lua_toboolean(L, 1),
                                   lua_toboolean(L, 2),
                                   lua_toboolean(L, 3),
                                   lua_toboolean(L, 4),
                                   lua_toboolean(L, 5),
                                   lua_tointeger(L, 6));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_proxy_settings(lua_State *L) {
    _Bool res = toggl_set_proxy_settings(app,
                                         lua_toboolean(L, 1),
                                         luaL_checkstring(L, 2),
                                         lua_tointeger(L, 3),
                                         luaL_checkstring(L, 4),
                                         luaL_checkstring(L, 5));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_logout(lua_State *L) {
    _Bool res = toggl_logout(app);
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_clear_cache(lua_State *L) {
    _Bool res = toggl_clear_cache(app);
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_start(lua_State *L) {
    char *guid = toggl_start(app,
                             luaL_checkstring(L, 1),
                             luaL_checkstring(L, 2),
                             lua_tointeger(L, 3),
                             lua_tointeger(L, 4));
    lua_pushstring(L, guid);
    free(guid);
    return 1;
}

static int l_toggl_add_project(lua_State *L) {
    _Bool res = toggl_add_project(app,
                                  luaL_checkstring(L, 1),
                                  lua_tointeger(L, 2),
                                  lua_tointeger(L, 3),
                                  luaL_checkstring(L, 4),
                                  lua_toboolean(L, 5));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_create_project(lua_State *L) {
    _Bool res = toggl_create_project(app,
                                     lua_tointeger(L, 1),
                                     lua_tointeger(L, 2),
                                     luaL_checkstring(L, 3),
                                     lua_toboolean(L, 4));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_create_client(lua_State *L) {
    _Bool res = toggl_create_client(app,
                                    lua_tointeger(L, 1),
                                    luaL_checkstring(L, 2));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_update_channel(lua_State *L) {
    _Bool res = toggl_set_update_channel(app,
                                         luaL_checkstring(L, -1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_get_update_channel(lua_State *L) {
    char_t *str = toggl_get_update_channel(app);
    lua_pushstring(L, str);
    return 1;
}

static int l_toggl_sync(lua_State *L) {
    toggl_sync(app);
    return 0;
}

static int l_toggl_timeline_toggle_recording(lua_State *L) {
    _Bool res = toggl_timeline_toggle_recording(app,
                lua_toboolean(L, 1));
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_timeline_is_recording_enabled(lua_State *L) {
    _Bool res = toggl_timeline_is_recording_enabled(app);
    lua_pushboolean(L, res);
    return 1;
}

static int l_toggl_set_sleep(lua_State *L) {
    toggl_set_sleep(app);
    return 0;
}

static int l_toggl_set_wake(lua_State *L) {
    toggl_set_wake(app);
    return 0;
}

static int l_toggl_set_online(lua_State *L) {
    toggl_set_online(app);
    return 0;
}

static int l_toggl_set_idle_seconds(lua_State *L) {
    toggl_set_idle_seconds(app,
                           lua_tointeger(L, -1));
    return 0;
}

static int l_toggl_format_tracking_time_duration(lua_State *L) {
    char_t *str = toggl_format_tracking_time_duration(
        lua_tointeger(L, -1));
    lua_pushstring(L, str);
    free(str);
    return 1;
}

static int l_toggl_format_tracked_time_duration(lua_State *L) {
    char_t *str = toggl_format_tracked_time_duration(
        lua_tointeger(L, -1));
    lua_pushstring(L, str);
    free(str);
    return 1;
}

static int l_toggl_debug(lua_State *L) {
    toggl_debug(
        luaL_checkstring(L, -1));
    return 0;
}

static const struct luaL_Reg toggl_f[] = {
    {"set_environment", l_toggl_set_environment},
    {"environment", l_toggl_environment},
    {"disable_update_check", l_toggl_disable_update_check},
    {"set_cacert_path", l_toggl_set_cacert_path},
    {"set_db_path", l_toggl_set_db_path},
    {"set_log_path", l_toggl_set_log_path},
    {"set_log_level", l_toggl_set_log_level},
    {"set_api_url", l_toggl_set_api_url},
    {"set_websocket_url", l_toggl_set_websocket_url},
    {"show_app", l_toggl_show_app},
    {"login", l_toggl_login},
    {"google_login", l_toggl_google_login},
    {"password_forgot", l_toggl_password_forgot},
    {"open_in_browser", l_toggl_open_in_browser},
    {"get_support", l_toggl_get_support},
    {"feedback_send", l_toggl_feedback_send},
    {"about", l_toggl_about},
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
    {"set_settings", l_toggl_set_settings},
    {"set_proxy_settings", l_toggl_set_proxy_settings},
    {"logout", l_toggl_logout},
    {"clear_cache", l_toggl_clear_cache},
    {"start", l_toggl_start},
    {"add_project", l_toggl_add_project},
    {"create_project", l_toggl_create_project},
    {"create_client", l_toggl_create_client},
    {"set_update_channel", l_toggl_set_update_channel},
    {"get_update_channel", l_toggl_get_update_channel},
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
    {NULL, NULL}
};

static int luaopen_toggl(lua_State *L) {
    luaL_newlibtable(L, toggl_f);
    luaL_setfuncs(L, toggl_f, 0);
    return 1;
}

static int toggl_register_lua(void *ctx, lua_State *L) {
    // FIXME: can't we keep it in lua state instead?
    app = ctx;

    luaL_requiref(L, "toggl", luaopen_toggl, 1);
    return 1;
}

#endif  // SRC_LIB_INCLUDE_TOGGL_API_LUA_H_
