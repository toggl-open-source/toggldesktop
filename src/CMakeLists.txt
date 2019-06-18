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
    base_model.cc
    batch_update_result.cc
    client.cc
    idle.cc
    analytics.cc
    help_article.cc
    autotracker.cc
    urls.cc
    migrations.cc
    context.cc
    custom_error_handler.cc
    database.cc
    feedback.cc
    formatter.cc
    get_focused_window_linux.cc
    error.cc
    gui.cc
    netconf.cc
    https_client.cc
    toggl_api.cc
    toggl_api_private.cc
    toggl_api_views.cc
    model_change.cc
    obm_action.cc
    project.cc
    proxy.cc
    random.cc
    rectangle.cc
    related_data.cc
    settings.cc
    tag.cc
    task.cc
    timeline_event.cc
    time_entry.cc
    timeline_uploader.cc
    user.cc
    websocket_client.cc
    window_change_recorder.cc
    workspace.cc
    json_helper.cc
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
