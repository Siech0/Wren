# Add a target to the project using the specified visibility and file type add_executable_target(target_name [WIN32]
# [MACOSX_BUNDLE] [EXCLUDE_FROM_ALL] [ <PUBLIC|PRIVATE> <HEADERS|SOURCES|CXX_MODULES> <files>... ]...)
function(add_executable_target target_name)
  set(prefix "ARG")
  set(noValues "WIN32;MACOSX_BUNDLE;EXCLUDE_FROM_ALL")
  set(singleValues "ALIAS_AS;CXX_STANDARD;C_STANDARD")
  set(multiValues "")
  cmake_parse_arguments(PARSE_ARGV 1 ${prefix} "${noValues}" "${singleValues}" "${multiValues}")

  # State maintenance
  set(current_visibility "")
  set(current_filetype "")

  # Initialize the file lists
  set(public_headers "")
  set(private_headers "")
  set(private_sources "")
  set(public_sources "")
  set(private_modules "")
  set(public_modules "")

  set(remaining_args ${ARG_UNPARSED_ARGUMENTS})

  while(remaining_args)
    # Pop the first argument
    list(GET remaining_args 0 current_arg)
    list(REMOVE_AT remaining_args 0)

    if(current_arg MATCHES "^(PUBLIC|PRIVATE)$")
      set(current_visibility ${current_arg})
    elseif(current_arg MATCHES "^(HEADERS|SOURCES|CXX_MODULES)$")
      set(current_filetype ${current_arg})
    elseif(current_visibility AND current_filetype)
      if(current_visibility STREQUAL "PUBLIC")
        if(current_filetype STREQUAL "HEADERS")
          list(APPEND public_headers ${current_arg})
        elseif(current_filetype STREQUAL "SOURCES")
          list(APPEND public_sources ${current_arg})
        elseif(current_filetype STREQUAL "CXX_MODULES")
          list(APPEND public_modules ${current_arg})
        endif()
      elseif(current_visibility STREQUAL "PRIVATE")
        if(current_filetype STREQUAL "HEADERS")
          list(APPEND private_headers ${current_arg})
        elseif(current_filetype STREQUAL "SOURCES")
          list(APPEND private_sources ${current_arg})
        elseif(current_filetype STREQUAL "CXX_MODULES")
          list(APPEND private_modules ${current_arg})
        endif()
      endif()
    else()
      message(
        FATAL_ERROR
          "Unexpected argument: ${current_arg}. Expected visibility (PUBLIC|PRIVATE) and filetype (HEADERS|SOURCES|CXX_MODULES)."
      )
    endif()
  endwhile()

  # Set executable options
  set_if_true(WIN32 ${ARG_WIN32} "WIN32")
  set_if_true(MACOSX_BUNDLE ${ARG_MACOSX_BUNDLE} "MACOSX_BUNDLE")
  set_if_true(EXCLUDE_FROM_ALL ${ARG_EXCLUDE_FROM_ALL} "EXCLUDE_FROM_ALL")
  if(ARG_CXX_STANDARD)
    set(cxx_standard ${ARG_CXX_STANDARD})
  elseif(CMAKE_CXX_STANDARD)
    set(cxx_standard ${CMAKE_CXX_STANDARD})
  else()
    set(cxx_standard "11")
  endif()

  if(ARG_C_STANDARD)
    set(c_standard ${ARG_C_STANDARD})
  elseif(CMAKE_C_STANDARD)
    set(c_standard ${CMAKE_C_STANDARD})
  else()
    set(c_standard "99")
  endif()

  # Add the executable target
  add_executable(${target_name} ${WIN32} ${MACOSX_BUNDLE} ${EXCLUDE_FROM_ALL})
  if(ARG_ALIAS_AS)
    add_executable(${ARG_ALIAS_AS} ALIAS ${target_name})
  endif()

  # cmake-format: off
  if(public_headers)
    target_sources(${target_name} PUBLIC FILE_SET public_headers TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${public_headers})
  endif()
  if(private_headers)
    target_sources(${target_name} PRIVATE FILE_SET private_headers TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${private_headers})
  endif()
  if(public_sources)
    target_sources(${target_name} PUBLIC ${public_sources})
  endif()
  if(private_sources)
    target_sources(${target_name} PRIVATE ${private_sources})
  endif()
  if(public_modules)
    target_sources(${target_name} PUBLIC FILE_SET public_modules TYPE CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${public_modules})
  endif()
  if(private_modules)
    target_sources(${target_name} PRIVATE FILE_SET private_modules TYPE CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${private_modules})
  endif()
  # cmake-format: on

  # If the target uses modules, we must set the target features to support c++20 at the minimum
  if(public_modules
     OR private_modules
     OR ARG_CXX_STANDARD
  )
    if(cxx_standard LESS 20)
      message(
        WARNING
          "Executable target '${target_name}' uses C++ modules but CXX_STANDARD is set to ${cxx_standard}. Setting C++ standard to 20."
      )
      set(cxx_standard 20)
    endif()
    target_compile_features(${target_name} INTERFACE cxx_std_${cxx_standard})
  endif()

  if(ARG_C_STANDARD)
    target_compile_features(${target_name} PUBLIC c_std_${ARG_C_STANDARD})
  endif()
