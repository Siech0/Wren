# Get renderer version from include/renderer/version.h and put it in RENDERER_VERSION
function(extract_version file var_root)
  file(READ "${file}" file_contents)
  string(REGEX MATCH "RENDERER_VERSION_MAJOR ([0-9]+)" _ "${file_contents}")
  if(NOT CMAKE_MATCH_COUNT EQUAL 1)
    message(FATAL_ERROR "Could not extract major version number from ${file}")
  endif()
  set(ver_major ${CMAKE_MATCH_1})

  string(REGEX MATCH "RENDERER_VERSION_MINOR ([0-9]+)" _ "${file_contents}")
  if(NOT CMAKE_MATCH_COUNT EQUAL 1)
    message(FATAL_ERROR "Could not extract minor version number from ${file}")
  endif()

  set(ver_minor ${CMAKE_MATCH_1})
  string(REGEX MATCH "RENDERER_VERSION_PATCH ([0-9]+)" _ "${file_contents}")
  if(NOT CMAKE_MATCH_COUNT EQUAL 1)
    message(FATAL_ERROR "Could not extract patch version number from ${file}")
  endif()
  set(ver_patch ${CMAKE_MATCH_1})

  set(${var_root}_VERSION_MAJOR
      ${ver_major}
      PARENT_SCOPE
  )
  set(${var_root}_VERSION_MINOR
      ${ver_minor}
      PARENT_SCOPE
  )
  set(${var_root}_VERSION_PATCH
      ${ver_patch}
      PARENT_SCOPE
  )
  set(${var_root}_VERSION
      "${ver_major}.${ver_minor}.${ver_patch}"
      PARENT_SCOPE
  )
endfunction()
