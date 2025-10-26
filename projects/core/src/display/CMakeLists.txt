add_library(core-display STATIC)
add_library(core::display ALIAS core-display)

target_sources(
  core-display
  PRIVATE window.cpp
  PUBLIC FILE_SET
         public_headers
         TYPE
         HEADERS
         BASE_DIRS
         "${CORE_INCLUDEDIR}"
         FILES
         "${CORE_INCLUDEDIR}/core/display/window.hpp"
)
set_target_properties(core-display PROPERTIES POSITION_INDEPENDENT_CODE ON)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
target_link_libraries(core-display PUBLIC core::export glfw Vulkan::Vulkan)
