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
    util/property.cc

    database/database.cc
    database/migrations.cc

    model/autotracker.cc
    model/base_model.cc
    model_change.cc
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

    analytics.cc
    batch_update_result.cc
    context.cc
    error.cc
    feedback.cc
    get_focused_window_linux.cc
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
