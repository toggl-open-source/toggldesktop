
set(OAUTH_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/qt-oauth-lib
    PARENT_SCOPE
)
set(BUGSNAG_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/bugsnag-qt
    PARENT_SCOPE
)

# And include dirs
include_directories(
    qt-oauth-lib
    bugsnag-qt
)

# OAuth has to be bundled
set(OAUTH2_SOURCE_FILES
    qt-oauth-lib/logindialog.cpp
    qt-oauth-lib/oauth2.cpp
)
add_library(OAuth2 SHARED ${OAUTH2_SOURCE_FILES})
target_link_libraries(OAuth2 PRIVATE
    Qt5::Widgets Qt5::WebEngine Qt5::WebEngineWidgets
)
install(TARGETS OAuth2 DESTINATION ${TOGGL_INTERNAL_LIB_DIR})

# Bugsnag has to be bundled
set(BUGSNAG_SOURCE_FILES
    bugsnag-qt/bugsnag.cpp
)
add_library(Bugsnag SHARED ${BUGSNAG_SOURCE_FILES})
target_link_libraries(Bugsnag PRIVATE
    Qt5::Widgets Qt5::Network
)
install(TARGETS Bugsnag DESTINATION ${TOGGL_INTERNAL_LIB_DIR})

if(NOT Poco_FOUND)
    # to avoid that annoying check
    option(DISABLE_CPP14 "" ON)
    # to not build unnecessary clutter
    option(ENABLE_XML "" OFF)
    option(ENABLE_JSON "" OFF)
    option(ENABLE_MONGODB "" OFF)
    option(ENABLE_REDIS "" OFF)
    option(ENABLE_DATA_MYSQL "" OFF)
    option(ENABLE_DATA_ODBC "" OFF)
    option(ENABLE_ZIP "" OFF)
    option(ENABLE_PAGECOMPILER "" OFF)
    option(ENABLE_PAGECOMPILER_FILE2PAGE "" OFF)
    add_subdirectory(poco/)
    set(POCO_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/poco/Crypto/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/poco/Foundation/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/poco/Net/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/poco/NetSSL_OpenSSL/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/poco/Data/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/poco/Data/SQLite/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/poco/Data/SQLite/src/
        ${CMAKE_CURRENT_SOURCE_DIR}/poco/Util/include/
        PARENT_SCOPE
    )
endif()

if(NOT LUA_FOUND)
    set(LUA_SOURCE_FILES
        lua/src/lapi.c
        lua/src/lauxlib.c
        lua/src/lbaselib.c
        lua/src/lbitlib.c
        lua/src/lcode.c
        lua/src/lcorolib.c
        lua/src/lctype.c
        lua/src/ldblib.c
        lua/src/ldebug.c
        lua/src/ldo.c
        lua/src/ldump.c
        lua/src/lfunc.c
        lua/src/lgc.c
        lua/src/linit.c
        lua/src/liolib.c
        lua/src/llex.c
        lua/src/lmathlib.c
        lua/src/lmem.c
        lua/src/loadlib.c
        lua/src/lobject.c
        lua/src/lopcodes.c
        lua/src/loslib.c
        lua/src/lparser.c
        lua/src/lstate.c
        lua/src/lstring.c
        lua/src/lstrlib.c
        lua/src/ltable.c
        lua/src/ltablib.c
        lua/src/ltm.c
        lua/src/luac.c
        lua/src/lundump.c
        lua/src/lutf8lib.c
        lua/src/lvm.c
        lua/src/lzio.c
    )
    add_library(Lua SHARED ${LUA_SOURCE_FILES})
    set(LUA_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/lua/src
        PARENT_SCOPE
    )
    set(LUA_LIBRARIES
        Lua
        PARENT_SCOPE
    )
    install(TARGETS Lua DESTINATION ${TOGGL_INTERNAL_LIB_DIR})
endif()

if(NOT jsoncpp_FOUND)
    set(JSON_SOURCE_FILES
        jsoncpp/src/lib_json/json_reader.cpp
        jsoncpp/src/lib_json/json_value.cpp
        jsoncpp/src/lib_json/json_writer.cpp
    )
    add_library(jsoncpp SHARED ${JSON_SOURCE_FILES})
    target_include_directories(jsoncpp PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/include
    )        
    set(JSONCPP_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/src/lib_json
        PARENT_SCOPE
    )
    set(JSONCPP_LIBRARIES
        jsoncpp
        PARENT_SCOPE
    )
    install(TARGETS jsoncpp DESTINATION ${TOGGL_INTERNAL_LIB_DIR})
endif()

if(NOT QXT_CORE_FOUND)
    set(QXT_SOURCE_FILES
        #qxtglobalshortcut5/gui/qxtglobalshortcut_win.cpp
        #qxtglobalshortcut5/gui/qxtwindowsystem_win.cpp
        #qxtglobalshortcut5/gui/qxtglobalshortcut_mac.cpp
        #qxtglobalshortcut5/gui/qxtwindowsystem_mac.cpp
        qxtglobalshortcut5/gui/qxtwindowsystem.cpp
        qxtglobalshortcut5/gui/qxtglobalshortcut_x11.cpp
        qxtglobalshortcut5/gui/x11info.cpp
        qxtglobalshortcut5/gui/qxtwindowsystem_x11.cpp
        qxtglobalshortcut5/gui/qxtglobalshortcut.cpp
        qxtglobalshortcut5/core/qxtglobal.cpp
    )
    set(QXT_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/qxtglobalshortcut5/gui
        ${CMAKE_CURRENT_SOURCE_DIR}/qxtglobalshortcut5/core
        PARENT_SCOPE
    )
    add_library(Qxt SHARED ${QXT_SOURCE_FILES})
    target_link_libraries(Qxt
        Qt5::Widgets
        Qt5::X11Extras
    )
    target_include_directories(Qxt PRIVATE
        qxtglobalshortcut5/gui
        qxtglobalshortcut5/core
    )
    set(QXT_LIBRARIES
        Qxt
        PARENT_SCOPE
    )
    install(TARGETS Qxt DESTINATION ${TOGGL_INTERNAL_LIB_DIR})
endif()

if(NOT GTEST_FOUND)
    set(GTEST_SOURCE_FILES
        googletest-read-only/src/gtest-all.cc
    )
    add_library(gtest SHARED ${GTEST_SOURCE_FILES})
    target_include_directories(gtest PRIVATE
        googletest-read-only
        googletest-read-only/include
    )
    target_link_libraries(gtest pthread)
    set(GTEST_MAIN_SOURCE_FILES
        googletest-read-only/src/gtest_main.cc
    )
    add_library(gtest_main SHARED ${GTEST_MAIN_SOURCE_FILES})
    target_include_directories(gtest_main PRIVATE
        googletest-read-only
        googletest-read-only/include
    )
    target_link_libraries(gtest_main pthread)
    set(GTEST_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/googletest-read-only/include
        PARENT_SCOPE
    )
endif()
