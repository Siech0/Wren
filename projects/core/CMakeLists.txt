add_test_subdirectory(test)
add_benchmark_subdirectory(bench)

add_library(render-core src/vulkan/vulkan.cpp src/vulkan/context.cpp)
target_include_directories(render-core PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
set_target_properties(
  render-core
  PROPERTIES CXX_VISIBILITY_PRESET hidden
             VISIBILITY_INLINES_HIDDEN YES
             VERSION "${PROJECT_VERSION}"
             SOVERSION "${PROJECT_VERSION_MAJOR}"
             EXPORT_NAME render-core
             OUTPUT_NAME render-core
)
generate_export_header(render-core BASE_NAME RENDER_CORE EXPORT_FILE_NAME export/render_core_export.hpp)
target_include_directories(
  render-core SYSTEM
  PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
         "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
)
target_link_libraries(render-core PUBLIC Vulkan::Vulkan glm::glm)
target_compile_definitions(render-core PUBLIC VK_NO_PROTOTYPES)

# This definition is required if we want to be able to compile as a static library
if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(render-core PUBLIC RENDER_CORE_STATIC_DEFINE)
endif()

add_library(render::core ALIAS render-core)
