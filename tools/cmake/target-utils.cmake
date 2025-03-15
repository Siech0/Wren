# Add a target to the project using the specified visibility and file type add_executable_target(target_name [WIN32]
# [MACOSX_BUNDLE] [EXCLUDE_FROM_ALL] [ <PUBLIC|PRIVATE> <HEADERS|SOURCES|CXX_MODULES> <files>... ]...)
function(add_executable_target target_name)
  set(prefix "ARG")
  set(noValues "WIN32;MACOSX_BUNDLE;EXCLUDE_FROM_ALL")
  set(singleValues "ALIAS_AS")
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

  # Add the executable target
  add_executable(${target_name} ${WIN32} ${MACOSX_BUNDLE} ${EXCLUDE_FROM_ALL})
  if(ARG_ALIAS_AS)
    add_executable(${ARG_ALIAS_AS} ALIAS ${target_name})
  endif()

  if(public_headers)
    target_sources(${target_name} PUBLIC FILE_SET public_headers TYPE HEADERS FILES ${public_headers})
  endif()
  if(private_headers)
    target_sources(${target_name} PRIVATE FILE_SET private_headers TYPE HEADERS FILES ${private_headers})
  endif()
  if(public_sources)
    target_sources(${target_name} PUBLIC ${public_sources})
  endif()
  if(private_sources)
    target_sources(${target_name} PRIVATE ${private_sources})
  endif()
  if(public_modules)
    target_sources(${target_name} PUBLIC FILE_SET public_modules TYPE CXX_MODULES FILES ${public_modules})
  endif()
  if(private_modules)
    target_sources(${target_name} PRIVATE FILE_SET private_modules TYPE CXX_MODULES FILES ${private_modules})
  endif()
endfunction()

function(add_library_target target_name)
  set(prefix "ARG")
  set(noValues "STATIC;SHARED;MODULE;OBJECT;INTERFACE;EXCLUDE_FROM_ALL")
  set(singleValues "ALIAS_AS")
  set(multiValues "")

  cmake_parse_arguments(PARSE_ARGV 1 ${prefix} "${noValues}" "${singleValues}" "${multiValues}")

  # State maintenance
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

  if(public_headers)
    target_sources(${target_name} PUBLIC FILE_SET public_headers TYPE HEADERS FILES ${public_headers})
  endif()
  if(private_headers)
    target_sources(${target_name} PRIVATE FILE_SET private_headers TYPE HEADERS FILES ${private_headers})
  endif()
  if(interface_headers)
    target_sources(${target_name} INTERFACE FILE_SET interface_headers TYPE HEADERS FILES ${interface_headers})
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
    target_sources(${target_name} PUBLIC FILE_SET public_modules TYPE CXX_MODULES FILES ${public_modules})
  endif()
  if(private_modules)
    target_sources(${target_name} PRIVATE FILE_SET private_modules TYPE CXX_MODULES FILES ${private_modules})
  endif()
  if(interface_modules)
    target_sources(${target_name} INTERFACE FILE_SET interface_modules TYPE CXX_MODULES FILES ${interface_modules})
  endif()
endfunction()
