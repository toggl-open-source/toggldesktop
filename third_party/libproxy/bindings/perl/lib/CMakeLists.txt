add_custom_target(PMlibproxy ALL ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Libproxy.pm ${CMAKE_BINARY_DIR}/perl/blib/lib/Libproxy.pm)
install( FILES Libproxy.pm DESTINATION ${PX_PERL_ARCH}/Net )
