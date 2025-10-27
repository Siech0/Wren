#include <print>
#include <exception>
#include <iostream>

#include <wren/version.hpp>
#include <wren/platform/window.hpp>
#include <wren/foundation/utility/scope_exit.hpp>


auto main(int argc, char *argv[]) -> int {
  try {
    (void)argc; // Mark unused
    (void)argv; // Mark unused

    wren::platform::window::init_system();
    const wren::foundation::utility::scope_exit glfw_init_guard{wren::platform::window::deinit_system};

    std::print("Wren Version: {}\n", WREN_VERSION_STRING);
    wren::platform::window window{800, 600, "Renderer"};
    while (window.should_close() == false) {
      window.poll_events();
    }
  } catch (const std::exception &e) {
    std::print(std::cerr, "Critical Error: {}\n", e.what());
    return 1;
  }
}
