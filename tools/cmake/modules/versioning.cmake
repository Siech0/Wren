
function(wren_extract_version)
  set(oneValueArgs VERSION_FILE STATE_FILE HEADER_OUT)
  cmake_parse_arguments(WREN "" "${oneValueArgs}" "" ${ARGN})

  if(NOT WREN_VERSION_FILE)
    set(WREN_VERSION_FILE "${CMAKE_SOURCE_DIR}/version.json")
  endif()
  if(NOT WREN_STATE_FILE)
    set(WREN_STATE_FILE "${CMAKE_BINARY_DIR}/last_build.json")
  endif()

  # --- Read version json
  if(NOT EXISTS "${WREN_VERSION_FILE}")
    message(FATAL_ERROR "version file not found: ${WREN_VERSION_FILE}")
  endif()
  file(READ "${WREN_VERSION_FILE}" _VER_JSON)

  # Parse required fields
  string(JSON _MAJ GET "${_VER_JSON}" "version" "major")
  string(JSON _MIN GET "${_VER_JSON}" "version" "minor")
  string(JSON _PAT GET "${_VER_JSON}" "version" "patch")

  # Optional "extra" from JSON
  string(JSON _JSON_EXTRA ERROR_VARIABLE _JERR GET "${_VER_JSON}" "version" "extra")
  if(_JERR)
    set(_JSON_EXTRA "")
  endif()

  # --- Git metadata (branch + short sha)
  execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE _GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _BR_OK)
  if(NOT _BR_OK EQUAL 0 OR _GIT_BRANCH STREQUAL "")
    set(_GIT_BRANCH "unknown")
  endif()

  execute_process(
    COMMAND git rev-parse --short=7 HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE _GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _SHA_OK)
  if(NOT _SHA_OK EQUAL 0 OR _GIT_SHA STREQUAL "")
    set(_GIT_SHA "nogit")
  endif()

  # --- Compute "extra" (priority 1..5)
  if(DEFINED WREN_VERSION_EXTRA AND NOT WREN_VERSION_EXTRA STREQUAL "")
    set(_EXTRA "${WREN_VERSION_EXTRA}")
  elseif(NOT _JSON_EXTRA STREQUAL "")
    set(_EXTRA "${_JSON_EXTRA}")
  else()
    if(_GIT_BRANCH STREQUAL "develop")
      set(_EXTRA "dev")
    elseif(_GIT_BRANCH STREQUAL "master" OR _GIT_BRANCH STREQUAL "main")
      set(_EXTRA "")
    else()
      set(_EXTRA "${_GIT_BRANCH}")
      string(TOLOWER "${_EXTRA}" _EXTRA)
      string(REGEX REPLACE "[/_]" "-" _EXTRA "${_EXTRA}")
      string(REGEX REPLACE "[^a-z0-9.-]" "-" _EXTRA "${_EXTRA}")
      string(REGEX REPLACE "-+" "-" _EXTRA "${_EXTRA}")
      string(REGEX REPLACE "^-|-$" "" _EXTRA "${_EXTRA}")
    endif()
  endif()

  # --- Build counter (STATE_FILE)  (compute BEFORE composing final string)
  set(_BUILD 0)
  if(EXISTS "${WREN_STATE_FILE}")
    file(READ "${WREN_STATE_FILE}" _STATE_JSON)

    string(JSON _PMAJ   ERROR_VARIABLE _e1 GET "${_STATE_JSON}" "last_version" "major")
    string(JSON _PMIN   ERROR_VARIABLE _e2 GET "${_STATE_JSON}" "last_version" "minor")
    string(JSON _PPAT   ERROR_VARIABLE _e3 GET "${_STATE_JSON}" "last_version" "patch")
    string(JSON _PBUILD ERROR_VARIABLE _e4 GET "${_STATE_JSON}" "last_version" "build")
    string(JSON _PEXTRA ERROR_VARIABLE _e5 GET "${_STATE_JSON}" "last_version" "extra")
    string(JSON _PCOMMIT ERROR_VARIABLE _e6 GET "${_STATE_JSON}" "last_version" "commit")

    if(_e1 OR _e2 OR _e3)
      set(_PMAJ "")
      set(_PMIN "")
      set(_PPAT "")
    endif()
    if(_e4)
      set(_PBUILD 0)
    endif()
    if(NOT _PBUILD MATCHES "^[0-9]+$")
      set(_PBUILD 0)
    endif()
    if(_e5)
      set(_PEXTRA "")
    endif()
    if(_e6)
      set(_PCOMMIT "")
    endif()

    if("${_PMAJ}" STREQUAL "${_MAJ}"
       AND "${_PMIN}" STREQUAL "${_MIN}"
       AND "${_PPAT}" STREQUAL "${_PAT}"
       AND "${_PEXTRA}" STREQUAL "${_EXTRA}"
       AND "${_PCOMMIT}" STREQUAL "${_GIT_SHA}")
      math(EXPR _BUILD "${_PBUILD} + 1")
    else()
      set(_BUILD 0)
    endif()
  endif()

  # --- Compose version string AFTER _BUILD is known (no stray punctuation)
  set(_BASE "${_MAJ}.${_MIN}.${_PAT}")

  set(_EXTRA_SUFFIX "")
  if(NOT _EXTRA STREQUAL "")
    set(_EXTRA_SUFFIX "-${_EXTRA}")
  endif()

  # always include build so it never renders as "+.g<sha>"
  set(_VERSION_STR "${_BASE}${_EXTRA_SUFFIX}+${_BUILD}.g${_GIT_SHA}")

  # --- Export variables to caller
  set(WREN_VERSION_MAJOR "${_MAJ}" PARENT_SCOPE)
  set(WREN_VERSION_MINOR "${_MIN}" PARENT_SCOPE)
  set(WREN_VERSION_PATCH "${_PAT}" PARENT_SCOPE)
  set(WREN_VERSION_EXTRA "${_EXTRA}" PARENT_SCOPE)
  set(WREN_VERSION       "${_BASE}" PARENT_SCOPE)
  set(WREN_VERSION_STRING "${_VERSION_STR}" PARENT_SCOPE)
  set(WREN_VERSION_BUILD "${_BUILD}" PARENT_SCOPE)
  set(WREN_GIT_BRANCH "${_GIT_BRANCH}" PARENT_SCOPE)
  set(WREN_GIT_COMMIT "${_GIT_SHA}" PARENT_SCOPE)

  # --- Persist new state JSON
  set(_STATE_TPL [=[
{
  "last_version": {
    "major": @WREN_VERSION_MAJOR@,
    "minor": @WREN_VERSION_MINOR@,
    "patch": @WREN_VERSION_PATCH@,
    "build": @WREN_VERSION_BUILD@,
    "commit": "@WREN_GIT_COMMIT@",
    "extra": "@WREN_VERSION_EXTRA@"
  }
}
]=])
  set(WREN_VERSION_MAJOR "${_MAJ}")
  set(WREN_VERSION_MINOR "${_MIN}")
  set(WREN_VERSION_PATCH "${_PAT}")
  set(WREN_VERSION_BUILD "${_BUILD}")
  set(WREN_GIT_COMMIT    "${_GIT_SHA}")
  set(WREN_VERSION_EXTRA "${_EXTRA}")
  string(CONFIGURE "${_STATE_TPL}" _STATE_OUT @ONLY)
  file(WRITE "${WREN_STATE_FILE}" "${_STATE_OUT}\n")

  # --- Optional header
  if(WREN_HEADER_OUT)
    set(_H_TPL [=[
#pragma once
#define WREN_VERSION_MAJOR @WREN_VERSION_MAJOR@
#define WREN_VERSION_MINOR @WREN_VERSION_MINOR@
#define WREN_VERSION_PATCH @WREN_VERSION_PATCH@
#define WREN_VERSION_BUILD @WREN_VERSION_BUILD@
#define WREN_VERSION_STRING "@WREN_VERSION_STRING@"
#define WREN_GIT_COMMIT "@WREN_GIT_COMMIT@"
#define WREN_GIT_BRANCH "@WREN_GIT_BRANCH@"
#define WREN_VERSION_EXTRA "@WREN_VERSION_EXTRA@"
]=])
    set(WREN_VERSION_STRING "${_VERSION_STR}")
    set(WREN_GIT_BRANCH     "${_GIT_BRANCH}")
    string(CONFIGURE "${_H_TPL}" _H_OUT @ONLY)
    file(WRITE "${WREN_HEADER_OUT}" "${_H_OUT}")
  endif()
endfunction()