endfunction()

function(add_library_target target_name)
  set(prefix "ARG")
  set(noValues "STATIC;SHARED;MODULE;OBJECT;INTERFACE;EXCLUDE_FROM_ALL")
  set(singleValues "ALIAS_AS;CXX_STANDARD;C_STANDARD")
  set(multiValues "")

  cmake_parse_arguments(PARSE_ARGV 1 ${prefix} "${noValues}" "${singleValues}" "${multiValues}")

  # State maintenance
  set(library_type) # Suppress pedantic warnings
  if(ARG_STATIC)
    set_once(library_type "STATIC"
             "Library type for target '${target_name}' already set to '${library_type}'. Cannot set it to 'STATIC'."
    )
  endif()
  if(ARG_SHARED)
    set_once(library_type "SHARED"
             "Library type for target '${target_name}' already set to '${library_type}'. Cannot set it to 'SHARED'."
    )
  endif()
  if(ARG_MODULE)
    set_once(library_type "MODULE"
             "Library type for target '${target_name}' already set to '${library_type}'. Cannot set it to 'MODULE'."
    )
  endif()
  if(ARG_OBJECT)
    set_once(library_type "OBJECT"
             "Library type for target '${target_name}' already set to '${library_type}'. Cannot set it to 'OBJECT'."
    )
  endif()
  if(ARG_INTERFACE)
    set_once(library_type "INTERFACE"
             "Library type for target '${target_name}' already set to '${library_type}'. Cannot set it to 'INTERFACE'."
    )
  endif()

  # State maintenance
  set(current_visibility "")
  set(current_filetype "")

  # Initialize the file lists
  set(public_headers "")
  set(private_headers "")
  set(interface_headers "")
  set(private_sources "")
  set(public_sources "")
  set(interface_sources "")
  set(private_modules "")
  set(public_modules "")
  set(interface_modules "")

  # Parse the nested arguments
  set(remaining_args ${ARG_UNPARSED_ARGUMENTS})

  while(remaining_args)
    # Pop the first argument
    list(GET remaining_args 0 current_arg)
    list(REMOVE_AT remaining_args 0)

    if(current_arg MATCHES "^(PUBLIC|PRIVATE|INTERFACE)$")
      set(current_visibility ${current_arg})
    elseif(current_arg MATCHES "^(HEADERS|SOURCES|CXX_MODULES)$")
      set(current_filetype ${current_arg})
    elseif(current_visibility AND current_filetype)
      if(current_visibility STREQUAL "PUBLIC")
        if(current_filetype STREQUAL "HEADERS")
          list(APPEND public_headers ${current_arg})
        elseif(current_filetype STREQUAL "SOURCES")
          list(APPEND public_sources ${current_arg})
        elseif(current_filetype STREQUAL "CXX_MODULES")
          list(APPEND public_modules ${current_arg})
        endif()
      elseif(current_visibility STREQUAL "PRIVATE")
        if(current_filetype STREQUAL "HEADERS")
          list(APPEND private_headers ${current_arg})
        elseif(current_filetype STREQUAL "SOURCES")
          list(APPEND private_sources ${current_arg})
        elseif(current_filetype STREQUAL "CXX_MODULES")
          list(APPEND private_modules ${current_arg})
        endif()
      elseif(current_visibility STREQUAL "INTERFACE")
        if(current_filetype STREQUAL "HEADERS")
          list(APPEND interface_headers ${current_arg})
        elseif(current_filetype STREQUAL "SOURCES")
          list(APPEND interface_sources ${current_arg})
        elseif(current_filetype STREQUAL "CXX_MODULES")
          list(APPEND interface_modules ${current_arg})
        endif()
      endif()
    else()
      message(
        FATAL_ERROR
          "Unexpected argument: ${current_arg}. Expected visibility (PUBLIC|PRIVATE) and filetype (HEADERS|SOURCES|CXX_MODULES)."
      )
    endif()
  endwhile()

  # Initialize library options
  set_if_true(EXCLUDE_FROM_ALL ${ARG_EXCLUDE_FROM_ALL} "EXCLUDE_FROM_ALL")
  if(ARG_CXX_STANDARD)
    set(cxx_standard ${ARG_CXX_STANDARD})
  elseif(CMAKE_CXX_STANDARD)
    set(cxx_standard ${CMAKE_CXX_STANDARD})
  else()
    set(cxx_standard "11")
  endif()

  if(ARG_C_STANDARD)
    set(c_standard ${ARG_C_STANDARD})
  elseif(CMAKE_C_STANDARD)
    set(c_standard ${CMAKE_C_STANDARD})
  else()
    set(c_standard "99")
  endif()

  # Validate if we are an interface with public or private sources
  if(library_type STREQUAL "INTERFACE")
    if(public_sources
       OR private_sources
       OR public_headers
       OR private_headers
       OR public_modules
       OR private_modules
    )
      message(
        FATAL_ERROR
          "Library target '${target_name}' is an INTERFACE library. Cannot have public or private sources, headers, or c++ modules."
      )
    endif()
  endif()

  # Add the library target
  add_library(${target_name} ${library_type} ${EXCLUDE_FROM_ALL})
  if(ARG_ALIAS_AS)
    add_library(${ARG_ALIAS_AS} ALIAS ${target_name})
  endif()

  # cmake-format: off
  if(public_headers)
    target_sources(${target_name} PUBLIC FILE_SET public_headers TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${public_headers})
  endif()
  if(private_headers)
    target_sources(${target_name} PRIVATE FILE_SET private_headers TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${private_headers})
  endif()
  if(interface_headers)
    target_sources(${target_name} INTERFACE FILE_SET interface_headers TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${interface_headers})
  endif()
  if(public_sources)
    target_sources(${target_name} PUBLIC ${public_sources})
  endif()
  if(private_sources)
    target_sources(${target_name} PRIVATE ${private_sources})
  endif()
  if(interface_sources)
    target_sources(${target_name} INTERFACE ${interface_sources})
  endif()
  if(public_modules)
    target_sources(${target_name} PUBLIC FILE_SET public_modules TYPE CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${public_modules})
  endif()
  if(private_modules)
    target_sources(${target_name} PRIVATE FILE_SET private_modules TYPE CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${private_modules})
  endif()
  if(interface_modules)
    target_sources(${target_name} INTERFACE FILE_SET interface_modules TYPE CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${interface_modules})
  endif()

  # If the target uses modules, we must set the target features to support c++20 at the minimum
  if(public_modules OR private_modules OR interface_modules OR ARG_CXX_STANDARD)
    if(cxx_standard LESS 20)
      message(WARNING "Library target '${target_name}' uses C++ modules but CXX_STANDARD is set to ${cxx_standard}. Setting C++ standard to 20.")
      set(cxx_standard 20)
    endif()
    target_compile_features(${target_name} INTERFACE cxx_std_${cxx_standard})
  endif()

  if(ARG_C_STANDARD)
    target_compile_features(${target_name} INTERFACE c_std_${c_standard})
  endif()
  # cmake-format: on
