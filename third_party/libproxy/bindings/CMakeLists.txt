option(WITH_PYTHON "Enables Python bindings" ON)
if (WITH_PYTHON)
  add_subdirectory(python)
endif()

option(WITH_DOTNET "Enables C# bindings" OFF)
if (WITH_DOTNET)
  add_subdirectory(csharp)
endif()

#perl bindings live in their own subdir.
option(WITH_PERL "Enables Perl bindings" ON)
if (WITH_PERL)
  add_subdirectory(perl)
endif()

# Install Vala bindings / Install only when -DWITH_VALA is passed to cmake.
option(WITH_VALA "Enables Vala bindings" OFF)
if(WITH_VALA)
  if (NOT VAPI_DIR)
    set(VAPI_DIR ${SHARE_INSTALL_DIR}/vala/vapi)
  endif()
  install(FILES vala/libproxy-1.0.vapi DESTINATION ${VAPI_DIR})
  set(VAPI_DIR ${VAPI_DIR} CACHE PATH "Vala API directory")
endif()
