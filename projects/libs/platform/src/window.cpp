#include <wren/platform/window.hpp>

#include <stdexcept>

#include <GLFW/glfw3.h>


namespace wren::platform {

    window::window(int width, int height, const std::string_view title) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    }

    window::window(window&& other) noexcept : _window(other._window) {
        other._window = nullptr;
    }

    auto window::operator=(window&& other) noexcept -> window& {
        if (this != &other) {
            release();
            std::swap(_window, other._window);
        }
        return *this;
    }

    window::~window() {
        glfwDestroyWindow(_window);
    }

    void window::init_system() {
        if (_system_initialized) {
            return;
        }

        if (glfwInit() == 0) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        _system_initialized = true;
    }

    void window::deinit_system() {
        if (!_system_initialized) {
            return;
        }

        glfwTerminate();
        _system_initialized = false;
    }

    auto window::is_system_initialized() -> bool {
        return _system_initialized;
    }

    auto window::should_close() const noexcept -> bool {
        return glfwWindowShouldClose(_window) == 0;
    }

    void window::release() {
        glfwDestroyWindow(_window);
    }

    void window::poll_events() noexcept {
        glfwPollEvents();
    }

} // namespace wren::platform
