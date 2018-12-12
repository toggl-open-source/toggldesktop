cmake_minimum_required(VERSION 3.2)
project(TogglDesktop)

# Set up automatic resource generation to make Qt development easier
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

option(USE_SYSTEMWIDE_LIBRARIES "Prefer system libraries to bundled ones" OFF)

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

if(USE_SYSTEMWIDE_LIBRARIES)
    # Look for Poco
    find_package(Poco REQUIRED Crypto DataSQLite NetSSL)
endif()

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
    -O2
)

set(TOGGLDESKTOP_BUILDROOT
    ${CMAKE_CURRENT_BINARY_DIR}/buildroot
)

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(src/ui/linux/TogglDesktop)
