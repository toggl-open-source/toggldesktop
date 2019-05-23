cmake_minimum_required(VERSION 3.2)
project(TogglDesktop)

set(CMAKE_CXX_STANDARD 11)

# Set up automatic resource generation to make Qt development easier
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(TOGGL_VERSION "7.0.0" CACHE STRING "TogglDesktop version string")
set(TOGGL_VERSION_RELEASE_DATE "1970-01-01" CACHE STRING "TogglDesktop version release date string")
set(TOGGL_VERSION_DESCRIPTION "<p>With this update, Toggl Desktop brings general bug fixes and usability improvements</p>" CACHE STRING "TogglDesktop version description")
option(TOGGL_PRODUCTION_BUILD "Use production servers in the app" OFF)
option(TOGGL_ALLOW_UPDATE_CHECK "Allow the app to check for updates" OFF)
option(USE_BUNDLED_LIBRARIES "Prefer bundled libraries to bundled ones" OFF)
option(INSTALL_HIRES_ICONS "Do not install icons over 512x512" OFF)

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
    if(jsoncpp_FOUND)
        set(JSONCPP_LIBRARIES jsoncpp_lib)
    endif()

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


set(TOGGLDESKTOP_BUILDROOT
    ${CMAKE_CURRENT_BINARY_DIR}/buildroot
)

set(TOGGL_BINARY_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE STRING "Binary install directory")
set(TOGGL_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/toggldesktop" CACHE STRING "Runtime data install directory")
set(TOGGL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE STRING "Library install directory")
set(TOGGL_INTERNAL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE STRING "Bundled library install directory")
set(CMAKE_INSTALL_RPATH "${TOGGL_LIB_DIR};${TOGGL_INTERNAL_LIB_DIR}" CACHE STRING "Custom RPATH for the binaries and libraries")

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(src/ui/linux/TogglDesktop)
