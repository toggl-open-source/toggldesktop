####
## Libproxy Tests
####

include_directories(${PROJECT_SOURCE_DIR})

# URL Test
add_executable(url-test ${CMAKE_CURRENT_SOURCE_DIR}/url-test.cpp ${PROJECT_SOURCE_DIR}/url.cpp)
if(WIN32)
  target_link_libraries(url-test ws2_32)
endif()
add_test(NAME url-test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/url-test)

# URL Encode
add_executable(url-encode ${CMAKE_CURRENT_SOURCE_DIR}/url-encode.cpp ${PROJECT_SOURCE_DIR}/url.cpp)
if(WIN32)
  target_link_libraries(url-encode ws2_32)
endif()
add_test(NAME url-encode COMMAND ${CMAKE_CURRENT_BINARY_DIR}/url-encode)

# Get PAC
if (NOT WIN32)
add_executable(get-pac-test ${CMAKE_CURRENT_SOURCE_DIR}/get-pac-test.cpp ${PROJECT_SOURCE_DIR}/url.cpp)
target_link_libraries(get-pac-test pthread)
add_test(NAME get-pac-test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/get-pac-test)
endif()
