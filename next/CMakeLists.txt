set(CMAKE_CXX_FLAGS
    -Wall
)

add_definitions(-DTOGGL_BUILD_TYPE="${TOGGL_BUILD_TYPE}")

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
    context.cc
    views.cc
    user_data.cc

    util.cc

    batch_update_result.cc
    database.cc
    formatter.cc
    migrations.cc
    proxy.cc
    urls.cc
    error.cc
    help_article.cc
    feedback.cc
    #netconf.cc
    platforminfo.cc
    rectangle.cc

    model/base_model.cc
    model/model_change.cc
    model/autotracker.cc
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

    toggl_api.cc

    main.cc
)

# Set up compilation targets
add_executable(TogglDesktopNext ${LIBRARY_SOURCE_FILES})

# And list the dependencies between them (and system libraries)
target_link_libraries(TogglDesktopNext
    ${JSONCPP_LIBRARIES}
    ${LUA_LIBRARIES}
    PocoCrypto PocoNetSSL PocoFoundation PocoData sqlite3 PocoDataSQLite PocoNet
    -pthread
    -lX11
)

