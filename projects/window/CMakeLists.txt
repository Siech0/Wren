set(WINDOW_INCLUDEDIR "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(renderer-window STATIC)
add_library(renderer::window ALIAS renderer-window)

set_target_properties(renderer-window PROPERTIES POSITION_INDEPENDENT_CODE ON)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

# Public headers
set_target_properties(renderer-window PROPERTIES PUBLIC_HEADER "${WINDOW_INCLUDEDIR}/renderer/window/window.hpp")

target_sources(
  renderer-window
  PRIVATE src/window.cpp
  PUBLIC FILE_SET
         public_headers
         TYPE
         HEADERS
         BASE_DIRS
         "${WINDOW_INCLUDEDIR}"
         FILES
         "${WINDOW_INCLUDEDIR}/renderer/window/window.hpp"
)

target_include_directories(renderer-window PUBLIC "${WINDOW_INCLUDEDIR}")

target_link_libraries(renderer-window PUBLIC glfw Vulkan::Vulkan)
