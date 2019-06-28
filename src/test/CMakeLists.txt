# And include dirs
include_directories(
    ${OAUTH_INCLUDE_DIR}
    ${BUGSNAG_INCLUDE_DIR}
    ${LUA_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    ${POCO_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
)

# TogglDesktopLibrary sources
set(API_TEST_SOURCE_FILES
    test_data.cc
    toggl_api_test.cc
)

# Set up compilation targets
add_executable(TogglApiTest ${API_TEST_SOURCE_FILES})

# And list the dependencies between them (and system libraries)
target_link_libraries(TogglApiTest PRIVATE
    TogglDesktopLibrary
    ${JSONCPP_LIBRARIES}
    ${LUA_LIBRARIES}
    PocoCrypto PocoDataSQLite PocoNetSSL PocoFoundation
    gtest_main gtest
    X11
)

# install(TARGETS TogglDesktopLibrary DESTINATION lib)
