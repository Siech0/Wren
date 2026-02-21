#include <print>
#include <exception>
#include <iostream>

#include <wren/version.hpp>
#include <wren/platform/window.hpp>
#include <wren/foundation/utility/scope_exit.hpp>
#include <wren/rhi/loader.hpp>


auto main(int argc, char *argv[]) -> int {
  try {
    (void)argc;
    (void)argv;

    std::print("Wren Version: {}\n", WREN_VERSION_STRING);

    // --- Backend selection --------------------------------------------------
    std::print("Select a graphics backend:\n");
    std::print("  1 = Vulkan\n");
    std::print("  2 = OpenGL\n");
    std::print("> ");

    int choice{};
    std::cin >> choice;

    wren::rhi::Backend selected{};
    switch (choice) {
      case 1:  selected = wren::rhi::Backend::Vulkan; break;
      case 2:  selected = wren::rhi::Backend::OpenGL; break;
      default:
        std::print(std::cerr, "Invalid selection '{}'. Expected 1 or 2.\n", choice);
        return 1;
    }

    // --- Load backend DLL ---------------------------------------------------
    auto result = wren::rhi::BackendLibrary::load(selected);
    if (!result) {
      std::print(std::cerr, "Failed to load backend: {}\n", result.error());
      return 1;
    }

    auto& backend = *result;
    const auto id = backend.backend_id();
    std::print("Backend loaded. backend_id() = {} ({})\n",
               static_cast<int>(id),
               wren::rhi::to_string(id));

    // --- Window + main loop -------------------------------------------------
    wren::platform::window::init_system();
    const wren::foundation::utility::scope_exit glfw_init_guard{wren::platform::window::deinit_system};

    wren::platform::window window{800, 600, "Renderer"};
    while (window.should_close() == false) {
      window.poll_events();
    }
  } catch (const std::exception &e) {
    std::print(std::cerr, "Critical Error: {}\n", e.what());
    return 1;
  }
}
