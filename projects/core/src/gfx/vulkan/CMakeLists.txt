add_library(core-gfx-vulkan STATIC)
add_library(core::gfx-vulkan ALIAS core-gfx-vulkan)

target_sources(
  core-gfx-vulkan
  PRIVATE test.cpp
  PUBLIC FILE_SET
         public_headers
         TYPE
         HEADERS
         BASE_DIRS
         "${CORE_INCLUDEDIR}"
         FILES
         "${CORE_INCLUDEDIR}/core/gfx/vulkan/test.hpp"
)
set_target_properties(core-gfx-vulkan PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(core-gfx-vulkan PUBLIC core::export)
