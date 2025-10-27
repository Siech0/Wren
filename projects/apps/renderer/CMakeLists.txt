find_package(glfw3)

add_executable(wren_renderer "src/main.cpp")
add_executable(wren::renderer ALIAS wren_renderer)

target_compile_features(wren_renderer PRIVATE cxx_std_23)

# Dependency setup
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(wren_renderer
    PRIVATE
        glm::glm
        glfw
        wren::foundation
        wren::platform
        wren::version_info
)
