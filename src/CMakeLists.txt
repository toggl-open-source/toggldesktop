set(CMAKE_CXX_FLAGS
    -Wall
)
if(TOGGL_PRODUCTION_BUILD)
    add_definitions(
        -DAPP_ENVIRONMENT="production"
        -DTOGGL_BUILD_TYPE="${TOGGL_BUILD_TYPE}"
        -DTOGGL_PRODUCTION_BUILD=1
    )
else()
    add_definitions(-DAPP_ENVIRONMENT="development")
endif()

if(TOGGL_ALLOW_UPDATE_CHECK)
    add_definitions(-DTOGGL_ALLOW_UPDATE_CHECK=1)
endif()

add_definitions(-DTOGGL_BUILD_TYPE="${TOGGL_BUILD_TYPE}" -DTOGGLDESKTOP_DLL_BUILD)

set(TOGGLDESKTOPLIBRARY_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARENT_SCOPE
)

# And include dirs
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OAUTH_INCLUDE_DIR}
    ${BUGSNAG_INCLUDE_DIR}
    ${LUA_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    ${POCO_INCLUDE_DIRS}
)

# TogglDesktopLibrary sources
set(LIBRARY_SOURCE_FILES
    util/custom_error_handler.cc
    util/formatter.cc
    util/logger.cc
    util/random.cc
    util/rectangle.cc
    util/json.cc
    util/error.cc

    database/database.cc
    database/migrations.cc

    model/alpha_features.cpp
    model/autotracker.cc
    model/base_model.cc
    model_change.cc
    model/client.cc
    model/project.cc
    model/settings.cc
    model/tag.cc
    model/task.cc
    model/time_entry.cc
    model/timeline_event.cc
    model/user.cc
    model/workspace.cc

    analytics.cc
    context.cc
    error.cc
    feedback.cc
    gui.cc
    help_article.cc
    https_client.cc
    idle.cc
    netconf.cc
    platforminfo.cc
    proxy.cc
    related_data.cc
    timeline_uploader.cc
    toggl_api.cc
    toggl_api_private.cc
    urls.cc
    websocket_client.cc
    window_change_recorder.cc
    color_convert.cc
    onboarding_service.cpp
)
if (UNIX AND NOT APPLE)
    set (LIBRARY_SOURCE_FILES ${LIBRARY_SOURCE_FILES}
        get_focused_window_linux.cc
    )
elseif(WIN32)
    set (LIBRARY_SOURCE_FILES ${LIBRARY_SOURCE_FILES}
        get_focused_window_windows.cc
    )
    set (LIBRARY_ADDITIONAL_LIBS
        -lWinhttp
        -lPsapi
    )
else()
    set (LIBRARY_SOURCE_FILES ${LIBRARY_SOURCE_FILES}
        get_focused_window_mac.cc
        lib/osx/Kopsik/MacOSVersionChecker.mm
    )
    set(LIBRARY_ADDITIONAL_LIBS
        "-framework Foundation"
        "-framework CFNetwork"
        "-framework AppKit"
    )
endif()

# Set up compilation targets
add_library(TogglDesktopLibrary SHARED ${LIBRARY_SOURCE_FILES})

# And list the dependencies between them (and system libraries)
target_link_libraries(TogglDesktopLibrary
    ${LIBRARY_ADDITIONAL_LIBS}
    ${JSONCPP_LIBRARIES}
    ${LUA_LIBRARIES}
    PocoCrypto PocoDataSQLite PocoNetSSL
)

install(TARGETS TogglDesktopLibrary DESTINATION lib)
