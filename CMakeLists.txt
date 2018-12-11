cmake_minimum_required(VERSION 3.2)
project(TogglDesktop)

# Set up automatic resource generation to make Qt development easier
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Use PkgConfig to look for packages without native CMake support
include(FindPkgConfig)

# Look for Qt
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5WebEngine CONFIG REQUIRED)
find_package(Qt5WebEngineWidgets CONFIG REQUIRED)
find_package(Qt5X11Extras CONFIG REQUIRED)
# We need to include private headers manually
include_directories( ${Qt5Widgets_PRIVATE_INCLUDE_DIRS} )

# Look for Poco
find_package(Poco REQUIRED Crypto DataSQLite NetSSL)

# Look for JSON
find_package(jsoncpp CONFIG REQUIRED)

# Look for Qxt
pkg_search_module(QXT_CORE REQUIRED QxtCore-qt5)
pkg_search_module(QXT_WIDGETS REQUIRED QxtWidgets-qt5)
# For PkgConfig packages, we need to set up the includes manually
include_directories(${QXT_CORE_INCLUDE_DIRS} ${QXT_WIDGETS_INCLUDE_DIRS})

# Look for Lua
pkg_search_module(LUA REQUIRED lua)
include_directories(${LUA_INCLUDE_DIRS})

# Set up some compilation definitions
add_definitions(
    -std=c++14
    -O2
    -Wall
    -Wextra
    -DAPP_ENVIRONMENT="CMAKE"
    -DAPP_VERSION="0"
)

# And include dirs
include_directories(
    third_party/qt-oauth-lib
    third_party/bugsnag-qt
    src/ui/linux/TogglDesktop/
    src/
)

# OAuth has to be bundled
set(OAUTH2_SOURCE_FILES
    third_party/qt-oauth-lib/logindialog.cpp
    third_party/qt-oauth-lib/oauth2.cpp
)

# Bugsnag has to be bundled
set(BUGSNAG_SOURCE_FILES
    third_party/bugsnag-qt/bugsnag.cpp
)

# TogglDesktopLibrary sources
set(LIBRARY_SOURCE_FILES
    src/base_model.cc
    src/batch_update_result.cc
    src/client.cc
    src/idle.cc
    src/analytics.cc
    src/help_article.cc
    src/autotracker.cc
    src/urls.cc
    src/migrations.cc
    src/context.cc
    src/custom_error_handler.cc
    src/database.cc
    src/feedback.cc
    src/formatter.cc
    src/get_focused_window_linux.cc
    src/error.cc
    src/gui.cc
    src/netconf.cc
    src/https_client.cc
    src/toggl_api.cc
    src/toggl_api_private.cc
    src/model_change.cc
    src/obm_action.cc
    src/project.cc
    src/proxy.cc
    src/related_data.cc
    src/settings.cc
    src/tag.cc
    src/task.cc
    src/timeline_event.cc
    src/time_entry.cc
    src/timeline_uploader.cc
    src/user.cc
    src/websocket_client.cc
    src/window_change_recorder.cc
    src/workspace.cc

    src/Rectangle.cc
)

# TogglDesktop sources
set(BINARY_SOURCE_FILES
    src/ui/linux/TogglDesktop/aboutdialog.cpp
    src/ui/linux/TogglDesktop/autocompleteview.cpp
    src/ui/linux/TogglDesktop/clickablelabel.cpp
    src/ui/linux/TogglDesktop/colorpicker.cpp
    src/ui/linux/TogglDesktop/countryview.cpp
    src/ui/linux/TogglDesktop/errorviewcontroller.cpp
    src/ui/linux/TogglDesktop/feedbackdialog.cpp
    src/ui/linux/TogglDesktop/genericview.cpp
    src/ui/linux/TogglDesktop/idlenotificationdialog.cpp
    src/ui/linux/TogglDesktop/loginwidget.cpp
    src/ui/linux/TogglDesktop/main.cpp
    src/ui/linux/TogglDesktop/mainwindowcontroller.cpp
    src/ui/linux/TogglDesktop/overlaywidget.cpp
    src/ui/linux/TogglDesktop/preferencesdialog.cpp
    src/ui/linux/TogglDesktop/settingsview.cpp
    src/ui/linux/TogglDesktop/singleapplication.cpp
    src/ui/linux/TogglDesktop/timeentrycellwidget.cpp
    src/ui/linux/TogglDesktop/timeentryeditorwidget.cpp
    src/ui/linux/TogglDesktop/timeentrylistwidget.cpp
    src/ui/linux/TogglDesktop/timeentryview.cpp
    src/ui/linux/TogglDesktop/timerwidget.cpp
    src/ui/linux/TogglDesktop/toggl.cpp

    # Resources have to be listed, .ui files don't
    src/ui/linux/TogglDesktop/Resources.qrc
)

# Set up compilation targets
add_library(OAuth2 SHARED ${OAUTH2_SOURCE_FILES})
add_library(Bugsnag SHARED ${BUGSNAG_SOURCE_FILES})
add_library(TogglDesktopLibrary SHARED ${LIBRARY_SOURCE_FILES})
add_executable(TogglDesktop ${BINARY_SOURCE_FILES})

# And list the dependencies between them (and system libraries)
target_link_libraries(OAuth2 PRIVATE
    Qt5::Widgets Qt5::WebEngine Qt5::WebEngineWidgets
)
target_link_libraries(Bugsnag PRIVATE
    Qt5::Widgets Qt5::Network
)
target_link_libraries(TogglDesktopLibrary PRIVATE
    Qt5::Widgets Qt5::WebEngine
    jsoncpp
    ${LUA_LIBRARIES}
)
target_link_libraries(TogglDesktop PRIVATE
    TogglDesktopLibrary
    Qt5::Widgets Qt5::WebEngine
    OAuth2
    ${QXT_CORE_LIBRARIES} ${QXT_WIDGETS_LIBRARIES}
    Bugsnag 
    Poco::Crypto Poco::DataSQLite Poco::NetSSL
    -lX11 -lXss
)
