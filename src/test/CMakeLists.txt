include_directories(
    ${TOGGLTRACKLIBRARY_INCLUDE_DIR}
    ${OAUTH_INCLUDE_DIR}
    ${BUGSNAG_INCLUDE_DIR}
    ${LUA_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    ${POCO_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
)

set(API_TEST_SOURCE_FILES
    test_data.cc
    toggl_api_test.cc
)
add_executable(TogglApiTest ${API_TEST_SOURCE_FILES})
target_link_libraries(TogglApiTest PRIVATE
    TogglTrackLibrary
    ${JSONCPP_LIBRARIES}
    ${LUA_LIBRARIES}
    PocoCrypto PocoDataSQLite PocoNetSSL PocoFoundation
    gtest_main gtest
    X11
)

set(APP_TEST_SOURCE_FILES
    test_data.cc
    app_test.cc
)
add_executable(TogglAppTest ${APP_TEST_SOURCE_FILES})
target_link_libraries(TogglAppTest PRIVATE
    TogglTrackLibrary
    ${JSONCPP_LIBRARIES}
    ${LUA_LIBRARIES}
    PocoCrypto PocoDataSQLite PocoNetSSL PocoFoundation
    gtest_main gtest
    X11
)

set(ONLINE_TEST_SOURCE_FILES
    online_test.cc
    online_test_app.cpp
    online_test_dispatcher.cc
    online_test_model.cc
)
add_executable(TogglOnlineTest ${ONLINE_TEST_SOURCE_FILES})
target_link_libraries(TogglOnlineTest PRIVATE
    TogglTrackLibrary
    ${JSONCPP_LIBRARIES}
    ${LUA_LIBRARIES}
    PocoCrypto PocoDataSQLite PocoNetSSL PocoFoundation
    gtest_main gtest
    X11
)