endfunction()

function(target_link_library_targets target_name)
  cmake_parse_arguments(PARSE_ARGV 1 "ARG" "" "" "")

  set(current_visibility "")

  # Parse the nested arguments
  set(remaining_args ${ARG_UNPARSED_ARGUMENTS})

  # Initialize the dependency lists
  set(public_dependencies "")
  set(private_dependencies "")
  set(interface_dependencies "")

  while(remaining_args)
    # Pop the first argument
    list(GET remaining_args 0 current_arg)
    list(REMOVE_AT remaining_args 0)

    if(current_arg MATCHES "^(PUBLIC|PRIVATE|INTERFACE)$")
      set(current_visibility ${current_arg})
    elseif(current_visibility)
      if(current_visibility STREQUAL "PUBLIC")
        list(APPEND public_dependencies ${current_arg})
      elseif(current_visibility STREQUAL "PRIVATE")
        list(APPEND private_dependencies ${current_arg})
      elseif(current_visibility STREQUAL "INTERFACE")
        list(APPEND interface_dependencies ${current_arg})
      endif()
    else()
      message(
        FATAL_ERROR
          "Unexpected argument: ${current_arg}. Expected visibility (PUBLIC|PRIVATE|INTERFACE) and target name."
      )
    endif()
  endwhile()

  # Ensure we are a valid linkage
  get_target_property(target_type ${current_arg} TYPE)
  if(target_type STREQUAL "INTERFACE_LIBRARY")
    if(public_dependencies OR private_dependencies)
      message(
        FATAL_ERROR
          "Library target '${current_arg}' is an INTERFACE library. Cannot have public or private dependencies."
      )
    endif()
  endif()

  if(public_dependencies)
    target_link_libraries(${target_name} PUBLIC ${public_dependencies})
  endif()

  if(private_dependencies)
    target_link_libraries(${target_name} PRIVATE ${private_dependencies})
  endif()

  # This hacky magic is needed to propagate module information across INTERFACE library boundaries.
  foreach(dependency IN LISTS interface_dependencies)
    target_link_libraries(${target_name} INTERFACE ${dependency})

    get_target_property(module_file_sets ${dependency} INTERFACE_CXX_MODULE_SETS)
    if(module_file_sets)
      foreach(file_set IN LISTS module_file_sets)
        get_target_property(files ${dependency} INTERFACE_CXX_MODULE_SET_${file_set})
        get_target_property(base_dirs ${dependency} INTERFACE_CXX_MODULE_DIRS_${file_set})

        if(files AND base_dirs)
          target_sources(
            ${target_name}
            INTERFACE FILE_SET
                      ${file_set}
                      TYPE
                      CXX_MODULES
                      BASE_DIRS
                      ${base_dirs}
                      FILES
                      ${files}
          )
        endif()
      endforeach()
    endif()
  endforeach()
