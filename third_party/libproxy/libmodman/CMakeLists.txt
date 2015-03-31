### Main library
# Flags / Definitions / Environment
if (WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
else(WIN32)
  set(CMAKE_CXX_FLAGS "-fvisibility=hidden -fPIC ${CMAKE_CXX_FLAGS}")
endif(WIN32)
include_directories(${CMAKE_SOURCE_DIR})

add_library(modman STATIC
            module.hpp
            module_manager.hpp
            module_manager.cpp)
if(NOT WIN32)
	target_link_libraries(modman ${CMAKE_DL_LIBS})
endif()

### Tests
add_testdirectory(test)
