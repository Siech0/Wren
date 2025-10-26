add_subdirectory(gfx)
add_subdirectory(display)
add_subdirectory(utils)

target_sources(
  core
  PRIVATE version.cpp
  PUBLIC FILE_SET
         public_headers
         TYPE
         HEADERS
         BASE_DIRS
         "${CORE_EXPORT_INCLUDEDIR}"
         "${CORE_INCLUDEDIR}"
         FILES
         "${CORE_INCLUDEDIR}/core/version.hpp"
         "${CORE_EXPORT_INCLUDEDIR}/core/export.hpp"
)
