macro(set_or_default var value default)
  if("${value}" STREQUAL "")
    set(${var} ${default})
  else()
    set(${var} ${value})
  endif()
endmacro()

macro(set_if_true var truthy value)
  if(${truthy})
    set(${var} ${value})
  else()
    set(${var} "")
  endif()
endmacro()

macro(set_once var value error_message)
  if(NOT DEFINED ${var})
    set(${var} ${value})
  else()
    if(NOT error_message STREQUAL "")
      message(FATAL_ERROR "${error_message}")
    else()
      message(FATAL_ERROR "Variable ${var} already set.")
    endif()
  endif()
endmacro()

# Get all properties that cmake supports
if(NOT CMAKE_PROPERTY_LIST)
  execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)

  # Convert command output into a CMake list
  string(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
  string(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
  list(REMOVE_DUPLICATES CMAKE_PROPERTY_LIST)
endif()

function(print_properties)
  message("CMAKE_PROPERTY_LIST = ${CMAKE_PROPERTY_LIST}")
endfunction()

function(print_target_properties target)
  if(NOT TARGET ${target})
    message(STATUS "There is no target named '${target}'")
    return()
  endif()

  foreach(property ${CMAKE_PROPERTY_LIST})
    string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" property ${property})

    # Fix
    # https://stackoverflow.com/questions/32197663/how-can-i-remove-the-the-location-property-may-not-be-read-from-target-error-i
    if(property STREQUAL "LOCATION"
       OR property MATCHES "^LOCATION_"
       OR property MATCHES "_LOCATION$"
    )
      continue()
    endif()

    get_property(
      was_set
      TARGET ${target}
      PROPERTY ${property}
      SET
    )
    if(was_set)
      get_target_property(value ${target} ${property})
      message("${target} ${property} = ${value}")
    endif()
  endforeach()
endfunction()

function(print_target_file_sets target_name)
  if(NOT TARGET ${target_name})
    message(FATAL_ERROR "There is no target named '${target_name}'")
  endif()

  get_target_property(module_file_sets ${target_name} CXX_MODULE_SETS)
  if(module_file_sets)
    foreach(file_set IN LISTS module_file_sets)
      get_target_property(files ${target_name} CXX_MODULE_SET_${file_set})
      get_target_property(base_dirs ${target_name} CXX_MODULE_DIRS_${file_set})
      message("${target_name} CXX_MODULE_SET_${file_set} MODULE FILES = ${files}")
      message("${target_name} CXX_MODULE_DIRS_${file_set} MODULE DIRS = ${base_dirs}")
    endforeach()
  else()
    message("No CXX_MODULE_SETS found for ${target_name}")
  endif()

  get_target_property(interface_module_sets ${target_name} INTERFACE_CXX_MODULE_SETS)
  if(interface_module_sets)
    message("${target_name} INTERFACE_CXX_MODULE_SETS = ${interface_module_sets}")
  else()
    message("No INTERFACE_CXX_MODULE_SETS found for ${target_name}")
  endif()

  get_target_property(header_file_sets ${target_name} HEADER_SETS)
  if(header_file_sets)
    foreach(file_set IN LISTS header_file_sets)
      get_target_property(files ${target_name} HEADER_SET_${file_set})
      get_target_property(base_dirs ${target_name} HEADER_DIRS_${file_set})
      message("${target_name} HEADER_SET_${file_set} HEADER FILES = ${files}")
      message("${target_name} HEADER_DIRS_${file_set} HEADER DIRS = ${base_dirs}")
    endforeach()
  else()
    message("No HEADER_SETS found for ${target_name}")
  endif()

  get_target_property(interface_header_file_sets ${target_name} INTERFACE_HEADER_SETS)
  if(interface_header_file_sets)
    foreach(file_set IN LISTS interface_header_file_sets)
      get_target_property(files ${target_name} INTERFACE_HEADER_SET_${file_set})
      get_target_property(base_dirs ${target_name} INTERFACE_HEADER_DIRS_${file_set})
      message("${target_name} INTERFACE_HEADER_SET_${file_set} IFACE HEADER FILES = ${files}")
      message("${target_name} INTERFACE_HEADER_DIRS_${file_set} IFACE HEADER DIRS = ${base_dirs}")
    endforeach()
  else()
    message("No INTERFACE_HEADER_SETS found for ${target_name}")
  endif()

  get_target_property(sources ${target_name} SOURCES)
  if(sources)
    message("${target_name} SOURCES = ${sources}")
  else()
    message("No SOURCES found for ${target_name}")
  endif()
endfunction()
