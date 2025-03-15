find_package(glfw3)

add_executable_target(renderer ALIAS_AS renderer::renderer PRIVATE SOURCES "src/main.cpp")

target_link_libraries(renderer PRIVATE core::core glm::glm Vulkan::Vulkan glfw)
