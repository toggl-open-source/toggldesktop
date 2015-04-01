find_package(PythonInterp)

if(PYTHONINTERP_FOUND)
  set(PYTHON_SITEPKG_DIR "${PYTHON_SITEPKG_DIR}" CACHE PATH "Python site-package directory.")

  if(NOT PYTHON_SITEPKG_DIR)
    execute_process(COMMAND
		    ${PYTHON_EXECUTABLE}
		    -c "import sys; print sys.version[0:3]"
		    OUTPUT_VARIABLE PYTHON_VERSION
		    OUTPUT_STRIP_TRAILING_WHITESPACE)

    message(STATUS "Found python version ${PYTHON_VERSION}")

    execute_process(COMMAND
		    ${PYTHON_EXECUTABLE}
		    -c "import distutils.sysconfig ; print distutils.sysconfig.get_python_lib(plat_specific=0)"
		    OUTPUT_VARIABLE PYTHON_SITEPKG_DIR
		    OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()
  
  message(STATUS "Using PYTHON_SITEPKG_DIR=${PYTHON_SITEPKG_DIR}")

  install(FILES libproxy.py DESTINATION ${PYTHON_SITEPKG_DIR})
endif()
