// Copyright 2015 Toggl Desktop developers.

#include <vector>

#include "gtest/gtest.h"

#include "./../lib/include/toggl_api_lua.h"
#include "./lua.h"
#include "./lualib.h"

namespace toggl {

TEST(UITest, String) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    void *app_state = 0;
    toggl_register_lua(app_state, L);
    lua_settop(L, 0);

    ASSERT_TRUE(true);  // FIXME: load Lua test script here

    lua_close(L);
}

}  // namespace toggl
