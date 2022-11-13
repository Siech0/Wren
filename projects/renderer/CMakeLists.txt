find_package(glfw3)

add_executable(renderer "src/main.cpp")
target_link_libraries(renderer PRIVATE render::core glfw)
add_executable(renderer::renderer ALIAS renderer)
