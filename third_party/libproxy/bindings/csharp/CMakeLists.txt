PROJECT(libproxy-sharp)

enable_language(CSharp)

SET(KEYFILE ${CMAKE_CURRENT_SOURCE_DIR}/libproxy.snk)

CSHARP_ADD_LIBRARY(libproxy-sharp libproxy.cs AssemblyInfo.cs COMPILE_FLAGS /warn:0 "/keyfile:${KEYFILE}")
SET_PROPERTY(TARGET libproxy-sharp PROPERTY pkg-config_template_basename libproxy-sharp-1.0)

GET_LIBRARY_OUTPUT_DIR(library_target_dir)
FILE(WRITE ${library_target_dir}/libproxy-sharp.dll.config "<configuration><dllmap dll=\"proxy\" target=\"libproxy.so.1\"/></configuration>")

INSTALL_ASSEMBLY(libproxy-sharp DESTINATION lib PACKAGE libproxy-sharp)

