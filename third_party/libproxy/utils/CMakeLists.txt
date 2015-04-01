include_directories("../libproxy")

add_executable(proxy proxy.c)
target_link_libraries(proxy libproxy)
install(TARGETS proxy RUNTIME DESTINATION ${BIN_INSTALL_DIR})
