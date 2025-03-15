find_package(glfw3)

add_executable(renderer "src/main.cpp")
add_executable(renderer::renderer ALIAS renderer)

target_link_libraries(renderer PRIVATE core::core glm::glm Vulkan::Vulkan glfw)
