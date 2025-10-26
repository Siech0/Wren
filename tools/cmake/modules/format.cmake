cmake_minimum_required(VERSION 3.14)

macro(default name)
  if(NOT DEFINED "${name}")
    set("${name}" "${ARGN}")
  endif()
endmacro()

default(FORMAT_COMMAND clang-format)
default(FORMAT_MIN_VERSION 18)
default(FORMAT_MAX_VERSION 20)
default(
  PATTERNS
  projects/*.cpp
  projects/*.hpp
  test/*.cpp
  test/*.hpp
  bench/*.cpp
  bench/*.hpp
)
default(FIX NO)

# Verify clang format version
execute_process(
  COMMAND "${FORMAT_COMMAND}" --version
  OUTPUT_VARIABLE version_out
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "clang-format version ([0-9]+).([0-9]+).([0-9]+)" _ ${version_out})
set(FORMAT_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(FORMAT_VERSION_MINOR "${CMAKE_MATCH_2}")
set(FORMAT_VERSION_PATCH "${CMAKE_MATCH_3}")
set(FORMAT_VERSION "${FORMAT_VERSION_MAJOR}.${FORMAT_VERSION_MINOR}.${FORMAT_VERSION_PATCH}")
message(STATUS "Found clang-format version ${FORMAT_VERSION}")

if(NOT FORMAT_VERSION VERSION_GREATER_EQUAL "${FORMAT_MIN_VERSION}")
  message(FATAL_ERROR "Expected clang-format version ${FORMAT_MIN_VERSION} or newer, got ${FORMAT_VERSION}")
endif()

if(NOT FORMAT_VERSION VERSION_LESS "${FORMAT_MAX_VERSION}")
  message(FATAL_ERROR "Expected clang-format version newer than ${FORMAT_MAX_VERSION}, got ${FORMAT_VERSION}")
endif()

set(flag --output-replacements-xml)
set(args OUTPUT_VARIABLE output)
if(FIX)
  set(flag -i)
  set(args "")
endif()

file(GLOB_RECURSE files ${PATTERNS})
set(badly_formatted "")
set(output "")
string(LENGTH "${CMAKE_SOURCE_DIR}/" path_prefix_length)

foreach(file IN LISTS files)
  execute_process(
    COMMAND "${FORMAT_COMMAND}" --style=file "${flag}" "${file}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE result ${args}
  )
  if(NOT result EQUAL "0")
    message(FATAL_ERROR "'${file}': formatter returned with ${result}")
  endif()
  if(NOT FIX AND output MATCHES "\n<replacement offset")
    string(SUBSTRING "${file}" "${path_prefix_length}" -1 relative_file)
    list(APPEND badly_formatted "${relative_file}")
  endif()
  set(output "")
endforeach()

if(NOT badly_formatted STREQUAL "")
  list(JOIN badly_formatted "\n" bad_list)
  message("The following files are badly formatted:\n\n${bad_list}\n")
  message(FATAL_ERROR "Run again with FIX=YES to fix these files.")
endif()
