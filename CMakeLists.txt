cmake_minimum_required(VERSION 3.10)
project(TogglDesktop)

set(CMAKE_CXX_STANDARD 11)

# Set up automatic resource generation to make Qt development easier
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

option(TOGGL_VERSION "TogglDesktop version string" "7.0.0")
option(TOGGL_PRODUCTION_BUILD "Use production servers in the app" OFF)
option(TOGGL_ALLOW_UPDATE_CHECK "Allow the app to check for updates" OFF)
option(USE_BUNDLED_LIBRARIES "Prefer bundled libraries to bundled ones" OFF)

# Use PkgConfig to look for packages without native CMake support
include(FindPkgConfig)

# Look for Qt
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5DBus CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5WebEngine CONFIG REQUIRED)
find_package(Qt5WebEngineWidgets CONFIG REQUIRED)
find_package(Qt5X11Extras CONFIG REQUIRED)
# We need to include private headers manually
include_directories( ${Qt5Widgets_PRIVATE_INCLUDE_DIRS} )

if(NOT USE_BUNDLED_LIBRARIES)
    # Look for Poco
    find_package(Poco COMPONENTS Crypto DataSQLite NetSSL)

    # Look for Lua
    pkg_search_module(LUA lua)
    include_directories(${LUA_INCLUDE_DIRS})
    
    # Look for JSON
    find_package(jsoncpp CONFIG)

    # Look for TBB
    pkg_search_module(TBB tbb)
    set(TBB_IMPORTED_TARGETS ${TBB_LIBRARIES})

    # Look for Qxt
    pkg_search_module(QXT_CORE QxtCore-qt5)
    pkg_search_module(QXT_WIDGETS QxtWidgets-qt5)
    set(QXT_LIBRARIES
        ${QXT_CORE_LIBRARIES} 
        ${QXT_WIDGETS_LIBRARIES}
    )
    set(QXT_INCLUDE_DIRS
        ${QXT_CORE_INCLUDE_DIRS} ${QXT_WIDGETS_INCLUDE_DIRS}
    )
endif()

# this has to be here instead of third_party because i couldn't find a way to export TBB::tbb from inside that folder
if(NOT TBB_FOUND)
    include(${PROJECT_SOURCE_DIR}/third_party/tbb/cmake/TBBBuild.cmake)
    tbb_build(TBB_ROOT ${PROJECT_SOURCE_DIR}/third_party/tbb CONFIG_DIR TBB_DIR)
    find_package(TBB REQUIRED tbb)
endif()

set(TOGGLDESKTOP_BUILDROOT
    ${CMAKE_CURRENT_BINARY_DIR}/buildroot
)

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(src/ui/linux/TogglDesktop)
