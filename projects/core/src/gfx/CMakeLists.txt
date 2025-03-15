add_subdirectory(vulkan)

add_library(core-gfx INTERFACE)
add_library(core::gfx ALIAS core-gfx)

target_link_libraries(core-gfx INTERFACE core::gfx-vulkan)
