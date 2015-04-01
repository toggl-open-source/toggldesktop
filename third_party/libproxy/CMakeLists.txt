### Required
cmake_minimum_required(VERSION 2.6)
if(POLICY CMP0011)
   cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

# Make sure we look in our cmake folder for additional definitions
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake )

### Make sure we have a build type
# This fixes an install issue where installs fail
# when find_package(KDE4) is present.  I'm still
# not sure why this happens, but this definitely fixes
# it.  It also fixes tests not running in this case.
# See issues 127 and 128.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
  set(CMAKE_INSTALL_CONFIG_NAME RelWithDebInfo)
else()
  set(CMAKE_INSTALL_CONFIG_NAME ${CMAKE_BUILD_TYPE})
endif()

include(cmake/cpack.cmk)
include(cmake/ctest.cmk)
include(cmake/paths.cmk)

option(WITH_WEBKIT3 "Build against gtk-3 version of webkitgtk" OFF)

### Subdirectories
# Conditionally build bundled libmodman
option(FORCE_SYSTEM_LIBMODMAN "Force using system libmodman" OFF)
find_package(libmodman QUIET)
if(LIBMODMAN_FOUND)
    if("${LIBMODMAN_VERSION_MAJOR}" STREQUAL "2")
      message(STATUS "Building with system libmodman")    
    else()
      message(FATAL_ERROR "Found incompatible libmodman on your system (libmodman 2.X is needed)")
    endif()
else()
    if(FORCE_SYSTEM_LIBMODMAN)
      message(FATAL_ERROR "Libmodman could not be found on your system")
    else()
      message(STATUS "Building with bundled libmodman")
      add_subdirectory(libmodman)
      include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endif()

# Conditionally build bindings
if(NOT WIN32)
    add_subdirectory(bindings)
endif()

# Always build these
add_subdirectory(libproxy)
add_subdirectory(utils)

