add_library_target(
  core-gfx-vulkan
  STATIC
  ALIAS_AS
  core::gfx-vulkan
  PRIVATE
  SOURCES
  test.cpp
)

target_sources(core-gfx-vulkan PRIVATE test.cpp)
target_link_libraries(core-gfx-vulkan PUBLIC core::common-inc)
