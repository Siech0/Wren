#include <print>
#include <exception>
#include <iostream>

#include <core/version.hpp>
#include <core/display/test.hpp>
#include <core/gfx/vulkan/test.hpp>
#include <core/utils/scope_exit.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>



class window {

  static inline bool _system_initialized = false;
  GLFWwindow *_window;

public:

  window() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    const int width = 800;
    const int height = 600;
    const char *title = "Renderer";
    _window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  }

  window(const window &) = delete;
  auto operator=(const window &) -> window & = delete;

  window(window &&other) noexcept : _window(other._window) {
    other._window = nullptr;
  }
  auto operator=(window&& other) noexcept -> window & {
    if (this != &other) {
      release();
      std::swap(_window, other._window);
    }
    return *this;
  }

  virtual ~window() {
    glfwDestroyWindow(_window);
  }

  static void init_system() {
    if (glfwInit() == 0) {
      throw std::runtime_error("Failed to initialize GLFW");
    }
    _system_initialized = true;
  }

  static void deinit_system() {
    glfwTerminate();
    _system_initialized = false;
  }

  static auto is_system_initialized() -> bool {
    return _system_initialized;
  }

  [[nodiscard]]
  auto should_close() const noexcept -> bool {
    return glfwWindowShouldClose(_window) == 0;
  }

private:
  void release() {
    glfwDestroyWindow(_window);
    _window = nullptr;
  }
};


auto main(int argc, char *argv[]) -> int {
  try {
    (void)argc; // Mark unused
    (void)argv; // Mark unused

    window::init_system();
    const core::utils::scope_exit glfw_guard{window::deinit_system};

    std::print("Renderer Version: {}\n", core::version_string());
    return core::display::test_display() + core::gfx::vulkan::test_vulkan();

  } catch (const std::exception &e) {
    std::print(std::cerr, "Critical Error: {}\n", e.what());
    return 1;
  }
}
