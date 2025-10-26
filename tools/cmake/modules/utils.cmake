# --------------------------------------------------------------
# Add directory as test-only directory
function(add_test_subdirectory dir)
  if(RENDERER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(${dir})
  endif()
endfunction()

# Add directory as test-only directory
function(add_benchmark_subdirectory dir)
  if(RENDERER_BUILD_BENCHMARKS)
    add_subdirectory(${dir})
  endif()
endfunction()

# --------------------------------------------------------------
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
