#####
## LibModMan Tests
####

# Functions
function(mm_create_module MODTYPE MODNAME MODCOND MODSYMB MODSMOD)
  configure_file(module.cpp.in
                 ${CMAKE_CURRENT_BINARY_DIR}/${MODTYPE}_${MODNAME}.cpp
                 @ONLY)
  add_library(${MODTYPE}_${MODNAME}
              MODULE
              ${CMAKE_CURRENT_BINARY_DIR}/${MODTYPE}_${MODNAME}.cpp)
  set_target_properties(${MODTYPE}_${MODNAME}
                        PROPERTIES PREFIX ""
                        LIBRARY_OUTPUT_DIRECTORY
                        ${CMAKE_CURRENT_BINARY_DIR}/modules/${MODTYPE})
  target_link_libraries(${MODTYPE}_${MODNAME} modman)
endfunction(mm_create_module)

function(mm_create_program name EXTTYPE)
  add_executable(${name} main.cpp)
  target_link_libraries(${name} modman)
  set_property(TARGET ${name} PROPERTY
               COMPILE_DEFINITIONS
               EXTTYPE=${EXTTYPE}_extension)
  if(${ARGC} GREATER 2)
    target_link_libraries(${name} ${ARGN};modman)
    set_property(TARGET ${name} PROPERTY 
                 COMPILE_DEFINITIONS
                 EXTTYPE=${EXTTYPE}_extension;SYMB=1)
    if(NOT WIN32 AND NOT APPLE)
      set_property(TARGET ${name} PROPERTY
                   LINK_FLAGS -Wl,--no-as-needed)
    endif()
  endif()
endfunction(mm_create_program)

# Modules
mm_create_module(condition one false NULL        NULL)
mm_create_module(condition two true  NULL        NULL)
mm_create_module(singleton one true  NULL        NULL)
mm_create_module(singleton two true  NULL        NULL)
mm_create_module(sorted    one true  NULL        NULL)
mm_create_module(sorted    two true  NULL        NULL)
mm_create_module(builtin   one true  NULL        NULL)
if (WIN32)
  mm_create_module(symbol  one true  \"asdfoia\" \"ws2_32\")
  mm_create_module(symbol  two true  \"recv\"    \"ws2_32\")
else()
  mm_create_module(symbol  one true  \"asdfoia\" \"z\")
  mm_create_module(symbol  two true  \"deflate\" \"z\")
endif()

# Programs
mm_create_program(condition condition)
mm_create_program(singleton singleton)
mm_create_program(sorted    sorted)
mm_create_program(symbol    symbol)
if (WIN32)
  mm_create_program(symbollnk symbol ws2_32)
else()
  mm_create_program(symbollnk symbol z)
endif()
add_executable(builtin
               builtin.cpp
               ${CMAKE_CURRENT_BINARY_DIR}/builtin_one.cpp)
target_link_libraries(builtin modman)
set_property(TARGET builtin PROPERTY COMPILE_DEFINITIONS
             EXTTYPE=builtin_extension;BUILTIN_MODULE=extension;MM_MODULE_BUILTIN=extension)

# Tests
add_test(NAME condition COMMAND condition $<TARGET_FILE_DIR:condition_one> two)
add_test(NAME singleton COMMAND singleton $<TARGET_FILE_DIR:singleton_one> one)
add_test(NAME sorted    COMMAND sorted    $<TARGET_FILE_DIR:sorted_one>    two one)
add_test(NAME symbol    COMMAND symbollnk $<TARGET_FILE_DIR:symbol_one>    two)
add_test(NAME nosymbol  COMMAND symbol    $<TARGET_FILE_DIR:symbol_one>)
add_test(NAME nosymreq  COMMAND symbol    $<TARGET_FILE_DIR:symbol_one>    one)
add_test(NAME builtin   COMMAND builtin)

