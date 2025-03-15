add_library_target(
  core-gfx-vulkan
  STATIC
  ALIAS_AS core::gfx-vulkan
  PUBLIC CXX_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/test.cppm"
)

target_link_libraries(core-gfx-vulkan PUBLIC core::common-inc)
