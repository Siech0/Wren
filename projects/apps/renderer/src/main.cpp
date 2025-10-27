#include <print>
#include <exception>
#include <iostream>

#include <core/version.hpp>
#include <core/display/window.hpp>
#include <core/utils/scope_exit.hpp>


auto main(int argc, char *argv[]) -> int {
  try {
    (void)argc; // Mark unused
    (void)argv; // Mark unused

    core::display::window::init_system();
    const core::utils::scope_exit glfw_init_guard{core::display::window::deinit_system};

    std::print("Core Version: {}\n", core::version());
    core::display::window window{800, 600, "Renderer"};
    while (window.should_close() == false) {
      glfwPollEvents();
    }
  } catch (const std::exception &e) {
    std::print(std::cerr, "Critical Error: {}\n", e.what());
    return 1;
  }
}
