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

add_definitions(-DTOGGL_BUILD_TYPE="${TOGGL_BUILD_TYPE}")

set(TOGGLDESKTOPLIBRARY_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARENT_SCOPE
)

# And include dirs
include_directories(
    ${OAUTH_INCLUDE_DIR}
    ${BUGSNAG_INCLUDE_DIR}
    ${LUA_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    ${POCO_INCLUDE_DIRS}
)

# TogglDesktopLibrary sources
set(LIBRARY_SOURCE_FILES
    util/error.cc
    util/formatter.cc
    util/get_focused_window_linux.cc
    util/logger.cc
    util/platforminfo.cc
    util/random.cc
    util/rectangle.cc

    model/autotracker.cc
    model/base_model.cc
    model/client.cc
    model/obm_action.cc
    model/project.cc
    model/settings.cc
    model/tag.cc
    model/task.cc
    model/time_entry.cc
    model/timeline_event.cc
    model/user.cc
    model/workspace.cc

    net/https_client.cc
    net/netconf.cc
    net/proxy.cc
    net/urls.cc
    net/websocket_client.cc

    batch_update_result.cc
    idle.cc
    analytics.cc
    help_article.cc
    migrations.cc
    context.cc
    custom_error_handler.cc
    database.cc
    feedback.cc
    gui.cc
    toggl_api.cc
    toggl_api_private.cc
    model_change.cc
    related_data.cc
    timeline_uploader.cc
    window_change_recorder.cc
    synchronization.cpp
)

# Set up compilation targets
add_library(TogglDesktopLibrary SHARED ${LIBRARY_SOURCE_FILES})

# And list the dependencies between them (and system libraries)
target_link_libraries(TogglDesktopLibrary PRIVATE
    ${JSONCPP_LIBRARIES}
    ${LUA_LIBRARIES}
    PocoCrypto PocoDataSQLite PocoNetSSL
)

install(TARGETS TogglDesktopLibrary DESTINATION lib)
