add_library(core-gfx-vulkan STATIC)
add_library(core::gfx-vulkan ALIAS core-gfx-vulkan)

target_sources(core-gfx-vulkan PRIVATE test.cpp)
target_link_libraries(core-gfx-vulkan PUBLIC core::common-inc)