endfunction()

# This function wraps the add_executable function to add additional test functionality such as code coverage
function(add_test_executable target_name)
  set(prefix "ARG")
  set(noValues WIN32 MACOSX_BUNDLE EXCLUDE_FROM_ALL)
  set(singleVaulues)
  set(multiValues)
  cmake_parse_arguments(PARSE_ARGV 1 ${prefix} "${noValues}" "${singleValues}" "${multiValues}")

  message(STATUS "Adding test executable ${target_name}")
  message(STATUS "  WIN32: ${ARG_WIN32}")
  message(STATUS "  MACOSX_BUNDLE: ${ARG_MACOSX_BUNDLE}")
  message(STATUS "  EXCLUDE_FROM_ALL: ${ARG_EXCLUDE_FROM_ALL}")
  message(STATUS "  UNPARSED_ARGUMENTS: ${ARG_UNPARSED_ARGUMENTS}")

  # All variadic arguments are put into ARG_UNPARSED_ARGUMENTS, that is, all of our sources should be here.
  if(NOT ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "add_test_executable requires at least one source file.")
  endif()

  set(win32)
  if(ARG_WIN32)
    set(win32 WIN32)
  endif()

  set(macosx_bundle)
  if(ARG_MACOSX_BUNDLE)
    set(macosx_bundle MACOSX_BUNDLE)
  endif()

  set(exclude_from_all)
  if(ARG_EXCLUDE_FROM_ALL)
    set(exclude_from_all EXCLUDE_FROM_ALL)
  endif()

  # Create our executable
  add_executable(${target_name} ${win32} ${macosx_bundle} ${exclude_from_all} ${ARG_UNPARSED_ARGUMENTS})

  # Catch2 management
  include(Catch)
  target_link_libraries(${target_name} PRIVATE Catch2::Catch2WithMain)
  catch_discover_tests(${target_name})
endfunction()
