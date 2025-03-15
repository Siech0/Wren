add_subdirectory(vulkan)

add_library_target(core-gfx INTERFACE ALIAS_AS core::gfx)

target_link_libraries(core-gfx INTERFACE core::gfx-vulkan)
