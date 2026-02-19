find_package(glfw3)

add_executable(wren.renderer "src/main.cpp")
add_executable(wren::renderer ALIAS wren.renderer)

target_compile_features(wren.renderer PRIVATE cxx_std_23)

# Dependency setup
find_package(glm REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(wren.renderer
    PRIVATE
        glm::glm
        glfw
        wren::foundation
        wren::platform
        wren::version_info
)

# Installation
install(TARGETS wren.renderer
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT applications
)
