set(CMAKE_CXX_FLAGS
    -Wall
)
if(TOGGL_PRODUCTION_BUILD)
    add_definitions(
        -DAPP_ENVIRONMENT="\\\"production\\\""
        -DTOGGL_PRODUCTION_BUILD=1
    )
else()
    add_definitions(-DAPP_ENVIRONMENT="\\\"development\\\"")
endif()

if(TOGGL_ALLOW_UPDATE_CHECK)
    add_definitions(-DTOGGL_ALLOW_UPDATE_CHECK=1)
endif()

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
    common/custom_error_handler.cc
    common/error.cc
    common/formatter.cc
    common/model_change.cc
    common/urls.cc
    common/rectangle.cc
    common/json_helper.cc
    common/random.cc
    model/autotracker.cc
    model/base_model.cc
    model/batch_update_result.cc
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
    network/https_client.cc
    network/netconf.cc
    network/proxy.cc
    network/websocket_client.cc
    timeline/get_focused_window_linux.cc
    timeline/timeline_uploader.cc
    timeline/window_change_recorder.cc
    analytics.cc
    context.cc
    database.cc
    feedback.cc
    gui.cc
    gui_update.cpp
    help_article.cc
    idle.cc
    migrations.cc
    related_data.cc
    toggl_api.cc
    toggl_api_private.cc
    toggl_api_views.cc
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
